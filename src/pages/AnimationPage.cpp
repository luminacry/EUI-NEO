#include "AnimationPage.h"
#include <algorithm>
#include <cmath>

namespace EUINEO {

namespace {

Color DimmedText(float alpha) {
    return Color(CurrentTheme->text.r, CurrentTheme->text.g, CurrentTheme->text.b, alpha);
}

bool ThemeIsDark() {
    return CurrentTheme == &DarkTheme;
}

bool FloatEq(float a, float b, float epsilon = 0.0001f) {
    return std::abs(a - b) <= epsilon;
}

bool FrameEq(const RectFrame& a, const RectFrame& b, float epsilon = 0.0001f) {
    return FloatEq(a.x, b.x, epsilon) &&
           FloatEq(a.y, b.y, epsilon) &&
           FloatEq(a.width, b.width, epsilon) &&
           FloatEq(a.height, b.height, epsilon);
}

} // namespace

void AnimationPage::TextBadge::Draw() {
    panel.Draw();
    text.Draw();
}

void AnimationPage::DemoCard::Draw() {
    panel.Draw();
    title.Draw();
    detailLine1.Draw();
    detailLine2.Draw();
    hintBadge.Draw();
    sample.Draw();
}

AnimationPage::AnimationPage() {
    pageTitle_ = Label("Animation Page", 0.0f, 0.0f);
    pageTitle_.fontSize = 31.0f;
    pageTitle_.useThemeColor = false;

    pageSubtitle_ = Label("Hover cards to preview rect property tracks. Click for a queued burst.",
                          0.0f, 0.0f);
    pageSubtitle_.fontSize = 17.0f;
    pageSubtitle_.useThemeColor = false;

    InitializeCards();
    SyncTheme();
}

void AnimationPage::InitializeCards() {
    cards_[0].title = Label("Fade Alpha", 0.0f, 0.0f);
    cards_[0].detailLine1 = Label("Use color.a to control", 0.0f, 0.0f);
    cards_[0].detailLine2 = Label("the fill opacity.", 0.0f, 0.0f);
    cards_[0].hintBadge.text = Label("Color.a", 0.0f, 0.0f);

    cards_[1].title = Label("Uniform Scale", 0.0f, 0.0f);
    cards_[1].detailLine1 = Label("Scale both axes together", 0.0f, 0.0f);
    cards_[1].detailLine2 = Label("for lift and focus.", 0.0f, 0.0f);
    cards_[1].hintBadge.text = Label("scaleX = scaleY", 0.0f, 0.0f);

    cards_[2].title = Label("Axis Stretch", 0.0f, 0.0f);
    cards_[2].detailLine1 = Label("Animate scaleX / scaleY", 0.0f, 0.0f);
    cards_[2].detailLine2 = Label("for directional stretch.", 0.0f, 0.0f);
    cards_[2].hintBadge.text = Label("scaleX / scaleY", 0.0f, 0.0f);

    cards_[3].title = Label("Queue + Combo", 0.0f, 0.0f);
    cards_[3].detailLine1 = Label("Blend move, rotate, scale", 0.0f, 0.0f);
    cards_[3].detailLine2 = Label("and gradient in queue.", 0.0f, 0.0f);
    cards_[3].hintBadge.text = Label("PlayTo + Queue", 0.0f, 0.0f);

    for (DemoCard& card : cards_) {
        card.panel.anchor = Anchor::TopLeft;
        card.sample.anchor = Anchor::TopLeft;

        card.title.fontSize = 23.0f;
        card.detailLine1.fontSize = 15.0f;
        card.detailLine2.fontSize = 15.0f;
        card.hintBadge.text.fontSize = 14.0f;

        card.title.useThemeColor = false;
        card.detailLine1.useThemeColor = false;
        card.detailLine2.useThemeColor = false;
        card.hintBadge.text.useThemeColor = false;
    }
}

void AnimationPage::ApplyThemeToCard(DemoCard& card, int index) {
    const bool dark = ThemeIsDark();

    card.panel.color = CurrentTheme->surface;
    card.panel.color.a = 0.98f;
    card.panel.gradient = RectGradient{};
    card.panel.rounding = 16.0f;
    card.panel.borderWidth = 0.0f;
    card.panel.borderColor = Color(0.0f, 0.0f, 0.0f, 0.0f);
    card.panel.blurAmount = 0.0f;
    card.panel.shadowBlur = 0.0f;
    card.panel.shadowOffsetX = 0.0f;
    card.panel.shadowOffsetY = 0.0f;
    card.panel.shadowColor = Color(0.0f, 0.0f, 0.0f, 0.0f);
    card.panel.transform = RectTransform{};

    card.panelRest = card.panel.GetState();
    card.panelHover = card.panelRest;
    card.panelHover.style.transform.translateY = -4.0f;

    card.sample.color = CurrentTheme->primary;
    card.sample.gradient = RectGradient{};
    card.sample.rounding = 12.0f;
    card.sample.borderWidth = 0.0f;
    card.sample.borderColor = Color(0.0f, 0.0f, 0.0f, 0.0f);
    card.sample.blurAmount = 0.0f;
    card.sample.shadowBlur = 0.0f;
    card.sample.shadowOffsetX = 0.0f;
    card.sample.shadowOffsetY = 0.0f;
    card.sample.shadowColor = Color(0.0f, 0.0f, 0.0f, 0.0f);
    card.sample.transform = RectTransform{};

    card.sampleRest = card.sample.GetState();
    card.sampleHover = card.sampleRest;

    if (index == 0) {
        card.sampleRest.style.color.a = dark ? 0.36f : 0.48f;
        card.sampleHover = card.sampleRest;
        card.sampleHover.style.color.a = 1.0f;
    } else if (index == 1) {
        card.sampleHover.style.transform.scaleX = 1.18f;
        card.sampleHover.style.transform.scaleY = 1.18f;
    } else if (index == 2) {
        card.sampleHover.style.transform.scaleX = 1.26f;
        card.sampleHover.style.transform.scaleY = 0.78f;
    } else {
        card.sampleHover.style.gradient = RectGradient::Vertical(
            Lerp(CurrentTheme->primary, Color(1.0f, 1.0f, 1.0f, 1.0f), dark ? 0.10f : 0.20f),
            Lerp(CurrentTheme->primary, Color(0.0f, 0.0f, 0.0f, 1.0f), dark ? 0.18f : 0.10f)
        );
        card.sampleHover.style.transform.translateY = -6.0f;
        card.sampleHover.style.transform.scaleX = 1.10f;
        card.sampleHover.style.transform.scaleY = 1.10f;
        card.sampleHover.style.transform.rotationDegrees = 10.0f;
    }

    card.title.color = DimmedText(0.96f);
    card.detailLine1.color = DimmedText(0.66f);
    card.detailLine2.color = DimmedText(0.66f);

    card.hintBadge.panel.color = CurrentTheme->surfaceHover;
    card.hintBadge.panel.rounding = 10.0f;
    card.hintBadge.panel.borderWidth = 0.0f;
    card.hintBadge.panel.borderColor = Color(0.0f, 0.0f, 0.0f, 0.0f);
    card.hintBadge.panel.blurAmount = 0.0f;
    card.hintBadge.panel.shadowBlur = 0.0f;
    card.hintBadge.panel.shadowOffsetX = 0.0f;
    card.hintBadge.panel.shadowOffsetY = 0.0f;
    card.hintBadge.panel.shadowColor = Color(0.0f, 0.0f, 0.0f, 0.0f);
    card.hintBadge.panel.transform = RectTransform{};
    card.hintBadge.text.color = DimmedText(0.82f);

    card.panelCurrent = card.hovered ? card.panelHover : card.panelRest;
    card.sampleCurrent = card.hovered ? card.sampleHover : card.sampleRest;

    card.panelAnimation.Clear();
    card.sampleAnimation.Clear();
    card.panelAnimation.Bind(&card.panelCurrent);
    card.sampleAnimation.Bind(&card.sampleCurrent);
    card.panel.SetState(card.panelCurrent);
    card.sample.SetState(card.sampleCurrent);
}

void AnimationPage::SyncTheme() {
    if (lastTheme_ == CurrentTheme) {
        return;
    }

    lastTheme_ = CurrentTheme;
    pageTitle_.color = DimmedText(0.98f);
    pageSubtitle_.color = DimmedText(0.72f);

    for (int index = 0; index < static_cast<int>(cards_.size()); ++index) {
        ApplyThemeToCard(cards_[index], index);
    }
}

void AnimationPage::SetBounds(float x, float y, float w, float h) {
    x_ = x;
    y_ = y;
    w_ = w;
    h_ = h;
}

void AnimationPage::OnThemeChanged() {
    lastTheme_ = nullptr;
    SyncTheme();
    MarkPageDirty(20.0f);
}

RectFrame AnimationPage::GetSampleFrame(const DemoCard& card) const {
    RectFrame frame;
    frame.width = std::min(84.0f, card.panel.width * 0.24f);
    frame.height = std::min(56.0f, card.panel.height * 0.30f);
    frame.x = card.panel.x + card.panel.width - frame.width - 26.0f;

    const float desiredY = card.panel.y + card.panel.height * 0.58f - frame.height * 0.5f;
    const float minY = card.panel.y + card.panel.height * 0.38f;
    const float maxY = card.panel.y + card.panel.height - frame.height - std::max(28.0f, card.panel.height * 0.18f);
    frame.y = maxY >= minY ? std::clamp(desiredY, minY, maxY) : desiredY;
    return frame;
}

void AnimationPage::LayoutCards() {
    if (w_ <= 0.0f || h_ <= 0.0f) {
        return;
    }

    pageTitle_.x = x_;
    pageTitle_.y = y_ + 24.0f;
    pageSubtitle_.x = x_;
    pageSubtitle_.y = y_ + 54.0f;

    const float gap = 18.0f;
    const float topOffset = 98.0f;
    const bool twoColumns = w_ >= 520.0f;
    const int columns = twoColumns ? 2 : 1;
    const float cardW = twoColumns ? (w_ - gap) * 0.5f : w_;
    const float availableH = std::max(240.0f, h_ - topOffset - gap);
    const float cardH = twoColumns
        ? std::clamp((availableH - gap) * 0.5f, 170.0f, 210.0f)
        : std::clamp((availableH - gap * 3.0f) * 0.25f, 116.0f, 152.0f);

    for (int index = 0; index < static_cast<int>(cards_.size()); ++index) {
        const int col = index % columns;
        const int row = index / columns;
        DemoCard& card = cards_[index];

        RectFrame nextPanelFrame;
        nextPanelFrame.x = x_ + col * (cardW + gap);
        nextPanelFrame.y = y_ + topOffset + row * (cardH + gap);
        nextPanelFrame.width = cardW;
        nextPanelFrame.height = cardH;

        card.panel.SetFrame(nextPanelFrame);
        RectFrame nextSampleFrame = GetSampleFrame(card);

        const bool panelFrameChanged = !FrameEq(card.panelRest.frame, nextPanelFrame);
        const bool sampleFrameChanged = !FrameEq(card.sampleRest.frame, nextSampleFrame);

        card.panelRest.frame = nextPanelFrame;
        card.panelHover.frame = nextPanelFrame;
        card.sampleRest.frame = nextSampleFrame;
        card.sampleHover.frame = nextSampleFrame;
        card.panelCurrent.frame = nextPanelFrame;
        card.sampleCurrent.frame = nextSampleFrame;
        card.panel.SetState(card.panelCurrent);
        card.sample.SetState(card.sampleCurrent);

        card.title.x = nextPanelFrame.x + 24.0f;
        card.title.y = nextPanelFrame.y + 40.0f;
        card.detailLine1.x = nextPanelFrame.x + 24.0f;
        card.detailLine1.y = card.title.y + 32.0f;
        card.detailLine2.x = nextPanelFrame.x + 24.0f;
        card.detailLine2.y = card.title.y + 50.0f;

        const float badgeScale = card.hintBadge.text.fontSize / 24.0f;
        const float badgeTextWidth = Renderer::MeasureTextWidth(card.hintBadge.text.text, badgeScale);
        card.hintBadge.panel.x = nextPanelFrame.x + 24.0f;
        card.hintBadge.panel.y = nextPanelFrame.y + nextPanelFrame.height - 38.0f;
        card.hintBadge.panel.width = badgeTextWidth + 24.0f;
        card.hintBadge.panel.height = 24.0f;
        card.hintBadge.text.x = card.hintBadge.panel.x + 12.0f;
        card.hintBadge.text.y = card.hintBadge.panel.y + 16.5f;

        if (panelFrameChanged || sampleFrameChanged) {
            card.panelAnimation.Clear();
            card.sampleAnimation.Clear();
            card.panelCurrent = card.hovered ? card.panelHover : card.panelRest;
            card.sampleCurrent = card.hovered ? card.sampleHover : card.sampleRest;
            card.panel.SetState(card.panelCurrent);
            card.sample.SetState(card.sampleCurrent);
        }
    }
}

void AnimationPage::UpdateCard(DemoCard& card, int index) {
    const bool hovered = card.panel.IsHovered();
    if (hovered != card.hovered) {
        card.hovered = hovered;
        card.panelAnimation.PlayTo(hovered ? card.panelHover : card.panelRest, 0.18f,
                                   hovered ? Easing::EaseOut : Easing::EaseInOut);
        card.sampleAnimation.PlayTo(hovered ? card.sampleHover : card.sampleRest, 0.18f,
                                    hovered ? Easing::EaseOut : Easing::EaseInOut);
    }

    if (hovered && State.mouseClicked) {
        PanelState panelBurst = card.panelHover;
        panelBurst.style.transform.translateY -= 3.0f;
        panelBurst.style.transform.scaleX += 0.01f;
        panelBurst.style.transform.scaleY += 0.01f;

        PanelState sampleBurst = card.sampleHover;
        if (index == 0) {
            sampleBurst.style.color.a = 1.0f;
        } else if (index == 1) {
            sampleBurst.style.transform.scaleX = 1.26f;
            sampleBurst.style.transform.scaleY = 1.26f;
        } else if (index == 2) {
            sampleBurst.style.transform.scaleX = 1.32f;
            sampleBurst.style.transform.scaleY = 0.72f;
        } else {
            sampleBurst.style.transform.translateY = -10.0f;
            sampleBurst.style.transform.scaleX = 1.18f;
            sampleBurst.style.transform.scaleY = 1.18f;
            sampleBurst.style.transform.rotationDegrees = 18.0f;
            sampleBurst.style.gradient = RectGradient::Vertical(
                Lerp(CurrentTheme->primary, Color(1.0f, 1.0f, 1.0f, 1.0f), 0.24f),
                Lerp(CurrentTheme->primary, Color(0.0f, 0.0f, 0.0f, 1.0f), 0.14f)
            );
        }

        card.panelAnimation.PlayTo(panelBurst, 0.10f, Easing::EaseOut);
        card.panelAnimation.Queue(card.panelHover, 0.16f, Easing::EaseInOut);
        card.sampleAnimation.PlayTo(sampleBurst, 0.10f, Easing::EaseOut);
        card.sampleAnimation.Queue(card.sampleHover, 0.18f, Easing::EaseInOut);
    }

    PanelState previousPanel = card.panelCurrent;
    if (card.panelAnimation.Update(State.deltaTime)) {
        card.panel.SetState(card.panelCurrent);
        card.panel.MarkDirty(previousPanel, card.panelCurrent, 12.0f);
    }

    PanelState previousSample = card.sampleCurrent;
    if (card.sampleAnimation.Update(State.deltaTime)) {
        card.sample.SetState(card.sampleCurrent);
        card.sample.MarkDirty(previousSample, card.sampleCurrent, 12.0f);
    }
}

void AnimationPage::Update() {
    if (w_ <= 0.0f || h_ <= 0.0f) {
        return;
    }

    SyncTheme();
    LayoutCards();
    for (int index = 0; index < static_cast<int>(cards_.size()); ++index) {
        UpdateCard(cards_[index], index);
    }
}

void AnimationPage::Draw() {
    if (w_ <= 0.0f || h_ <= 0.0f) {
        return;
    }

    pageTitle_.Draw();
    pageSubtitle_.Draw();
    for (DemoCard& card : cards_) {
        card.Draw();
    }
}

void AnimationPage::MarkPageDirty(float expand) const {
    if (w_ <= 0.0f || h_ <= 0.0f) {
        return;
    }
    Renderer::AddDirtyRect(x_ - expand, y_ - expand, w_ + expand * 2.0f, h_ + expand * 2.0f);
    Renderer::RequestRepaint();
}

} // namespace EUINEO
