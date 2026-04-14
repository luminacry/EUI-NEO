cmake_minimum_required(VERSION 3.14)
project(EUI-NEO VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(CTest)

# 禁用 HTTPS 证书验证，解决国内或代理环境下下载 Github 源码失败的问题
set(CMAKE_TLS_VERIFY OFF)
set(EUI_GITHUB_ARCHIVE_HOST "https://codeload.github.com" CACHE STRING
    "GitHub archive CDN host used for third-party source downloads")

# 禁用 GLFW 测试、示例和文档以加快构建速度
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "Generate installation target" FORCE)

# 使用 FetchContent 从 GitHub 归档 CDN 下载，避免 github.com/archive 在部分网络环境下长时间卡住。
include(FetchContent)
function(eui_declare_github_archive dependency_name repository archive_ref)
    FetchContent_Declare(
        ${dependency_name}
        URL "${EUI_GITHUB_ARCHIVE_HOST}/${repository}/zip/${archive_ref}"
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        TIMEOUT 30
        TLS_VERIFY OFF
    )
endfunction()

option(EUI_BUILD_RUNTIME "Build the EUI-NEO desktop runtime targets" ON)

find_package(OpenGL QUIET)
find_package(CURL QUIET)

set(EUI_RUNTIME_TARGETS_ENABLED ${EUI_BUILD_RUNTIME})
if(EUI_BUILD_RUNTIME AND NOT OpenGL_FOUND)
    message(STATUS "OpenGL development files not found; skipping EUI desktop runtime targets")
    set(EUI_RUNTIME_TARGETS_ENABLED OFF)
endif()

if(EUI_RUNTIME_TARGETS_ENABLED)
    eui_declare_github_archive(glfw "glfw/glfw" "refs/tags/3.4")

    # 强制 GLFW 构建为动态库 (DLL) 以节省内存
    set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries" FORCE)
    if(UNIX AND NOT APPLE)
        find_program(EUI_WAYLAND_SCANNER_EXECUTABLE wayland-scanner)
        find_package(X11 QUIET)

        if(EUI_WAYLAND_SCANNER_EXECUTABLE)
            set(GLFW_BUILD_WAYLAND ON CACHE BOOL "Build GLFW with Wayland support" FORCE)
        else()
            message(STATUS "wayland-scanner not found; disabling GLFW Wayland backend")
            set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "Disable Wayland when scanner is unavailable" FORCE)
        endif()

        if(X11_FOUND)
            set(GLFW_BUILD_X11 ON CACHE BOOL "Build GLFW with X11 support" FORCE)
        else()
            message(STATUS "X11 development files not found; disabling GLFW X11 backend")
            set(GLFW_BUILD_X11 OFF CACHE BOOL "Disable X11 when development files are unavailable" FORCE)
        endif()

        if(NOT EUI_WAYLAND_SCANNER_EXECUTABLE AND NOT X11_FOUND)
            message(STATUS "No Linux desktop backend dependencies found; GLFW will fall back to the null backend")
        endif()
    endif()

    FetchContent_MakeAvailable(glfw)

    # GLAD 使用静态库编译，因为动态库在 MSVC 下如果不加 dllexport 会不生成 .lib
    # 这也是你之前遇到 LNK1181 无法打开 glad.lib 的原因
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)

    # 获取预编译的 GLAD（用于访问可编程管线和现代 OpenGL 函数）
    eui_declare_github_archive(glad "libigl/libigl-glad" "refs/heads/master")
    FetchContent_MakeAvailable(glad)

    eui_declare_github_archive(stb "nothings/stb" "refs/heads/master")
    FetchContent_MakeAvailable(stb)

    eui_declare_github_archive(nanosvg "memononen/nanosvg" "refs/heads/master")
    FetchContent_MakeAvailable(nanosvg)

    # 包含 src 目录和第三方库目录
    include_directories(src)
    include_directories(third_party)

    # 自动收集 src 下的所有 cpp 文件 (包含所有组件如 InputBox, ComboBox, Panel 等)
    file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "src/*.cpp")
    file(GLOB APP_ENTRY_FILES CONFIGURE_DEPENDS "app/*.cpp")

    add_library(eui_neo_core STATIC ${SRC_FILES})
    target_include_directories(eui_neo_core PUBLIC src third_party ${stb_SOURCE_DIR} ${nanosvg_SOURCE_DIR}/src)
    target_link_libraries(eui_neo_core PUBLIC glfw glad OpenGL::GL)
    if(CURL_FOUND)
        target_link_libraries(eui_neo_core PUBLIC CURL::libcurl)
        target_compile_definitions(eui_neo_core PUBLIC EUI_HAS_CURL=1)
    elseif(WIN32)
        target_link_libraries(eui_neo_core PUBLIC urlmon)
    endif()

    set(EUI_FONT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/font")
    if(EXISTS "${EUI_FONT_SOURCE_DIR}")
        add_custom_target(eui_copy_fonts
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/font"
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${EUI_FONT_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/font"
        )
    endif()

    function(configure_eui_target target_name)
        if(MSVC)
            target_compile_options(${target_name} PRIVATE /GS- /sdl- /utf-8)
            target_link_options(${target_name} PRIVATE /OPT:REF /OPT:ICF /ENTRY:mainCRTStartup /SUBSYSTEM:WINDOWS)
        else()
            target_compile_options(${target_name} PRIVATE -Os -fno-exceptions -fno-rtti)
            target_link_options(${target_name} PRIVATE -Wl,--gc-sections -s)
        endif()

        target_link_libraries(${target_name} PRIVATE eui_neo_core)
        if(TARGET eui_copy_fonts)
            add_dependencies(${target_name} eui_copy_fonts)
        endif()

        if(WIN32)
            add_custom_command(TARGET ${target_name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_RUNTIME_DLLS:${target_name}>
                $<TARGET_FILE_DIR:${target_name}>
                COMMAND_EXPAND_LISTS
            )
        endif()
    endfunction()

    add_executable(eui_neo main.cpp)
    configure_eui_target(eui_neo)

    foreach(app_source IN LISTS APP_ENTRY_FILES)
        get_filename_component(app_name "${app_source}" NAME_WE)
        string(MAKE_C_IDENTIFIER "${app_name}" app_id)
        set(app_target "app_${app_id}")
        add_executable(${app_target} "${app_source}")
        configure_eui_target(${app_target})
    endforeach()
endif()

if(BUILD_TESTING)
    add_executable(eui_neo_ui_update_policy_tests
        tests/UIUpdatePolicyTests.cpp
    )
    target_include_directories(eui_neo_ui_update_policy_tests PRIVATE src)
    add_test(NAME eui_neo_ui_update_policy_tests COMMAND eui_neo_ui_update_policy_tests)
endif()
