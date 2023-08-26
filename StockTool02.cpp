#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <thread>
#include <chrono>
#include <ctime>
#include <mutex>
#include <csignal>
#include <functional>

#include "sqlite3.h"

//#include <stringapiset.h>
//#include <iconv.h>
//#include <iostream>
//#include <cstring>
//#include <cerrno>

//std::string utf8ToGbk(const std::string& utf8String) {
//	std::string gbkString;
//
//	iconv_t cd = iconv_open("GBK", "UTF-8");
//	if (cd == (iconv_t)-1) {
//		std::cerr << "Failed to open iconv: " << strerror(errno) << std::endl;
//		return gbkString;
//	}
//
//	size_t inBytesLeft = utf8String.length();
//	char* inBuf = const_cast<char*>(utf8String.c_str());
//
//	size_t outBytesLeft = inBytesLeft * 2; // 预估输出缓冲区大小为输入的两倍
//	char* outBuf = new char[outBytesLeft];
//	char* outBufStart = outBuf;
//
//	size_t result = iconv(cd, &inBuf, &inBytesLeft, &outBuf, &outBytesLeft);
//	if (result == (size_t)-1) {
//		std::cerr << "Failed to convert: " << strerror(errno) << std::endl;
//		delete[] outBufStart;
//		iconv_close(cd);
//		return gbkString;
//	}
//
//	gbkString.assign(outBufStart, outBuf - outBufStart);
//
//	delete[] outBufStart;
//	iconv_close(cd);
//
//	return gbkString;
//}
//
//std::string gbkToUtf8(const std::string& gbkString) {
//	std::string utf8String;
//
//	iconv_t cd = iconv_open("UTF-8", "GBK");
//	if (cd == (iconv_t)-1) {
//		std::cerr << "Failed to open iconv: " << strerror(errno) << std::endl;
//		return utf8String;
//	}
//
//	size_t inBytesLeft = gbkString.length();
//	char* inBuf = const_cast<char*>(gbkString.c_str());
//
//	size_t outBytesLeft = inBytesLeft * 4; // 预估输出缓冲区大小为输入的四倍
//	char* outBuf = new char[outBytesLeft];
//	char* outBufStart = outBuf;
//
//	size_t result = iconv(cd, &inBuf, &inBytesLeft, &outBuf, &outBytesLeft);
//	if (result == (size_t)-1) {
//		std::cerr << "Failed to convert: " << strerror(errno) << std::endl;
//		delete[] outBufStart;
//		iconv_close(cd);
//		return utf8String;
//	}
//
//	utf8String.assign(outBufStart, outBuf - outBufStart);
//
//	delete[] outBufStart;
//	iconv_close(cd);
//
//	return utf8String;
//}


// Global variables
std::mutex mtx;
bool stopProcessing = false;
volatile std::sig_atomic_t stop_flag = 0;

void signalHandler(int signal) {
	stop_flag = 1;
}

// Function to check if a file exists
bool fileExists(const std::string& filename) {
	std::ifstream file(filename);
	return file.good();
}

// Function to read and process the file
void processFile(const std::string& filename) {
	std::ifstream file(filename);
	if (!file) {
		std::cout << "[" << std::this_thread::get_id() << "]" << "Failed to open file: " << filename << std::endl;
		return;
	}
	// Open SQLite database
	sqlite3* db;
	int rc = sqlite3_open("mydb.db", &db);
	if (rc) {
		std::cout << "[" << std::this_thread::get_id() << "]" << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
		return;
	}

	// Check if stock_name table exists
	std::string checkTableQuery = "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='stock_name';";
	sqlite3_stmt* stmt;
	rc = sqlite3_prepare_v2(db, checkTableQuery.c_str(), -1, &stmt, nullptr);

	if (rc != SQLITE_OK) {
		std::cout << "[" << std::this_thread::get_id() << "]" << "Error executing query: " << sqlite3_errmsg(db) << std::endl;
		sqlite3_close(db);
		return;
	}

	rc = sqlite3_step(stmt);
	int result = sqlite3_column_int(stmt, 0);
	sqlite3_finalize(stmt);

	if (result == 0) {
		// Create stock_name table
		std::string createTableQuery = "CREATE TABLE stock_name (id varchar(20) PRIMARY KEY, name text(50),update_time text(20),upload_state varchar(20);";
		rc = sqlite3_exec(db, createTableQuery.c_str(), nullptr, nullptr, nullptr);

		if (rc != SQLITE_OK) {
			std::cout << "[" << std::this_thread::get_id() << "]" << "Error executing query: " << sqlite3_errmsg(db) << std::endl;
			sqlite3_close(db);
			return;
		}

		std::cout << "[" << std::this_thread::get_id() << "]" << "Created stock_name table." << std::endl;
	}


	std::string line;
	while (std::getline(file, line)) {
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

		if (rc != SQLITE_OK) {
			std::cout << "[" << std::this_thread::get_id() << "]" << "Error executing query: " << sqlite3_errmsg(db) << std::endl;
			sqlite3_close(db);
			return;
		}

		if (sqlite3_step(stmt) == SQLITE_ROW) {
			std::string id(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
			std::string name(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));

			if (name != column3) {
				// Update SymbolName in stock_name table
				query = "UPDATE stock_name SET name = '" + column3 + "' WHERE id = " + id + ";";
				rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, nullptr);

				if (rc != SQLITE_OK) {
					std::cout << "[" << std::this_thread::get_id() << "]" << "Error executing query: " << sqlite3_errmsg(db) << std::endl;
					sqlite3_close(db);
					return;
				}

				std::cout << "[" << std::this_thread::get_id() << "]" << "Updated record  SecurityID: " << column2 << ", SymbolName: " << name << " to " << column3 << std::endl;
			}
		}
		else {
			// Insert new record into stock_name table
			query = "INSERT INTO stock_name (id, name) VALUES (" + column2 + ", '" + column3 + "');";
			rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, nullptr);

			if (rc != SQLITE_OK) {
				std::cout << "[" << std::this_thread::get_id() << "]" << "Error executing query: " << sqlite3_errmsg(db) << std::endl;
				sqlite3_close(db);
				return;
			}

			std::cout << "[" << std::this_thread::get_id() << "]" << "Inserted new record into stock_name table. SecurityID: " << column2 << ", SymbolName: " << column3 << std::endl;
		}

		sqlite3_finalize(stmt);

	}


	// 查询并打印数据
	//const char* selectQuery = "SELECT id,name FROM stock_name";
	//rc = sqlite3_exec(db, selectQuery, [](void* data, int argc, char** argv, char** columnNames) {
	//    std::cout << "id: " << argv[0] << ",name: "<<argv[1]<<std::endl;
	//    return 0;
	//    }, 0, 0);

	// Close SQLite database
	sqlite3_close(db);
}

// Function to monitor and process the configuration file
/**
Error	C4996	'localtime': This function or variable may be unsafe.Consider using localtime_s instead.
To disable deprecation, use _CRT_SECURE_NO_WARNINGS.See

**/
void monitorConfigFile() {
	std::time_t startTime = 0, endTime = 0;
	while (true) {
		std::this_thread::sleep_for(std::chrono::seconds(10));

		// Check if it's time to start processing
		std::time_t now = std::time(nullptr);
		std::tm* tm = std::localtime(&now);
		int hour = tm->tm_hour;
		int minute = tm->tm_min;
		int second = tm->tm_sec;
		int year = tm->tm_year;
		int month = tm->tm_mon;
		int day = tm->tm_mday;

		// Read start and end times from configuration file
		std::ifstream configFile("StockTool.conf");
		std::string line;
		bool processFiles = false;

		while (std::getline(configFile, line)) {
			if (line.find("[timer]") != std::string::npos) {

				std::getline(configFile, line);  // Read the next line after "[timer]"

				while (line.find("[") == std::string::npos) {
					std::string key, value;
					std::istringstream iss(line);
					if (iss >> key >> value) {
						if (value == "=")
							iss >> value;
						if (key == "start_time") {
							std::tm startTm{};
							std::istringstream iss(value);
							iss >> std::get_time(&startTm, "%H:%M");
							startTm.tm_year = year;
							startTm.tm_mon = month;
							startTm.tm_mday = day;

							startTime = std::mktime(&startTm);
							//std::cout << iss.str() << "," << startTime << std::endl;

						}
						else if (key == "end_time") {
							std::tm endTm{};
							std::istringstream iss(value);
							iss >> std::get_time(&endTm, "%H:%M");
							endTm.tm_year = year;
							endTm.tm_mon = month;
							endTm.tm_mday = day;
							endTime = std::mktime(&endTm);
							//std::cout << iss.str() << "," << endTime << std::endl;
						}
					}
					if (!std::getline(configFile, line)) {
						break;  // Exit the loop when there are no more lines to read
					}
				}
				// break;  // Exit the loop after processing the "timer" section
			}
		}

		if (now >= startTime && now < endTime) {
			if (!processFiles) {
				std::cout << "[" << std::this_thread::get_id() << "]" << "Processing started at: " << hour << ":" << minute << ":" << second << std::endl;
				processFiles = true;
			}

			// Read filenames from configuration file
			configFile.clear();  // Clear the end-of-file flag
			configFile.seekg(0, std::ios::beg);  // Reset file pointer to the beginning

			std::vector<std::string> filenames;
			bool foundFilenames = false;
			while (std::getline(configFile, line)) {
				if (line.empty()) {
					continue;  // 跳过空行
				}
				else if (line[0] == '[') {
					if (line.find("[filename]") != std::string::npos) {
						foundFilenames = true;
					}
					else {
						foundFilenames = false;
						break;  // 遇到新的配置项，结束文件路径配置识别
					}
				}
				else if (foundFilenames) {
					filenames.push_back(line);
				}
			}

			// Process each file
			for (const std::string& filename : filenames) {
				if (fileExists(filename)) {
					std::cout << "[" << std::this_thread::get_id() << "]" << "Processing file: " << filename <<"  Begin."<< std::endl;
					processFile(filename);
					std::cout << "[" << std::this_thread::get_id() << "]" << "Processing file: " << filename <<"  End. "<< std::endl;
				}
				else {
					std::cout << "File not found: " << filename << std::endl;
				}
			}
		}
		else {
			std::time_t t2 = std::time(nullptr);
			std::cout << "[" << std::this_thread::get_id() << "]" << std::put_time(std::localtime(&t2), "%Y-%m-%d %H.%M.%S") << "  check timer."<<std::endl;
			if (processFiles) {
				std::cout << "["<<std::this_thread::get_id()<<"]" << "Processing stopped at: " << hour << ":" << minute << ":" << second << std::endl;
				processFiles = false;
			}
		}

		if (stopProcessing)
			break;
	}
}


int main() {
	// 注册SIGTERM信号处理函数
	std::signal(SIGTERM, signalHandler);

	std::thread configFileThread(monitorConfigFile);

	while (!stop_flag) {
		// Wait for the user to stop the program or receive a signal
		std::string input;
		while (std::cin >> input) {
			if (input == "stop") {
				stop_flag = 1;
				break;
			}
		}
	}

	configFileThread.join();

	return 0;
}

/*
	  std::string utf8String = "你好，世界！";
	   std::string gbkString = utf8ToGbk(utf8String);
	   std::string convertedUtf8String = gbkToUtf8(gbkString);
	   std::cout << "GBK: " << gbkString << std::endl;
	   std::cout << "UTF-8: " << convertedUtf8String << std::endl;
	   return 0;

*/