cmake_minimum_required(VERSION 3.15)
project(TradeSim VERSION 1.0 LANGUAGES CXX)

# Require C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use vcpkg toolchain if available
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

# Find packages installed by vcpkg (or system)
find_package(Threads REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(IXWebSocket CONFIG REQUIRED)
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)

# Include headers
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/external/imgui/backends
    ${PROJECT_SOURCE_DIR}/external/imgui
)

# Collect source files
file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")

# Create executable target
add_executable(tradesim 
    src/main.cpp
    src/logger.cpp
    src/orderbook.cpp
    src/webSocketClient.cpp
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_sdl2.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

# Link libraries
target_link_libraries(tradesim PRIVATE
    nlohmann_json::nlohmann_json
    ixwebsocket::ixwebsocket bcrypt
    Threads::Threads
    SDL2::SDL2
    SDL2::SDL2main
    OpenGL::GL
    imgui::imgui
    # SDL2::SDL2_image
    # SDL2::SDL2_ttf
    $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
)

# Platform-specific stuff
if(WIN32)
    target_link_libraries(tradesim PRIVATE ws2_32 crypt32)
elseif(APPLE)
    # macOS specific flags here if needed
elseif(UNIX)
    # Linux specific libs (e.g. pthread if needed)
endif()
