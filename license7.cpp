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

    // ��stock_nameת��ΪUTF-16LE�����wstring
    std::wstring_convert<std::codecvt_utf16<wchar_t, 0x10ffff, std::little_endian>> converter;
    std::wstring wstr = converter.from_bytes(reinterpret_cast<const char*>(stock_name), reinterpret_cast<const char*>(stock_name + sizeof(stock_name)));

    // ��ӡ�����ַ���
    std::wcout << wstr << std::endl;

    // �������ַ���д������ļ�
    std::ofstream file("stock_name.txt");
    file << converter.to_bytes(wstr);
    file.close();

    return 0;
}