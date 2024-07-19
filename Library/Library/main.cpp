#include <iostream>
#include <string>
#include <vector>
#include "sqlite3.h"
#include <fstream>


class DatabaseManager {
protected:
    sqlite3* db;
    char* errmsg = nullptr;
    std::string filename;

    static int callback(void* NotUsed, int argc, char** argv, char** azColName) {
        for (int i = 0; i < argc; i++) {
            std::cout << azColName[i] << ": " << (argv[i] ? argv[i] : "NULL") << std::endl;
        }
        std::cout << std::endl;
        return 0;
    }

public:
    DatabaseManager(const std::string& dbFile) : filename(dbFile), db(nullptr), errmsg(nullptr) {
        if (sqlite3_open(filename.c_str(), &db) != SQLITE_OK) {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        }
    }

    virtual ~DatabaseManager() {
        if (db) {
            sqlite3_close(db);
        }
    }
};

class FolderManager : public DatabaseManager {
public:
    FolderManager(const std::string& dbFile) : DatabaseManager(dbFile) {
        create_table();
    }
    void open_folder(int folderId) {
        // Display folder details
        std::string sql = "SELECT * FROM Folders WHERE ID = " + std::to_string(folderId) + ";";
        std::cout << "Folder Details:\n";
        execute_query(sql.c_str());

        // Display only book names and IDs in this folder
        std::cout << "Books in this Folder:\n";
        sql = "SELECT ID, BookName FROM Books WHERE FolderID = " + std::to_string(folderId) + ";";
        execute_query(sql.c_str());
    }

    void delete_folder(int folderId) {
        std::string sql = "DELETE FROM Folders WHERE ID = " + std::to_string(folderId) + ";";
        execute_query(sql.c_str());
        std::cout << "Folder (and related books) deleted successfully!\n";
    }
    void list_all_folders() {
        const char* sql = "SELECT ID, FolderName FROM Folders;";
        execute_query(sql);
    }

    void create_table() {
        const char* sql = "CREATE TABLE IF NOT EXISTS Folders ("
            "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
            "FolderName TEXT NOT NULL);";
        execute_query(sql);
    }

    void add_folder(const std::string& folderName) {
        std::string sql = "INSERT INTO Folders (FolderName) VALUES ('" + folderName + "');";
        execute_query(sql.c_str());
    }

private:
    void execute_query(const char* sql) {
        if (sqlite3_exec(db, sql, callback, 0, &errmsg) != SQLITE_OK) {
            std::cerr << "SQL error: " << errmsg << std::endl;
            sqlite3_free(errmsg);
        }
    }
};

class BookManager : public DatabaseManager {
public:
    BookManager(const std::string& dbFile) : DatabaseManager(dbFile) {
        create_table();
    }
    void browse_books_in_folder(int folderId) {
        std::string sql = "SELECT * FROM Books WHERE FolderID = " + std::to_string(folderId) + ";";
        execute_query(sql.c_str());
    }
    void delete_book(int bookId) {
        std::string sql = "DELETE FROM Books WHERE ID = " + std::to_string(bookId) + ";";
        execute_query(sql.c_str());
        std::cout << "Book deleted successfully!\n";
    }

    void list_all_books() {
        const char* sql = "SELECT ID, BookName, Author FROM Books;";
        execute_query(sql);
    }

    void create_table() {
        const char* sql = "CREATE TABLE IF NOT EXISTS Books ("
            "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
            "Author TEXT NOT NULL, "
            "BookName TEXT NOT NULL, "
            "Content TEXT, "
            "FolderID INTEGER, "
            "FOREIGN KEY(FolderID) REFERENCES Folders(ID));";
        execute_query(sql);
    }

    void add_book(const std::string& author, const std::string& bookName, const std::string& content, int folderId) {
        std::string sql = "INSERT INTO Books (Author, BookName, Content, FolderID) VALUES ('" +
            author + "', '" + bookName + "', '" + content + "', " + std::to_string(folderId) + ");";
        execute_query(sql.c_str());
    }

    void search_book_by_name(const std::string& bookName) {
        std::string sql = "SELECT * FROM Books WHERE BookName LIKE '%" + bookName + "%';";
        execute_query(sql.c_str());
    }

private:
    void execute_query(const char* sql) {
        if (sqlite3_exec(db, sql, callback, 0, &errmsg) != SQLITE_OK) {
            std::cerr << "SQL error: " << errmsg << std::endl;
            sqlite3_free(errmsg);
        }
    }
};

int main() {
    std::cout << "Welcome to the Book Management System!\n";

    FolderManager folderManager("library.db");
    BookManager bookManager("library.db");

    bool running = true;
    while (running) {
        std::cout << "\nMenu:\n"
            << "1. Add Folder\n"
            << "2. Add Book\n"
            << "3. Search Books by Name\n"
            << "4. Browse Books by Folder\n"
            << "5. List All Folders\n"
            << "6. List All Books\n"
            << "7. Delete Folder\n"
            << "8. Delete Book\n"
            << "9. Exit\n"
            << "10. Open Folder\n"
            << "Select an option: ";

        int choice, id;
        std::cin >> choice;

        switch (choice) {
        case 1: {
            std::cout << "Enter folder name: ";
            std::string folderName;
            std::cin.ignore();  // Clear newline character from the input buffer
            std::getline(std::cin, folderName);
            folderManager.add_folder(folderName);
            std::cout << "Folder added successfully!\n";
            break;
        }
        case 2: {
            std::cout << "Enter book details:\n";
            std::string author, bookName, filePath;
            int folderId;

            std::cout << "Author: ";
            std::cin.ignore();
            std::getline(std::cin, author);

            std::cout << "Book Name: ";
            std::getline(std::cin, bookName);

            std::cout << "Enter the file path for book content: ";
            std::getline(std::cin, filePath);

            std::ifstream file(filePath);
            std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            file.close();

            std::cout << "Folder ID: ";
            std::cin >> folderId;

            bookManager.add_book(author, bookName, content, folderId);
            std::cout << "Book added successfully!\n";
            break;
        }
        case 3: {
            std::cout << "Enter book name to search: ";
            std::string bookName;
            std::cin.ignore();
            std::getline(std::cin, bookName);

            std::cout << "Searching for books...\n";
            bookManager.search_book_by_name(bookName);
            break;
        }
        case 4: {
            std::cout << "Enter folder ID to browse books: ";
            int folderId;
            std::cin >> folderId;

            std::cout << "Books in the selected folder:\n";
            bookManager.browse_books_in_folder(folderId);
            break;
        }
        case 5:
            std::cout << "All Folders:\n";
            folderManager.list_all_folders();
            break;
        case 6:
            std::cout << "All Books:\n";
            bookManager.list_all_books();
            break;
        case 7:
            std::cout << "Enter folder ID to delete: ";
            std::cin >> id;
            folderManager.delete_folder(id);
            break;
        case 8:
            std::cout << "Enter book ID to delete: ";
            std::cin >> id;
            bookManager.delete_book(id);
            break;
        case 9:
            std::cout << "Exiting the application...\n";
            running = false;
            break;
        case 10: {
            std::cout << "Enter folder ID to open: ";
            int folderId;
            std::cin >> folderId;
            folderManager.open_folder(folderId);
            break;
        }

        default:
            std::cout << "Invalid option, please try again!\n";
        }
    }

    return 0;
}
