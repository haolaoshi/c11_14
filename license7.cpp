#include <iostream>
#include <fstream>
#include <codecvt>

int main() {
    unsigned char stock_name[32] = {
        0x7f, 0xe6, 0x99, 0xab, 0x54, 0x20, 0x30, 0x20,
        0x30, 0x20, 0x30, 0x20, 0x30, 0x20, 0x30, 0x20,
        0x30, 0x20, 0x30, 0x20, 0x30, 0x20, 0x30, 0x20,
        0x30, 0x20, 0x30, 0x20, 0x30, 0x20, 0x30, 0x20
    };

    // 将stock_name转换为UTF-16LE编码的wstring
    std::wstring_convert<std::codecvt_utf16<wchar_t, 0x10ffff, std::little_endian>> converter;
    std::wstring wstr = converter.from_bytes(reinterpret_cast<const char*>(stock_name), reinterpret_cast<const char*>(stock_name + sizeof(stock_name)));

    // 打印中文字符串
    std::wcout << wstr << std::endl;

    // 将中文字符串写入磁盘文件
    std::ofstream file("stock_name.txt");
    file << converter.to_bytes(wstr);
    file.close();

    return 0;
}