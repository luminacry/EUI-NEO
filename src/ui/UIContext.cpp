#include "UIContext.h"
#include <algorithm>

namespace EUINEO {

namespace {

UIClipRect IntersectClipRect(const UIClipRect& lhs, const UIClipRect& rhs) {
    const float x1 = std::max(lhs.x, rhs.x);
    const float y1 = std::max(lhs.y, rhs.y);
    const float x2 = std::min(lhs.x + lhs.width, rhs.x + rhs.width);
    const float y2 = std::min(lhs.y + lhs.height, rhs.y + rhs.height);

    UIClipRect clipped;
    clipped.x = x1;
    clipped.y = y1;
    clipped.width = std::max(0.0f, x2 - x1);
    clipped.height = std::max(0.0f, y2 - y1);
    return clipped;
}

RectFrame UnionFrame(const RectFrame& lhs, const RectFrame& rhs) {
    if (lhs.width <= 0.0f || lhs.height <= 0.0f) {
        return rhs;
    }
    if (rhs.width <= 0.0f || rhs.height <= 0.0f) {
        return lhs;
    }

    const float x1 = std::min(lhs.x, rhs.x);
    const float y1 = std::min(lhs.y, rhs.y);
    const float x2 = std::max(lhs.x + lhs.width, rhs.x + rhs.width);
    const float y2 = std::max(lhs.y + lhs.height, rhs.y + rhs.height);
    return RectFrame{x1, y1, x2 - x1, y2 - y1};
}

int LayerDrawPriority(RenderLayer layer) {
    switch (layer) {
        case RenderLayer::Backdrop:
            return 0;
        case RenderLayer::Content:
            return 1;
        case RenderLayer::Chrome:
            return 2;
        case RenderLayer::Popup:
            return 3;
        default:
            return 0;
    }
}

} // namespace

void UIContext::begin(const std::string& pageId) {
    pageId_ = pageId;
    ++composeStamp_;
    order_.clear();
    drawOrder_.clear();
    drawOrderStamp_ = 0;
    clipStack_.clear();
    offsetStack_.clear();
    currentOffsetX_ = 0.0f;
    currentOffsetY_ = 0.0f;
    treeChanged_ = false;
    needsRecompose_ = false;
    layerBoundsStamp_ = 0;
    if (layerBounds_.size() != static_cast<std::size_t>(RenderLayer::Count)) {
        layerBounds_.assign(static_cast<std::size_t>(RenderLayer::Count), RectFrame{});
    }
}

void UIContext::end() {
    bool dirtyLayers[static_cast<std::size_t>(RenderLayer::Count)] = {};
    for (auto& entry : nodes_) {
        UINode* node = entry.second.get();
        if (node == nullptr) {
            continue;
        }
        if (!node->composedIn(composeStamp_)) {
            treeChanged_ = true;
            const std::size_t layerIndex = static_cast<std::size_t>(node->renderLayer());
            if (layerIndex < static_cast<std::size_t>(RenderLayer::Count)) {
                dirtyLayers[layerIndex] = true;
            }
        }
    }

    for (UINode* node : order_) {
        if (!node->cacheDirty()) {
            continue;
        }
        const std::size_t layerIndex = static_cast<std::size_t>(node->renderLayer());
        if (layerIndex < static_cast<std::size_t>(RenderLayer::Count)) {
            dirtyLayers[layerIndex] = true;
        }
    }

    bool anyDirty = treeChanged_;
    for (std::size_t index = 0; index < static_cast<std::size_t>(RenderLayer::Count); ++index) {
        if (!dirtyLayers[index]) {
            continue;
        }
        anyDirty = true;
        Renderer::InvalidateLayer(static_cast<RenderLayer>(index));
    }

    if (anyDirty) {
        Renderer::RequestRepaint();
    }
}

void UIContext::pushClip(float x, float y, float width, float height) {
    UIClipRect clip;
    clip.x = x;
    clip.y = y;
    clip.width = width;
    clip.height = height;

    if (!clipStack_.empty()) {
        clip = IntersectClipRect(clipStack_.back(), clip);
    }

    clipStack_.push_back(clip);
}

void UIContext::popClip() {
    if (!clipStack_.empty()) {
        clipStack_.pop_back();
    }
}

float UIContext::pushScrollArea(const std::string& id, float x, float y, float width, float height,
                                float contentHeight, float scrollStep) {
    ScrollAreaNode& node = acquire<ScrollAreaNode>(id);
    UIPrimitive& primitive = node.primitive();
    primitive.x = x;
    primitive.y = y;
    primitive.width = width;
    primitive.height = height;

    node.trackComposeValue("contentHeight", contentHeight);
    node.trackComposeValue("scrollStep", scrollStep);
    node.setContentHeight(contentHeight);
    node.setScrollStep(scrollStep);
    node.finishCompose();

    const float scrollOffsetY = node.scrollOffsetY();
    pushClip(x, y, width, height);
    pushOffset(0.0f, -scrollOffsetY);
    return scrollOffsetY;
}

void UIContext::popScrollArea() {
    popOffset();
    popClip();
}

void UIContext::update() {
    bool dirtyLayers[static_cast<std::size_t>(RenderLayer::Count)] = {};
    for (UINode* node : order_) {
        if (node->visible()) {
            node->update();
        }
        if (node->cacheDirty()) {
            const std::size_t layerIndex = static_cast<std::size_t>(node->renderLayer());
            if (layerIndex < static_cast<std::size_t>(RenderLayer::Count)) {
                dirtyLayers[layerIndex] = true;
            }
        }
        if (node->consumeRecomposeRequest()) {
            needsRecompose_ = true;
        }
    }

    for (std::size_t index = 0; index < static_cast<std::size_t>(RenderLayer::Count); ++index) {
        if (dirtyLayers[index]) {
            Renderer::InvalidateLayer(static_cast<RenderLayer>(index));
        }
    }
    refreshLayerBounds();
}

void UIContext::draw() {
    if (drawOrderStamp_ != composeStamp_) {
        drawOrder_ = order_;
        std::stable_sort(drawOrder_.begin(), drawOrder_.end(), [](const UINode* lhs, const UINode* rhs) {
            const int lhsLayer = LayerDrawPriority(lhs->renderLayer());
            const int rhsLayer = LayerDrawPriority(rhs->renderLayer());
            if (lhsLayer != rhsLayer) {
                return lhsLayer < rhsLayer;
            }
            return lhs->zIndex() < rhs->zIndex();
        });
        drawOrderStamp_ = composeStamp_;
    }

    for (UINode* node : drawOrder_) {
        if (!node->visible() || node->renderLayer() == RenderLayer::Backdrop) {
            continue;
        }
        if (node->primitive().blur <= 0.0f) {
            const RectFrame bounds = node->paintBounds();
            Renderer::DrawCachedSurface(node->key(), bounds, node->cacheDirty(), [node]() {
                node->draw();
            });
        } else {
            node->draw();
        }
        node->clearCacheDirty();
    }
}

void UIContext::draw(RenderLayer layer) {
    if (drawOrderStamp_ != composeStamp_) {
        drawOrder_ = order_;
        std::stable_sort(drawOrder_.begin(), drawOrder_.end(), [](const UINode* lhs, const UINode* rhs) {
            const int lhsLayer = LayerDrawPriority(lhs->renderLayer());
            const int rhsLayer = LayerDrawPriority(rhs->renderLayer());
            if (lhsLayer != rhsLayer) {
                return lhsLayer < rhsLayer;
            }
            return lhs->zIndex() < rhs->zIndex();
        });
        drawOrderStamp_ = composeStamp_;
    }

    for (UINode* node : drawOrder_) {
        if (node->visible() && node->renderLayer() == layer) {
            node->draw();
            node->clearCacheDirty();
        }
    }
}

RectFrame UIContext::layerBounds(RenderLayer layer) const {
    const std::size_t index = static_cast<std::size_t>(layer);
    if (index >= layerBounds_.size()) {
        return RectFrame{};
    }
    return layerBounds_[index];
}

bool UIContext::wantsContinuousUpdate() const {
    for (const UINode* node : order_) {
        if (node->visible() && node->wantsContinuousUpdate()) {
            return true;
        }
    }
    return false;
}

bool UIContext::consumeRecomposeRequest() {
    const bool requested = needsRecompose_;
    needsRecompose_ = false;
    return requested;
}

void UIContext::markAllNodesDirty() {
    for (auto& entry : nodes_) {
        if (entry.second) {
            entry.second->forceComposeDirty();
        }
    }
    Renderer::RequestRepaint();
}

void UIContext::refreshLayerBounds() {
    if (layerBounds_.size() != static_cast<std::size_t>(RenderLayer::Count)) {
        layerBounds_.assign(static_cast<std::size_t>(RenderLayer::Count), RectFrame{});
    } else {
        std::fill(layerBounds_.begin(), layerBounds_.end(), RectFrame{});
    }

    for (UINode* node : order_) {
        if (!node->visible()) {
            continue;
        }
        const std::size_t index = static_cast<std::size_t>(node->renderLayer());
        if (index >= layerBounds_.size()) {
            continue;
        }
        layerBounds_[index] = UnionFrame(layerBounds_[index], node->paintBounds());
    }

    layerBoundsStamp_ = composeStamp_;
}

} // namespace EUINEO
