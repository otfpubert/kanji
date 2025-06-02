# Kanji Learning System - Split Project Version

A Japanese kanji learning application with spaced repetition system (SRS), split into two dynamically linked projects.

## Project Structure

```
KanjiLearningSystem/
├── CMakeLists.txt           # Root build configuration
├── KanjiCore/               # Core Library (DLL/Shared Library)
│   ├── CMakeLists.txt
│   ├── kanji_database.h     # Database interface with DLL exports
│   ├── kanji_database.cpp   # Database implementation
│   └── KanjiCoreConfig.cmake.in
└── KanjiGUI/                # GUI Application
    ├── CMakeLists.txt
    ├── kanji_main.cpp       # Application entry point
    ├── kanji_main_window.*  # Main window implementation
    ├── kanji_learning_window.* # Learning/review interface
    └── assets/              # UI resources
```

## Academic Requirements Compliance

### ✅ **MSVC Compiler Support**
- Full MSVC compatibility with `/W4` warnings and `/MP` multi-processor compilation
- Windows-specific configurations for DLL handling

### ✅ **GUI Application (Qt6)**
- Modern Qt6-based graphical user interface
- Interactive kanji learning and review system
- Real-time progress tracking

### ✅ **Object-Oriented Design**
- `KanjiDatabase` class for data management
- `KanjiMainWindow` and `KanjiLearningWindow` classes for UI
- Proper encapsulation and inheritance

### ✅ **File Handling**
- SQLite database for persistent kanji data storage
- Configuration files and assets management
- Automatic database creation and population

### ✅ **Exception Handling**
- Try-catch blocks in critical database operations
- Proper error propagation and user feedback
- Safe resource management

### ✅ **Git Version Control**
- Full Git history maintained
- Proper branching and commit structure

### ✅ **Dynamic Linking (DLL)**
- **KanjiCore**: Shared library (.dll on Windows, .so on Linux)
- **KanjiGUI**: Executable that dynamically links to KanjiCore
- Proper DLL export/import macros (`KANJICORE_API`)

## Building the Project

### Prerequisites
- CMake 3.16+
- Qt6 (Core, Widgets, Sql)
- MSVC 2019+ (Windows) or GCC/Clang (Linux)

### Build Steps

1. **Configure and build both projects:**
   ```bash
   mkdir build
   cd build
   cmake ..
   cmake --build . --config Release
   ```

2. **On Windows with MSVC:**
   ```cmd
   mkdir build
   cd build
   cmake .. -G "Visual Studio 16 2019" -A x64
   cmake --build . --config Release
   ```

### Output Files
- `build/lib/KanjiCore.dll` (Windows) or `build/lib/libKanjiCore.so` (Linux)
- `build/bin/KanjiGUI.exe` (Windows) or `build/bin/KanjiGUI` (Linux)

## Features

- **Learning Mode**: Study new kanji with meanings and readings
- **Review Mode**: SRS-based spaced repetition for learned kanji
- **Real-time Statistics**: Track progress and review schedule
- **Romaji Input**: Automatic conversion to hiragana for reading questions
- **Multiple Meanings**: Support for kanji with alternative meanings

## Dynamic Linking Architecture

### KanjiCore Library
- Contains all database functionality and business logic
- Exports classes and functions via `KANJICORE_API` macros
- Independent of GUI framework
- Can be used by multiple applications

### KanjiGUI Application
- Implements user interface using Qt6
- Links dynamically to KanjiCore at runtime
- Handles user interactions and visual feedback
- Platform-specific executable

## Development Notes

- The project demonstrates proper separation of concerns
- Core logic is isolated from UI implementation
- Easy to extend with additional GUI frameworks
- Follows modern C++ and CMake best practices

## License

Educational project demonstrating advanced C++ concepts and dynamic linking. 