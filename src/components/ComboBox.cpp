#include "ComboBox.h"
#include <cmath>

namespace EUINEO {

ComboBox::ComboBox(std::string placeholderText, float x, float y, float w, float h)
    : placeholder(placeholderText) {
    this->x = x;
    this->y = y;
    this->width = w;
    this->height = h;
}

void ComboBox::AddItem(const std::string& item) {
    items.push_back(item);
    itemHoverAnims.push_back(0.0f);
}

static float VisibleListHeight(float itemHeight, std::size_t itemCount, float openFactor) {
    return static_cast<float>(itemCount) * itemHeight * std::clamp(openFactor, 0.0f, 1.0f);
}

static float ListOverlap(float visibleListHeight) {
    return visibleListHeight > 6.0f ? 1.0f : 0.0f;
}

static void MarkComboDirty(Widget& widget, float width, float height, std::size_t itemCount,
                           float fromOpenFactor, float toOpenFactor) {
    float absX = 0.0f;
    float absY = 0.0f;
    widget.GetAbsoluteBounds(absX, absY);

    float visibleListHeight = std::max(
        VisibleListHeight(height, itemCount, fromOpenFactor),
        VisibleListHeight(height, itemCount, toOpenFactor)
    );
    float overlap = ListOverlap(visibleListHeight);
    float padding = visibleListHeight > 0.0f ? 12.0f : 0.0f;
    float totalHeight = height + visibleListHeight + overlap + padding;

    Renderer::AddDirtyRect(absX - 6.0f, absY - 6.0f, width + 12.0f, totalHeight + 12.0f);
    Renderer::RequestRepaint();
}

void ComboBox::Update() {
    float absX = 0.0f;
    float absY = 0.0f;
    GetAbsoluteBounds(absX, absY);

    bool hoveredMain = IsHovered();
    float previousOpenAnim = openAnim;

    float targetHover = hoveredMain ? 1.0f : 0.0f;
    if (hoverAnim != targetHover) {
        hoverAnim = Lerp(hoverAnim, targetHover, State.deltaTime * 15.0f);
        if (std::abs(hoverAnim - targetHover) < 0.01f) hoverAnim = targetHover;
        MarkComboDirty(*this, width, height, items.size(), previousOpenAnim, openAnim);
    }

    float targetOpen = isOpen ? 1.0f : 0.0f;
    if (openAnim != targetOpen) {
        float fromOpenAnim = openAnim;
        openAnim = Lerp(openAnim, targetOpen, State.deltaTime * 20.0f);
        if (std::abs(openAnim - targetOpen) < 0.01f) openAnim = targetOpen;
        MarkComboDirty(*this, width, height, items.size(), fromOpenAnim, openAnim);
    }

    if (isOpen || openAnim > 0.0f) {
        float listY = absY + height;
        for (size_t i = 0; i < items.size(); ++i) {
            float itemY = listY + i * height;
            bool itemHovered = State.mouseX >= absX && State.mouseX <= absX + width &&
                               State.mouseY >= itemY && State.mouseY <= itemY + height;
            float targetItemHover = (itemHovered && isOpen) ? 1.0f : 0.0f;
            if (itemHoverAnims[i] != targetItemHover) {
                itemHoverAnims[i] = Lerp(itemHoverAnims[i], targetItemHover, State.deltaTime * 15.0f);
                if (std::abs(itemHoverAnims[i] - targetItemHover) < 0.01f) itemHoverAnims[i] = targetItemHover;
                MarkComboDirty(*this, width, height, items.size(), openAnim, openAnim);
            }
        }
    }

    if (State.mouseClicked) {
        if (isOpen) {
            float listY = absY + height;
            float listH = items.size() * height;
            bool hoveredList = State.mouseX >= absX && State.mouseX <= absX + width &&
                               State.mouseY >= listY && State.mouseY <= listY + listH;

            if (hoveredList) {
                int index = (int)((State.mouseY - listY) / height);
                if (index >= 0 && index < (int)items.size()) {
                    selectedIndex = index;
                    if (onSelectionChanged) onSelectionChanged(selectedIndex);
                }
            }
            float fromOpenAnim = openAnim;
            isOpen = false;
            MarkComboDirty(*this, width, height, items.size(), fromOpenAnim, 0.0f);
        } else if (hoveredMain) {
            float fromOpenAnim = openAnim;
            isOpen = true;
            MarkComboDirty(*this, width, height, items.size(), fromOpenAnim, 1.0f);
        }
    }
}

void ComboBox::Draw() {
    float absX = 0.0f;
    float absY = 0.0f;
    GetAbsoluteBounds(absX, absY);
    const float textScale = fontSize / 24.0f;
    const float textY = absY + height / 2.0f + (fontSize / 4.0f);
    const float textX = absX + 10.0f;
    const float visibleOpen = std::clamp(openAnim, 0.0f, 1.0f);
    const float visibleListHeight = VisibleListHeight(height, items.size(), visibleOpen);
    if (visibleListHeight > 1.0f) {
        const float overlap = ListOverlap(visibleListHeight);
        float listY = absY + height - overlap;
        float listH = visibleListHeight + overlap;
        Renderer::DrawRect(absX, listY, width, listH, CurrentTheme->surface, 6.0f);

        for (size_t i = 0; i < items.size(); ++i) {
            float itemY = absY + height + i * height;
            float currentListBottomY = listY + listH;
            float visibleHeight = std::clamp(currentListBottomY - itemY, 0.0f, height);

            if (itemY > currentListBottomY) break;
            if (visibleHeight <= 0.0f) {
                break;
            }

            float itemAlpha = visibleHeight / height;
            Color itemBgColor = Lerp(CurrentTheme->surface, CurrentTheme->surfaceHover, itemHoverAnims[i]);
            if (itemHoverAnims[i] > 0.01f) {
                itemBgColor.a *= visibleOpen;
                Renderer::DrawRect(absX + 1.0f, itemY, width - 2.0f, visibleHeight,
                                   itemBgColor, visibleHeight < height ? 0.0f : 4.0f);
            }

            Color itemColor = (i == (size_t)selectedIndex) ? CurrentTheme->primary : CurrentTheme->text;
            itemColor.a *= (visibleOpen * itemAlpha);
            if (visibleHeight >= fontSize * 0.90f) {
                Renderer::DrawTextStr(items[i], textX, itemY + height / 2.0f + (fontSize / 4.0f), itemColor, textScale);
            }
        }
    }

    Color baseColor = Lerp(CurrentTheme->surface, CurrentTheme->surfaceActive, openAnim);
    Color hoverColor = Lerp(CurrentTheme->surfaceHover, CurrentTheme->surfaceActive, openAnim);
    Color bgColor = Lerp(baseColor, hoverColor, hoverAnim);

    Renderer::DrawRect(absX, absY, width, height, bgColor, 6.0f);

    std::string displayText = (selectedIndex >= 0 && selectedIndex < (int)items.size()) ? items[selectedIndex] : placeholder;
    Color textColor = (selectedIndex >= 0)
        ? CurrentTheme->text
        : Color(CurrentTheme->text.r, CurrentTheme->text.g, CurrentTheme->text.b, 0.5f);
    Renderer::DrawTextStr(displayText, textX, textY, textColor, textScale);
    Renderer::DrawTextStr(isOpen ? "\xEF\x84\x86" : "\xEF\x84\x87", absX + width - 25.0f, textY, CurrentTheme->text, textScale);
}

} // namespace EUINEO
