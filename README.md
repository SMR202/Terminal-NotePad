# Notepad CMD - Advanced Text Editor

A feature-rich command-line text editor built in C++ with advanced text manipulation capabilities, word suggestions, and intelligent search functionality.

## Features

### Core Text Editing
- Multi-line text editing with cursor navigation
- Insert and delete operations with real-time display
- Line wrapping at 100 characters
- Arrow key navigation (Up, Down, Left, Right)

### Advanced Functionality
- Undo/Redo operations (Ctrl+Z / Ctrl+Y) with 5-state history
- File operations (Save, Load, Create New)
- Search functionality with text highlighting
- Word suggestions using n-ary tree structure
- Sentence completion using graph-based word relationships

## Data Structures Used
- **2D Linked List:** Core text storage with bidirectional navigation
- **n-ary Tree (Trie):** Word storage and prefix-based suggestions
- **Stack:** Undo/redo state management
- **Graph:** Word relationship mapping for sentence completion

## Controls

### Basic Navigation
- **Arrow Keys:** Navigate cursor
- **Enter:** New line
- **Backspace:** Delete character
- **Space:** Insert space

### File Operations
- **ESC:** Exit application
- **0:** Save and quit
- **1:** Save file
- **2:** Load file (or Shift+2 for word suggestions)
- **3:** Create new file

### Advanced Features
- **4:** Search mode
- **Ctrl+Z:** Undo
- **Ctrl+Y:** Redo
- **Shift+8:** Sentence completion suggestions

## Technical Implementation

### Core Components

#### Node Structure
```cpp
class Node {
    char data;
    Node* up, *down, *left, *right;
    int line;
    bool highlighted;
};
```

#### n-ary Tree for Word Storage
- Stores words in a trie structure
- Provides O(m) search time where m is word length
- Supports prefix-based word suggestions

#### Stack-based Undo/Redo
- Maintains up to 5 previous states
- Clones entire document state for each operation
- Efficient memory management with automatic cleanup

#### Graph-based Sentence Completion
- Tracks word relationships and sequences
- Provides context-aware word suggestions
- Uses visited tracking to prevent infinite loops

## Building and Running

### Prerequisites
- Windows OS (uses Windows.h for console operations)
- C++ compiler (Visual Studio recommended)

### Compilation
```bash
g++ -o notepad assignment2.cpp
```

### Execution
```bash
./notepad
```

## File Structure
- `assignment2.cpp`: Main application file containing all functionality
- `file.txt`: Default save/load file (created automatically)

## Key Algorithms

### Text Insertion
- Handles insertion at beginning, middle, and end of lines
- Maintains bidirectional links in 2D structure
- Automatic word wrapping and line management

### Search with Highlighting
- Case-insensitive search functionality
- Visual highlighting of found terms
- Line number reporting for search results

### Word Suggestion System
- Real-time word completion based on partial input
- Frequency-based suggestion ranking
- Interactive selection interface

### Memory Management
- Automatic cleanup of linked list structures
- Stack-based state management with size limits
- Dynamic memory allocation for word storage
- Proper destructor implementation for all classes

## Interface Layout

```
MENU => Exit: esc   Save and Quit: 0   Save: 1   Load: 2   New File: 3   Search: 4
Welcome to the Notepad.
This is the area where you are supposed to write the content.
|                                                                                                    |
|  [Text editing area - 100x20 characters]                                                           |
|                                                                                                    |
|____________________________________________________________________________________________________|
Word Suggestions: [Suggestion area]
```

## Performance Characteristics
- **Space Complexity:** O(n) where n is total characters
- **Search Time:** O(n) for text search, O(m) for word lookup
- **Insertion/Deletion:** O(1) for most operations
- **Memory Usage:** Optimized with automatic cleanup and size limits

## Future Enhancements
- Syntax highlighting support
- Multiple file tabs
- Advanced find/replace functionality
- Configurable key bindings
- Plugin system for extensions

## License
This project is developed as an educational assignment and is available for learning purposes.
