cmake_minimum_required(VERSION 3.15)
project(cs2_internal_cheat)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SOURCES
    Internal/dllmain.cpp
    Internal/hooks/present.cpp
    Internal/features/esp.cpp
    Internal/features/aimbot.cpp
    Internal/features/triggerbot.cpp
    Internal/features/misc.cpp
    Internal/features/config_system.cpp
)

# ImGui sources
set(IMGUI_SOURCES
    Internal/imgui/imgui.cpp
    Internal/imgui/imgui_draw.cpp
    Internal/imgui/imgui_tables.cpp
    Internal/imgui/imgui_widgets.cpp
    Internal/imgui/backends/imgui_impl_dx11.cpp
    Internal/imgui/backends/imgui_impl_win32.cpp
)

# MinHook sources (Building from source)
set(MINHOOK_SOURCES
    Internal/minhook/src/buffer.c
    Internal/minhook/src/hook.c
    Internal/minhook/src/trampoline.c
    Internal/minhook/src/hde/hde32.c
    Internal/minhook/src/hde/hde64.c
)

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${IMGUI_SOURCES} ${MINHOOK_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    Internal
    Internal/imgui
    Internal/imgui/backends
    Internal/minhook/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    d3d11
    dxgi
    user32
)
