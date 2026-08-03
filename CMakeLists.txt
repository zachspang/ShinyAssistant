cmake_minimum_required(VERSION 3.15)
project(ShinyAssistant LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_language(RC)
set(CMAKE_RC_COMPILER_INIT windres)
set(CMAKE_RC_COMPILE_OBJECT
    "<CMAKE_RC_COMPILER> <FLAGS> -O coff -o <OBJECT> <SOURCE>")

# --- wxWidgets (shared MSYS2 install) ---
set(wxWidgets_ROOT_DIR "C:/msys64/mingw64")
find_package(wxWidgets REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})

# --- ViGEmClient (static) ---
add_library(ViGEmClient STATIC
    third_party/ViGEmClient/src/ViGEmClient.cpp
)
target_include_directories(ViGEmClient PUBLIC
    third_party/ViGEmClient/include
)
target_link_libraries(ViGEmClient PRIVATE setupapi)

# --- Main app ---
add_executable(ShinyAssistant WIN32
    src/main.cpp
    app.rc
)

target_link_libraries(ShinyAssistant PRIVATE
    -Wl,--start-group
    ${wxWidgets_LIBRARIES}
    ViGEmClient
    -Wl,--end-group
    xinput9_1_0
)

set_target_properties(ShinyAssistant PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)