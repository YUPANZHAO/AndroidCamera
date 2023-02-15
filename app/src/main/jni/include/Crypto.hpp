#pragma once

#include <types.h>

class Crypto {
public:
    Crypto();
    ~Crypto();

public:
    static void encrypt(const uint8_t* key, uint32_t key_len, uint8_t* buf, uint32_t buf_len) {
        crypt(key, key_len, buf, buf_len);
    }
    
    static void decrypt(const uint8_t* key, uint32_t key_len, uint8_t* buf, uint32_t buf_len) {
        crypt(key, key_len, buf, buf_len);
    }

private:
    static void crypt(const uint8_t* key, uint32_t key_len, uint8_t* buf, uint32_t buf_len) {
        uint32_t len = 0;
        while(len < buf_len) {
            xor_unit(key, key_len, buf + len, (key_len < buf_len - len ? key_len : buf_len - len));
            len += (key_len < buf_len - len ? key_len : buf_len - len);
        }
    }

    static void xor_unit(const uint8_t* key, uint32_t key_len, uint8_t* buf, uint32_t buf_len) {
        for(uint32_t i=0; i < key_len && i < buf_len; ++i) {
            buf[i] ^= key[i];
        }
    }
};