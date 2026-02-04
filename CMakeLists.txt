cmake_minimum_required(VERSION 3.20)
project(img2spec VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Multimedia)

# Remove OpenGL-related frameworks on macOS to avoid AGL issue
if(APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-framework,CoreFoundation")
    # Filter out OpenGL and AGL from link libraries
    list(REMOVE_ITEM Qt6Gui_FRAMEWORKS "OpenGL" "AGL")
endif()

# FetchContent for dependencies
include(FetchContent)

# kissfft
FetchContent_Declare(
    kissfft
    GIT_REPOSITORY https://github.com/mborgerding/kissfft.git
    GIT_TAG 131.1.0
)
set(KISSFFT_PKGCONFIG OFF CACHE BOOL "" FORCE)
set(KISSFFT_STATIC ON CACHE BOOL "" FORCE)
set(KISSFFT_TEST OFF CACHE BOOL "" FORCE)
set(KISSFFT_TOOLS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(kissfft)

# libsndfile - Always use FetchContent for simplicity
message(STATUS "Fetching libsndfile from source")
FetchContent_Declare(
    libsndfile
    GIT_REPOSITORY https://github.com/libsndfile/libsndfile.git
    GIT_TAG 1.2.2
)
set(BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(ENABLE_CPACK OFF CACHE BOOL "" FORCE)
set(ENABLE_PACKAGE_CONFIG OFF CACHE BOOL "" FORCE)
set(INSTALL_PKGCONFIG_MODULE OFF CACHE BOOL "" FORCE)
set(BUILD_REGTEST OFF CACHE BOOL "" FORCE)
set(ENABLE_EXTERNAL_LIBS OFF CACHE BOOL "" FORCE)
set(ENABLE_MPEG OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(libsndfile)
set(SNDFILE_LINK_LIBRARIES sndfile)
set(SNDFILE_INCLUDE_DIRS ${libsndfile_SOURCE_DIR}/include)

# stb_image (header-only)
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
)
FetchContent_MakeAvailable(stb)

# Core library
add_library(img2spec_core STATIC
    core/ImageLoader.cpp
    core/ImageLoader.h
    core/SpectrogramBuilder.cpp
    core/SpectrogramBuilder.h
    core/Stft.cpp
    core/Stft.h
    core/GriffinLim.cpp
    core/GriffinLim.h
    core/Leveling.cpp
    core/Leveling.h
    core/WavWriter.cpp
    core/WavWriter.h
)

target_include_directories(img2spec_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${stb_SOURCE_DIR}
    ${SNDFILE_INCLUDE_DIRS}
)

target_link_libraries(img2spec_core PUBLIC
    Qt6::Core
    kissfft::kissfft
    ${SNDFILE_LINK_LIBRARIES}
)

# Application
add_executable(img2spec
    app/main.cpp
    app/MainWindow.cpp
    app/MainWindow.h
    app/ImagePreviewWidget.cpp
    app/ImagePreviewWidget.h
)

# Workaround for AGL framework issue on macOS
# AGL is deprecated and not in SDK, but Qt6 still references it
# Create dummy AGL framework and add to search path
if(APPLE)
    # Create dummy AGL framework if it doesn't exist
    set(DUMMY_AGL_DIR "${CMAKE_CURRENT_BINARY_DIR}/AGL.framework")
    if(NOT EXISTS "${DUMMY_AGL_DIR}/Versions/A/AGL")
        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/empty_agl.c" "void _dummy_agl(void) {}\n")
        file(MAKE_DIRECTORY "${DUMMY_AGL_DIR}/Versions/A")
        execute_process(
            COMMAND cc -dynamiclib -o "${DUMMY_AGL_DIR}/Versions/A/AGL" "${CMAKE_CURRENT_BINARY_DIR}/empty_agl.c"
            RESULT_VARIABLE AGL_BUILD_RESULT
        )
        if(AGL_BUILD_RESULT EQUAL 0)
            execute_process(COMMAND ln -sf Versions/A/AGL "${DUMMY_AGL_DIR}/AGL")
            execute_process(COMMAND ln -sf A "${DUMMY_AGL_DIR}/Versions/Current")
            message(STATUS "Created dummy AGL framework at ${DUMMY_AGL_DIR}")
        endif()
    endif()

    target_link_options(img2spec PRIVATE
        "-F${CMAKE_CURRENT_BINARY_DIR}"
        "-F/System/Library/Frameworks"
        "-Wl,-rpath,@executable_path/../Frameworks"
    )

    # Install dummy AGL framework into app bundle
    install(DIRECTORY "${DUMMY_AGL_DIR}"
        DESTINATION "$<TARGET_BUNDLE_DIR:img2spec>/Contents/Frameworks"
        OPTIONAL
    )

    # For development, create symlink
    add_custom_command(TARGET img2spec POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_BUNDLE_DIR:img2spec>/Contents/Frameworks"
        COMMAND ${CMAKE_COMMAND} -E rm -rf "$<TARGET_BUNDLE_DIR:img2spec>/Contents/Frameworks/AGL.framework"
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        "${DUMMY_AGL_DIR}"
        "$<TARGET_BUNDLE_DIR:img2spec>/Contents/Frameworks/AGL.framework"
        # Fix AGL install_name to use rpath
        COMMAND install_name_tool -change "AGL" "@rpath/AGL.framework/Versions/A/AGL"
        "$<TARGET_FILE:img2spec>"
        COMMENT "Linking dummy AGL framework to app bundle and fixing install_name"
        VERBATIM
    )
endif()

target_link_libraries(img2spec PRIVATE
    img2spec_core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Multimedia
)

# Platform-specific settings
if(WIN32)
    set_target_properties(img2spec PROPERTIES
        WIN32_EXECUTABLE ON
    )
endif()

if(APPLE)
    set_target_properties(img2spec PROPERTIES
        MACOSX_BUNDLE ON
    )
endif()
