#include "Panel.h"

namespace EUINEO {

namespace {

void ExpandBounds(RectBounds& target, const RectBounds& source, bool& hasBounds) {
    if (!hasBounds) {
        target = source;
        hasBounds = true;
        return;
    }

    float x1 = std::min(target.x, source.x);
    float y1 = std::min(target.y, source.y);
    float x2 = std::max(target.x + target.w, source.x + source.w);
    float y2 = std::max(target.y + target.h, source.y + source.h);
    target.x = x1;
    target.y = y1;
    target.w = x2 - x1;
    target.h = y2 - y1;
}

RectBounds MeasurePanelStateBounds(const PanelState& state) {
    RectBounds bounds;
    bool hasBounds = false;

    if (state.borderWidth > 0.0f && state.borderColor.a > 0.0f &&
        state.frame.width > state.borderWidth * 2.0f && state.frame.height > state.borderWidth * 2.0f) {
        RectStyle borderStyle;
        borderStyle.color = state.borderColor;
        borderStyle.rounding = state.style.rounding;
        borderStyle.transform = state.style.transform;
        ExpandBounds(bounds,
                     Renderer::MeasureRectBounds(state.frame.x, state.frame.y, state.frame.width, state.frame.height, borderStyle),
                     hasBounds);

        RectStyle fillStyle = state.style;
        fillStyle.rounding = std::max(0.0f, fillStyle.rounding - state.borderWidth);
        ExpandBounds(bounds,
                     Renderer::MeasureRectBounds(state.frame.x + state.borderWidth, state.frame.y + state.borderWidth,
                                                 state.frame.width - state.borderWidth * 2.0f,
                                                 state.frame.height - state.borderWidth * 2.0f,
                                                 fillStyle),
                     hasBounds);
        return bounds;
    }

    return Renderer::MeasureRectBounds(state.frame.x, state.frame.y, state.frame.width, state.frame.height, state.style);
}

} // namespace

Panel::Panel(float x, float y, float w, float h) {
    this->x = x;
    this->y = y;
    this->width = w;
    this->height = h;
    this->color = CurrentTheme ? CurrentTheme->surface : Color(1, 1, 1, 1);
}

void Panel::Update() {
}

RectFrame Panel::GetFrame() const {
    RectFrame frame;
    frame.x = x;
    frame.y = y;
    frame.width = width;
    frame.height = height;
    return frame;
}

void Panel::SetFrame(const RectFrame& frame) {
    x = frame.x;
    y = frame.y;
    width = frame.width;
    height = frame.height;
}

RectStyle Panel::GetStyle() const {
    RectStyle style;
    style.color = color;
    style.gradient = gradient;
    style.rounding = rounding;
    style.blurAmount = blurAmount;
    style.shadowBlur = shadowBlur;
    style.shadowOffsetX = shadowOffsetX;
    style.shadowOffsetY = shadowOffsetY;
    style.shadowColor = shadowColor;
    style.transform = transform;
    return style;
}

void Panel::SetStyle(const RectStyle& style) {
    color = style.color;
    gradient = style.gradient;
    rounding = style.rounding;
    blurAmount = style.blurAmount;
    shadowBlur = style.shadowBlur;
    shadowOffsetX = style.shadowOffsetX;
    shadowOffsetY = style.shadowOffsetY;
    shadowColor = style.shadowColor;
    transform = style.transform;
}

PanelState Panel::GetState() const {
    PanelState state;
    state.frame = GetFrame();
    state.style = GetStyle();
    state.borderWidth = borderWidth;
    state.borderColor = borderColor;
    return state;
}

void Panel::SetState(const PanelState& state) {
    SetFrame(state.frame);
    SetStyle(state.style);
    borderWidth = state.borderWidth;
    borderColor = state.borderColor;
}

void Panel::Draw() {
    float absX = 0.0f;
    float absY = 0.0f;
    GetAbsoluteBounds(absX, absY);

    if (borderWidth > 0.0f && borderColor.a > 0.0f &&
        width > borderWidth * 2.0f && height > borderWidth * 2.0f) {
        RectStyle borderStyle;
        borderStyle.color = borderColor;
        borderStyle.rounding = rounding;
        borderStyle.transform = transform;
        Renderer::DrawRect(absX, absY, width, height, borderStyle);

        RectStyle fillStyle = GetStyle();
        fillStyle.rounding = std::max(0.0f, rounding - borderWidth);
        Renderer::DrawRect(absX + borderWidth, absY + borderWidth,
                           width - borderWidth * 2.0f, height - borderWidth * 2.0f, fillStyle);
        return;
    }

    Renderer::DrawRect(absX, absY, width, height, GetStyle());
}

void Panel::MarkDirty(float expand, float duration) {
    Widget::MarkDirty(GetStyle(), expand, duration);
}

void Panel::MarkDirty(const PanelState& fromState, const PanelState& toState, float expand, float duration) {
    RectBounds fromBounds = MeasurePanelStateBounds(fromState);
    RectBounds toBounds = MeasurePanelStateBounds(toState);

    float x1 = std::min(fromBounds.x, toBounds.x) - expand;
    float y1 = std::min(fromBounds.y, toBounds.y) - expand;
    float x2 = std::max(fromBounds.x + fromBounds.w, toBounds.x + toBounds.w) + expand;
    float y2 = std::max(fromBounds.y + fromBounds.h, toBounds.y + toBounds.h) + expand;

    Renderer::AddDirtyRect(x1, y1, x2 - x1, y2 - y1);
    Renderer::RequestRepaint(duration);
}

} // namespace EUINEO
