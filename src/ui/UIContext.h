#pragma once

#include "UIBuilder.h"
#include "../components/Button.h"
#include "../components/ComboBox.h"
#include "../components/InputBox.h"
#include "../components/Label.h"
#include "../components/Panel.h"
#include "../components/ProgressBar.h"
#include "../components/ScrollArea.h"
#include "../components/SegmentedControl.h"
#include "../components/Sidebar.h"
#include "../components/Slider.h"
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace EUINEO {

class UIContext {
public:
    void begin(const std::string& pageId);
    void end();
    void update();
    void draw();
    void draw(RenderLayer layer);
    RectFrame layerBounds(RenderLayer layer) const;
    bool wantsContinuousUpdate() const;
    void markAllNodesDirty();
    void pushClip(float x, float y, float width, float height);
    void popClip();
    float pushScrollArea(const std::string& id, float x, float y, float width, float height,
                         float contentHeight, float scrollStep = 48.0f);
    void popScrollArea();
    bool consumeRecomposeRequest();

    template <typename NodeT>
    GenericNodeBuilder<NodeT> node(const std::string& id) {
        NodeT& instance = acquire<NodeT>(id);
        return GenericNodeBuilder<NodeT>(*this, instance);
    }

    template <typename NodeT>
    typename NodeT::Builder component(const std::string& id) {
        NodeT& node = acquire<NodeT>(id);
        return typename NodeT::Builder(*this, node);
    }

#define EUI_UI_COMPONENT(name, type) \
    typename type::Builder name(const std::string& id) { \
        return component<type>(id); \
    }
#include "UIComponents.def"
#undef EUI_UI_COMPONENT

    template <typename Fn>
    float scrollArea(const std::string& id, float x, float y, float width, float height,
                     float contentHeight, Fn&& compose, float scrollStep = 48.0f) {
        const float scrollOffsetY = pushScrollArea(id, x, y, width, height, contentHeight, scrollStep);
        if constexpr (std::is_invocable_v<Fn, float>) {
            std::forward<Fn>(compose)(scrollOffsetY);
        } else {
            std::forward<Fn>(compose)();
        }
        popScrollArea();
        return scrollOffsetY;
    }

    template <typename Fn>
    void popup(const std::string& id, float x, float y, float width, float height, Fn&& compose) {
        popupPanel(id)
            .position(x, y)
            .size(width, height)
            .build();
        pushClip(x, y, width, height);
        if constexpr (std::is_invocable_v<Fn, const RectFrame&>) {
            std::forward<Fn>(compose)(RectFrame{x, y, width, height});
        } else {
            std::forward<Fn>(compose)();
        }
        popClip();
    }

private:
    struct Offset {
        float x = 0.0f;
        float y = 0.0f;
    };

    template <typename NodeT>
    NodeT& acquire(const std::string& id) {
        const std::string fullKey = pageId_.empty() ? id : pageId_ + "." + id;
        auto it = nodes_.find(fullKey);
        if (it == nodes_.end() || std::string(it->second->typeName()) != NodeT::StaticTypeName()) {
            treeChanged_ = true;
            auto replacement = std::make_unique<NodeT>(fullKey);
            UINode* raw = replacement.get();
            it = nodes_.insert_or_assign(fullKey, std::move(replacement)).first;
            raw->beginCompose(composeStamp_);
            applyCurrentContext(raw->primitive());
            order_.push_back(raw);
            return static_cast<NodeT&>(*raw);
        }

        UINode* node = it->second.get();
        if (!node->composedIn(composeStamp_)) {
            node->beginCompose(composeStamp_);
            order_.push_back(node);
        }
        applyCurrentContext(node->primitive());
        return static_cast<NodeT&>(*node);
    }

    void applyCurrentContext(UIPrimitive& primitive) {
        primitive.contextOffsetX = currentOffsetX_;
        primitive.contextOffsetY = currentOffsetY_;
        if (clipStack_.empty() || !primitive.clipToParent) {
            primitive.hasClipRect = false;
            primitive.clipRect = UIClipRect{};
            return;
        }
        primitive.hasClipRect = true;
        primitive.clipRect = clipStack_.back();
    }

    void pushOffset(float x, float y) {
        offsetStack_.push_back(Offset{x, y});
        currentOffsetX_ += x;
        currentOffsetY_ += y;
    }

    void popOffset() {
        if (offsetStack_.empty()) {
            return;
        }
        currentOffsetX_ -= offsetStack_.back().x;
        currentOffsetY_ -= offsetStack_.back().y;
        offsetStack_.pop_back();
    }

    void refreshLayerBounds();

    std::string pageId_;
    std::uint64_t composeStamp_ = 0;
    std::unordered_map<std::string, std::unique_ptr<UINode>> nodes_;
    std::vector<UINode*> order_;
    std::vector<UINode*> drawOrder_;
    std::vector<UIClipRect> clipStack_;
    std::vector<Offset> offsetStack_;
    mutable std::vector<RectFrame> layerBounds_;
    bool treeChanged_ = false;
    bool needsRecompose_ = false;
    std::uint64_t drawOrderStamp_ = 0;
    std::uint64_t layerBoundsStamp_ = 0;
    float currentOffsetX_ = 0.0f;
    float currentOffsetY_ = 0.0f;
};

} // namespace EUINEO
