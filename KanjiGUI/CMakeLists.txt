cmake_minimum_required(VERSION 3.16)

project(KanjiGUI VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MSVC specific configuration
if(MSVC)
    # Use static runtime library for easier deployment
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    
    # MSVC specific compiler flags
    add_compile_options(/W4)  # Warning level 4
    add_compile_options(/MP)  # Multi-processor compilation
    
    message(STATUS "Building KanjiGUI with MSVC compiler")
endif()

# Find Qt6 (adjust path for Windows if needed)
if(WIN32)
    set(CMAKE_PREFIX_PATH "C:/Qt/6.5.0/msvc2022_64")  # Adjust to your Qt installation
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Enable automatic MOC processing
set(CMAKE_AUTOMOC ON)

# Create GUI executable
if(WIN32)
    add_executable(KanjiGUI WIN32  # WIN32 for no console window on Windows
        kanji_main.cpp
        kanji_main_window.cpp
        kanji_main_window.h
        kanji_learning_window.cpp
        kanji_learning_window.h
    )
else()
    add_executable(KanjiGUI
        kanji_main.cpp
        kanji_main_window.cpp
        kanji_main_window.h
        kanji_learning_window.cpp
        kanji_learning_window.h
    )
endif()

# Include KanjiCore headers
target_include_directories(KanjiGUI PRIVATE ${CMAKE_SOURCE_DIR}/KanjiCore)

# Link libraries - KanjiCore is available as target from parent CMakeLists
target_link_libraries(KanjiGUI 
    Qt6::Core 
    Qt6::Widgets
    KanjiCore
)

# Set output directory
set_target_properties(KanjiGUI PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Copy KanjiCore DLL to output directory on Windows
if(WIN32)
    add_custom_command(TARGET KanjiGUI POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:KanjiCore>
        $<TARGET_FILE_DIR:KanjiGUI>
    )
endif()

# Install configuration
include(GNUInstallDirs)

install(TARGETS KanjiGUI
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

message(STATUS "KanjiGUI application configured")
message(STATUS "Dependencies: Qt6 + KanjiCore library")
message(STATUS "Output: ${CMAKE_BINARY_DIR}/bin/KanjiGUI.exe (Windows) or KanjiGUI (Linux)") 