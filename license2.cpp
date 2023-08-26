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

// ��ȡ����MAC��ַ�б�
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
    const int keyLength = 128; // ʹ��128λ��Կ
    const int ivLength = EVP_CIPHER_iv_length(cipher); // ��ȡIV����

    // �������IV
    // ���䶯̬�ڴ�
    //unsigned char* iv = (unsigned char*)malloc(ivLength);
    //// ����ʹ�ã�unsigned char* iv = new unsigned char[ivLength];
    //if (RAND_bytes(iv, ivLength) != 1) {
    //    throw std::runtime_error("Failed to generate random IV");
    //}
    //for (int i = 0; i < ivLength; i++) iv[i] = (unsigned char)(i % 10);
    //iv[ivLength] = '\0';
    unsigned char iv[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 ,0,1,2,3,4,5};

    // ��ʼ������������
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, cipher, NULL, (const unsigned char*)key.c_str(), iv);

    // ���ü������볤��Ϊ���ĳ��ȼ���һ�����ܿ鳤��
    int plaintextLength = plaintext.length();
    int ciphertextLength = (plaintextLength / AES_BLOCK_SIZE + 1) * AES_BLOCK_SIZE;
    unsigned char* ciphertext = new unsigned char[ciphertextLength];

    // ִ�м��ܲ���
    int len;
    EVP_EncryptUpdate(ctx, ciphertext, &len, (const unsigned char*)plaintext.c_str(), plaintextLength);
    ciphertextLength = len;

    // ��ɼ��ܲ���
    EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
    ciphertextLength += len;

    // ��IV�ͼ��ܺ������ƴ����һ��
    std::string encryptedText;
    encryptedText.append(reinterpret_cast<const char*>(iv), ivLength);
    encryptedText.append(reinterpret_cast<const char*>(ciphertext), ciphertextLength);

    // ������������Ĳ��ͷ��ڴ�
    EVP_CIPHER_CTX_free(ctx);
    delete[] ciphertext;

    return encryptedText;
}

std::string AES128Decrypt(const std::string& ciphertext, const std::string& key)
{
    const EVP_CIPHER* cipher = EVP_aes_128_cbc();
    const int keyLength = 128; // ʹ��128λ��Կ
    const int ivLength = EVP_CIPHER_iv_length(cipher); // ��ȡIV����

    // ����������ȡIV�ͼ�������
    const unsigned char* iv = reinterpret_cast<const unsigned char*>(ciphertext.data());
    const unsigned char* ciphertextData = reinterpret_cast<const unsigned char*>(ciphertext.data() + ivLength);
    int ciphertextLength = ciphertext.length() - ivLength;

    // ��ʼ������������
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, cipher, NULL, (const unsigned char*)key.c_str(), iv);

    // ���ý������볤��Ϊ���ĳ���
    int decryptedTextLength = ciphertextLength;
    unsigned char* decryptedText = new unsigned char[decryptedTextLength];

    // ִ�н��ܲ���
    int len;
    EVP_DecryptUpdate(ctx, decryptedText, &len, ciphertextData, ciphertextLength);
    decryptedTextLength = len;

    // ��ɽ��ܲ���
    EVP_DecryptFinal_ex(ctx, decryptedText + len, &len);
    decryptedTextLength += len;

    // ������������Ĳ��ͷ��ڴ�
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
    //���Ե�ԭ��
    std::string plaintext = "98-AF-65-1F-24-4E";
    std::string key = "hundsun";
    //����
    std::string ciphertext = AES128Encrypt(plaintext, key);


    // ������ת��Ϊ16���Ʊ�ʾ
    //std::ostringstream oss;
    //for (char c : ciphertext) {
    //    oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(static_cast<unsigned char>(c));
    //}
    //std::string hexEncryptedText = oss.str();

    // ʹ��OpenSSL���е�base64_encode����
    std::string encodedCiphertext = base64_encode(ciphertext); 
    std::cout  <<"base64:[" << encodedCiphertext <<"]"<< std::endl;

    //����base64���ģ��Ƚ���base64���ٽ��� 
    //[xeal7/3IhQS9adnikhp8kriNL9ebW+SocJqThihAbdhU/vPuoKljH2WPMXBpQrDc]
    std::string decodedText = base64_decode(encodedCiphertext);
    std::string decryptedText = AES128Decrypt(ciphertext, key);
    std::cout << "DecryptedText: " << decryptedText << std::endl;

    // ��ȡ������MAC��ַ������ܺ��ԭ�Ľ��бȶ�
    std::vector<std::string> macAddresses = getMacAddresses();
    bool isDecrypted = false;

    for (const std::string& mac : macAddresses) {
        if (strcmp(mac.c_str(), decryptedText.c_str()) == 0) {
            isDecrypted = true;
            break;
        }
        std::cout << mac << std::endl;
    }
    //��ӡ�ȶԽ��
    if (isDecrypted)
        std::cout << "��ĵ�����Ȩ����" << std::endl;
    else
        std::cout << "��������Ȩ" << std::endl;

    return 0;
}




bool isValidMacAddress(const std::string& macAddress) {
    std::regex macRegex("^([0-9A-Fa-f]{2}-){5}([0-9A-Fa-f]{2})$");
    return std::regex_match(macAddress, macRegex);
}

std::string generateKeyFromMac(const std::string& macAddress) {
    std::string key = "hundsun";
    //����
    std::string ciphertext = AES128Encrypt(macAddress, key);

    // ʹ��OpenSSL���е�base64_encode����
    std::string encodedCiphertext = base64_encode(ciphertext);
    std::cout << "base64:[" << encodedCiphertext << "]" << std::endl;
    return encodedCiphertext;
}

std::string decryptMacFromKey(const std::string& encodedCiphertext) {
    std::string key = "hundsun";
    //����base64���ģ��Ƚ���base64���ٽ��� 
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
                std::cout << "MAC��ַ��ʽ����ΪXX-XX-XX-XX-XX-XX." << std::endl;
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
                std::cout << "����ʧ��." << std::endl;
                return 1;
            }
        }
    }

    std::cout << "��ָ��һ��ԭ��/���Ĳ���." << std::endl;
    return 1;
}