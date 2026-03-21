#pragma once

#include "../EUINEO.h"
#include "../components/Button.h"
#include "../components/ComboBox.h"
#include "../components/InputBox.h"
#include "../components/Label.h"
#include "../components/Panel.h"
#include "../components/ProgressBar.h"
#include "../components/SegmentedControl.h"
#include "../components/Slider.h"
#include "AnimationPage.h"
#include <array>
#include <functional>

namespace EUINEO {

enum class MainPageView {
    Home = 0,
    Animation = 1,
};

class MainPage {
public:
    struct SidebarItemVisual {
        Panel hitBox;
        Label icon;
        float hover = 0.0f;
        std::function<void()> onClick;

        void SetFrame(float x, float y, float size);
        void Update();
        void ApplyVisual(bool selected);
        void Draw();
    };

    struct ThemeToggleVisual {
        Panel plate;
        Label moonIcon;
        Label sunIcon;
        float hover = 0.0f;
        bool pressed = false;
        float rotation = 0.0f;
        FloatAnimation rotationAnimation;
        float iconBlend = 0.0f;
        FloatAnimation iconBlendAnimation;
        std::function<void()> onToggle;
        std::function<bool()> isDarkMode;

        ThemeToggleVisual();
        void SetFrame(float x, float y, float size);
        void Update();
        void ApplyVisual();
        void Draw();
    };

    struct SidebarVisual {
        Panel shell;
        Label brandTop;
        Label brandBottom;
        Panel indicator;
        std::array<SidebarItemVisual, 2> items;
        ThemeToggleVisual themeToggle;
        float indicatorY = 0.0f;
        bool indicatorReady = false;

        void Draw();
    };

    struct HomeVisual {
        Label title;
        Button primary;
        Button outline;
        Button icon;
        ProgressBar progress;
        Slider slider;
        SegmentedControl segmented;
        InputBox input;
        ComboBox combo;

        void CloseTransientState();
        void Update();
        void Draw();
    };

    Panel bgCircle1;
    Panel bgCircle2;
    Panel bgCircle3;
    SidebarVisual sidebar;
    Panel contentShell;
    HomeVisual home;
    AnimationPage animationPage;

    MainPageView currentView = MainPageView::Home;
    float pageReveal = 1.0f;
    int pageRevealDirection = 1;

    float previousShellBlurAmount = -1.0f;
    Color previousShellColor;
    bool hasPreviousShellState = false;

    MainPage();
    void Update();
    void Draw();

private:
    float shellPadding = 22.0f;
    float sidebarX = 0.0f;
    float sidebarY = 0.0f;
    float sidebarW = 86.0f;
    float sidebarH = 0.0f;
    float contentX = 0.0f;
    float contentY = 0.0f;
    float contentW = 0.0f;
    float contentH = 0.0f;
    float contentInset = 34.0f;
    float currentContentOffsetX = 0.0f;
    bool homeTwoColumn = true;
    float homeLeftX = 0.0f;
    float homeTopY = 0.0f;
    float homeLeftW = 0.0f;
    float homeRightX = 0.0f;
    float homeRightW = 0.0f;
    float homeButtonsY = 0.0f;

    void UpdateLayout();
    void ApplyShellTheme();
    void ToggleTheme();
    void UpdateBackgroundLayout();
    void UpdateSidebar();
    void UpdatePageReveal();
    void UpdateHome();
    void SwitchView(MainPageView view);

    void DrawBackground();

    int ViewIndex(MainPageView view) const;
    float SidebarItemX() const;
    float SidebarItemY(int index) const;
    float SidebarItemSize() const;
    float SidebarThemeToggleX() const;
    float SidebarThemeToggleY() const;

    void MarkSidebarDirty(float expand = 18.0f, float duration = 0.0f);
    void MarkContentDirty(float expand = 18.0f, float duration = 0.0f);
};

} // namespace EUINEO
