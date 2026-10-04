#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cstdlib>
#include <cstring>

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

    BSTNode(const Book& b)
        : book(b), left(nullptr), right(nullptr) {}
};

class BST {
private:
    BSTNode* root;

    BSTNode* insertNode(BSTNode* node, const Book& book) {
        if (node == nullptr) {
            return new BSTNode(book);
        }

        if (book.id < node->book.id) {
            node->left = insertNode(node->left, book);
        } else if (book.id > node->book.id) {
            node->right = insertNode(node->right, book);
        } else {
            node->book = book;
        }

        return node;
    }

    BSTNode* searchNode(BSTNode* node, int id) {
        if (node == nullptr) {
            return nullptr;
        }

        if (id == node->book.id) {
            return node;
        }

        if (id < node->book.id) {
            return searchNode(node->left, id);
        }

        return searchNode(node->right, id);
    }

    BSTNode* minimumNode(BSTNode* node) {
        BSTNode* current = node;

        while (current != nullptr && current->left != nullptr) {
            current = current->left;
        }

        return current;
    }

    BSTNode* deleteNode(BSTNode* node, int id) {
        if (node == nullptr) {
            return nullptr;
        }

        if (id < node->book.id) {
            node->left = deleteNode(node->left, id);
        }
        else if (id > node->book.id) {
            node->right = deleteNode(node->right, id);
        }
        else {
            if (node->left == nullptr) {
                BSTNode* temp = node->right;
                delete node;
                return temp;
            }

            if (node->right == nullptr) {
                BSTNode* temp = node->left;
                delete node;
                return temp;
            }

            BSTNode* temp = minimumNode(node->right);
            node->book = temp->book;
            node->right = deleteNode(node->right, temp->book.id);
        }

        return node;
    }

    void inorderNode(BSTNode* node, vector<Book>& books) {
        if (node == nullptr) {
            return;
        }

        inorderNode(node->left, books);
        books.push_back(node->book);
        inorderNode(node->right, books);
    }

public:
    BST() : root(nullptr) {}

    void insert(const Book& book) {
        root = insertNode(root, book);
    }

    Book* search(int id) {
        BSTNode* node = searchNode(root, id);

        if (node == nullptr) {
            return nullptr;
        }

        return &node->book;
    }

    void remove(int id) {
        root = deleteNode(root, id);
    }

    vector<Book> getAll() {
        vector<Book> books;
        inorderNode(root, books);
        return books;
    }
};

// ============================================================
// HASH TABLE
// ============================================================

class HashTable {
private:
    static const int TABLE_SIZE = 101;

    vector<vector<Book>> table;

    int hashFunction(int id) {
        if (id < 0) {
            id = -id;
        }

        return id % TABLE_SIZE;
    }

public:
    HashTable() : table(TABLE_SIZE) {}

    void insert(const Book& book) {
        int index = hashFunction(book.id);

        for (Book& existing : table[index]) {
            if (existing.id == book.id) {
                existing = book;
                return;
            }
        }

        table[index].push_back(book);
    }

    Book* search(int id) {
        int index = hashFunction(id);

        for (Book& book : table[index]) {
            if (book.id == id) {
                return &book;
            }
        }

        return nullptr;
    }

    void remove(int id) {
        int index = hashFunction(id);

        auto& bucket = table[index];

        bucket.erase(
            remove_if(
                bucket.begin(),
                bucket.end(),
                [id](const Book& book) {
                    return book.id == id;
                }
            ),
            bucket.end()
        );
    }

    void clear() {
        for (auto& bucket : table) {
            bucket.clear();
        }
    }
};

// ============================================================
// GLOBAL DATA
// ============================================================

BST bst;
HashTable hashTable;

const string DATA_FILE = "books.txt";

// ============================================================
// URL DECODE
// ============================================================

string urlDecode(const string& value) {
    string result;

    for (size_t i = 0; i < value.length(); i++) {
        if (value[i] == '%') {
            if (i + 2 < value.length()) {
                string hex = value.substr(i + 1, 2);

                try {
                    char decoded =
                        static_cast<char>(strtol(hex.c_str(), nullptr, 16));

                    result += decoded;
                    i += 2;
                }
                catch (...) {
                    result += '%';
                }
            }
            else {
                result += '%';
            }
        }
        else if (value[i] == '+') {
            result += ' ';
        }
        else {
            result += value[i];
        }
    }

    return result;
}

// ============================================================
// GET FORM VALUE
// ============================================================

string getFormValue(const string& body, const string& key) {
    string target = key + "=";

    size_t start = 0;

    while (start < body.length()) {
        size_t end = body.find('&', start);

        if (end == string::npos) {
            end = body.length();
        }

        string pair = body.substr(start, end - start);

        if (pair.rfind(target, 0) == 0) {
            return urlDecode(pair.substr(target.length()));
        }

        start = end + 1;
    }

    return "";
}

// ============================================================
// JSON ESCAPE
// ============================================================

string jsonEscape(const string& value) {
    string result;

    for (char c : value) {
        switch (c) {
            case '"':
                result += "\\\"";
                break;

            case '\\':
                result += "\\\\";
                break;

            case '\n':
                result += "\\n";
                break;

            case '\r':
                result += "\\r";
                break;

            case '\t':
                result += "\\t";
                break;

            default:
                result += c;
        }
    }

    return result;
}

// ============================================================
// BOOK TO JSON
// ============================================================

string bookToJson(const Book& book) {
    stringstream ss;

    ss << "{";
    ss << "\"id\":" << book.id << ",";
    ss << "\"title\":\"" << jsonEscape(book.title) << "\",";
    ss << "\"author\":\"" << jsonEscape(book.author) << "\",";
    ss << "\"category\":\"" << jsonEscape(book.category) << "\",";
    ss << "\"issued\":" << (book.issued ? "true" : "false") << ",";
    ss << "\"memberId\":\"" << jsonEscape(book.memberId) << "\"";
    ss << "}";

    return ss.str();
}

// ============================================================
// BOOK LIST TO JSON
// ============================================================

string booksToJson(const vector<Book>& books) {
    stringstream ss;

    ss << "[";

    for (size_t i = 0; i < books.size(); i++) {
        if (i > 0) {
            ss << ",";
        }

        ss << bookToJson(books[i]);
    }

    ss << "]";

    return ss.str();
}

// ============================================================
// SAVE BOOKS
// ============================================================

void saveBooks() {
    ofstream file(DATA_FILE);

    if (!file.is_open()) {
        cerr << "Could not open books.txt for writing." << endl;
        return;
    }

    vector<Book> books = bst.getAll();

    for (const Book& book : books) {
        file
            << book.id << "|"
            << book.title << "|"
            << book.author << "|"
            << book.category << "|"
            << (book.issued ? 1 : 0) << "|"
            << book.memberId
            << "\n";
    }

    file.close();
}

// ============================================================
// LOAD BOOKS
// ============================================================

void loadBooks() {
    ifstream file(DATA_FILE);

    if (!file.is_open()) {
        cout << "books.txt not found. Starting with empty library." << endl;
        return;
    }

    string line;

    while (getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        stringstream ss(line);

        string idStr;
        string title;
        string author;
        string category;
        string issuedStr;
        string memberId;

        getline(ss, idStr, '|');
        getline(ss, title, '|');
        getline(ss, author, '|');
        getline(ss, category, '|');
        getline(ss, issuedStr, '|');
        getline(ss, memberId);

        try {
            Book book;

            book.id = stoi(idStr);
            book.title = title;
            book.author = author;
            book.category = category;
            book.issued = (issuedStr == "1");
            book.memberId = memberId;

            bst.insert(book);
            hashTable.insert(book);
        }
        catch (...) {
            cerr << "Skipping invalid record: " << line << endl;
        }
    }

    file.close();
}

// ============================================================
// SEND HTTP RESPONSE
// ============================================================

void sendResponse(
    int clientSocket,
    const string& contentType,
    const string& body,
    int statusCode = 200,
    const string& statusText = "OK"
) {
    stringstream response;

    response
        << "HTTP/1.1 "
        << statusCode
        << " "
        << statusText
        << "\r\n";

    response
        << "Content-Type: "
        << contentType
        << "\r\n";

    response
        << "Content-Length: "
        << body.size()
        << "\r\n";

    response << "Access-Control-Allow-Origin: *\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << body;

    string output = response.str();

    send(
        clientSocket,
        output.c_str(),
        output.size(),
        0
    );
}

// ============================================================
// JSON RESPONSE HELPERS
// ============================================================

void sendJson(
    int clientSocket,
    const string& json,
    int statusCode = 200,
    const string& statusText = "OK"
) {
    sendResponse(
        clientSocket,
        "application/json; charset=UTF-8",
        json,
        statusCode,
        statusText
    );
}

void sendError(
    int clientSocket,
    const string& message,
    int statusCode = 400,
    const string& statusText = "Bad Request"
) {
    stringstream ss;

    ss << "{";
    ss << "\"success\":false,";
    ss << "\"message\":\"" << jsonEscape(message) << "\"";
    ss << "}";

    sendJson(
        clientSocket,
        ss.str(),
        statusCode,
        statusText
    );
}

void sendSuccess(
    int clientSocket,
    const string& message
) {
    stringstream ss;

    ss << "{";
    ss << "\"success\":true,";
    ss << "\"message\":\"" << jsonEscape(message) << "\"";
    ss << "}";

    sendJson(clientSocket, ss.str());
}

// ============================================================
// READ HTTP REQUEST
// ============================================================

bool readRequest(
    int clientSocket,
    string& request
) {
    request.clear();

    char buffer[8192];

    size_t headerEnd = string::npos;

    int contentLength = 0;

    while (true) {
        ssize_t bytesRead = recv(
            clientSocket,
            buffer,
            sizeof(buffer),
            0
        );

        if (bytesRead <= 0) {
            return false;
        }

        request.append(buffer, bytesRead);

        headerEnd = request.find("\r\n\r\n");

        if (headerEnd != string::npos) {
            break;
        }

        if (request.size() > 1024 * 1024) {
            return false;
        }
    }

    string headers = request.substr(
        0,
        headerEnd
    );

    size_t contentLengthPos =
        headers.find("Content-Length:");

    if (contentLengthPos == string::npos) {
        contentLengthPos =
            headers.find("content-length:");
    }

    if (contentLengthPos != string::npos) {
        size_t valueStart =
            contentLengthPos + 15;

        while (
            valueStart < headers.length() &&
            (headers[valueStart] == ' ' ||
             headers[valueStart] == '\t')
        ) {
            valueStart++;
        }

        size_t valueEnd =
            headers.find("\r\n", valueStart);

        string lengthValue =
            headers.substr(
                valueStart,
                valueEnd == string::npos
                    ? string::npos
                    : valueEnd - valueStart
            );

        try {
            contentLength = stoi(lengthValue);
        }
        catch (...) {
            contentLength = 0;
        }
    }

    size_t bodyStart = headerEnd + 4;

    while (
        request.size() - bodyStart <
        static_cast<size_t>(contentLength)
    ) {
        ssize_t bytesRead = recv(
            clientSocket,
            buffer,
            sizeof(buffer),
            0
        );

        if (bytesRead <= 0) {
            break;
        }

        request.append(buffer, bytesRead);
    }

    return true;
}

// ============================================================
// PARSE PATH ID
// ============================================================

bool parseIdFromPath(
    const string& path,
    const string& prefix,
    int& id
) {
    if (path.rfind(prefix, 0) != 0) {
        return false;
    }

    string idString =
        path.substr(prefix.length());

    if (idString.empty()) {
        return false;
    }

    for (char c : idString) {
        if (c < '0' || c > '9') {
            return false;
        }
    }

    try {
        id = stoi(idString);
    }
    catch (...) {
        return false;
    }

    return true;
}

// ============================================================
// STATIC FILE
// ============================================================

bool serveStaticFile(
    int clientSocket,
    const string& path
) {
    string filePath;

    if (path == "/" || path == "/index.html") {
        filePath = "public/index.html";
    }
    else if (path == "/style.css") {
        filePath = "public/style.css";
    }
    else if (path == "/app.js") {
        filePath = "public/app.js";
    }
    else {
        return false;
    }

    ifstream file(filePath, ios::binary);

    if (!file.is_open()) {
        return false;
    }

    string content(
        (istreambuf_iterator<char>(file)),
        istreambuf_iterator<char>()
    );

    file.close();

    string contentType = "text/plain; charset=UTF-8";

    if (path == "/" || path == "/index.html") {
        contentType = "text/html; charset=UTF-8";
    }
    else if (path == "/style.css") {
        contentType = "text/css; charset=UTF-8";
    }
    else if (path == "/app.js") {
        contentType = "application/javascript; charset=UTF-8";
    }

    sendResponse(
        clientSocket,
        contentType,
        content
    );

    return true;
}

// ============================================================
// HANDLE REQUEST
// ============================================================

void handleRequest(int clientSocket) {
    string request;

    if (!readRequest(clientSocket, request)) {
        return;
    }

    size_t firstLineEnd =
        request.find("\r\n");

    if (firstLineEnd == string::npos) {
        sendError(
            clientSocket,
            "Invalid HTTP request",
            400,
            "Bad Request"
        );

        return;
    }

    string requestLine =
        request.substr(0, firstLineEnd);

    stringstream requestStream(requestLine);

    string method;
    string path;
    string version;

    requestStream
        >> method
        >> path
        >> version;

    if (method.empty() || path.empty()) {
        sendError(
            clientSocket,
            "Invalid HTTP request",
            400,
            "Bad Request"
        );

        return;
    }

    size_t headerEnd =
        request.find("\r\n\r\n");

    string body;

    if (headerEnd != string::npos) {
        body = request.substr(headerEnd + 4);
    }

    cout
        << method
        << " "
        << path
        << endl;

    // ========================================================
    // GET /api/books
    // ========================================================

    if (method == "GET" && path == "/api/books") {
        vector<Book> books = bst.getAll();

        sendJson(
            clientSocket,
            booksToJson(books)
        );

        return;
    }

    // ========================================================
    // GET /api/books/:id
    // ========================================================

    if (
        method == "GET" &&
        path.rfind("/api/books/", 0) == 0
    ) {
        int id;

        if (!parseIdFromPath(
                path,
                "/api/books/",
                id
            )) {
            sendError(
                clientSocket,
                "Invalid book ID"
            );

            return;
        }

        Book* book = hashTable.search(id);

        if (book == nullptr) {
            sendError(
                clientSocket,
                "Book not found",
                404,
                "Not Found"
            );

            return;
        }

        sendJson(
            clientSocket,
            bookToJson(*book)
        );

        return;
    }

    // ========================================================
    // GET /api/search?id=101
    // ========================================================

    if (
        method == "GET" &&
        path.rfind("/api/search", 0) == 0
    ) {
        size_t idPos = path.find("id=");

        if (idPos == string::npos) {
            sendError(
                clientSocket,
                "Book ID is required"
            );

            return;
        }

        string idValue =
            path.substr(idPos + 3);

        size_t amp =
            idValue.find('&');

        if (amp != string::npos) {
            idValue =
                idValue.substr(0, amp);
        }

        int id;

        try {
            id = stoi(idValue);
        }
        catch (...) {
            sendError(
                clientSocket,
                "Invalid book ID"
            );

            return;
        }

        Book* book = hashTable.search(id);

        if (book == nullptr) {
            sendError(
                clientSocket,
                "Book not found",
                404,
                "Not Found"
            );

            return;
        }

        sendJson(
            clientSocket,
            bookToJson(*book)
        );

        return;
    }

    // ========================================================
    // POST /api/books
    // ========================================================

    if (
        method == "POST" &&
        path == "/api/books"
    ) {
        string idValue =
            getFormValue(body, "id");

        string title =
            getFormValue(body, "title");

        string author =
            getFormValue(body, "author");

        string category =
            getFormValue(body, "category");

        if (
            idValue.empty() ||
            title.empty() ||
            author.empty()
        ) {
            sendError(
                clientSocket,
                "ID, title and author are required"
            );

            return;
        }

        int id;

        try {
            id = stoi(idValue);
        }
        catch (...) {
            sendError(
                clientSocket,
                "Invalid book ID"
            );

            return;
        }

        if (hashTable.search(id) != nullptr) {
            sendError(
                clientSocket,
                "Book ID already exists"
            );

            return;
        }

        Book book;

        book.id = id;
        book.title = title;
        book.author = author;
        book.category =
            category.empty()
                ? "General"
                : category;
        book.issued = false;
        book.memberId = "";

        bst.insert(book);
        hashTable.insert(book);

        saveBooks();

        sendSuccess(
            clientSocket,
            "Book added successfully"
        );

        return;
    }

    // ========================================================
    // POST /api/add
    // Backward compatibility
    // ========================================================

    if (
        method == "POST" &&
        path == "/api/add"
    ) {
        string idValue =
            getFormValue(body, "id");

        string title =
            getFormValue(body, "title");

        string author =
            getFormValue(body, "author");

        string category =
            getFormValue(body, "category");

        if (
            idValue.empty() ||
            title.empty() ||
            author.empty()
        ) {
            sendError(
                clientSocket,
                "ID, title and author are required"
            );

            return;
        }

        int id;

        try {
            id = stoi(idValue);
        }
        catch (...) {
            sendError(
                clientSocket,
                "Invalid book ID"
            );

            return;
        }

        if (hashTable.search(id) != nullptr) {
            sendError(
                clientSocket,
                "Book ID already exists"
            );

            return;
        }

        Book book;

        book.id = id;
        book.title = title;
        book.author = author;
        book.category =
            category.empty()
                ? "General"
                : category;
        book.issued = false;
        book.memberId = "";

        bst.insert(book);
        hashTable.insert(book);

        saveBooks();

        sendSuccess(
            clientSocket,
            "Book added successfully"
        );

        return;
    }

    // ========================================================
    // DELETE /api/books/:id
    // ========================================================

    if (
        method == "DELETE" &&
        path.rfind("/api/books/", 0) == 0
    ) {
        int id;

        if (!parseIdFromPath(
                path,
                "/api/books/",
                id
            )) {
            sendError(
                clientSocket,
                "Invalid book ID"
            );

            return;
        }

        Book* book = hashTable.search(id);

        if (book == nullptr) {
            sendError(
                clientSocket,
                "Book not found",
                404,
                "Not Found"
            );

            return;
        }

        if (book->issued) {
            sendError(
                clientSocket,
                "Cannot delete an issued book"
            );

            return;
        }

        bst.remove(id);
        hashTable.remove(id);

        saveBooks();

        sendSuccess(
            clientSocket,
            "Book deleted successfully"
        );

        return;
    }

    // ========================================================
    // POST /api/delete
    // Backward compatibility
    // ========================================================

    if (
        method == "POST" &&
        path == "/api/delete"
    ) {
        string idValue =
            getFormValue(body, "id");

        if (idValue.empty()) {
            sendError(
                clientSocket,
                "Book ID is required"
            );

            return;
        }

        int id;

        try {
            id = stoi(idValue);
        }
        catch (...) {
            sendError(
                clientSocket,
                "Invalid book ID"
            );

            return;
        }

        Book* book = hashTable.search(id);

        if (book == nullptr) {
            sendError(
                clientSocket,
                "Book not found",
                404,
                "Not Found"
            );

            return;
        }

        if (book->issued) {
            sendError(
                clientSocket,
                "Cannot delete an issued book"
            );

            return;
        }

        bst.remove(id);
        hashTable.remove(id);

        saveBooks();

        sendSuccess(
            clientSocket,
            "Book deleted successfully"
        );

        return;
    }

    // ========================================================
    // POST /api/issue/:id
    // ========================================================

    if (
        method == "POST" &&
        path.rfind("/api/issue/", 0) == 0
    ) {
        int id;

        if (!parseIdFromPath(
                path,
                "/api/issue/",
                id
            )) {
            sendError(
                clientSocket,
                "Invalid book ID"
            );

            return;
        }

        string memberId =
            getFormValue(body, "memberId");

        if (memberId.empty()) {
            sendError(
                clientSocket,
                "Member ID is required"
            );

            return;
        }

        Book* book = hashTable.search(id);

        if (book == nullptr) {
            sendError(
                clientSocket,
                "Book not found",
                404,
                "Not Found"
            );

            return;
        }

        if (book->issued) {
            sendError(
                clientSocket,
                "Book is already issued"
            );

            return;
        }

        book->issued = true;
        book->memberId = memberId;

        Book* bstBook = bst.search(id);

        if (bstBook != nullptr) {
            bstBook->issued = true;
            bstBook->memberId = memberId;
        }

        saveBooks();

        sendSuccess(
            clientSocket,
            "Book issued successfully"
        );

        return;
    }

    // ========================================================
    // POST /api/issue
    // Backward compatibility
    // ========================================================

    if (
        method == "POST" &&
        path == "/api/issue"
    ) {
        string idValue =
            getFormValue(body, "id");

        string memberId =
            getFormValue(body, "memberId");

        if (
            idValue.empty() ||
            memberId.empty()
        ) {
            sendError(
                clientSocket,
                "Book ID and Member ID are required"
            );

            return;
        }

        int id;

        try {
            id = stoi(idValue);
        }
        catch (...) {
            sendError(
                clientSocket,
                "Invalid book ID"
            );

            return;
        }

        Book* book = hashTable.search(id);

        if (book == nullptr) {
            sendError(
                clientSocket,
                "Book not found",
                404,
                "Not Found"
            );

            return;
        }

        if (book->issued) {
            sendError(
                clientSocket,
                "Book is already issued"
            );

            return;
        }

        book->issued = true;
        book->memberId = memberId;

        Book* bstBook = bst.search(id);

        if (bstBook != nullptr) {
            bstBook->issued = true;
            bstBook->memberId = memberId;
        }

        saveBooks();

        sendSuccess(
            clientSocket,
            "Book issued successfully"
        );

        return;
    }

    // ========================================================
    // POST /api/return/:id
    // ========================================================

    if (
        method == "POST" &&
        path.rfind("/api/return/", 0) == 0
    ) {
        int id;

        if (!parseIdFromPath(
                path,
                "/api/return/",
                id
            )) {
            sendError(
                clientSocket,
                "Invalid book ID"
            );

            return;
        }

        Book* book = hashTable.search(id);

        if (book == nullptr) {
            sendError(
                clientSocket,
                "Book not found",
                404,
                "Not Found"
            );

            return;
        }

        if (!book->issued) {
            sendError(
                clientSocket,
                "Book is not currently issued"
            );

            return;
        }

        book->issued = false;
        book->memberId = "";

        Book* bstBook = bst.search(id);

        if (bstBook != nullptr) {
            bstBook->issued = false;
            bstBook->memberId = "";
        }

        saveBooks();

        sendSuccess(
            clientSocket,
            "Book returned successfully"
        );

        return;
    }

    // ========================================================
    // POST /api/return
    // Backward compatibility
    // ========================================================

    if (
        method == "POST" &&
        path == "/api/return"
    ) {
        string idValue =
            getFormValue(body, "id");

        if (idValue.empty()) {
            sendError(
                clientSocket,
                "Book ID is required"
            );

            return;
        }

        int id;

        try {
            id = stoi(idValue);
        }
        catch (...) {
            sendError(
                clientSocket,
                "Invalid book ID"
            );

            return;
        }

        Book* book = hashTable.search(id);

        if (book == nullptr) {
            sendError(
                clientSocket,
                "Book not found",
                404,
                "Not Found"
            );

            return;
        }

        if (!book->issued) {
            sendError(
                clientSocket,
                "Book is not currently issued"
            );

            return;
        }

        book->issued = false;
        book->memberId = "";

        Book* bstBook = bst.search(id);

        if (bstBook != nullptr) {
            bstBook->issued = false;
            bstBook->memberId = "";
        }

        saveBooks();

        sendSuccess(
            clientSocket,
            "Book returned successfully"
        );

        return;
    }

    // ========================================================
    // STATIC FILES
    // ========================================================

    if (method == "GET") {
        if (serveStaticFile(clientSocket, path)) {
            return;
        }
    }

    // ========================================================
    // 404
    // ========================================================

    sendError(
        clientSocket,
        "Endpoint not found",
        404,
        "Not Found"
    );
}

// ============================================================
// MAIN
// ============================================================

int main() {
    cout << "========================================" << endl;
    cout << " Library Management System" << endl;
    cout << " BST + Hashing + C++ Web Server" << endl;
    cout << "========================================" << endl;

    loadBooks();

    const char* portEnv =
        getenv("PORT");

    int port = 10000;

    if (portEnv != nullptr) {
        try {
            port = stoi(portEnv);
        }
        catch (...) {
            port = 10000;
        }
    }

    int serverSocket =
        socket(
            AF_INET,
            SOCK_STREAM,
            0
        );

    if (serverSocket < 0) {
        cerr << "Failed to create socket." << endl;
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
            reinterpret_cast<sockaddr*>(&serverAddress),
            sizeof(serverAddress)
        ) < 0
    ) {
        cerr << "Failed to bind port "
             << port
             << endl;

        close(serverSocket);

        return 1;
    }

    if (
        listen(
            serverSocket,
            20
        ) < 0
    ) {
        cerr << "Failed to listen." << endl;

        close(serverSocket);

        return 1;
    }

    cout
        << "Server running on port "
        << port
        << endl;

    while (true) {
        sockaddr_in clientAddress{};

        socklen_t clientLength =
            sizeof(clientAddress);

        int clientSocket =
            accept(
                serverSocket,
                reinterpret_cast<sockaddr*>(&clientAddress),
                &clientLength
            );

        if (clientSocket < 0) {
            cerr << "Failed to accept connection."
                 << endl;

            continue;
        }

        handleRequest(clientSocket);

        close(clientSocket);
    }

    close(serverSocket);

    return 0;
}