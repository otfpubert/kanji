cmake_minimum_required(VERSION 3.16)

project(KanjiCore VERSION 1.0.0)

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
    
    message(STATUS "Building KanjiCore with MSVC compiler")
endif()

# Find Qt6 (adjust path for Windows if needed)
if(WIN32)
    set(CMAKE_PREFIX_PATH "C:/Qt/6.5.0/msvc2022_64")  # Adjust to your Qt installation
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Sql)

# Enable automatic MOC processing
set(CMAKE_AUTOMOC ON)

# Create shared library
add_library(KanjiCore SHARED
    kanji_database.cpp
    kanji_database.h
    japanese_text_utils.cpp
    japanese_text_utils.h
)

# Set library properties
set_target_properties(KanjiCore PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    OUTPUT_NAME "KanjiCore"
)

# Define export symbols for Windows DLL
target_compile_definitions(KanjiCore PRIVATE KANJICORE_EXPORTS)

# Include directories
target_include_directories(KanjiCore PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# Link Qt libraries
target_link_libraries(KanjiCore 
    Qt6::Core 
    Qt6::Sql
)

# Set output directories
set_target_properties(KanjiCore PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# Install configuration
include(GNUInstallDirs)

install(TARGETS KanjiCore
    EXPORT KanjiCoreTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES kanji_database.h japanese_text_utils.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT KanjiCoreTargets
    FILE KanjiCoreTargets.cmake
    NAMESPACE KanjiCore::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KanjiCore
)

# Create config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    KanjiCoreConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/KanjiCoreConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/KanjiCoreConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KanjiCore
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/KanjiCoreConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/KanjiCoreConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KanjiCore
)

message(STATUS "KanjiCore library configured")
message(STATUS "Output: ${CMAKE_BINARY_DIR}/lib/KanjiCore.dll (Windows) or libKanjiCore.so (Linux)") 