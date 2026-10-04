#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

#pragma comment(lib, "ws2_32.lib")
using namespace std;

// ============================================================
// LIBRARY MANAGEMENT SYSTEM
// BST + CUSTOM HASH TABLE + SIMPLE C++ HTTP SERVER
// ============================================================

struct Book {
    int id;
    string title;
    string author;
    string category;
    bool issued;
    int memberId;
};

string jsonEscape(const string& s) {
    string out;
    for (char c : s) {
        if (c == '"') out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else out += c;
    }
    return out;
}

class BST {
    struct Node {
        Book book;
        Node* left;
        Node* right;
        Node(const Book& b) : book(b), left(nullptr), right(nullptr) {}
    };
    Node* root = nullptr;

    Node* insert(Node* n, const Book& b) {
        if (!n) return new Node(b);
        if (b.id < n->book.id) n->left = insert(n->left, b);
        else if (b.id > n->book.id) n->right = insert(n->right, b);
        else n->book = b;
        return n;
    }

    Node* minNode(Node* n) {
        while (n && n->left) n = n->left;
        return n;
    }

    Node* remove(Node* n, int id) {
        if (!n) return nullptr;
        if (id < n->book.id) n->left = remove(n->left, id);
        else if (id > n->book.id) n->right = remove(n->right, id);
        else {
            if (!n->left) {
                Node* r = n->right;
                delete n;
                return r;
            }
            if (!n->right) {
                Node* l = n->left;
                delete n;
                return l;
            }
            Node* s = minNode(n->right);
            n->book = s->book;
            n->right = remove(n->right, s->book.id);
        }
        return n;
    }

    Node* search(Node* n, int id) {
        if (!n || n->book.id == id) return n;
        if (id < n->book.id) return search(n->left, id);
        return search(n->right, id);
    }

    void inorder(Node* n, vector<Book>& out) {
        if (!n) return;
        inorder(n->left, out);
        out.push_back(n->book);
        inorder(n->right, out);
    }

    void destroy(Node* n) {
        if (!n) return;
        destroy(n->left);
        destroy(n->right);
        delete n;
    }

public:
    ~BST() { destroy(root); }

    void insert(const Book& b) { root = insert(root, b); }
    void remove(int id) { root = remove(root, id); }
    Book* search(int id) {
        Node* n = search(root, id);
        return n ? &n->book : nullptr;
    }
    vector<Book> getAll() {
        vector<Book> v;
        inorder(root, v);
        return v;
    }
};

// Custom hash table: separate chaining
class HashTable {
    static const int SIZE = 101;
    struct Entry {
        int key;
        Book book;
        Entry* next;
        Entry(int k, const Book& b) : key(k), book(b), next(nullptr) {}
    };
    Entry* table[SIZE]{};

    int hashFunc(int key) const {
        if (key < 0) key = -key;
        return key % SIZE;
    }

public:
    ~HashTable() {
        for (int i = 0; i < SIZE; ++i) {
            Entry* p = table[i];
            while (p) {
                Entry* n = p->next;
                delete p;
                p = n;
            }
        }
    }

    void put(const Book& b) {
        int h = hashFunc(b.id);
        Entry* p = table[h];
        while (p) {
            if (p->key == b.id) {
                p->book = b;
                return;
            }
            p = p->next;
        }
        Entry* e = new Entry(b.id, b);
        e->next = table[h];
        table[h] = e;
    }

    bool get(int key, Book& result) const {
        int h = hashFunc(key);
        Entry* p = table[h];
        while (p) {
            if (p->key == key) {
                result = p->book;
                return true;
            }
            p = p->next;
        }
        return false;
    }

    void erase(int key) {
        int h = hashFunc(key);
        Entry* p = table[h];
        Entry* prev = nullptr;
        while (p) {
            if (p->key == key) {
                if (prev) prev->next = p->next;
                else table[h] = p->next;
                delete p;
                return;
            }
            prev = p;
            p = p->next;
        }
    }
};

BST bst;
HashTable hashTable;

string trim(const string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

string urlDecode(const string& s) {
    string out;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '+') out += ' ';
        else if (s[i] == '%' && i + 2 < s.size()) {
            string hex = s.substr(i + 1, 2);
            char c = static_cast<char>(strtol(hex.c_str(), nullptr, 16));
            out += c;
            i += 2;
        } else out += s[i];
    }
    return out;
}

vector<pair<string,string>> parseForm(const string& body) {
    vector<pair<string,string>> result;
    string part;
    stringstream ss(body);
    while (getline(ss, part, '&')) {
        size_t eq = part.find('=');
        if (eq == string::npos) continue;
        result.push_back({urlDecode(part.substr(0, eq)), urlDecode(part.substr(eq + 1))});
    }
    return result;
}

string getField(const vector<pair<string,string>>& fields, const string& key) {
    for (auto& p : fields) if (p.first == key) return p.second;
    return "";
}

void saveData() {
    ofstream f("books.txt");
    for (const Book& b : bst.getAll()) {
        f << b.id << "|" << b.title << "|" << b.author << "|" << b.category
          << "|" << (b.issued ? 1 : 0) << "|" << b.memberId << "\n";
    }
}

void loadData() {
    ifstream f("books.txt");
    if (!f) return;
    string line;
    while (getline(f, line)) {
        stringstream ss(line);
        string x;
        vector<string> p;
        while (getline(ss, x, '|')) p.push_back(x);
        if (p.size() != 6) continue;
        Book b;
        b.id = stoi(p[0]);
        b.title = p[1];
        b.author = p[2];
        b.category = p[3];
        b.issued = (p[4] == "1");
        b.memberId = stoi(p[5]);
        bst.insert(b);
        hashTable.put(b);
    }
}

string bookJson(const Book& b) {
    stringstream ss;
    ss << "{\"id\":" << b.id
       << ",\"title\":\"" << jsonEscape(b.title)
       << "\",\"author\":\"" << jsonEscape(b.author)
       << "\",\"category\":\"" << jsonEscape(b.category)
       << "\",\"issued\":" << (b.issued ? "true" : "false")
       << ",\"memberId\":" << b.memberId << "}";
    return ss.str();
}

string allBooksJson() {
    vector<Book> books = bst.getAll();
    stringstream ss;
    ss << "[";
    for (size_t i = 0; i < books.size(); ++i) {
        if (i) ss << ",";
        ss << bookJson(books[i]);
    }
    ss << "]";
    return ss.str();
}

string jsonMessage(bool ok, const string& message) {
    return string("{\"success\":") + (ok ? "true" : "false") +
           ",\"message\":\"" + jsonEscape(message) + "\"}";
}

string handleRequest(const string& method, const string& path, const string& body) {
    if (method == "GET" && path == "/api/books") return allBooksJson();

    if (method == "GET" && path.rfind("/api/books/", 0) == 0) {
        int id = stoi(path.substr(11));
        Book b;
        // Search using Hash Table to demonstrate O(1) average lookup.
        if (hashTable.get(id, b)) return bookJson(b);
        return jsonMessage(false, "Book not found");
    }

    if (method == "POST" && path == "/api/books") {
        auto fields = parseForm(body);
        try {
            Book b;
            b.id = stoi(getField(fields, "id"));
            b.title = trim(getField(fields, "title"));
            b.author = trim(getField(fields, "author"));
            b.category = trim(getField(fields, "category"));
            b.issued = false;
            b.memberId = 0;

            if (b.id <= 0 || b.title.empty() || b.author.empty()) 
                return jsonMessage(false, "ID, title and author are required");

            Book old;
            if (hashTable.get(b.id, old))
                return jsonMessage(false, "Book ID already exists");

            bst.insert(b);
            hashTable.put(b);
            saveData();
            return jsonMessage(true, "Book added successfully");
        } catch (...) {
            return jsonMessage(false, "Invalid book data");
        }
    }

    if (method == "DELETE" && path.rfind("/api/books/", 0) == 0) {
        try {
            int id = stoi(path.substr(11));
            Book b;
            if (!hashTable.get(id, b)) return jsonMessage(false, "Book not found");
            if (b.issued) return jsonMessage(false, "Return the book before deleting it");
            bst.remove(id);
            hashTable.erase(id);
            saveData();
            return jsonMessage(true, "Book deleted successfully");
        } catch (...) {
            return jsonMessage(false, "Invalid book ID");
        }
    }

    if (method == "POST" && path.rfind("/api/issue/", 0) == 0) {
        try {
            int id = stoi(path.substr(11));
            auto fields = parseForm(body);
            int member = stoi(getField(fields, "memberId"));
            Book b;
            if (!hashTable.get(id, b)) return jsonMessage(false, "Book not found");
            if (b.issued) return jsonMessage(false, "Book is already issued");
            b.issued = true;
            b.memberId = member;
            bst.insert(b);
            hashTable.put(b);
            saveData();
            return jsonMessage(true, "Book issued successfully");
        } catch (...) {
            return jsonMessage(false, "Invalid issue request");
        }
    }

    if (method == "POST" && path.rfind("/api/return/", 0) == 0) {
        try {
            int id = stoi(path.substr(12));
            Book b;
            if (!hashTable.get(id, b)) return jsonMessage(false, "Book not found");
            if (!b.issued) return jsonMessage(false, "Book is not currently issued");
            b.issued = false;
            b.memberId = 0;
            bst.insert(b);
            hashTable.put(b);
            saveData();
            return jsonMessage(true, "Book returned successfully");
        } catch (...) {
            return jsonMessage(false, "Invalid return request");
        }
    }

    return jsonMessage(false, "API route not found");
}

string contentType(const string& path) {
    if (path.size() >= 5 && path.substr(path.size()-5) == ".html") return "text/html";
    if (path.size() >= 4 && path.substr(path.size()-4) == ".css") return "text/css";
    if (path.size() >= 3 && path.substr(path.size()-3) == ".js") return "application/javascript";
    return "text/plain";
}

string readFile(const string& filename) {
    ifstream f(filename, ios::binary);
    if (!f) return "";
    stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

void sendResponse(SOCKET client, const string& status, const string& type, const string& body) {
    string response = "HTTP/1.1 " + status + "\r\n"
        "Content-Type: " + type + "; charset=utf-8\r\n"
        "Content-Length: " + to_string(body.size()) + "\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Connection: close\r\n\r\n" + body;
    send(client, response.c_str(), (int)response.size(), 0);
}

void serveClient(SOCKET client) {
    char buffer[65536]{};
    int received = recv(client, buffer, sizeof(buffer)-1, 0);
    if (received <= 0) {
        closesocket(client);
        return;
    }

    string req(buffer, received);
    size_t firstSpace = req.find(' ');
    size_t secondSpace = req.find(' ', firstSpace + 1);
    if (firstSpace == string::npos || secondSpace == string::npos) {
        closesocket(client);
        return;
    }

    string method = req.substr(0, firstSpace);
    string path = req.substr(firstSpace + 1, secondSpace - firstSpace - 1);

    size_t headerEnd = req.find("\r\n\r\n");
    string body = headerEnd == string::npos ? "" : req.substr(headerEnd + 4);

    if (path.rfind("/api/", 0) == 0) {
        string result = handleRequest(method, path, body);
        sendResponse(client, "200 OK", "application/json", result);
    } else {
        if (path == "/") path = "/index.html";
        string file = "public" + path;
        string data = readFile(file);
        if (data.empty()) sendResponse(client, "404 Not Found", "text/plain", "File not found");
        else sendResponse(client, "200 OK", contentType(file), data);
    }

    closesocket(client);
}

int main() {
    loadData();

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        cerr << "WSAStartup failed.\n";
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "Socket creation failed.\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Bind failed. Is port 8080 already in use?\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, 10) == SOCKET_ERROR) {
        cerr << "Listen failed.\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "\n============================================\n";
    cout << " Library Management System - C++ Backend\n";
    cout << " BST + Custom Hash Table\n";
    cout << "============================================\n";
    cout << "Server running at: http://localhost:8080\n";
    cout << "Press Ctrl+C to stop.\n\n";

    while (true) {
        SOCKET client = accept(serverSocket, nullptr, nullptr);
        if (client != INVALID_SOCKET) serveClient(client);
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
