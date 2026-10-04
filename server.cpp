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

// ==================== BOOK STRUCTURE ====================

struct Book {
    int id;
    string title;
    string author;
    string category;
    bool issued;
    string memberId;
};

// ==================== BST ====================

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
        if (!node) return;

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
            node->right = deleteNode(node->right, temp->book.id);
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

// ==================== HASH TABLE ====================

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
        return id % SIZE;
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

        Entry* newEntry = new Entry(book.id, book);

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

// ==================== GLOBAL DATA ====================

BST bst;
HashTable hashTable;

// ==================== FILE STORAGE ====================

void saveBooks() {

    ofstream file("books.txt");

    vector<Book> books = bst.getAll();

    for (Book& b : books) {

        file << b.id << "|"
             << b.title << "|"
             << b.author << "|"
             << b.category << "|"
             << b.issued << "|"
             << b.memberId << "\n";
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

// ==================== URL DECODE ====================

string urlDecode(string value) {

    string result;

    for (size_t i = 0; i < value.length(); i++) {

        if (value[i] == '+') {
            result += ' ';
        }
        else if (value[i] == '%' && i + 2 < value.length()) {

            string hex = value.substr(i + 1, 2);

            char ch = static_cast<char>(
                strtol(hex.c_str(), nullptr, 16)
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

// ==================== FORM VALUE ====================

string getValue(string body, string key) {

    string searchKey = key + "=";

    size_t start = body.find(searchKey);

    if (start == string::npos)
        return "";

    start += searchKey.length();

    size_t end = body.find('&', start);

    if (end == string::npos)
        end = body.length();

    return urlDecode(body.substr(start, end - start));
}

// ==================== JSON ESCAPE ====================

string jsonEscape(string value) {

    string result;

    for (char c : value) {

        if (c == '"')
            result += "\\\"";

        else if (c == '\\')
            result += "\\\\";

        else if (c == '\n')
            result += "\\n";

        else
            result += c;
    }

    return result;
}

// ==================== BOOK JSON ====================

string bookToJson(Book b) {

    string json = "{";

    json += "\"id\":" + to_string(b.id) + ",";
    json += "\"title\":\"" + jsonEscape(b.title) + "\",";
    json += "\"author\":\"" + jsonEscape(b.author) + "\",";
    json += "\"category\":\"" + jsonEscape(b.category) + "\",";
    json += "\"issued\":" + string(b.issued ? "true" : "false") + ",";
    json += "\"memberId\":\"" + jsonEscape(b.memberId) + "\"";

    json += "}";

    return json;
}

// ==================== HTTP RESPONSE ====================

void sendResponse(
    int client,
    string body,
    string contentType = "text/html"
) {

    string response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: " + contentType + "\r\n"
        "Content-Length: " + to_string(body.size()) + "\r\n"
        "Connection: close\r\n"
        "\r\n" +
        body;

    send(client, response.c_str(), response.size(), 0);
}

// ==================== HTTP SERVER ====================

void handleRequest(int client) {

    string request;

    char buffer[8192];

    int received;

    while ((received = recv(client, buffer, sizeof(buffer), 0)) > 0) {

        request.append(buffer, received);

        if (received < (int)sizeof(buffer))
            break;
    }

    if (request.empty())
        return;

    size_t firstLineEnd = request.find("\r\n");

    if (firstLineEnd == string::npos)
        return;

    string requestLine =
        request.substr(0, firstLineEnd);

    stringstream requestStream(requestLine);

    string method;
    string path;
    string version;

    requestStream >> method >> path >> version;

    // ==================== GET ====================

    if (method == "GET") {

        // ---------- GET BOOKS ----------

        if (path == "/api/books") {

            vector<Book> books = bst.getAll();

            string json = "[";

            for (size_t i = 0; i < books.size(); i++) {

                if (i > 0)
                    json += ",";

                json += bookToJson(books[i]);
            }

            json += "]";

            sendResponse(
                client,
                json,
                "application/json"
            );

            return;
        }

        // ---------- SEARCH BOOK ----------

        if (path.rfind("/api/search?id=", 0) == 0) {

            string idText =
                path.substr(string("/api/search?id=").length());

            try {

                int id = stoi(idText);

                Book* book = hashTable.search(id);

                if (!book) {

                    sendResponse(
                        client,
                        "{\"error\":\"Book not found\"}",
                        "application/json"
                    );

                    return;
                }

                sendResponse(
                    client,
                    bookToJson(*book),
                    "application/json"
                );

            }
            catch (...) {

                sendResponse(
                    client,
                    "{\"error\":\"Invalid ID\"}",
                    "application/json"
                );
            }

            return;
        }

        // ---------- STATIC FILES ----------

        string filePath;

        if (path == "/" || path == "/index.html")
            filePath = "public/index.html";

        else if (path == "/style.css")
            filePath = "public/style.css";

        else if (path == "/app.js")
            filePath = "public/app.js";

        else {

            sendResponse(
                client,
                "404 Not Found"
            );

            return;
        }

        ifstream file(filePath);

        if (!file) {

            sendResponse(
                client,
                "File not found"
            );

            return;
        }

        stringstream contents;

        contents << file.rdbuf();

        string contentType = "text/plain";

        if (path.find(".html") != string::npos)
            contentType = "text/html";

        else if (path.find(".css") != string::npos)
            contentType = "text/css";

        else if (path.find(".js") != string::npos)
            contentType = "application/javascript";

        sendResponse(
            client,
            contents.str(),
            contentType
        );

        return;
    }

    // ==================== POST ====================

    if (method == "POST") {

        size_t headerEnd = request.find("\r\n\r\n");

        if (headerEnd == string::npos)
            return;

        string body =
            request.substr(headerEnd + 4);

        // ---------- ADD BOOK ----------

        if (path == "/api/add") {

            try {

                Book book;

                book.id =
                    stoi(getValue(body, "id"));

                book.title =
                    getValue(body, "title");

                book.author =
                    getValue(body, "author");

                book.category =
                    getValue(body, "category");

                book.issued = false;
                book.memberId = "";

                if (hashTable.search(book.id)) {

                    sendResponse(
                        client,
                        "{\"success\":false,\"message\":\"Book ID already exists\"}",
                        "application/json"
                    );

                    return;
                }

                bst.insert(book);
                hashTable.insert(book);

                saveBooks();

                sendResponse(
                    client,
                    "{\"success\":true,\"message\":\"Book added successfully\"}",
                    "application/json"
                );

            }
            catch (...) {

                sendResponse(
                    client,
                    "{\"success\":false,\"message\":\"Invalid book data\"}",
                    "application/json"
                );
            }

            return;
        }

        // ---------- ISSUE BOOK ----------

        if (path == "/api/issue") {

            try {

                int id =
                    stoi(getValue(body, "id"));

                string memberId =
                    getValue(body, "memberId");

                Book* book =
                    hashTable.search(id);

                if (!book) {

                    sendResponse(
                        client,
                        "{\"success\":false,\"message\":\"Book not found\"}",
                        "application/json"
                    );

                    return;
                }

                if (book->issued) {

                    sendResponse(
                        client,
                        "{\"success\":false,\"message\":\"Book already issued\"}",
                        "application/json"
                    );

                    return;
                }

                book->issued = true;
                book->memberId = memberId;

                bst.remove(id);
                bst.insert(*book);

                hashTable.insert(*book);

                saveBooks();

                sendResponse(
                    client,
                    "{\"success\":true,\"message\":\"Book issued successfully\"}",
                    "application/json"
                );

            }
            catch (...) {

                sendResponse(
                    client,
                    "{\"success\":false,\"message\":\"Invalid request\"}",
                    "application/json"
                );
            }

            return;
        }

        // ---------- RETURN BOOK ----------

        if (path == "/api/return") {

            try {

                int id =
                    stoi(getValue(body, "id"));

                Book* book =
                    hashTable.search(id);

                if (!book) {

                    sendResponse(
                        client,
                        "{\"success\":false,\"message\":\"Book not found\"}",
                        "application/json"
                    );

                    return;
                }

                book->issued = false;
                book->memberId = "";

                bst.remove(id);
                bst.insert(*book);

                hashTable.insert(*book);

                saveBooks();

                sendResponse(
                    client,
                    "{\"success\":true,\"message\":\"Book returned successfully\"}",
                    "application/json"
                );

            }
            catch (...) {

                sendResponse(
                    client,
                    "{\"success\":false,\"message\":\"Invalid request\"}",
                    "application/json"
                );
            }

            return;
        }

        // ---------- DELETE BOOK ----------

        if (path == "/api/delete") {

            try {

                int id =
                    stoi(getValue(body, "id"));

                Book* book =
                    hashTable.search(id);

                if (!book) {

                    sendResponse(
                        client,
                        "{\"success\":false,\"message\":\"Book not found\"}",
                        "application/json"
                    );

                    return;
                }

                bst.remove(id);
                hashTable.remove(id);

                saveBooks();

                sendResponse(
                    client,
                    "{\"success\":true,\"message\":\"Book deleted successfully\"}",
                    "application/json"
                );

            }
            catch (...) {

                sendResponse(
                    client,
                    "{\"success\":false,\"message\":\"Invalid request\"}",
                    "application/json"
                );
            }

            return;
        }
    }

    sendResponse(
        client,
        "404 Not Found"
    );
}

// ==================== MAIN ====================

int main() {

    loadBooks();

    int port = 10000;

    const char* envPort =
        getenv("PORT");

    if (envPort)
        port = atoi(envPort);

    int serverSocket =
        socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket < 0) {

        cerr << "Socket creation failed\n";

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

    if (bind(
        serverSocket,
        (sockaddr*)&serverAddress,
        sizeof(serverAddress)
    ) < 0) {

        cerr << "Bind failed\n";

        close(serverSocket);

        return 1;
    }

    if (listen(serverSocket, 10) < 0) {

        cerr << "Listen failed\n";

        close(serverSocket);

        return 1;
    }

    cout << "Library Management Server running on port "
         << port << endl;

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

        handleRequest(clientSocket);

        close(clientSocket);
    }

    close(serverSocket);

    return 0;
}