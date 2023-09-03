#include "FileReader.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "sqlite3.h" 
#include <filesystem>


namespace fs = std::filesystem;

 

int FileReader::processFile(const std::string filename)
{
	std::ifstream file(filename);
	if (!file)
	{
		std::cout << "[" << std::this_thread::get_id() << "]"
			<< "Failed to open file: " << filename << std::endl;
		return 1;
	}
	// Open SQLite database
	sqlite3* db;
	int rc = sqlite3_open("myonedb.db", &db);
	if (rc)
	{
		std::cout << "[" << std::this_thread::get_id() << "]"
			<< "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
		return 1;
	}

	// Check if stock_name table exists
	std::string checkTableQuery = "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='stock_name';";
	sqlite3_stmt* stmt;
	rc = sqlite3_prepare_v2(db, checkTableQuery.c_str(), -1, &stmt, nullptr);

	if (rc != SQLITE_OK)
	{
		std::cout << "[" << std::this_thread::get_id() << "]"
			<< "Error executing query: " << sqlite3_errmsg(db) << std::endl;
		sqlite3_close(db);
		return 1;
	}

	rc = sqlite3_step(stmt);
	int result = sqlite3_column_int(stmt, 0);
	sqlite3_finalize(stmt);

	if (result == 0)
	{
		// Create stock_name table
		std::string createTableQuery = "CREATE TABLE stock_name (id varchar(20) PRIMARY KEY, name text(50),update_time text(20),upload_state varchar(20) );";
		rc = sqlite3_exec(db, createTableQuery.c_str(), nullptr, nullptr, nullptr);

		if (rc != SQLITE_OK)
		{
			std::cout << "[" << std::this_thread::get_id() << "]"
				<< "Error executing query: " << sqlite3_errmsg(db) << std::endl;
			sqlite3_close(db);
			return 1;
		}

		std::cout << "[" << std::this_thread::get_id() << "]"
			<< "Created stock_name table." << std::endl;
	}

	std::string line;
	while (std::getline(file, line))
	{
		// Process each line
		std::istringstream iss(line);

		std::string column1, column2, column3;
		std::getline(iss, column1, '|');
		std::getline(iss, column2, '|');
		std::getline(iss, column3, '|');

		if ("HEADER" == column1 || "TRAILER" == column1)
			continue;

		// Check if SecurityID exists in stock_name table
		std::string query = "SELECT id, name FROM stock_name WHERE id = " + column2 + ";";
		sqlite3_stmt* stmt;
		rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);

		if (rc != SQLITE_OK)
		{
			std::cout << "[" << std::this_thread::get_id() << "]"
				<< "Error executing query: " << sqlite3_errmsg(db) << std::endl;
			sqlite3_close(db);
			return 1;
		}

		if (sqlite3_step(stmt) == SQLITE_ROW)
		{
			std::string id(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
			std::string name(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));

			if (name != column3)
			{
				// Update SymbolName in stock_name table
				query = "UPDATE stock_name SET name = '" + column3 + "' WHERE id = " + id + ";";
				rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, nullptr);

				if (rc != SQLITE_OK)
				{
					std::cout << "[" << std::this_thread::get_id() << "]"
						<< "Error executing query: " << sqlite3_errmsg(db) << std::endl;
					sqlite3_close(db);
					return 1;
				}

				std::cout << "[" << std::this_thread::get_id() << "]"
					<< "Updated record  SecurityID: " << column2 << ", SymbolName: " << name << " to " << column3 << std::endl;
			}
		}
		else
		{
			// Insert new record into stock_name table
			query = "INSERT INTO stock_name (id, name) VALUES (" + column2 + ", '" + column3 + "');";
			rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, nullptr);

			if (rc != SQLITE_OK)
			{
				std::cout << "[" << std::this_thread::get_id() << "]"
					<< "Error executing query: " << sqlite3_errmsg(db) << std::endl;
				sqlite3_close(db);
				return 1;
			}

			std::cout << "[" << std::this_thread::get_id() << "]"
				<< "Inserted new record into stock_name table. SecurityID: " << column2 << ", SymbolName: " << column3 << std::endl;
		}

		sqlite3_finalize(stmt);
	}

	// 查询并打印数据
	// const char* selectQuery = "SELECT id,name FROM stock_name";
	// rc = sqlite3_exec(db, selectQuery, [](void* data, int argc, char** argv, char** columnNames) {
	//    std::cout << "id: " << argv[0] << ",name: "<<argv[1]<<std::endl;
	//    return 0;
	//    }, 0, 0);

	// Close SQLite database
	sqlite3_close(db);
	return 0;
}
