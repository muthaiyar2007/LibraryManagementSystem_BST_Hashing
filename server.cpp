cpp
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <algorithm>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

using namespace std;

// ============================================================
// BOOK STRUCTURE
// ============================================================

struct Book {
    int id;
    string title;
    string author;
    string category;
    bool issued;
    string memberId;
};

// ============================================================
// BST
// ============================================================

struct BSTNode {
    Book book;
    BSTNode* left;
    BSTNode* right;

    BSTNode(Book b) {
        book = b;
        left = nullptr;
        right = nullptr;
    }
};

class BST {
private:
    BSTNode* root = nullptr;

    BSTNode* insertNode(BSTNode* node, Book book) {
        if (!node)
            return new BSTNode(book);

        if (book.id < node->book.id)
            node->left = insertNode(node->left, book);
        else if (book.id > node->book.id)
            node->right = insertNode(node->right, book);

        return node;
    }

    BSTNode* searchNode(BSTNode* node, int id) {
        if (!node || node->book.id == id)
            return node;

        if (id < node->book.id)
            return searchNode(node->left, id);

        return searchNode(node->right, id);
    }

    void inorder(BSTNode* node, vector<Book>& books) {
        if (!node)
            return;

        inorder(node->left, books);
        books.push_back(node->book);
        inorder(node->right, books);
    }

    BSTNode* minNode(BSTNode* node) {
        BSTNode* current = node;

        while (current && current->left)
            current = current->left;

        return current;
    }

    BSTNode* deleteNode(BSTNode* node, int id) {
        if (!node)
            return nullptr;

        if (id < node->book.id) {
            node->left = deleteNode(node->left, id);
        }
        else if (id > node->book.id) {
            node->right = deleteNode(node->right, id);
        }
        else {

            if (!node->left) {
                BSTNode* temp = node->right;
                delete node;
                return temp;
            }

            if (!node->right) {
                BSTNode* temp = node->left;
                delete node;
                return temp;
            }

            BSTNode* temp = minNode(node->right);

            node->book = temp->book;

            node->right =
                deleteNode(node->right, temp->book.id);
        }

        return node;
    }

public:

    void insert(Book book) {
        root = insertNode(root, book);
    }

    Book* search(int id) {
        BSTNode* result = searchNode(root, id);

        if (result)
            return &result->book;

        return nullptr;
    }

    void remove(int id) {
        root = deleteNode(root, id);
    }

    vector<Book> getAll() {
        vector<Book> books;
        inorder(root, books);
        return books;
    }
};

// ============================================================
// HASH TABLE
// ============================================================

class HashTable {

private:

    static const int SIZE = 101;

    struct Entry {
        int id;
        Book book;
        Entry* next;

        Entry(int i, Book b) {
            id = i;
            book = b;
            next = nullptr;
        }
    };

    Entry* table[SIZE]{};

    int hashFunction(int id) {
        int index = id % SIZE;

        if (index < 0)
            index += SIZE;

        return index;
    }

public:

    void insert(Book book) {

        int index = hashFunction(book.id);

        Entry* current = table[index];

        while (current) {

            if (current->id == book.id) {
                current->book = book;
                return;
            }

            current = current->next;
        }

        Entry* newEntry =
            new Entry(book.id, book);

        newEntry->next = table[index];

        table[index] = newEntry;
    }

    Book* search(int id) {

        int index = hashFunction(id);

        Entry* current = table[index];

        while (current) {

            if (current->id == id)
                return &current->book;

            current = current->next;
        }

        return nullptr;
    }

    void remove(int id) {

        int index = hashFunction(id);

        Entry* current = table[index];
        Entry* previous = nullptr;

        while (current) {

            if (current->id == id) {

                if (previous)
                    previous->next = current->next;
                else
                    table[index] = current->next;

                delete current;

                return;
            }

            previous = current;
            current = current->next;
        }
    }
};

// ============================================================
// GLOBAL DATA
// ============================================================

BST bst;
HashTable hashTable;

// ============================================================
// FILE STORAGE
// ============================================================

void saveBooks() {

    ofstream file("books.txt");

    vector<Book> books = bst.getAll();

    for (Book& b : books) {

        file << b.id << "|"
             << b.title << "|"
             << b.author << "|"
             << b.category << "|"
             << b.issued << "|"
             << b.memberId
             << "\n";
    }

    file.close();
}

void loadBooks() {

    ifstream file("books.txt");

    if (!file)
        return;

    string line;

    while (getline(file, line)) {

        if (line.empty())
            continue;

        stringstream ss(line);

        string id;
        string title;
        string author;
        string category;
        string issued;
        string memberId;

        getline(ss, id, '|');
        getline(ss, title, '|');
        getline(ss, author, '|');
        getline(ss, category, '|');
        getline(ss, issued, '|');
        getline(ss, memberId, '|');

        try {

            Book book;

            book.id = stoi(id);
            book.title = title;
            book.author = author;
            book.category = category;
            book.issued = (issued == "1");
            book.memberId = memberId;

            bst.insert(book);
            hashTable.insert(book);

        }
        catch (...) {
            continue;
        }
    }

    file.close();
}

// ============================================================
// URL DECODE
// ============================================================

string urlDecode(const string& value) {

    string result;

    for (size_t i = 0; i < value.length(); i++) {

        if (value[i] == '+') {

            result += ' ';
        }

        else if (
            value[i] == '%' &&
            i + 2 < value.length()
        ) {

            string hex =
                value.substr(i + 1, 2);

            char ch =
                static_cast<char>(
                    strtol(
                        hex.c_str(),
                        nullptr,
                        16
                    )
                );

            result += ch;

            i += 2;
        }

        else {

            result += value[i];
        }
    }

    return result;
}

// ============================================================
// FORM VALUE
// ============================================================

string getValue(
    const string& body,
    const string& key
) {

    string searchKey = key + "=";

    size_t start =
        body.find(searchKey);

    if (start == string::npos)
        return "";

    start += searchKey.length();

    size_t end =
        body.find('&', start);

    if (end == string::npos)
        end = body.length();

    return urlDecode(
        body.substr(
            start,
            end - start
        )
    );
}

// ============================================================
// JSON ESCAPE
// ============================================================

string jsonEscape(const string& value) {

    string result;

    for (char c : value) {

        if (c == '"')
            result += "\\\"";

        else if (c == '\\')
            result += "\\\\";

        else if (c == '\n')
            result += "\\n";

        else if (c == '\r')
            result += "\\r";

        else
            result += c;
    }

    return result;
}

// ============================================================
// BOOK TO JSON
// ============================================================

string bookToJson(const Book& b) {

    string json = "{";

    json += "\"id\":" +
            to_string(b.id) + ",";

    json += "\"title\":\"" +
            jsonEscape(b.title) + "\",";

    json += "\"author\":\"" +
            jsonEscape(b.author) + "\",";

    json += "\"category\":\"" +
            jsonEscape(b.category) + "\",";

    json += "\"issued\":" +
            string(
                b.issued
                ? "true"
                : "false"
            ) + ",";

    json += "\"memberId\":\"" +
            jsonEscape(b.memberId) +
            "\"";

    json += "}";

    return json;
}

// ============================================================
// HTTP RESPONSE
// ============================================================

void sendResponse(
    int client,
    const string& body,
    const string& contentType =
        "text/html; charset=UTF-8"
) {

    string response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: " +
        contentType +
        "\r\n"
        "Content-Length: " +
        to_string(body.size()) +
        "\r\n"
        "Connection: close\r\n"
        "\r\n" +
        body;

    send(
        client,
        response.c_str(),
        response.size(),
        0
    );
}

// ============================================================
// GET BOOKS JSON
// ============================================================

void getAllBooks(int client) {

    vector<Book> books =
        bst.getAll();

    string json = "[";

    for (size_t i = 0;
         i < books.size();
         i++) {

        if (i > 0)
            json += ",";

        json += bookToJson(books[i]);
    }

    json += "]";

    sendResponse(
        client,
        json,
        "application/json; charset=UTF-8"
    );
}

// ============================================================
// REQUEST BODY READER
// ============================================================

string getRequestBody(
    const string& request
) {

    size_t headerEnd =
        request.find("\r\n\r\n");

    if (headerEnd == string::npos)
        return "";

    return request.substr(
        headerEnd + 4
    );
}

// ============================================================
// HTTP SERVER
// ============================================================

void handleRequest(int client) {

    string request;

    char buffer[8192];

    int received;

    while (
        (received =
            recv(
                client,
                buffer,
                sizeof(buffer),
                0
            )) > 0
    ) {

        request.append(
            buffer,
            received
        );

        if (
            request.find(
                "\r\n\r\n"
            ) != string::npos
        ) {

            size_t headerEnd =
                request.find(
                    "\r\n\r\n"
                );

            string headers =
                request.substr(
                    0,
                    headerEnd
                );

            size_t contentLengthPos =
                headers.find(
                    "Content-Length:"
                );

            if (
                contentLengthPos ==
                string::npos
            ) {
                break;
            }

            size_t lineEnd =
                headers.find(
                    "\r\n",
                    contentLengthPos
                );

            string lengthText =
                headers.substr(
                    contentLengthPos +
                    15,
                    lineEnd -
                    (
                        contentLengthPos +
                        15
                    )
                );

            int contentLength =
                atoi(
                    lengthText.c_str()
                );

            size_t bodyStart =
                headerEnd + 4;

            if (
                request.size() >=
                bodyStart +
                contentLength
            ) {
                break;
            }
        }

        if (
            received <
            (int)sizeof(buffer)
        ) {
            break;
        }
    }

    if (request.empty())
        return;

    // ========================================================
    // REQUEST LINE
    // ========================================================

    size_t firstLineEnd =
        request.find("\r\n");

    if (
        firstLineEnd ==
        string::npos
    )
        return;

    string requestLine =
        request.substr(
            0,
            firstLineEnd
        );

    stringstream requestStream(
        requestLine
    );

    string method;
    string path;
    string version;

    requestStream
        >> method
        >> path
        >> version;

    // ========================================================
    // REMOVE QUERY FROM PATH
    // ========================================================

    string cleanPath = path;

    size_t queryPos =
        cleanPath.find('?');

    if (
        queryPos !=
        string::npos
    ) {
        cleanPath =
            cleanPath.substr(
                0,
                queryPos
            );
    }

    // ========================================================
    // GET
    // ========================================================

    if (method == "GET") {

        // ----------------------------------------------------
        // GET ALL BOOKS
        // ----------------------------------------------------

        if (
            cleanPath ==
            "/api/books"
        ) {

            getAllBooks(client);

            return;
        }

        // ----------------------------------------------------
        // SEARCH BOOK
        // GET /api/books/101
        // ----------------------------------------------------

        if (
            cleanPath.rfind(
                "/api/books/",
                0
            ) == 0
        ) {

            string idText =
                cleanPath.substr(
                    string(
                        "/api/books/"
                    ).length()
                );

            try {

                int id =
                    stoi(idText);

                Book* book =
                    hashTable.search(id);

                if (!book) {

                    sendResponse(
                        client,
                        "{\"success\":false,\"message\":\"Book not found\"}",
                        "application/json; charset=UTF-8"
                    );

                    return;
                }

                string json =
                    bookToJson(*book);

                sendResponse(
                    client,
                    json,
                    "application/json; charset=UTF-8"
                );

            }
            catch (...) {

                sendResponse(
                    client,
                    "{\"success\":false,\"message\":\"Invalid Book ID\"}",
                    "application/json; charset=UTF-8"
                );
            }

            return;
        }

        // ----------------------------------------------------
        // STATIC FILES
        // ----------------------------------------------------

        string filePath;

        string contentType;

        if (
            cleanPath == "/" ||
            cleanPath == "/index.html"
        ) {

            filePath =
                "public/index.html";

            contentType =
                "text/html; charset=UTF-8";
        }

        else if (
            cleanPath ==
            "/style.css"
        ) {

            filePath =
                "public/style.css";

            contentType =
                "text/css; charset=UTF-8";
        }

        else if (
            cleanPath ==
            "/app.js"
        ) {

            filePath =
                "public/app.js";

            contentType =
                "application/javascript; charset=UTF-8";
        }

        else {

            sendResponse(
                client,
                "404 Not Found",
                "text/plain; charset=UTF-8"
            );

            return;
        }

        ifstream file(filePath);

        if (!file) {

            sendResponse(
                client,
                "File not found",
                "text/plain; charset=UTF-8"
            );

            return;
        }

        stringstream contents;

        contents << file.rdbuf();

        sendResponse(
            client,
            contents.str(),
            contentType
        );

        return;
    }

    // ========================================================
    // POST
    // ========================================================

    if (method == "POST") {

        string body =
            getRequestBody(request);

        // ----------------------------------------------------
        // ADD BOOK
        // POST /api/books
        // ----------------------------------------------------

        if (
            cleanPath ==
            "/api/books"
        ) {

            try {

                string idText =
                    getValue(body, "id");

                string title =
                    getValue(body, "title");

                string author =
                    getValue(body, "author");

                string category =
                    getValue(body, "category");

                if (
                    idText.empty() ||
                    title.empty() ||
                    author.empty()
                ) {

                    sendResponse(
                        client,
                        "{\"success\":false,\"message\":\"Please fill all required fields\"}",
                        "application/json; charset=UTF-8"
                    );

                    return;
                }

                Book book;

                book.id =
                    stoi(idText);

                book.title =
                    title;

                book.author =
                    author;

                book.category =
                    category.empty()
                    ? "General"
                    : category;

                book.issued = false;

                book.memberId = "";

                // Check duplicate

                if (
                    hashTable.search(
                        book.id
                    )
                ) {

                    sendResponse(
                        client,
                        "{\"success\":false,\"message\":\"Book ID already exists\"}",
                        "application/json; charset=UTF-8"
                    );

                    return;
                }

                bst.insert(book);

                hashTable.insert(book);

                saveBooks();

                sendResponse(
                    client,
                    "{\"success\":true,\"message\":\"Book added successfully\"}",
                    "application/json; charset=UTF-8"
                );

            }
            catch (...) {

                sendResponse(
                    client,
                    "{\"success\":false,\"message\":\"Invalid book data\"}",
                    "application/json; charset=UTF-8"
                );
            }

            return;
        }

        // ----------------------------------------------------
        // ISSUE BOOK
        // POST /api/issue/101
        // body: memberId=555
        // ----------------------------------------------------

        if (
            cleanPath.rfind(
                "/api/issue/",
                0
            ) == 0
        ) {

            string idText =
                cleanPath.substr(
                    string(
                        "/api/issue/"
                    ).length()
                );

            try {

                int id =
                    stoi(idText);

                string memberId =
                    getValue(
                        body,
                        "memberId"
                    );

                if (memberId.empty()) {

                    sendResponse(
                        client,
                        "{\"success\":false,\"message\":\"Member ID is required\"}",
                        "application/json; charset=UTF-8"
                    );

                    return;
                }

                Book* hashBook =
                    hashTable.search(id);

                if (!hashBook) {

                    sendResponse(
                        client,
                        "{\"success\":false,\"message\":\"Book not found\"}",
                        "application/json; charset=UTF-8"
                    );

                    return;
                }

                if (hashBook->issued) {

                    sendResponse(
                        client,
                        "{\"success\":false,\"message\":\"Book already issued\"}",
                        "application/json; charset=UTF-8"
                    );

                    return;
                }

                // Create updated book copy

                Book updatedBook =
                    *hashBook;

                updatedBook.issued =
                    true;

                updatedBook.memberId =
                    memberId;

                // Update BST

                bst.remove(id);

                bst.insert(
                    updatedBook
                );

                // Update Hash Table

                hashTable.insert(
                    updatedBook
                );

                saveBooks();

                sendResponse(
                    client,
                    "{\"success\":true,\"message\":\"Book issued successfully\"}",
                    "application/json; charset=UTF-8"
                );

            }
            catch (...) {

                sendResponse(
                    client,
                    "{\"success\":false,\"message\":\"Invalid request\"}",
                    "application/json; charset=UTF-8"
                );
            }

            return;
        }

        // ----------------------------------------------------
        // RETURN BOOK
        // POST /api/return/101
        // ----------------------------------------------------

        if (
            cleanPath.rfind(
                "/api/return/",
                0
            ) == 0
        ) {

            string idText =
                cleanPath.substr(
                    string(
                        "/api/return/"
                    ).length()
                );

            try {

                int id =
                    stoi(idText);

                Book* hashBook =
                    hashTable.search(id);

                if (!hashBook) {

                    sendResponse(
                        client,
                        "{\"success\":false,\"message\":\"Book not found\"}",
                        "application/json; charset=UTF-8"
                    );

                    return;
                }

                if (!hashBook->issued) {

                    sendResponse(
                        client,
                        "{\"success\":false,\"message\":\"Book is already available\"}",
                        "application/json; charset=UTF-8"
                    );

                    return;
                }

                Book updatedBook =
                    *hashBook;

                updatedBook.issued =
                    false;

                updatedBook.memberId =
                    "";

                // Update BST

                bst.remove(id);

                bst.insert(
                    updatedBook
                );

                // Update Hash Table

                hashTable.insert(
                    updatedBook
                );

                saveBooks();

                sendResponse(
                    client,
                    "{\"success\":true,\"message\":\"Book returned successfully\"}",
                    "application/json; charset=UTF-8"
                );

            }
            catch (...) {

                sendResponse(
                    client,
                    "{\"success\":false,\"message\":\"Invalid request\"}",
                    "application/json; charset=UTF-8"
                );
            }

            return;
        }
    }

    // ========================================================
    // DELETE
    // DELETE /api/books/101
    // ========================================================

    if (method == "DELETE") {

        if (
            cleanPath.rfind(
                "/api/books/",
                0
            ) == 0
        ) {

            string idText =
                cleanPath.substr(
                    string(
                        "/api/books/"
                    ).length()
                );

            try {

                int id =
                    stoi(idText);

                Book* book =
                    hashTable.search(id);

                if (!book) {

                    sendResponse(
                        client,
                        "{\"success\":false,\"message\":\"Book not found\"}",
                        "application/json; charset=UTF-8"
                    );

                    return;
                }

                bst.remove(id);

                hashTable.remove(id);

                saveBooks();

                sendResponse(
                    client,
                    "{\"success\":true,\"message\":\"Book deleted successfully\"}",
                    "application/json; charset=UTF-8"
                );

            }
            catch (...) {

                sendResponse(
                    client,
                    "{\"success\":false,\"message\":\"Invalid Book ID\"}",
                    "application/json; charset=UTF-8"
                );
            }

            return;
        }
    }

    // ========================================================
    // 404
    // ========================================================

    sendResponse(
        client,
        "404 Not Found",
        "text/plain; charset=UTF-8"
    );
}

// ============================================================
// MAIN
// ============================================================

int main() {

    loadBooks();

    int port = 10000;

    const char* envPort =
        getenv("PORT");

    if (envPort)
        port = atoi(envPort);

    int serverSocket =
        socket(
            AF_INET,
            SOCK_STREAM,
            0
        );

    if (serverSocket < 0) {

        cerr
            << "Socket creation failed\n";

        return 1;
    }

    int option = 1;

    setsockopt(
        serverSocket,
        SOL_SOCKET,
        SO_REUSEADDR,
        &option,
        sizeof(option)
    );

    sockaddr_in serverAddress{};

    serverAddress.sin_family =
        AF_INET;

    serverAddress.sin_addr.s_addr =
        INADDR_ANY;

    serverAddress.sin_port =
        htons(port);

    if (
        bind(
            serverSocket,
            (sockaddr*)&serverAddress,
            sizeof(serverAddress)
        ) < 0
    ) {

        cerr
            << "Bind failed\n";

        close(serverSocket);

        return 1;
    }

    if (
        listen(
            serverSocket,
            10
        ) < 0
    ) {

        cerr
            << "Listen failed\n";

        close(serverSocket);

        return 1;
    }

    cout
        << "Library Management Server running on port "
        << port
        << endl;

    while (true) {

        sockaddr_in clientAddress{};

        socklen_t clientLength =
            sizeof(clientAddress);

        int clientSocket =
            accept(
                serverSocket,
                (sockaddr*)&clientAddress,
                &clientLength
            );

        if (clientSocket < 0)
            continue;

        handleRequest(
            clientSocket
        );

        close(
            clientSocket
        );
    }

    close(
        serverSocket
    );

    return 0;
}
