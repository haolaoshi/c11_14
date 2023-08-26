#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <sqlite3.h>
#ifdef DEBUG

#include <unistd.h> // for sleep
#endif // DEBUG
#include <thread>


// SQLite3 database file name
const std::string DB_FILE = "stocks.db";
const std::string DB_TABLE_NAME = "stockname";


// Stock structure
struct Stock {
    std::string code;
    std::string name;
    std::string updateTime;
    std::string uploadStatus;
};

// Function to check if a table exists in the database
bool doesTableExist(sqlite3* db) {
    std::string query = "SELECT 1 FROM sqlite_master WHERE type='table' AND name='"+ DB_TABLE_NAME+"';";
    sqlite3_stmt* stmt;
    int result = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        std::cerr << "Failed to execute query: " << sqlite3_errmsg(db) << std::endl;
    }

    bool tableExists = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        tableExists = true;
    }

    sqlite3_finalize(stmt);
    return tableExists;
}

// Function to create the stockname table
void createTable(sqlite3* db) {
    std::string query = "CREATE TABLE " + DB_TABLE_NAME + " (SecurityID TEXT PRIMARY KEY NOT NULL, Symbol TEXT NOT NULL,updateTime varchar(30),uploadStatus varchar(50);";
    char* errmsg;
    int result = sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errmsg);
    if (result != SQLITE_OK) {
        std::cerr << "Failed to create table: " << errmsg << std::endl;
        sqlite3_free(errmsg);
    }
}

// Function to insert stock data into the stockname table
void insertStock(sqlite3* db, const Stock& stock) {
    std::string query = "INSERT INTO " + DB_TABLE_NAME + " (SecurityID, Symbol) VALUES (?, ?);";
    sqlite3_stmt* stmt;
    int result = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        std::cerr << "Failed to execute query: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_bind_text(stmt, 1, stock.code.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, stock.name.c_str(), -1, SQLITE_STATIC);

    result = sqlite3_step(stmt);
    if (result != SQLITE_DONE) {
        std::cerr << "Failed to insert data: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
}

// Function to update stock data in the stockname table
void updateStock(sqlite3* db, const Stock& stock) {
    std::string query = "UPDATE " + DB_TABLE_NAME + " SET Symbol = ? WHERE SecurityID = ?;";
    sqlite3_stmt* stmt;
    int result = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        std::cerr << "Failed to execute query: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_bind_text(stmt, 1, stock.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, stock.code.c_str(), -1, SQLITE_STATIC);

    result = sqlite3_step(stmt);
    if (result != SQLITE_DONE) {
        std::cerr << "Failed to update data: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
}

// Function to connect to the SQLite3 database
sqlite3* connectToDatabase() {
    sqlite3* db;
    int result = sqlite3_open(DB_FILE.c_str(), &db);
    if (result != SQLITE_OK) {
        std::cerr << "Failed to open database: " << sqlite3_errmsg(db) << std::endl;
        return nullptr;
    }
    return db;
}

// Function to disconnect from the SQLite3 database
void disconnectFromDatabase(sqlite3* db) {
    sqlite3_close(db);
}

// Function to parse the text file and extract stock data
std::vector<Stock> parseTextFile(const std::string& filename) {
    std::vector<Stock> stocks;

    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return stocks;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string col1, col2;

        if (!(iss >> col1 >> col2)) {
            continue;
        }

        Stock stock;
        stock.code = col1;
        stock.name = col2;

        stocks.push_back(stock);
    }

    file.close();
    return stocks;
}

int main1() {
    sqlite3* db = connectToDatabase();
    if (!db) {
        return 1;
    }

    // Create stockname table if it doesn't exist
    if (!doesTableExist(db)) {
        createTable(db);
    }

    while (true) {
        std::vector<Stock> stocks = parseTextFile("mktdt00.txt");

        for (const auto& stock : stocks) {
            // Check if stock code exists in the table
            std::string query = "SELECT Symbol FROM " + DB_TABLE_NAME + " WHERE SecurityID = ?;";
            sqlite3_stmt* stmt;
            int result = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
            if (result != SQLITE_OK) {
                std::cerr << "Failed to execute query: " << sqlite3_errmsg(db) << std::endl;
            }

            sqlite3_bind_text(stmt, 1, stock.code.c_str(), -1, SQLITE_STATIC);

            bool stockExists = (sqlite3_step(stmt) == SQLITE_ROW);
            sqlite3_finalize(stmt);

            if (!stockExists) {
                // Insert new stock code and name
                insertStock(db, stock);
                std::cout << "Inserted new stock: " << stock.code << ", " << stock.name << std::endl;
            }
            else {
                // Compare stock name and update if different
                std::string storedName(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
                if (storedName != stock.name) {
                    updateStock(db, stock);
                    std::cout << "Updated stock: " << stock.code << ", " << stock.name << std::endl;
                }
            }
        }

        //sleep(10); // Pause for 10 seconds before processing the file again
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    disconnectFromDatabase(db);
    return 0;
}
#include <algorithm>
#include <numeric>

using namespace std;

int main()
{
    int iat[] = { 27,149,12,47,109,82,9 };
    int val = 83;
    int* result = find(begin(iat), end(iat), val);
    if (result == end(iat))
        cout << "not found" << endl;
    else
        cout << *result << endl;

    int sum = accumulate(begin(iat), end(iat), 0);
    cout << sum << endl;

    vector<string> vec;
    fill(vec.begin(), vec.end(), "haha");
    auto it = back_inserter(vec);
    *it = "hello";
    
    string str = accumulate(vec.begin(),vec.end(), string(""));
    cout << str << endl;


 
     

}