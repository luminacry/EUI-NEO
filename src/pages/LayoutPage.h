#pragma once

#include "../EUINEO.h"
#include "../ui/UIContext.h"
#include "../ui/ThemeTokens.h"
#include <algorithm>
#include <array>
#include <functional>
#include <string>

namespace EUINEO {

class LayoutPage {
public:
    struct Actions {
        std::function<void(float)> onSplitChange;
    };

    static void Compose(UIContext& ui, const std::string& idPrefix, const RectFrame& bounds,
                        float splitRatio, const Actions& actions) {
        if (bounds.width <= 0.0f || bounds.height <= 0.0f) {
            return;
        }

        const PageVisualTokens visuals = CurrentPageVisuals();
        const PageHeaderLayout header = ComposePageHeader(
            ui,
            idPrefix,
            bounds,
            "Layout Playground",
            "Drag the slider to test left and right layout balance."
        );
        const float gap = visuals.sectionGap;
        const float split = std::clamp(splitRatio, 0.28f, 0.72f);
        const float sliderValue = (split - 0.28f) / 0.44f;

        const float controlY = header.contentY;
        const float controlH = 76.0f;
        const float previewY = controlY + controlH + gap;
        const float previewH = std::max(0.0f, bounds.y + bounds.height - previewY);

        ComposePageSection(ui, idPrefix + ".control", RectFrame{bounds.x, controlY, bounds.width, controlH});

        ui.label(idPrefix + ".control.label")
            .text("Left / Right Split")
            .position(bounds.x + 20.0f, controlY + 26.0f)
            .fontSize(visuals.labelSize)
            .build();

        ui.label(idPrefix + ".control.value")
            .text(std::to_string(static_cast<int>(split * 100.0f)) + "%")
            .position(std::max(bounds.x + 20.0f, bounds.x + bounds.width - 72.0f), controlY + 26.0f)
            .fontSize(16.0f)
            .color(visuals.bodyColor)
            .build();

        ui.slider(idPrefix + ".control.slider")
            .position(bounds.x + 20.0f, controlY + 46.0f)
            .size(std::max(0.0f, bounds.width - 40.0f), 18.0f)
            .value(sliderValue)
            .onChange([action = actions.onSplitChange](float value) {
                if (action) {
                    action(0.28f + std::clamp(value, 0.0f, 1.0f) * 0.44f);
                }
            })
            .build();

        if (previewH <= 0.0f) {
            return;
        }

        ComposePageSection(ui, idPrefix + ".preview", RectFrame{bounds.x, previewY, bounds.width, previewH});

        const float innerX = bounds.x + visuals.sectionInset;
        const float innerY = previewY + visuals.sectionInset;
        const float innerW = std::max(0.0f, bounds.width - visuals.sectionInset * 2.0f);
        const float innerH = std::max(0.0f, previewH - visuals.sectionInset * 2.0f);
        if (innerW <= 0.0f || innerH <= 0.0f) {
            return;
        }

        const float splitW = std::max(0.0f, innerW - gap);
        float leftW = splitW * split;
        if (splitW > 180.0f) {
            leftW = std::clamp(leftW, 84.0f, splitW - 84.0f);
        } else {
            leftW = splitW * 0.5f;
        }
        const float rightW = std::max(0.0f, splitW - leftW);

        ui.panel(idPrefix + ".divider")
            .position(innerX + leftW, innerY + 18.0f)
            .size(gap, std::max(0.0f, innerH - 36.0f))
            .background(visuals.softAccentColor)
            .rounding(8.0f)
            .build();

        ComposePane(ui, idPrefix + ".left.info", innerX, innerY, leftW, innerH,
                    "Left Pane", "Navigation, filters or tools.");
        ComposePane(ui, idPrefix + ".right.info", innerX + leftW + gap, innerY, rightW, innerH,
                    "Right Pane", "Content, preview or inspector.");
    }

private:
    static void ComposePane(UIContext& ui, const std::string& idPrefix,
                            float x, float y, float width, float height,
                            const char* title, const char* note) {
        if (width <= 0.0f || height <= 0.0f) {
            return;
        }

        const PageVisualTokens visuals = CurrentPageVisuals();

        ui.label(idPrefix + ".title")
            .text(title)
            .position(x + 18.0f, y + 30.0f)
            .fontSize(22.0f)
            .build();

        if (height < 120.0f) {
            ui.label(idPrefix + ".width")
                .text(std::to_string(static_cast<int>(width)) + " px")
                .position(x + 18.0f, y + 62.0f)
                .fontSize(18.0f)
                .color(Color(CurrentTheme->primary.r, CurrentTheme->primary.g, CurrentTheme->primary.b, 0.92f))
                .build();
            return;
        }

        ui.label(idPrefix + ".note")
            .text(note)
            .position(x + 18.0f, y + 56.0f)
            .fontSize(14.0f)
            .color(visuals.bodyColor)
            .build();

        ui.label(idPrefix + ".width")
            .text(std::to_string(static_cast<int>(width)) + " px")
            .position(x + 18.0f, y + 92.0f)
            .fontSize(18.0f)
            .color(Color(CurrentTheme->primary.r, CurrentTheme->primary.g, CurrentTheme->primary.b, 0.92f))
            .build();

        if (height < 150.0f) {
            return;
        }

        const float previewX = x + 18.0f;
        const float previewY = y + 122.0f;
        const float previewW = std::max(0.0f, width - 36.0f);
        const float previewH = std::max(0.0f, height - 142.0f);
        ComposePageSection(ui, idPrefix + ".preview", RectFrame{previewX, previewY, previewW, previewH}, visuals.mutedCardColor);

        const float scrollX = previewX + 12.0f;
        const float scrollY = previewY + 12.0f;
        const float scrollW = std::max(0.0f, previewW - 24.0f);
        const float scrollH = std::max(0.0f, previewH - 24.0f);
        if (scrollW <= 0.0f || scrollH <= 0.0f) {
            return;
        }

        static const std::array<const char*, 10> rows{{
            "Header / Toolbar",
            "Sidebar / Filters",
            "Primary Content",
            "Inspector / Details",
            "Footer Actions",
            "Overflow Content",
            "Search Results",
            "Selection Summary",
            "Pinned Notes",
            "Bottom Utilities",
        }};
        const float rowHeight = 28.0f;
        const float contentHeight = std::max(
            scrollH + 180.0f,
            28.0f + static_cast<float>(rows.size()) * rowHeight + 20.0f
        );
        ui.scrollArea(idPrefix + ".preview.scroll", scrollX, scrollY, scrollW, scrollH, contentHeight, [&] {
            for (std::size_t index = 0; index < rows.size(); ++index) {
                const float rowY = scrollY + 22.0f + static_cast<float>(index) * rowHeight;
                ui.label(idPrefix + ".preview.row" + std::to_string(index))
                    .text(rows[index])
                    .position(scrollX, rowY)
                    .fontSize(index == 0 ? 15.0f : 14.0f)
                    .color(index == 0 ? visuals.titleColor : visuals.bodyColor)
                    .build();
            }
        });
    }
};

} // namespace EUINEO
