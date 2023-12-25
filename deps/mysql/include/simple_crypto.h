/*********************************************
 File Name：   simple_crypto.h
 Description： 简单的加密算法
 Function List：
     simple_encrypt    加密char* 数据
     simple_decrypt    解密char* 数据
     EncodeString      加密std::string 数据
     DecodeString      解密std::string 数据
 ..............................
*********************************************/
#ifndef ISON_USER_SIMPLE_CRYPTO_H
#define ISON_USER_SIMPLE_CRYPTO_H

namespace ison_user {
#include <stdint.h>

/*********************************************
 Name： simple_encrypt
 Description：  字符串简单加密函数
 Calls: "EncodePasswd",""
 Table Updated: ""
 Input：
    plain: 类型：const char* 说明：要加密的明文字符串
    plain_len: 类型 int  说明： 要加密的字符串长度
 Output:
    cipher： 类型： char*, 说明： 密文字符串， cipher 参数的长度最少应该是原明长度的2倍
    cipher_len： 类型：int* 说明： 密文字符串长度
 Return：
    int: 0 执行  -1 失败
 Other: 无
 History: 无

*********************************************/
inline int simple_encrypt(const char* plain, int plain_len, char* cipher, int* cipher_len) {
    if (!cipher_len || *cipher_len < plain_len * 2) return -1;
    int len = 0;
    for (len = 0; len < plain_len; ++len) {
        char c = plain[plain_len - 1 - len] ^ ((1 << ((len % 7) + 1)) - 1);
        cipher[len * 2] = (c & 0x0F) + 63;
        cipher[len * 2 + 1] = ((c & 0xF0) >> 4) + 63;
    }
    cipher[len * 2] = '\0';
    *cipher_len = len * 2;
    return 0;
}

/*********************************************
 Name： simple_decrypt
 Description：  字符串简单解密函数
 Calls: "EncodePasswd"
 Table Updated: ""
 Input：
    cipher: 类型：const char* 说明：要解密的明文字符串
    cipher_len: 类型 int  说明： 要解密的字符串长度
 Output:
    plain： 类型： char*, 说明： 明密文字符串, plain 参数的长度最少应该时原密文长度的一半
    plain_len： 类型：int* 说明： 明密文字符串长度,
 Return：
    int: 0 执行  -1 失败
 Other: 无
 History: 无

*********************************************/
inline int simple_decrypt(const char* cipher, int cipher_len, char* plain, int* plain_len) {
    if (!plain_len || *plain_len < cipher_len / 2)
    {
        return -1;
    }
    plain[cipher_len / 2] = '\0';
    int len = 0;
    for (len = 0; len * 2 + 1 < cipher_len; ++len) {
        int dst_i = cipher_len / 2 - 1 - len;
        plain[dst_i] = ((cipher[len * 2 + 1] - 63) << 4) + cipher[len * 2] - 63;
        plain[dst_i] ^= ((1 << ((len % 7) + 1)) - 1);
    }
    *plain_len = len;
    return 0;
}

/*********************************************
 Name： EncodeString
 Description：  字符串加密
 Calls: ""
 Table Updated: ""
 Input：
    plaintext:  类型：std::string  说明： 待加密的字符串
 Output:
    ciphertext：类型： std::string, 说明： 明密文字符串
 Return：
    int: 0 执行  -1 失败
 Other: 无
 History: 无

*********************************************/
int EncodeString(const std::string& plaintext, std::string& ciphertext){
    char cipher[256];
    int cipher_len = sizeof(cipher);
    if (simple_encrypt(plaintext.c_str(), plaintext.length(), cipher, &cipher_len) == 0)
    {
        ciphertext = std::string(cipher, cipher_len);
        return 0;
    }
    return -1;
}

/*********************************************
 Name： DecodeString
 Description：  字符串解密
 Calls: ""
 Table Updated: ""
 Input：
    ciphertext:  类型：std::string  说明： 待加密的字符串
 Output:
    plaintext：类型： std::string, 说明： 明密文字符串
 Return：
    int: 0 执行  -1 失败
 Other: 无
 History: 无

*********************************************/
int DecodeString(const std::string& ciphertext, std::string& plaintext){
    char plain[256];
    int plain_len = sizeof(plain);
    if (simple_decrypt(ciphertext.c_str(), ciphertext.length(), plain, &plain_len) == 0)
    {
        plaintext = std::string(plain, plain_len);
        return 0;
    }
    return -1;
}

} //!namespace ison_user

#endif //ISON_USER_SIMPLE_CRYPTO_H
