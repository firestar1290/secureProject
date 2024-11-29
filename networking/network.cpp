#include "network.h"

std::string network::encryptMessage(const std::string& plaintext, const std::string& key, const std::string& iv) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("Failed to create encryption context");

    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, (unsigned char*)key.c_str(), (unsigned char*)iv.c_str())) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize encryption");
    }

    std::string ciphertext(plaintext.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()), '\0');
    int len = 0, ciphertext_len = 0;
    if (1 != EVP_EncryptUpdate(ctx, (unsigned char*)ciphertext.data(), &len, (unsigned char*)plaintext.c_str(), plaintext.size())) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Encryption failed");
    }
    ciphertext_len += len;

    if (1 != EVP_EncryptFinal_ex(ctx, (unsigned char*)ciphertext.data() + ciphertext_len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Final encryption step failed");
    }
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    ciphertext.resize(ciphertext_len);
    return ciphertext;
}

std::string network::decryptMessage(const std::string& ciphertext, const std::string& key, const std::string& iv) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("Failed to create decryption context");

    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, (unsigned char*)key.c_str(), (unsigned char*)iv.c_str())) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize decryption");
    }

    std::string plaintext(ciphertext.size(), '\0');
    int len = 0, plaintext_len = 0;

    if (1 != EVP_DecryptUpdate(ctx, (unsigned char*)plaintext.data(), &len, (unsigned char*)ciphertext.c_str(), ciphertext.size())) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Decryption failed");
    }
    plaintext_len += len;

    if (1 != EVP_DecryptFinal_ex(ctx, (unsigned char*)plaintext.data() + plaintext_len, &len)) {    
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Final decryption step failed");
    }
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    plaintext.resize(plaintext_len);
    return plaintext;
}