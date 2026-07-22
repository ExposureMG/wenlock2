cmake_minimum_required(VERSION 3.22)
project(wenlock VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# Force static linking
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libraries" FORCE)
set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(DPP_BUILD_TEST      OFF CACHE BOOL "" FORCE)
set(DPP_INSTALL         OFF CACHE BOOL "" FORCE)
set(BUILD_VOICE_SUPPORT OFF CACHE BOOL "" FORCE)
set(ZLIB_ROOT_DIR ${zlib_SOURCE_DIR} CACHE PATH "")
set(LIBXML2_WITH_ICONV OFF CACHE BOOL "")
set(LIBXML2_WITH_LZMA OFF CACHE BOOL "")
set(LIBXML2_WITH_PYTHON OFF CACHE BOOL "")


# Dependencies
FetchContent_Declare(
    dpp
    GIT_REPOSITORY https://github.com/brainboxdotcc/DPP.git
    GIT_TAG        master
)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.17.0
)

FetchContent_Declare(
    SQLiteCpp
    GIT_REPOSITORY https://github.com/SRombauts/SQLiteCpp.git
    GIT_TAG        master
)

FetchContent_Declare(
    zlib
    GIT_REPOSITORY https://github.com/madler/zlib.git
    GIT_TAG        v1.3.1
    CMAKE_ARGS -DBUILD_SHARED_LIBS=OFF
)

FetchContent_Declare(
    libxml2
    GIT_REPOSITORY https://gitlab.gnome.org/GNOME/libxml2.git
    GIT_TAG        v2.12.6
    CMAKE_ARGS
        -DBUILD_SHARED_LIBS=OFF
        -DLIBXML2_WITH_PYTHON=OFF
        -DLIBXML2_WITH_LZMA=OFF
        -DLIBXML2_WITH_ICONV=OFF
        -DLIBXML2_WITH_ZLIB=ON
        -DZLIB_ROOT=${zlib_SOURCE_DIR}
)

FetchContent_Declare(
    openssl
    GIT_REPOSITORY https://github.com/openssl/openssl.git
    GIT_TAG        openssl-3.2.1
    CMAKE_ARGS
        -DBUILD_SHARED_LIBS=OFF
        -Dno-zlib=ON
)

FetchContent_Declare(
    curl
    GIT_REPOSITORY https://github.com/curl/curl.git
    GIT_TAG        curl-8_5_0
    CMAKE_ARGS
        -DBUILD_SHARED_LIBS=OFF
        -DCURL_USE_OPENSSL=ON
        -DCURL_USE_LIBSSH2=OFF
        -DOPENSSL_ROOT_DIR=${openssl_SOURCE_DIR}
        -DZLIB_ROOT=${zlib_SOURCE_DIR}
)

# Order matters: build dependencies first
FetchContent_MakeAvailable(zlib openssl)
FetchContent_MakeAvailable(libxml2 curl)
FetchContent_MakeAvailable(dpp spdlog SQLiteCpp)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/handler.cpp
    src/log.cpp
    src/integrations/phonedb.cpp
    src/integrations/scraper.cpp
    src/integrations/database.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    dpp
    spdlog::spdlog
    SQLiteCpp
    sqlite3
    pthread
    dl
    zlibstatic
    LibXml2::LibXml2
    libcurl
    ssl
    crypto
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${zlib_SOURCE_DIR}
    ${openssl_SOURCE_DIR}
    ${libxml2_SOURCE_DIR}/include
    ${curl_SOURCE_DIR}/include
)

if(WIN32)
    file(GLOB DPP_WIN32_DLLS "${dpp_SOURCE_DIR}/win32/bin/*.dll")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${DPP_WIN32_DLLS}
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND_EXPAND_LISTS
    )
else()
    set_target_properties(${PROJECT_NAME} PROPERTIES
        BUILD_RPATH "$<TARGET_FILE_DIR:dpp>"
    )
endif()