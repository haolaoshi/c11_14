#include <iostream>
#include <string>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>  
#include <openssl/bio.h> 
#include <vector>
#include <regex>
#ifdef _WIN32
#include <Windows.h>
#include <iphlpapi.h>
#include <iomanip>
#include <sstream>
#pragma comment(lib, "iphlpapi.lib")
#else
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

// 获取本机MAC地址列表
std::vector<std::string> getMacAddresses() {
    std::vector<std::string> macAddresses;

#ifdef _WIN32
    
    IP_ADAPTER_INFO* adapterInfo = nullptr;
    ULONG bufferSize = 0;

    if (GetAdaptersInfo(adapterInfo, &bufferSize) == ERROR_BUFFER_OVERFLOW) {
        adapterInfo = reinterpret_cast<IP_ADAPTER_INFO*>(new char[bufferSize]);
        if (GetAdaptersInfo(adapterInfo, &bufferSize) == ERROR_SUCCESS) {
            for (IP_ADAPTER_INFO* adapter = adapterInfo; adapter != nullptr; adapter = adapter->Next) {
                std::stringstream ss;
                for (unsigned int i = 0; i < adapter->AddressLength; ++i) {
                    if (i > 0) {
                        ss << "-";
                    }
                    ss << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(adapter->Address[i]);
                }
                macAddresses.push_back(ss.str());
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


std::string AES128Encrypt(const std::string& plaintext, const std::string& key)
{
    const EVP_CIPHER* cipher = EVP_aes_128_cbc();
    const int keyLength = 128; // 使用128位密钥
    const int ivLength = EVP_CIPHER_iv_length(cipher); // 获取IV长度

    // 生成随机IV
    // 分配动态内存
    //unsigned char* iv = (unsigned char*)malloc(ivLength);
    //// 或者使用：unsigned char* iv = new unsigned char[ivLength];
    //if (RAND_bytes(iv, ivLength) != 1) {
    //    throw std::runtime_error("Failed to generate random IV");
    //}
    //for (int i = 0; i < ivLength; i++) iv[i] = (unsigned char)(i % 10);
    //iv[ivLength] = '\0';
    unsigned char iv[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 ,0,1,2,3,4,5};

    // 初始化加密上下文
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, cipher, NULL, (const unsigned char*)key.c_str(), iv);

    // 设置加密输入长度为明文长度加上一个加密块长度
    int plaintextLength = plaintext.length();
    int ciphertextLength = (plaintextLength / AES_BLOCK_SIZE + 1) * AES_BLOCK_SIZE;
    unsigned char* ciphertext = new unsigned char[ciphertextLength];

    // 执行加密操作
    int len;
    EVP_EncryptUpdate(ctx, ciphertext, &len, (const unsigned char*)plaintext.c_str(), plaintextLength);
    ciphertextLength = len;

    // 完成加密操作
    EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
    ciphertextLength += len;

    // 将IV和加密后的数据拼接在一起
    std::string encryptedText;
    encryptedText.append(reinterpret_cast<const char*>(iv), ivLength);
    encryptedText.append(reinterpret_cast<const char*>(ciphertext), ciphertextLength);

    // 清除加密上下文并释放内存
    EVP_CIPHER_CTX_free(ctx);
    delete[] ciphertext;

    return encryptedText;
}

std::string AES128Decrypt(const std::string& ciphertext, const std::string& key)
{
    const EVP_CIPHER* cipher = EVP_aes_128_cbc();
    const int keyLength = 128; // 使用128位密钥
    const int ivLength = EVP_CIPHER_iv_length(cipher); // 获取IV长度

    // 从密文中提取IV和加密数据
    const unsigned char* iv = reinterpret_cast<const unsigned char*>(ciphertext.data());
    const unsigned char* ciphertextData = reinterpret_cast<const unsigned char*>(ciphertext.data() + ivLength);
    int ciphertextLength = ciphertext.length() - ivLength;

    // 初始化解密上下文
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, cipher, NULL, (const unsigned char*)key.c_str(), iv);

    // 设置解密输入长度为密文长度
    int decryptedTextLength = ciphertextLength;
    unsigned char* decryptedText = new unsigned char[decryptedTextLength];

    // 执行解密操作
    int len;
    EVP_DecryptUpdate(ctx, decryptedText, &len, ciphertextData, ciphertextLength);
    decryptedTextLength = len;

    // 完成解密操作
    EVP_DecryptFinal_ex(ctx, decryptedText + len, &len);
    decryptedTextLength += len;

    // 清除解密上下文并释放内存
    EVP_CIPHER_CTX_free(ctx);

    std::string decryptedTextStr((char*)decryptedText, decryptedTextLength);
    delete[] decryptedText;

    return decryptedTextStr;
}
 


std::string base64_encode(const std::string& input)
{
    BIO* bio, * b64;
    BUF_MEM* bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, input.c_str(), input.length());
    BIO_flush(bio);

    BIO_get_mem_ptr(bio, &bufferPtr);
    std::string encodedString(bufferPtr->data, bufferPtr->length);

    BIO_free_all(bio);

    return encodedString;
}
 

std::string base64_decode(const std::string& encodedText) {
    BIO* bio = BIO_new(BIO_f_base64());
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

    BIO* bmem = BIO_new_mem_buf(encodedText.c_str(), encodedText.length());
    bio = BIO_push(bio, bmem);

    char* buffer = new char[encodedText.length()];
    memset(buffer, 0, encodedText.length());

    BIO_read(bio, buffer, encodedText.length());
    BIO_free_all(bio);

    std::string decodedText(buffer);
    delete[] buffer;

    return decodedText;
}

int main1()
{
    //测试的原文
    std::string plaintext = "98-AF-65-1F-24-4E";
    std::string key = "hundsun";
    //密文
    std::string ciphertext = AES128Encrypt(plaintext, key);


    // 将密文转换为16进制表示
    //std::ostringstream oss;
    //for (char c : ciphertext) {
    //    oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(static_cast<unsigned char>(c));
    //}
    //std::string hexEncryptedText = oss.str();

    // 使用OpenSSL库中的base64_encode函数
    std::string encodedCiphertext = base64_encode(ciphertext); 
    std::cout  <<"base64:[" << encodedCiphertext <<"]"<< std::endl;

    //根据base64密文，先解码base64，再解密 
    //[xeal7/3IhQS9adnikhp8kriNL9ebW+SocJqThihAbdhU/vPuoKljH2WPMXBpQrDc]
    std::string decodedText = base64_decode(encodedCiphertext);
    std::string decryptedText = AES128Decrypt(ciphertext, key);
    std::cout << "DecryptedText: " << decryptedText << std::endl;

    // 获取本机的MAC地址，与解密后的原文进行比对
    std::vector<std::string> macAddresses = getMacAddresses();
    bool isDecrypted = false;

    for (const std::string& mac : macAddresses) {
        if (strcmp(mac.c_str(), decryptedText.c_str()) == 0) {
            isDecrypted = true;
            break;
        }
        std::cout << mac << std::endl;
    }
    //打印比对结果
    if (isDecrypted)
        std::cout << "你的电脑授权正常" << std::endl;
    else
        std::cout << "请申请授权" << std::endl;

    return 0;
}




bool isValidMacAddress(const std::string& macAddress) {
    std::regex macRegex("^([0-9A-Fa-f]{2}-){5}([0-9A-Fa-f]{2})$");
    return std::regex_match(macAddress, macRegex);
}

std::string generateKeyFromMac(const std::string& macAddress) {
    std::string key = "hundsun";
    //密文
    std::string ciphertext = AES128Encrypt(macAddress, key);

    // 使用OpenSSL库中的base64_encode函数
    std::string encodedCiphertext = base64_encode(ciphertext);
    std::cout << "base64:[" << encodedCiphertext << "]" << std::endl;
    return encodedCiphertext;
}

std::string decryptMacFromKey(const std::string& encodedCiphertext) {
    std::string key = "hundsun";
    //根据base64密文，先解码base64，再解密 
    //[xeal7/3IhQS9adnikhp8kriNL9ebW+SocJqThihAbdhU/vPuoKljH2WPMXBpQrDc]
    std::string decodedText = base64_decode(encodedCiphertext);
    std::cout << decodedText << std::endl;

    std::string decryptedText = AES128Decrypt(decodedText, key);
    std::cout << "DecryptedText: " << decryptedText << std::endl;
    return decryptedText;
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        //std::string argument = argv[1];
        std::string argument = "AAECAwQFBgcICQABAgMEBXx/Bhwc29UMYeEzRcL2usedZdbKeKKufH5Oo11D2BaV";
        if (argument.length() == 17) {
            if (isValidMacAddress(argument)) {
                std::cout << generateKeyFromMac(argument) << std::endl;
                return 0;
            }
            else {
                std::cout << "MAC地址格式必须为XX-XX-XX-XX-XX-XX." << std::endl;
                return 1;
            }
        }
        else /*if (argument.length() > 17)*/ {         

            std::string macAddress = decryptMacFromKey(argument);
            if (isValidMacAddress(macAddress)) {
                std::cout << macAddress << std::endl;
                return 0;
            }
            else {
                std::cout << "解码失败." << std::endl;
                return 1;
            }
        }
    }

    std::cout << "请指定一个原文/密文参数." << std::endl;
    return 1;
}