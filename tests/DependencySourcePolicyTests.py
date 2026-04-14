from pathlib import Path
import sys


def main() -> int:
    cmake_text = Path("/root/luminacry-EUI-NEO/CMakeLists.txt").read_text(encoding="utf-8")

    if "https://github.com/glfw/glfw/archive/refs/tags/3.4.zip" in cmake_text:
        print("GLFW still downloads from github.com/archive instead of codeload.", file=sys.stderr)
        return 1

    if "https://codeload.github.com" not in cmake_text:
        print("Expected codeload.github.com dependency host in CMakeLists.txt.", file=sys.stderr)
        return 1

    if "function(eui_declare_github_archive" not in cmake_text:
        print("Expected helper function for normalized GitHub archive declarations.", file=sys.stderr)
        return 1

    if 'find_program(EUI_WAYLAND_SCANNER_EXECUTABLE wayland-scanner)' not in cmake_text:
        print("Expected explicit wayland-scanner detection for GLFW Linux builds.", file=sys.stderr)
        return 1

    if 'set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "Disable Wayland when scanner is unavailable" FORCE)' not in cmake_text:
        print("Expected GLFW Wayland fallback policy in CMakeLists.txt.", file=sys.stderr)
        return 1

    if 'find_package(X11 QUIET)' not in cmake_text:
        print("Expected quiet X11 detection for GLFW Linux builds.", file=sys.stderr)
        return 1

    if 'set(GLFW_BUILD_X11 OFF CACHE BOOL "Disable X11 when development files are unavailable" FORCE)' not in cmake_text:
        print("Expected GLFW X11 fallback policy in CMakeLists.txt.", file=sys.stderr)
        return 1

    if 'find_package(OpenGL QUIET)' not in cmake_text:
        print("Expected quiet OpenGL detection for optional desktop runtime targets.", file=sys.stderr)
        return 1

    if 'message(STATUS "OpenGL development files not found; skipping EUI desktop runtime targets")' not in cmake_text:
        print("Expected OpenGL runtime fallback message in CMakeLists.txt.", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
