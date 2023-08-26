#include <iostream>
#include <string>
#include <openssl/md5.h>
#include <openssl/aes.h>
#include <sstream>
#include <iomanip>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")
#else
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
 

// MD5加密函数
std::string md5(const std::string& str) {
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, str.c_str(), str.length());

    unsigned char result[MD5_DIGEST_LENGTH];
    MD5_Final(result, &ctx);

    std::ostringstream sout;
    sout << std::hex << std::setfill('0');
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
        sout << std::setw(2) << static_cast<unsigned int>(result[i]);
    }

    return sout.str();
}

// AES128加密函数
std::string encryptAES(const std::string& plaintext, const std::string& key, const std::string& iv) {
    AES_KEY aesKey;
    AES_set_encrypt_key(reinterpret_cast<const unsigned char*>(key.c_str()), 128, &aesKey);

    unsigned char encrypted[AES_BLOCK_SIZE];
    memset(encrypted, 0, AES_BLOCK_SIZE);

    AES_cbc_encrypt(reinterpret_cast<const unsigned char*>(plaintext.c_str()), encrypted, plaintext.length(),
        &aesKey, reinterpret_cast<unsigned char*>(const_cast<char*>(iv.c_str())), AES_ENCRYPT);

    return std::string(reinterpret_cast<char*>(encrypted), AES_BLOCK_SIZE);
}

// AES128解密函数
std::string decryptAES(const std::string& ciphertext, const std::string& key, const std::string& iv) {
    AES_KEY aesKey;
    AES_set_decrypt_key(reinterpret_cast<const unsigned char*>(key.c_str()), 128, &aesKey);

    unsigned char decrypted[AES_BLOCK_SIZE];
    memset(decrypted, 0, AES_BLOCK_SIZE);

    AES_cbc_encrypt(reinterpret_cast<const unsigned char*>(ciphertext.c_str()), decrypted, ciphertext.length(),
        &aesKey, reinterpret_cast<unsigned char*>(const_cast<char*>(iv.c_str())), AES_DECRYPT);

    return std::string(reinterpret_cast<char*>(decrypted), AES_BLOCK_SIZE);
}

// 获取本机MAC地址列表
std::vector<std::string> getMacAddresses() {
    std::vector<std::string> macAddresses;

#ifdef _WIN32
    IP_ADAPTER_INFO* adapterInfo = nullptr;
    ULONG bufferSize = 0;

    // 获取缓冲区大小
    if (GetAdaptersInfo(adapterInfo, &bufferSize) == ERROR_BUFFER_OVERFLOW) {
        adapterInfo = reinterpret_cast<IP_ADAPTER_INFO*>(new char[bufferSize]);
        if (GetAdaptersInfo(adapterInfo, &bufferSize) == ERROR_SUCCESS) {
            for (IP_ADAPTER_INFO* adapter = adapterInfo; adapter != nullptr; adapter = adapter->Next) {
                std::string str(reinterpret_cast<char*>(adapter->Address), 8);                
                macAddresses.push_back(str);
            }
        }
        delete[] reinterpret_cast<char*>(adapterInfo);
    }
#else
    struct ifaddrs* ifAddrStruct = nullptr;

    if (getifaddrs(&ifAddrStruct) == -1) {
        return macAddresses;
    }

    for (struct ifaddrs* ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) {
            continue;
        }

        if (ifa->ifa_addr->sa_family == AF_PACKET) {
            struct sockaddr_ll* sa = reinterpret_cast<struct sockaddr_ll*>(ifa->ifa_addr);

            unsigned char mac[6];
            for (int i = 0; i < 6; ++i) {
                mac[i] = sa->sll_addr[i];
            }

            std::ostringstream sout;
            sout << std::hex << std::setfill('0');
            for (unsigned int i = 0; i < 6; ++i) {
                sout << std::setw(2) << static_cast<unsigned int>(mac[i]);
            }

            macAddresses.push_back(sout.str());
        }
    }

    freeifaddrs(ifAddrStruct);
#endif

    return macAddresses;
}

int main() {
    // 加密
    std::string plaintext = "hundsun";
    //std::string key = md5(plaintext);
    //std::cout << key <<  std::endl;

    std::string iv = "98-AF-65-1F-24-4E";
    std::string ciphertext = encryptAES(plaintext, key, iv);

    std::cout << "AES128密文: " << std::hex<< ciphertext.c_str() << std::endl;

    // 解密
    std::vector<std::string> macAddresses = getMacAddresses();
    bool isDecrypted = false;

    for (const std::string& mac : macAddresses) {
        std::cout << mac << std::endl;

        std::string decrypted = decryptAES(ciphertext, md5(mac), iv);
        if (plaintext == decrypted) {
            isDecrypted = true;
            break;
        }
    }

    std::cout << "解密结果: " << (isDecrypted ? "TRUE" : "FALSE") << std::endl;

    return 0;
}