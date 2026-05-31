cmake_minimum_required(VERSION 3.10)
project(SteamReader)

set(CMAKE_CXX_STANDARD 17)

# Include nlohmann JSON library
include(FetchContent)
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
)
FetchContent_MakeAvailable(json)

# Console Application
add_executable(steam_reader_console steam_reader_console.cpp)
target_link_libraries(steam_reader_console PRIVATE wininet nlohmann_json::nlohmann_json)

# GUI Application
add_executable(steam_reader_gui WIN32 steam_reader_gui.cpp)
target_link_libraries(steam_reader_gui PRIVATE wininet comctl32 nlohmann_json::nlohmann_json)