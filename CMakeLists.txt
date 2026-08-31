cmake_minimum_required(VERSION 3.15)
project(ShinyAssistant LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_language(RC)
set(CMAKE_RC_COMPILER_INIT windres)
set(CMAKE_RC_COMPILE_OBJECT
    "<CMAKE_RC_COMPILER> <FLAGS> -O coff -o <OBJECT> <SOURCE>")

# --- wxWidgets (custom slim build from source — webrequest/curl disabled, sockets kept) ---
set(wxWidgets_DIR "C:/wx-slim/install/lib/cmake/wxWidgets-3.2" CACHE PATH "" FORCE)
find_package(wxWidgets CONFIG REQUIRED COMPONENTS core base net)

# --- OpenCV (slim custom build — core/imgproc/videoio only) ---
set(OpenCV_DIR "C:/opencv-slim/install/x64/mingw/lib")
find_package(OpenCV REQUIRED COMPONENTS core imgproc videoio)

# --- CURL (statically linked, Schannel TLS, custom slim build from source) ---
set(CURL_DIR "C:/curl-static/install/lib/cmake/CURL" CACHE PATH "" FORCE)
find_package(CURL REQUIRED)

# --- nlohmann JSON (shared MSYS2 install) --
find_package(nlohmann_json REQUIRED)

# --- ViGEmClient (static) ---
add_library(ViGEmClient STATIC
    third_party/ViGEmClient/src/ViGEmClient.cpp
)
target_include_directories(ViGEmClient PUBLIC
    third_party/ViGEmClient/include
)
target_link_libraries(ViGEmClient PRIVATE setupapi)

# --- Main app (auto-glob sources under src/) ---
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

add_executable(ShinyAssistant WIN32
    ${APP_SOURCES}
    app.rc
)

# include from anywhere under src/
target_include_directories(ShinyAssistant PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${OpenCV_INCLUDE_DIRS}
)

# Tell curl's headers we're linking it statically, or we can get
# unresolved __imp_ symbol errors at link time.
target_compile_definitions(ShinyAssistant PRIVATE CURL_STATICLIB)

target_link_libraries(ShinyAssistant PRIVATE
    -Wl,--start-group
    wx::core
    wx::base
    wx::net
    ${OpenCV_LIBS}
    ViGEmClient
    CURL::libcurl
    nlohmann_json::nlohmann_json
    -Wl,--end-group
    crypt32
    secur32
    ws2_32
    bcrypt
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_link_options(ShinyAssistant PRIVATE
        -s
        -static-libgcc
        -static-libstdc++
        -Wl,-Bstatic -lwinpthread -Wl,-Bdynamic
    )
endif()

set_target_properties(ShinyAssistant PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)