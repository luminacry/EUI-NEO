#pragma once

#include "../EUINEO.h"
#include "../components/Label.h"
#include "../components/Panel.h"
#include <array>
#include <string>

namespace EUINEO {

class AnimationPage {
public:
    AnimationPage();

    void SetBounds(float x, float y, float w, float h);
    void OnThemeChanged();
    void Update();
    void Draw();

private:
    struct TextBadge {
        Panel panel;
        Label text;

        void Draw();
    };

    struct DemoCard {
        Panel panel;
        Label title;
        Label detailLine1;
        Label detailLine2;
        TextBadge hintBadge;
        Panel sample;

        PanelState panelRest;
        PanelState panelHover;
        PanelState panelCurrent;
        PanelStateAnimation panelAnimation;

        PanelState sampleRest;
        PanelState sampleHover;
        PanelState sampleCurrent;
        PanelStateAnimation sampleAnimation;

        bool hovered = false;

        void Draw();
    };

    Label pageTitle_;
    Label pageSubtitle_;
    float x_ = 0.0f;
    float y_ = 0.0f;
    float w_ = 0.0f;
    float h_ = 0.0f;
    Theme* lastTheme_ = nullptr;

    std::array<DemoCard, 4> cards_{};

    void InitializeCards();
    void SyncTheme();
    void ApplyThemeToCard(DemoCard& card, int index);
    void LayoutCards();
    void UpdateCard(DemoCard& card, int index);
    RectFrame GetSampleFrame(const DemoCard& card) const;
    void MarkPageDirty(float expand = 18.0f) const;
};

} // namespace EUINEO
