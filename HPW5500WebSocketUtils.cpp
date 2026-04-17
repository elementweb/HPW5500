#include "HPW5500WebSocketUtils.h"

#include <cstring>

namespace {
    uint32_t rol32(uint32_t value, uint8_t bits) {
        return (value << bits) | (value >> (32 - bits));
    }
}

bool HPW5500_ws_sha1(const uint8_t *data, uint32_t len, uint8_t out[20]) {
    if(data == nullptr || out == nullptr) return false;

    uint32_t h0 = 0x67452301;
    uint32_t h1 = 0xEFCDAB89;
    uint32_t h2 = 0x98BADCFE;
    uint32_t h3 = 0x10325476;
    uint32_t h4 = 0xC3D2E1F0;

    uint64_t total_bits = static_cast<uint64_t>(len) * 8u;
    uint32_t padded_len = ((len + 1 + 8 + 63) / 64) * 64;
    uint32_t block_count = padded_len / 64;

    for(uint32_t bi = 0; bi < block_count; bi++) {
        uint8_t block[64];

        for(uint8_t i = 0; i < 64; i++) {
            uint32_t pos = bi * 64 + i;
            if(pos < len) {
                block[i] = data[pos];
            } else if(pos == len) {
                block[i] = 0x80;
            } else if(pos >= padded_len - 8) {
                block[i] = static_cast<uint8_t>(total_bits >> ((padded_len - 1 - pos) * 8));
            } else {
                block[i] = 0x00;
            }
        }

        uint32_t w[80];
        for(uint8_t i = 0; i < 16; i++) {
            w[i] = static_cast<uint32_t>(block[i * 4]) << 24
                | static_cast<uint32_t>(block[i * 4 + 1]) << 16
                | static_cast<uint32_t>(block[i * 4 + 2]) << 8
                | static_cast<uint32_t>(block[i * 4 + 3]);
        }
        for(uint8_t i = 16; i < 80; i++) {
            w[i] = rol32(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
        }

        uint32_t a = h0, b = h1, c = h2, d = h3, e = h4;

        for(uint8_t i = 0; i < 80; i++) {
            uint32_t f, k;
            if(i < 20)      { f = (b & c) | (~b & d);           k = 0x5A827999; }
            else if(i < 40) { f = b ^ c ^ d;                    k = 0x6ED9EBA1; }
            else if(i < 60) { f = (b & c) | (b & d) | (c & d);  k = 0x8F1BBCDC; }
            else             { f = b ^ c ^ d;                    k = 0xCA62C1D6; }

            uint32_t temp = rol32(a, 5) + f + e + k + w[i];
            e = d; d = c; c = rol32(b, 30); b = a; a = temp;
        }

        h0 += a; h1 += b; h2 += c; h3 += d; h4 += e;
    }

    uint32_t hs[5] = { h0, h1, h2, h3, h4 };
    for(uint8_t i = 0; i < 5; i++) {
        out[i * 4]     = static_cast<uint8_t>((hs[i] >> 24) & 0xFF);
        out[i * 4 + 1] = static_cast<uint8_t>((hs[i] >> 16) & 0xFF);
        out[i * 4 + 2] = static_cast<uint8_t>((hs[i] >> 8) & 0xFF);
        out[i * 4 + 3] = static_cast<uint8_t>(hs[i] & 0xFF);
    }

    return true;
}

uint16_t HPW5500_ws_base64_encode(const uint8_t *data, uint16_t len, char *out, uint16_t out_max) {
    static const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    if(out == nullptr || out_max == 0) return 0;

    uint16_t out_len = 0;
    uint16_t i = 0;

    while(i < len && out_len + 4 < out_max) {
        uint32_t triple = 0;
        uint8_t bytes = 0;
        for(uint8_t j = 0; j < 3; j++) {
            triple <<= 8;
            if(i < len) {
                triple |= data[i++];
                bytes++;
            }
        }

        uint8_t pad = 3 - bytes;
        for(uint8_t j = 0; j < 4; j++) {
            if(j > 3 - pad) {
                out[out_len++] = '=';
            } else {
                uint8_t index = static_cast<uint8_t>((triple >> (18 - j * 6)) & 0x3F);
                out[out_len++] = table[index];
            }
        }
    }

    if(out_len < out_max) out[out_len] = '\0';
    return out_len;
}
