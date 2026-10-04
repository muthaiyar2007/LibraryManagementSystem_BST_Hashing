# Library Management System using BST and Hashing

## Technology
- C++
- HTML
- CSS
- JavaScript
- Windows Winsock HTTP server
- File-based storage (`books.txt`)

## Data Structures
1. Binary Search Tree (BST)
   - Insert
   - Delete
   - Search
   - Inorder traversal
2. Custom Hash Table
   - Separate chaining
   - Book ID lookup
   - Average O(1) search

## Run in VS Code

### 1. Requirements
Install a C++ compiler such as MinGW-w64 / MSYS2 and make sure `g++` works in the VS Code terminal.

### 2. Compile
Open terminal inside this project folder:

g++ server.cpp -o library_server.exe -lws2_32

### 3. Run
library_server.exe

### 4. Open
http://localhost:8080

The server automatically creates/updates `books.txt` in the project folder.

## Demo flow
1. Add 4-5 books.
2. Click "Show All (BST Inorder)" and explain that IDs appear sorted.
3. Search a Book ID and explain that the search uses the custom Hash Table.
4. Issue a book with a Member ID.
5. Return it.
6. Delete a book.
7. Refresh the page and show that data persists.

## Project explanation
Frontend sends HTTP requests to the C++ backend. The backend stores each book in both the BST and Hash Table. The BST is used for sorted display and structural operations. The Hash Table provides fast Book ID lookup. Data is persisted in books.txt.
