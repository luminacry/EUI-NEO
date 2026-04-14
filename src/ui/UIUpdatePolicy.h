#pragma once

namespace EUINEO {

struct UIFrameActivity {
    bool pointerMoved = false;
    bool mouseDown = false;
    bool mouseClicked = false;
    bool mouseReleased = false;
    bool mouseRightDown = false;
    bool mouseRightClicked = false;
    bool mouseRightReleased = false;
    float scrollDeltaX = 0.0f;
    float scrollDeltaY = 0.0f;
    bool hasTextInput = false;
    bool hasKeyPress = false;
};

inline bool HasFrameInputActivity(const UIFrameActivity& activity) {
    if (activity.pointerMoved || activity.mouseClicked || activity.mouseReleased ||
        activity.mouseRightClicked || activity.mouseRightReleased ||
        activity.mouseDown || activity.mouseRightDown) {
        return true;
    }
    if (activity.scrollDeltaX != 0.0f || activity.scrollDeltaY != 0.0f) {
        return true;
    }
    return activity.hasTextInput || activity.hasKeyPress;
}

inline bool IsPointerMoveOnlyActivity(const UIFrameActivity& activity) {
    return activity.pointerMoved &&
           !activity.mouseClicked &&
           !activity.mouseReleased &&
           !activity.mouseRightClicked &&
           !activity.mouseRightReleased &&
           !activity.mouseDown &&
           !activity.mouseRightDown &&
           activity.scrollDeltaX == 0.0f &&
           activity.scrollDeltaY == 0.0f &&
           !activity.hasTextInput &&
           !activity.hasKeyPress;
}

inline bool ShouldReuseComposedTreeForFrame(const UIFrameActivity& activity,
                                            bool hasComposedTree,
                                            bool requiresComposeAnimation,
                                            bool hasRetainedVisualAnimation) {
    if (!hasComposedTree || requiresComposeAnimation) {
        return false;
    }
    return IsPointerMoveOnlyActivity(activity) ||
           (!HasFrameInputActivity(activity) && hasRetainedVisualAnimation);
}

inline bool ShouldUseNodeSurfaceCache(bool usesCachedSurface,
                                      bool hasBlur,
                                      bool wantsContinuousUpdate) {
    return usesCachedSurface && !hasBlur && !wantsContinuousUpdate;
}

} // namespace EUINEO
