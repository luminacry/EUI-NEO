#include "ui/UIUpdatePolicy.h"
#include <cstdlib>
#include <iostream>

namespace {

void Expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << std::endl;
        std::exit(1);
    }
}

} // namespace

int main() {
    using namespace EUINEO;

    UIFrameActivity pointerMoveOnly;
    pointerMoveOnly.pointerMoved = true;
    Expect(HasFrameInputActivity(pointerMoveOnly), "pointer move should count as frame activity");
    Expect(IsPointerMoveOnlyActivity(pointerMoveOnly), "pointer move without other input should be pointer-only");
    Expect(ShouldReuseComposedTreeForFrame(pointerMoveOnly, true, false, false),
           "pointer-only frames should reuse the composed tree");

    UIFrameActivity textInputFrame;
    textInputFrame.hasTextInput = true;
    Expect(!IsPointerMoveOnlyActivity(textInputFrame), "text input is not pointer-only activity");
    Expect(!ShouldReuseComposedTreeForFrame(textInputFrame, true, false, true),
           "text input should force a compose pass");

    UIFrameActivity idleAnimationFrame;
    Expect(!HasFrameInputActivity(idleAnimationFrame), "idle animation frame should have no input activity");
    Expect(ShouldReuseComposedTreeForFrame(idleAnimationFrame, true, false, true),
           "animation-only frames should reuse the composed tree");
    Expect(!ShouldReuseComposedTreeForFrame(idleAnimationFrame, true, false, false),
           "frames without retained visual animation should still recompose");
    Expect(!ShouldReuseComposedTreeForFrame(idleAnimationFrame, true, true, true),
           "external compose animations must keep compose enabled");
    Expect(!ShouldReuseComposedTreeForFrame(idleAnimationFrame, false, false, true),
           "frames without a composed tree cannot be reused");

    Expect(ShouldUseNodeSurfaceCache(true, false, false),
           "static cacheable nodes should keep using cached surfaces");
    Expect(!ShouldUseNodeSurfaceCache(true, true, false),
           "blurred nodes should bypass per-node surface caches");
    Expect(!ShouldUseNodeSurfaceCache(true, false, true),
           "continuously updating nodes should bypass per-node surface caches");
    Expect(!ShouldUseNodeSurfaceCache(false, false, false),
           "nodes that opt out of caching should render directly");

    return 0;
}
