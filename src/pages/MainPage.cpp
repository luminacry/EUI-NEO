#include "MainPage.h"
#include <algorithm>
#include <cmath>

namespace EUINEO {

namespace {

bool FloatEq(float a, float b, float epsilon = 0.0001f) {
    return std::abs(a - b) <= epsilon;
}

bool ColorEq(const Color& a, const Color& b, float epsilon = 0.0001f) {
    return FloatEq(a.r, b.r, epsilon) &&
           FloatEq(a.g, b.g, epsilon) &&
           FloatEq(a.b, b.b, epsilon) &&
           FloatEq(a.a, b.a, epsilon);
}

Color WithAlpha(const Color& color, float alpha) {
    return Color(color.r, color.g, color.b, alpha);
}

void CenterLabelInSquare(Label& label, float boxX, float boxY, float boxSize) {
    const float scale = label.fontSize / 24.0f;
    const float textWidth = Renderer::MeasureTextWidth(label.text, scale);
    label.x = boxX + (boxSize - textWidth) * 0.5f;
    label.y = boxY + boxSize * 0.5f + label.fontSize * 0.375f;
}

const char* SidebarIconForIndex(int index) {
    return index == 0 ? "\xEF\x80\x95" : "\xEF\x81\x8B";
}

bool IsDarkMode() {
    return CurrentTheme == &DarkTheme;
}

} // namespace

void MainPage::SidebarItemVisual::SetFrame(float x, float y, float size) {
    hitBox.x = x;
    hitBox.y = y;
    hitBox.width = size;
    hitBox.height = size;
}

void MainPage::SidebarItemVisual::Update() {
    const bool hovered = hitBox.IsHovered();
    const float target = hovered ? 1.0f : 0.0f;
    if (std::abs(hover - target) > 0.001f) {
        hover = Lerp(hover, target, State.deltaTime * 15.0f);
        if (std::abs(hover - target) < 0.01f) {
            hover = target;
        }
        hitBox.MarkDirty(20.0f);
    }

    if (hovered && State.mouseClicked && onClick) {
        onClick();
    }
}

void MainPage::SidebarItemVisual::ApplyVisual(bool selected) {
    const Color textBase = CurrentTheme->text;
    const Color mutedIcon = WithAlpha(textBase, IsDarkMode() ? 0.62f : 0.78f);
    const Color hoveredIcon = WithAlpha(textBase, 0.96f);

    icon.fontSize = 24.0f * (selected ? 1.0f : (0.96f + hover * 0.03f));
    icon.color = selected
        ? Color(1.0f, 1.0f, 1.0f, 0.98f)
        : Lerp(mutedIcon, hoveredIcon, hover);
    icon.rotationDegrees = 0.0f;
    CenterLabelInSquare(icon, hitBox.x, hitBox.y, hitBox.width);
}

void MainPage::SidebarItemVisual::Draw() {
    icon.Draw();
}

MainPage::ThemeToggleVisual::ThemeToggleVisual() {
    rotationAnimation.Bind(&rotation);
    iconBlendAnimation.Bind(&iconBlend);
}

void MainPage::ThemeToggleVisual::SetFrame(float x, float y, float size) {
    plate.x = x;
    plate.y = y;
    plate.width = size;
    plate.height = size;
    plate.rounding = size * 0.5f;
}

void MainPage::ThemeToggleVisual::Update() {
    const bool hovered = plate.IsHovered();
    const float targetHover = hovered ? 1.0f : 0.0f;
    if (std::abs(this->hover - targetHover) > 0.001f) {
        this->hover = Lerp(this->hover, targetHover, State.deltaTime * 15.0f);
        if (std::abs(this->hover - targetHover) < 0.01f) {
            this->hover = targetHover;
        }
        plate.MarkDirty(24.0f, 0.16f);
    }

    if (State.mouseClicked && hovered) {
        pressed = true;
        rotationAnimation.PlayTo(28.0f, 0.14f, Easing::EaseOut);
        plate.MarkDirty(24.0f, 0.20f);
    }

    if (pressed && !State.mouseDown) {
        const bool shouldToggleTheme = hovered;
        pressed = false;
        rotationAnimation.PlayTo(-24.0f, 0.12f, Easing::EaseOut);
        rotationAnimation.Queue(0.0f, 0.18f, Easing::EaseInOut);

        if (shouldToggleTheme && onToggle) {
            onToggle();
            const float targetBlend = isDarkMode && isDarkMode() ? 0.0f : 1.0f;
            iconBlendAnimation.PlayTo(targetBlend, 0.18f, Easing::EaseInOut);
        } else {
            plate.MarkDirty(24.0f, 0.20f);
        }
    }

    if (rotationAnimation.Update(State.deltaTime)) {
        plate.MarkDirty(24.0f, 0.20f);
    }

    if (iconBlendAnimation.Update(State.deltaTime)) {
        plate.MarkDirty(24.0f, 0.20f);
    }
}

void MainPage::ThemeToggleVisual::ApplyVisual() {
    const Color textBase = CurrentTheme->text;
    const Color mutedIcon = WithAlpha(textBase, IsDarkMode() ? 0.62f : 0.78f);
    const Color hoveredIcon = WithAlpha(textBase, 0.96f);

    const float plateMix = std::clamp(0.18f + hover * 0.18f + (pressed ? 0.20f : 0.0f), 0.0f, 0.42f);
    plate.borderColor = WithAlpha(CurrentTheme->border, IsDarkMode() ? 0.70f : 0.85f);
    plate.color = Lerp(CurrentTheme->surfaceHover, CurrentTheme->primary, plateMix);

    const float iconScale = 24.0f * (0.98f + hover * 0.04f);
    const Color iconColor = Lerp(mutedIcon, hoveredIcon, 0.40f + hover * 0.40f);
    moonIcon.fontSize = iconScale;
    sunIcon.fontSize = iconScale;
    moonIcon.rotationDegrees = rotation;
    sunIcon.rotationDegrees = rotation;
    moonIcon.color = WithAlpha(iconColor, 1.0f - std::clamp(iconBlend, 0.0f, 1.0f));
    sunIcon.color = WithAlpha(iconColor, std::clamp(iconBlend, 0.0f, 1.0f));
    CenterLabelInSquare(moonIcon, plate.x, plate.y, plate.width);
    CenterLabelInSquare(sunIcon, plate.x, plate.y, plate.width);
}

void MainPage::ThemeToggleVisual::Draw() {
    plate.Draw();
    moonIcon.Draw();
    sunIcon.Draw();
}

void MainPage::SidebarVisual::Draw() {
    shell.Draw();
    brandTop.Draw();
    brandBottom.Draw();
    indicator.Draw();
    for (SidebarItemVisual& item : items) {
        item.Draw();
    }
    themeToggle.Draw();
}

void MainPage::HomeVisual::CloseTransientState() {
    combo.isOpen = false;
    combo.openAnim = 0.0f;
    for (float& anim : combo.itemHoverAnims) {
        anim = 0.0f;
    }

    input.isFocused = false;
    input.focusAnim = 0.0f;
    input.cursorVisible = false;
    slider.isDragging = false;
}

void MainPage::HomeVisual::Update() {
    primary.Update();
    outline.Update();
    icon.Update();
    progress.Update();
    slider.Update();
    segmented.Update();
    input.Update();
    combo.Update();
}

void MainPage::HomeVisual::Draw() {
    primary.Draw();
    outline.Draw();
    icon.Draw();
    progress.Draw();
    slider.Draw();
    segmented.Draw();
    input.Draw();
    combo.Draw();
}

MainPage::MainPage() {
    bgCircle1.color = Color(0.98f, 0.36f, 0.36f, 0.92f);
    bgCircle1.rounding = 999.0f;

    bgCircle2.color = Color(0.30f, 0.92f, 0.58f, 0.88f);
    bgCircle2.rounding = 999.0f;

    bgCircle3.color = Color(0.34f, 0.52f, 1.0f, 0.90f);
    bgCircle3.rounding = 999.0f;

    sidebar.shell.rounding = 20.0f;
    sidebar.shell.borderWidth = 1.0f;
    sidebar.shell.shadowBlur = 0.0f;
    sidebar.shell.shadowOffsetX = 0.0f;
    sidebar.shell.shadowOffsetY = 0.0f;
    sidebar.shell.shadowColor = Color(0.0f, 0.0f, 0.0f, 0.0f);

    sidebar.indicator.rounding = 12.0f;

    sidebar.themeToggle.plate.borderWidth = 1.0f;
    sidebar.themeToggle.plate.shadowBlur = 0.0f;
    sidebar.themeToggle.plate.shadowOffsetX = 0.0f;
    sidebar.themeToggle.plate.shadowOffsetY = 0.0f;
    sidebar.themeToggle.plate.shadowColor = Color(0.0f, 0.0f, 0.0f, 0.0f);

    contentShell.color = CurrentTheme->surface;
    contentShell.color.a = 0.60f;
    contentShell.rounding = 16.0f;
    contentShell.shadowBlur = 20.0f;
    contentShell.shadowOffsetY = 10.0f;
    contentShell.shadowColor = Color(0.0f, 0.0f, 0.0f, 0.30f);

    home.title = Label("EUI-NEO", 0.0f, 0.0f);
    home.title.fontSize = 32.0f;

    sidebar.brandTop = Label("EUI", 0.0f, 0.0f, Color(1.0f, 1.0f, 1.0f, 1.0f));
    sidebar.brandTop.fontSize = 21.0f;
    sidebar.brandBottom = Label("NEO", 0.0f, 0.0f, Color(1.0f, 1.0f, 1.0f, 1.0f));
    sidebar.brandBottom.fontSize = 21.0f;

    sidebar.items[0].icon = Label(SidebarIconForIndex(0), 0.0f, 0.0f, Color(1.0f, 1.0f, 1.0f, 1.0f));
    sidebar.items[0].icon.fontSize = 24.0f;
    sidebar.items[0].onClick = [this]() { SwitchView(MainPageView::Home); };

    sidebar.items[1].icon = Label(SidebarIconForIndex(1), 0.0f, 0.0f, Color(1.0f, 1.0f, 1.0f, 1.0f));
    sidebar.items[1].icon.fontSize = 24.0f;
    sidebar.items[1].onClick = [this]() { SwitchView(MainPageView::Animation); };

    sidebar.themeToggle.moonIcon = Label("\xEF\x86\x86", 0.0f, 0.0f, Color(1.0f, 1.0f, 1.0f, 1.0f));
    sidebar.themeToggle.moonIcon.fontSize = 24.0f;
    sidebar.themeToggle.sunIcon = Label("\xEF\x86\x85", 0.0f, 0.0f, Color(1.0f, 1.0f, 1.0f, 1.0f));
    sidebar.themeToggle.sunIcon.fontSize = 24.0f;
    sidebar.themeToggle.onToggle = [this]() { ToggleTheme(); };
    sidebar.themeToggle.isDarkMode = []() { return CurrentTheme == &DarkTheme; };
    sidebar.themeToggle.iconBlend = IsDarkMode() ? 0.0f : 1.0f;

    home.primary = Button("Primary", 0.0f, 0.0f, 120.0f, 40.0f);
    home.primary.style = ButtonStyle::Primary;
    home.primary.fontSize = 20.0f;
    home.primary.onClick = [this]() {
        home.progress.value += 0.1f;
        if (home.progress.value > 1.0f) {
            home.progress.value = 0.0f;
        }
    };

    home.outline = Button("Outline", 0.0f, 0.0f, 120.0f, 40.0f);
    home.outline.style = ButtonStyle::Outline;
    home.outline.fontSize = 20.0f;
    home.outline.onClick = [this]() {
        home.slider.value = 0.0f;
        home.progress.value = 0.0f;
    };

    home.icon = Button("Icon  \xEF\x80\x93", 0.0f, 0.0f, 120.0f, 40.0f);
    home.icon.style = ButtonStyle::Default;
    home.icon.fontSize = 20.0f;

    home.progress = ProgressBar(0.0f, 0.0f, 300.0f, 15.0f);
    home.progress.value = 0.30f;

    home.slider = Slider(0.0f, 0.0f, 300.0f, 20.0f);
    home.slider.value = 0.30f;
    home.slider.onValueChanged = [this](float value) {
        home.progress.value = value;
    };

    home.segmented = SegmentedControl({"Apple", "Banana", "Cherry"}, 0.0f, 0.0f, 300.0f, 35.0f);
    home.segmented.fontSize = 20.0f;

    home.input = InputBox("Type something...", 0.0f, 0.0f, 300.0f, 35.0f);
    home.input.fontSize = 20.0f;

    home.combo = ComboBox("Select an option", 0.0f, 0.0f, 300.0f, 35.0f);
    home.combo.fontSize = 20.0f;
    home.combo.AddItem("Item 1");
    home.combo.AddItem("Item 2");
    home.combo.AddItem("Item 3");

    ApplyShellTheme();
    UpdateLayout();
}

int MainPage::ViewIndex(MainPageView view) const {
    return static_cast<int>(view);
}

float MainPage::SidebarItemSize() const {
    return 56.0f;
}

float MainPage::SidebarItemX() const {
    return sidebarX + (sidebarW - SidebarItemSize()) * 0.5f;
}

float MainPage::SidebarItemY(int index) const {
    return sidebarY + 130.0f + index * (SidebarItemSize() + 14.0f);
}

float MainPage::SidebarThemeToggleX() const {
    return SidebarItemX();
}

float MainPage::SidebarThemeToggleY() const {
    return sidebarY + sidebarH - SidebarItemSize() - 20.0f;
}

void MainPage::MarkSidebarDirty(float expand, float duration) {
    Renderer::AddDirtyRect(sidebar.shell.x - expand, sidebar.shell.y - expand,
                           sidebar.shell.width + expand * 2.0f, sidebar.shell.height + expand * 2.0f);
    Renderer::RequestRepaint(duration);
}

void MainPage::MarkContentDirty(float expand, float duration) {
    Renderer::AddDirtyRect(contentShell.x - expand, contentShell.y - expand,
                           contentShell.width + expand * 2.0f, contentShell.height + expand * 2.0f);
    Renderer::RequestRepaint(duration);
}

void MainPage::SwitchView(MainPageView view) {
    if (view == currentView) {
        return;
    }

    if (currentView == MainPageView::Home) {
        home.CloseTransientState();
    }

    const int previousIndex = ViewIndex(currentView);
    const int nextIndex = ViewIndex(view);
    currentView = view;
    pageReveal = 0.0f;
    pageRevealDirection = nextIndex >= previousIndex ? 1 : -1;

    MarkSidebarDirty(24.0f, 0.24f);
    MarkContentDirty(24.0f, 0.24f);
}

void MainPage::ToggleTheme() {
    CurrentTheme = IsDarkMode() ? &LightTheme : &DarkTheme;
    ApplyShellTheme();
    animationPage.OnThemeChanged();
    hasPreviousShellState = false;
    Renderer::InvalidateBackdrop();
    Renderer::InvalidateAll();
}

void MainPage::ApplyShellTheme() {
    sidebar.shell.color = CurrentTheme->surface;
    sidebar.shell.color.a = 1.0f;
    sidebar.shell.gradient = RectGradient{};
    sidebar.shell.borderColor = CurrentTheme->border;
    sidebar.shell.shadowColor = Color(0.0f, 0.0f, 0.0f, 0.0f);

    contentShell.color = CurrentTheme->surface;
    contentShell.color.a = 0.60f;
    contentShell.gradient = RectGradient{};
    if (IsDarkMode()) {
        contentShell.shadowBlur = 20.0f;
        contentShell.shadowOffsetY = 10.0f;
        contentShell.shadowColor = Color(0.0f, 0.0f, 0.0f, 0.30f);
    } else {
        contentShell.shadowBlur = 18.0f;
        contentShell.shadowOffsetY = 8.0f;
        contentShell.shadowColor = Color(0.10f, 0.14f, 0.22f, 0.18f);
    }
}

void MainPage::UpdateBackgroundLayout() {
    bgCircle1.x = contentX + contentW * 0.10f - 84.0f;
    bgCircle1.y = contentY + 58.0f;
    bgCircle1.width = 196.0f;
    bgCircle1.height = 196.0f;
    bgCircle1.rounding = bgCircle1.width * 0.5f;

    bgCircle2.x = contentX + contentW * 0.58f;
    bgCircle2.y = contentY + 86.0f;
    bgCircle2.width = 164.0f;
    bgCircle2.height = 164.0f;
    bgCircle2.rounding = bgCircle2.width * 0.5f;

    bgCircle3.x = contentX + contentW * 0.30f;
    bgCircle3.y = contentY + contentH * 0.44f;
    bgCircle3.width = 246.0f;
    bgCircle3.height = 246.0f;
    bgCircle3.rounding = bgCircle3.width * 0.5f;
}

void MainPage::UpdateLayout() {
    ApplyShellTheme();

    sidebarX = shellPadding;
    sidebarY = shellPadding;
    sidebarH = std::max(240.0f, State.screenH - shellPadding * 2.0f);
    contentX = sidebarX + sidebarW + 24.0f;
    contentY = shellPadding;
    contentW = std::max(280.0f, State.screenW - contentX - shellPadding);
    contentH = std::max(240.0f, State.screenH - shellPadding * 2.0f);

    sidebar.shell.x = sidebarX;
    sidebar.shell.y = sidebarY;
    sidebar.shell.width = sidebarW;
    sidebar.shell.height = sidebarH;
    sidebar.shell.rounding = 20.0f;

    sidebar.brandTop.color = WithAlpha(CurrentTheme->primary, IsDarkMode() ? 1.0f : 0.92f);
    sidebar.brandBottom.color = WithAlpha(CurrentTheme->text, IsDarkMode() ? 0.92f : 0.84f);
    const float brandTopScale = sidebar.brandTop.fontSize / 24.0f;
    const float brandBottomScale = sidebar.brandBottom.fontSize / 24.0f;
    const float brandTopWidth = Renderer::MeasureTextWidth(sidebar.brandTop.text, brandTopScale);
    const float brandBottomWidth = Renderer::MeasureTextWidth(sidebar.brandBottom.text, brandBottomScale);
    sidebar.brandTop.x = sidebarX + (sidebarW - brandTopWidth) * 0.5f;
    sidebar.brandTop.y = sidebarY + 44.0f;
    sidebar.brandBottom.x = sidebarX + (sidebarW - brandBottomWidth) * 0.5f;
    sidebar.brandBottom.y = sidebarY + 72.0f;

    const float itemSize = SidebarItemSize();
    const float itemX = SidebarItemX();
    sidebar.indicator.x = itemX;
    sidebar.indicator.width = itemSize;
    sidebar.indicator.height = itemSize;

    sidebar.items[0].SetFrame(itemX, SidebarItemY(0), itemSize);
    sidebar.items[1].SetFrame(itemX, SidebarItemY(1), itemSize);
    sidebar.themeToggle.SetFrame(SidebarThemeToggleX(), SidebarThemeToggleY(), itemSize);

    contentShell.x = contentX;
    contentShell.y = contentY;
    contentShell.width = contentW;
    contentShell.height = contentH;
    contentShell.blurAmount = home.slider.value * 0.15f;

    currentContentOffsetX = (1.0f - pageReveal) * 28.0f * static_cast<float>(pageRevealDirection);
    const float contentCenterX = contentX + contentW * 0.5f + currentContentOffsetX;
    const float contentCenterY = contentY + contentH * 0.5f;

    homeLeftX = contentCenterX - 150.0f;
    homeTopY = contentY + 30.0f;
    homeButtonsY = contentY + 62.0f;
    homeLeftW = 300.0f;

    const float titleScale = home.title.fontSize / 24.0f;
    const float titleWidth = Renderer::MeasureTextWidth(home.title.text, titleScale);
    home.title.x = contentCenterX - titleWidth * 0.5f;
    home.title.y = homeTopY;

    home.primary.x = contentCenterX - 130.0f;
    home.primary.y = homeButtonsY;
    home.outline.x = contentCenterX + 10.0f;
    home.outline.y = homeButtonsY;
    home.icon.x = contentCenterX - 60.0f;
    home.icon.y = homeButtonsY + 60.0f;

    home.progress.x = contentCenterX - 150.0f;
    home.progress.y = contentCenterY - 60.0f;
    home.progress.width = 300.0f;

    home.slider.x = contentCenterX - 150.0f;
    home.slider.y = contentCenterY - 10.0f;
    home.slider.width = 300.0f;

    home.segmented.x = contentCenterX - 150.0f;
    home.segmented.y = contentCenterY + 40.0f;
    home.segmented.width = 300.0f;

    home.input.x = contentCenterX - 150.0f;
    home.input.y = contentCenterY + 100.0f;
    home.input.width = 300.0f;

    home.combo.x = contentCenterX - 150.0f;
    home.combo.y = contentCenterY + 160.0f;
    home.combo.width = 300.0f;

    animationPage.SetBounds(contentX + contentInset + currentContentOffsetX,
                            contentY + 18.0f,
                            contentW - contentInset * 2.0f,
                            contentH - 36.0f);

    UpdateBackgroundLayout();
}

void MainPage::UpdateSidebar() {
    sidebar.items[0].Update();
    sidebar.items[1].Update();

    const float targetY = SidebarItemY(ViewIndex(currentView)) - 1.0f;
    if (!sidebar.indicatorReady) {
        sidebar.indicatorY = targetY;
        sidebar.indicatorReady = true;
    } else if (std::abs(sidebar.indicatorY - targetY) > 0.001f) {
        sidebar.indicatorY = Lerp(sidebar.indicatorY, targetY, State.deltaTime * 14.0f);
        if (std::abs(sidebar.indicatorY - targetY) < 0.01f) {
            sidebar.indicatorY = targetY;
        }
        MarkSidebarDirty(22.0f, 0.18f);
    }

    sidebar.themeToggle.Update();

    sidebar.indicator.y = sidebar.indicatorY;
    sidebar.indicator.color = CurrentTheme->primary;
    sidebar.indicator.color.a = 1.0f;

    sidebar.items[0].ApplyVisual(ViewIndex(currentView) == 0);
    sidebar.items[1].ApplyVisual(ViewIndex(currentView) == 1);
    sidebar.themeToggle.ApplyVisual();
}

void MainPage::UpdatePageReveal() {
    if (pageReveal >= 1.0f) {
        return;
    }

    const float previous = pageReveal;
    pageReveal = Lerp(pageReveal, 1.0f, State.deltaTime * 11.0f);
    if (std::abs(1.0f - pageReveal) < 0.01f) {
        pageReveal = 1.0f;
    }

    if (!FloatEq(previous, pageReveal)) {
        MarkContentDirty(28.0f, 0.18f);
    }
}

void MainPage::UpdateHome() {
    home.Update();
    contentShell.blurAmount = home.slider.value * 0.15f;

    if (!hasPreviousShellState ||
        !FloatEq(previousShellBlurAmount, contentShell.blurAmount) ||
        !ColorEq(previousShellColor, contentShell.color)) {
        MarkContentDirty(20.0f);
        previousShellBlurAmount = contentShell.blurAmount;
        previousShellColor = contentShell.color;
        hasPreviousShellState = true;
    }
}

void MainPage::Update() {
    UpdateLayout();
    UpdateSidebar();
    const float previousReveal = pageReveal;
    UpdatePageReveal();
    if (!FloatEq(previousReveal, pageReveal)) {
        UpdateLayout();
    }

    if (currentView == MainPageView::Home) {
        UpdateHome();
    } else {
        animationPage.Update();
    }
}

void MainPage::DrawBackground() {
    bgCircle1.Draw();
    bgCircle2.Draw();
    bgCircle3.Draw();
}

void MainPage::Draw() {
    DrawBackground();
    sidebar.Draw();
    Renderer::CaptureBackdrop();
    contentShell.Draw();

    if (currentView == MainPageView::Home) {
        home.Draw();
    } else {
        animationPage.Draw();
    }
}

} // namespace EUINEO
