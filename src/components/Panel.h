#pragma once
#include "../EUINEO.h"

namespace EUINEO {

class Panel : public Widget {
public:
    Color color;
    RectGradient gradient;
    RectTransform transform;
    float rounding = 0.0f;
    float borderWidth = 0.0f;
    Color borderColor = Color(0, 0, 0, 0);
    float blurAmount = 0.0f;
    float shadowBlur = 0.0f;
    float shadowOffsetX = 0.0f;
    float shadowOffsetY = 0.0f;
    Color shadowColor = Color(0, 0, 0, 0);

    Panel() = default;
    Panel(float x, float y, float w, float h);

    using Widget::MarkDirty;

    void Update() override;
    void Draw() override;
    RectFrame GetFrame() const;
    void SetFrame(const RectFrame& frame);
    RectStyle GetStyle() const;
    void SetStyle(const RectStyle& style);
    PanelState GetState() const;
    void SetState(const PanelState& state);
    void MarkDirty(float expand = 0.0f, float duration = 0.0f);
    void MarkDirty(const PanelState& fromState, const PanelState& toState, float expand = 0.0f,
                   float duration = 0.0f);
};

} // namespace EUINEO
