#include <iostream>
#include <fstream>
#include <string>
#include <locale>
#include <codecvt>
#include <set>

inline std::wstring to_wide_string(const std::string& input)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(input);
}

inline std::string to_byte_string(const std::wstring& input)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(input);
}

std::set<std::string> codes;

int main() {
    std::string fileName = "mktdt042.txt";
    std::ifstream inputFile(fileName, std::ios::binary); // 使用二进制模式打开文件

    if (!inputFile.is_open()) {
        std::cout << "Failed to open file." << std::endl;
        return 1;
    }

    std::ofstream outputFile("output.txt");

    if (!outputFile.is_open()) {
        std::cout << "Failed to open output file." << std::endl;
        return 1;
    }

    std::string line;

    std::wstring_convert<std::codecvt_utf16<wchar_t, 0x10ffff, std::little_endian>, wchar_t> utf16Converter;

    std::getline(inputFile, line); // 忽略第1行
    std::string last_line = "";
    bool repare_returned = false;

    while (true) {
       
        std::getline(inputFile, line);
        if (line.empty()) {
            std::cout << "读到了空行，结束 " << std::endl;
            break;
        }
        if(line.length() >= 7  && (strcmp(line.substr(0, 7).c_str(), "TRAILER") == 0)) {
            std::cout << "文件结束 了" << std::endl;
            break;
        }
        else if (repare_returned && (last_line.length() > 0))
        {
            line = last_line + line;
            last_line = "";
        }
        
        if (line.length() < 42)
        {
            if(repare_returned)
                last_line = line;
            continue;
        }

        
        std::string dataType = line.substr(0, 5);
        if (dataType[0] != 'M')
            continue;



        std::string securityCode = line.substr(6, 5);

        if (codes.count(securityCode) == 1)
            continue;
        codes.insert(securityCode);

        std::string c1 = line.substr(12, 32);
        std::wstring securityName = utf16Converter.from_bytes(c1);

        if (securityCode.compare("00303") == 0)
        {
            std::cout << "zero";
        }

        

       

        securityName.erase(std::remove(securityName.begin(), securityName.end(), L'　'), securityName.end());

        // 在此处使用已删除右侧空格的字符串 `str`
        // 
        // 写入解码后的数据到文件
        std::string text(to_byte_string(securityName));
      
        if (text.empty()) {
            continue;
        }
        outputFile << dataType << " - " << securityCode << " - " << text << std::endl;
        
    }

    inputFile.close();
    outputFile.close();

    return 0;
}