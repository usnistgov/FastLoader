cmake_minimum_required(VERSION 3.23)
project(FastLoader VERSION 4.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent) # Used to download GTEST

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules")
find_package(Hedgehog REQUIRED)

option(TEST_FAST_LOADER "Downloads google unit test API and runs google test scripts to test FastLoader core and api" OFF)
option(BUILD_MAIN "Compiles main function for testing changes to API" OFF)

# Create version.h file in api folder
configure_file(inputs_cmake/version.h.in "${PROJECT_SOURCE_DIR}/fast_loader/version.h")

# Gtest
if (TEST_FAST_LOADER)
    message("Fetch Gtest")
    if (POLICY CMP0135)
        FetchContent_Declare(
                googletest
                URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
                DOWNLOAD_EXTRACT_TIMESTAMP true
        )
    else ()
        FetchContent_Declare(
                googletest
                URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
        )
    endif ()

    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    file(GLOB_RECURSE test_fast_loader_header tests/*.h)
    add_executable(gtest_fast_loader ${test_fast_loader_header} tests/test_fast_loader.cc)

    if (MSVC)
        target_compile_options(gtest_fast_loader PRIVATE /bigobj)
    endif (MSVC)

    target_link_libraries(gtest_fast_loader gtest_main)

    include(GoogleTest)
    gtest_discover_tests(gtest_fast_loader)

endif (TEST_FAST_LOADER)

if (BUILD_MAIN)
    if (MSVC)
        set(CMAKE_CXX_FLAGS "-Wall")
    else (MSVC)
        set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wconversion -Wshadow -Werror -pedantic -pipe -v")
    endif (MSVC)

    file(GLOB_RECURSE fast_loader_header fast_loader/*.h)

    add_executable(FastLoaderMain main.cpp ${fast_loader_header})
endif (BUILD_MAIN)

# Add a dummy target to make the export work
add_library(FastLoader INTERFACE)
target_include_directories(FastLoader INTERFACE $<INSTALL_INTERFACE:include>)

# Install the target
install(TARGETS FastLoader EXPORT FastLoaderTargets)

configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/FastLoaderConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/FastLoaderConfig.cmake"
        @ONLY
)

install(
        FILES
        "${CMAKE_CURRENT_BINARY_DIR}/FastLoaderConfig.cmake"
        DESTINATION
        "${CMAKE_INSTALL_PREFIX}/share/FastLoader"
)

install(
        EXPORT
        FastLoaderTargets
        NAMESPACE
        FastLoader::
        DESTINATION
        "${CMAKE_INSTALL_PREFIX}/share/FastLoader"
        FILE FastLoaderTargets.cmake
)


install(DIRECTORY fast_loader
        DESTINATION include
        COMPONENT headers)
