
#include <cstring>
#include <string>
#include <stdexcept>
#include <iostream>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <openssl/rand.h>
#include <vector>
#include <regex>

using namespace std;
static const std::string key = "hundsun000000000";
static const std::string iv = "1234567890123456";


std::string Base64Encode(const std::string& input) {
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





std::string AESEncrypt(const std::string& plaintext) {


    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), nullptr, (const unsigned char*)key.c_str(), (const unsigned char*)iv.c_str());

    // Determine the output buffer size
    int ciphertextLen = plaintext.length() + EVP_MAX_BLOCK_LENGTH;
    std::string ciphertext(ciphertextLen, '\0');

    int len = 0;
    EVP_EncryptUpdate(ctx, (unsigned char*)&ciphertext[0], &len, (const unsigned char*)plaintext.c_str(), plaintext.length());
    int finalLen = 0;
    EVP_EncryptFinal_ex(ctx, (unsigned char*)&ciphertext[len], &finalLen);

    ciphertext.resize(len + finalLen);

    EVP_CIPHER_CTX_free(ctx);

    return Base64Encode(ciphertext);
}

std::string Base64Decode2(const std::string& input) {
    std::string decodedString;
    BIO* bio, * b64;

    std::vector<char> buffer(input.begin(), input.end());

    bio = BIO_new_mem_buf(buffer.data(), -1);
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    int length = BIO_read(bio, &buffer[0], input.size());

    decodedString.assign(buffer.data(), length);

    BIO_free_all(bio);

    return decodedString;
}
std::string Base64Decode3(const std::string& input) {
    std::string decodedString;
    BIO* bio, * b64;

    std::vector<char> buffer(input.begin(), input.end());

    bio = BIO_new_mem_buf(buffer.data(), -1);
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    int length = BIO_read(bio, &buffer[0], input.size());

    decodedString.assign(buffer.data(), length);

    BIO_free_all(bio);

    return decodedString;
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

std::string AESDecrypt(const string cipherText) {
    //std::string cipherText = "hsuuZ0+3z3T3PnWAQZiKWw=";

    std::string decodedData = base64_decode(cipherText);
    std::cout << std::hex << decodedData << std::endl;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), nullptr, (const unsigned char*)key.c_str(), (const unsigned char*)iv.c_str());

    int plaintextLen = decodedData.length() + EVP_MAX_BLOCK_LENGTH;
    std::string plaintext(plaintextLen, '\0');
    std::cout << plaintext << "," << plaintextLen << std::endl;

    int len = 0;
    EVP_DecryptUpdate(ctx, (unsigned char*)&plaintext[0], &len, (const unsigned char*)decodedData.c_str(), decodedData.length());
    int finalLen = 0;
    EVP_DecryptFinal_ex(ctx, (unsigned char*)&plaintext[len], &finalLen);
    int z_len = len + finalLen;
    std::cout << z_len << std::endl;

    plaintext.resize(len + finalLen);
   /* std::cout << plaintext << std::endl;*/

    EVP_CIPHER_CTX_free(ctx);

    return plaintext;
}


bool isValidMacAddress(const std::string& macAddress) {
    std::regex macRegex("^([0-9A-Fa-f]{2}-){5}([0-9A-Fa-f]{2})$");
    return std::regex_match(macAddress, macRegex);
}

 
int main(int argc, char* argv[]) {
    if (argc == 2) {
        std::string input = argv[1];
        if (input.length() == 17) {
            // 检查MAC地址的格式
            std::regex macPattern("^([0-9A-Fa-f]{2}-){5}[0-9A-Fa-f]{2}$");
            if (std::regex_match(input, macPattern)) {
                // 去除中间的"-"
                std::string mac = input.substr(0, 2) + input.substr(3, 2) + input.substr(6, 2) + input.substr(9, 2) + input.substr(12, 2) + input.substr(15, 2);
                std::cout << AESEncrypt(mac) << std::endl;
            }
            else {
                std::cout << "MAC地址格式必须为XX-XX-XX-XX-XX-XX." << std::endl;
            }
        }
        else if (input.length() > 17) {
            // 执行解密操作
            std::string decrypted = AESDecrypt(input);
            if (decrypted.length() == 12) {
                // 使用"-"拼接MAC地址
                std::string macAddress = decrypted.substr(0, 2) + "-" + decrypted.substr(2, 2) + "-" + decrypted.substr(4, 2) + "-" + decrypted.substr(6, 2) + "-" + decrypted.substr(8, 2) + "-" + decrypted.substr(10, 2);

                std::cout << macAddress << std::endl;
            }
            else {
                std::cout << "解码失败." << std::endl;
            }
        }
      
    }
    else {
        std::cout << "请指定一个原文/密文参数." << std::endl;
    }

    return 0;
}