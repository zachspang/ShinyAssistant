cmake_minimum_required(VERSION 3.15)
project(wxApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_language(RC)
set(CMAKE_RC_COMPILER_INIT windres)
set(CMAKE_RC_COMPILE_OBJECT
    "<CMAKE_RC_COMPILER> <FLAGS> -O coff -o <OBJECT> <SOURCE>")

set(wxWidgets_ROOT_DIR "C:/msys64/mingw64")
find_package(wxWidgets REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})

add_executable(ShinyAssistant WIN32
    src/main.cpp
    app.rc
)

target_link_libraries(ShinyAssistant PRIVATE ${wxWidgets_LIBRARIES})

set_target_properties(ShinyAssistant PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)