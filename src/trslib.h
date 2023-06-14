#pragma once
#include <stdint.h>
#include <vector>
#include "sodium.h"

#ifdef __cplusplus
extern "C" {
#endif
    void ed25519_sign_rust(uint8_t private_key[32], const uint8_t* msg, size_t msg_len, uint8_t signature[64]);

    int ed25519_verify_rust(const uint8_t public_key[32], const uint8_t* msg, size_t msg_len, const uint8_t signature[64]);

    void trs_generate_keypair(uint8_t* privatekey, uint8_t* publickey);
    void trs_keypair_from_hash(uint8_t* hash, uint8_t* privatekey, uint8_t* publickey);
    void trs_keypair_from_seed(uint8_t* privatekey, uint8_t* publickey);
    void trs_sign(
        const void* set_publickey,
        size_t set_publickey_len,
        uint8_t secret_key[64],
        uint8_t issue[32],
        uint8_t msg[32],
        uint8_t a_1[32],
        uint8_t* c_n,
        uint8_t* z_n
    );
    bool trs_verify(
        const void* set_publickey,
        size_t set_publickey_len,
        uint8_t issue[32],
        uint8_t msg[32],
        uint8_t a_1[32],
        const uint8_t* c_n,
        const uint8_t* z_n
    );

    bool is_valid_scalar(uint8_t*);

    int32_t trs_trace(
        const void* set_publickey,
        size_t set_publickey_len,
        uint8_t issue[32],
        uint8_t msg1[32],
        uint8_t msg2[32],
        uint8_t a_11[32],
        uint8_t a_12[32],
        const uint8_t* c_n1,
        const uint8_t* c_n2,
        const uint8_t* z_n1,
        const uint8_t* z_n2
    );

#ifdef __cplusplus
}
#endif

namespace TRS {
    struct Size32 {
        uint8_t data[32]{};
        Size32() {

        }
    };

    struct Tag {
        uint8_t issue[32]{};
        std::vector<Size32> publickey;

        Tag() {

        }
        ~Tag() {
            publickey.clear();
        }
    };

    struct Signature {
        uint8_t A_1[32]{};
        uint8_t* C_n;
        uint8_t* Z_n;
        size_t ring_size{ 0 };
        Signature() {
            C_n = nullptr;
            Z_n = nullptr;
        }
        ~Signature() {
            if (C_n != nullptr) {
                delete[] C_n;
            }

            if (Z_n != nullptr) {
                delete[] Z_n;
            }
            ring_size = 0;
        }
    };

    static void Alloc_Signature(Signature& signature, int ringsize) {
        signature.ring_size = static_cast<size_t>(ringsize);
        size_t size = signature.ring_size * 32;
        signature.C_n = new uint8_t[size]{};
        signature.Z_n = new uint8_t[size]{};
    }

    static void Free_Signature(Signature& signature) {
        delete[] signature.C_n;
        delete[] signature.Z_n;
        signature.C_n = nullptr;
        signature.Z_n = nullptr;
    }

    static void Copy_Signature(Signature* in, Signature& out) {
        if (out.C_n != nullptr) {
            delete[] out.C_n;
        }
        if (out.Z_n != nullptr) {
            delete[] out.Z_n;
        }

        memcpy(out.A_1, in->A_1, 32);
        out.ring_size = in->ring_size;
        long long size = out.ring_size * 32;
        out.C_n = new uint8_t[size]{};
        out.Z_n = new uint8_t[size]{};
        memcpy(out.C_n, in->C_n, size);
        memcpy(out.Z_n, in->Z_n, size);
    }
}

static void ed25519_sign(const uint8_t* private_key, const uint8_t* message, size_t message_len,
    uint8_t* signature) {
    uint8_t public_key[crypto_core_ristretto255_BYTES];
    uint8_t hash_privkey[crypto_hash_sha512_BYTES];
    uint8_t hash_msg[crypto_hash_sha512_BYTES];
    uint8_t hash_r[crypto_hash_sha512_BYTES];

    // Calculate public key
    crypto_scalarmult_ristretto255_base(public_key, private_key);

    // Hash private key
    crypto_hash_sha512(hash_privkey, private_key, crypto_core_ristretto255_SCALARBYTES);

    // Hash message
    crypto_hash_sha512(hash_msg, message, message_len);

    // Calculate secret integer r
    uint8_t hash_privkey_msg[crypto_hash_sha512_BYTES + crypto_hash_sha512_BYTES];
    memcpy(hash_privkey_msg, hash_privkey, crypto_hash_sha512_BYTES);
    memcpy(hash_privkey_msg + crypto_hash_sha512_BYTES, hash_msg, crypto_hash_sha512_BYTES);

    crypto_hash_sha512(hash_r, hash_privkey_msg, crypto_hash_sha512_BYTES + crypto_hash_sha512_BYTES);
    uint8_t scalar_hash_r[crypto_core_ristretto255_BYTES]{};
    crypto_core_ristretto255_scalar_reduce(scalar_hash_r, hash_r);

    // Calculate R = r * G
    uint8_t R[crypto_core_ristretto255_BYTES];
    crypto_scalarmult_ristretto255_base(R, scalar_hash_r);

    // Calculate h = hash(R + pubKey + msg)
    uint8_t* hash_r_pub_msg = new uint8_t[crypto_hash_sha512_BYTES + crypto_core_ristretto255_BYTES + message_len];
    memcpy(hash_r_pub_msg, R, crypto_core_ristretto255_BYTES);
    memcpy(hash_r_pub_msg + crypto_core_ristretto255_BYTES, public_key, crypto_core_ristretto255_BYTES);
    memcpy(hash_r_pub_msg + crypto_core_ristretto255_BYTES + crypto_core_ristretto255_BYTES, message, message_len);

    uint8_t hash_h[crypto_hash_sha512_BYTES];
    crypto_hash_sha512(hash_h, hash_r_pub_msg, crypto_hash_sha512_BYTES + crypto_core_ristretto255_BYTES + message_len);

    uint8_t scalar_hash_h[crypto_core_ristretto255_BYTES]{};
    crypto_core_ristretto255_scalar_reduce(scalar_hash_h, hash_h);

    // Calculate s = (r + h * privKey) mod q
    uint8_t s[crypto_core_ristretto255_SCALARBYTES];
    crypto_core_ristretto255_scalar_mul(s, scalar_hash_h, private_key);
    crypto_core_ristretto255_scalar_add(s, scalar_hash_r, s);

    // Set signature as { R, s }
    memcpy(signature, R, crypto_core_ristretto255_BYTES);
    memcpy(signature + crypto_core_ristretto255_BYTES, s, crypto_core_ristretto255_SCALARBYTES);
    delete[] hash_r_pub_msg;
}

static bool ed25519_verify(const uint8_t* public_key, const uint8_t* message, size_t message_len,
    const uint8_t* signature) {
    uint8_t hash_msg[crypto_hash_sha512_BYTES];
    uint8_t* hash_r_pub_msg = new uint8_t[crypto_hash_sha512_BYTES + crypto_core_ristretto255_BYTES + message_len];
    uint8_t P1[crypto_core_ristretto255_BYTES];
    uint8_t P2[crypto_core_ristretto255_BYTES];

    // Hash message
    crypto_hash_sha512(hash_msg, message, message_len);

    // Extract R and s from signature
    const uint8_t* R = signature;
    const uint8_t* s = signature + crypto_core_ristretto255_BYTES;

    // Calculate h = hash(R + pubKey + msg)
    memcpy(hash_r_pub_msg, R, crypto_core_ristretto255_BYTES);
    memcpy(hash_r_pub_msg + crypto_core_ristretto255_BYTES, public_key, crypto_core_ristretto255_BYTES);
    memcpy(hash_r_pub_msg + crypto_core_ristretto255_BYTES + crypto_core_ristretto255_BYTES, message, message_len);

    uint8_t hash_h[crypto_hash_sha512_BYTES];
    crypto_hash_sha512(hash_h, hash_r_pub_msg, crypto_hash_sha512_BYTES + crypto_core_ristretto255_BYTES + message_len);
    uint8_t scalar_hash_h[crypto_core_ristretto255_BYTES]{};
    crypto_core_ristretto255_scalar_reduce(scalar_hash_h, hash_h);

    // Calculate P1 = s * G
    crypto_scalarmult_ristretto255_base(P1, s);

    // Calculate P2 = R + h * pubKey
    crypto_scalarmult_ristretto255(P2, scalar_hash_h, public_key);
    crypto_core_ristretto255_add(P2, R, P2);

    // Check if P1 == P2
    bool is_valid = (crypto_verify_32(P1, P2) == 0);

    delete[] hash_r_pub_msg;
    return is_valid;
}

struct AES_Data {
    uint8_t* ciphertext;
    size_t cipherlen;
    uint8_t tag[16]{};
    uint8_t nonce[12]{};
    AES_Data() {
        ciphertext = nullptr;
        cipherlen = 0;
    }

    ~AES_Data() {
        if (ciphertext != nullptr) {
            delete[] ciphertext;
        }
    }
};

static void aes_encrypt(uint8_t* privatekey_sender, uint8_t* publickey_receiver, uint8_t* msg, size_t msglen, AES_Data* out) {
    uint8_t sharedkey[32]{};
    crypto_scalarmult_ristretto255(sharedkey, privatekey_sender, publickey_receiver);
    out->ciphertext = new uint8_t[msglen]{};
    out->cipherlen = msglen;
    randombytes_buf(out->nonce, sizeof(out->nonce));
    crypto_aead_aes256gcm_encrypt_detached(
        out->ciphertext,
        out->tag,
        nullptr,
        msg,
        msglen,
        nullptr,
        0,
        nullptr,
        out->nonce,
        sharedkey
    );
}

static bool aes_decrypt(uint8_t* privatekey_receiver, uint8_t* publickey_sender, AES_Data* in, uint8_t *& msg) {
    if (msg != nullptr) {
        delete[] msg;
    }
    msg = new uint8_t[in->cipherlen]{};
    uint8_t sharedkey[32]{};
    crypto_scalarmult_ristretto255(sharedkey, privatekey_receiver, publickey_sender);
    int decryptResult = crypto_aead_aes256gcm_decrypt_detached(
        msg,
        nullptr,
        in->ciphertext,
        in->cipherlen,
        in->tag,
        nullptr,
        0,
        in->nonce,
        sharedkey
    );
    if (decryptResult) {
        return true;
    }
    else {
        return false;
    }
}