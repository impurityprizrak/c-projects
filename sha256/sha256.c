#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define CH(x,y,z) ((x & y) ^ (~x & z))
#define MAJ(x,y,z) ((x & y) ^ (x & z) ^ (y & z))
#define ROTR(x,n) ((x >> n) | (x << (32 - n)))
#define SHR(x,n) (x >> n)
#define SIG0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define SIG1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define sig0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ SHR(x, 3))
#define sig1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ SHR(x, 10))

static const uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

unsigned char *toBinary(int n, int size)
{
    unsigned char *binary = malloc(size);
    memset(binary, 0x00, size);

    for (int i = size - 1; i >= 0; i--) {
        binary[i] = (n & 1);
        n >>= 1;
    }
    return binary;
}

unsigned char **padding(unsigned char *buffer, int length, int *blocks)
{
    int total_bits = 512;
    int total_bytes = total_bits / 8;
    int free_bits = total_bits - (length * 8 % total_bits);
    int pad_count = 0;
    *blocks = length / total_bytes + 1;
    if (free_bits < 64) (*blocks) += 1;

    unsigned char **padded = malloc(*blocks * sizeof(unsigned char *));
    unsigned char *block = malloc(total_bytes);

    while (length > 0) {
        if (length < 56) {
            memcpy(block, buffer, length);
            block[length] = 0x80;
            memset(block + length + 1, 0x00, 55 - length);
            memcpy(block + 56, toBinary(length * 8, 8), 8);
            padded[pad_count] = malloc(total_bytes);
            memcpy(padded[pad_count], block, total_bytes);
            break;
        } else if (length >= 56 && length < total_bytes) {
            memcpy(block, buffer, length);
            block[length] = 0x80;
            memset(block + length + 1, 0x00, total_bytes - length - 1);
            padded[pad_count] = malloc(total_bytes);
            memcpy(padded[pad_count], block, total_bytes);
            pad_count++;

            unsigned char *final_block = malloc(total_bytes);
            memset(final_block, 0x00, total_bytes);
            memcpy(final_block + 56, toBinary(length * 8, 8), 8);
            padded[pad_count] = final_block;
            break;
        } else {
            memcpy(block, buffer, total_bytes);
            padded[pad_count] = malloc(total_bytes);
            memcpy(padded[pad_count], block, total_bytes);
        }
        
        buffer += total_bytes;
        length -= total_bytes;
        pad_count++;
    }

    free(block);
    return padded;
}

void sha256Transform(unsigned char *block, unsigned int hash[8]) 
{
    uint32_t W[64];
    uint32_t a, b, c, d, e, f, g, h, T1, T2;

    for (int i = 0; i < 16; i++) {
        W[i] = (block[i * 4] << 24) | (block[i * 4 + 1] << 16) | 
               (block[i * 4 + 2] << 8) | (block[i * 4 + 3]);
    }

    uint32_t temp1, temp2;
    for (int t = 16; t < 64; t++) {
        temp1 = sig0(W[t - 15]);
        temp2 = sig1(W[t - 2]);
        W[t] = W[t - 16] + temp1 + W[t - 7] + temp2;
    }

    a = hash[0];
    b = hash[1];
    c = hash[2];
    d = hash[3];
    e = hash[4];
    f = hash[5];
    g = hash[6];
    h = hash[7];

    for (int t = 0; t < 64; t++) {
        T1 = h + SIG1(e) + CH(e,f,g) + K[t] + W[t];
        T2 = SIG0(a) + MAJ(a,b,c);
        h = g;
        g = f;
        f = e;
        e = d;
        d = c;
        c = b;
        b = a;
        a = T1 + T2;
    }

    hash[0] += a;
    hash[1] += b;
    hash[2] += c;
    hash[3] += d;
    hash[4] += e;
    hash[5] += f;
    hash[6] += g;
    hash[7] += h;
}

char *generateHash(int *ptr, int size)
{
    unsigned int hash[8];
    hash[0] = 0x6a09e667;
    hash[1] = 0xbb67ae85;
    hash[2] = 0x3c6ef372;
    hash[3] = 0xa54ff53a;
    hash[4] = 0x510e527f;
    hash[5] = 0x9b05688c;
    hash[6] = 0x1f83d9ab;
    hash[7] = 0x5be0cd19;

    char *result = malloc(65 * sizeof(char));
    int blocks;
    int buffer_size = size * 8;
    unsigned char *buffer = calloc(buffer_size + 1, sizeof(unsigned char));

    for (int i = 0; ptr[i] != '\0'; i++) {
        unsigned char *binary = toBinary(ptr[i], 8);
        memcpy(buffer + i * 8, binary, 8);
        free(binary);
    }

    unsigned char **padded = padding(buffer, buffer_size / 8, &blocks);

    for (int i = 0; i < blocks; i++) {
        sha256Transform(padded[i], hash);
    }

    for (int i = 0; i < 8; i++) {
        sprintf(result + i * 8, "%08x", hash[i]);
    }

    hash[65] = '\0';

    free(buffer);
    for (int i = 0; i < blocks; i++) {
        free(padded[i]);
    }
    free(padded);

    return result;
}

int main(void)
{
    // Example with array of integers

    int *example1 = malloc(5 * sizeof(int));
    example1[0] = 1;
    example1[1] = 2;
    example1[2] = 3;
    example1[3] = 4;
    example1[4] = 5;

    char *sha256 = generateHash(example1, 5);

    printf("SHA-256 Hash: ");

    for (int i = 0; i < 65; i++) {
        printf("%c", sha256[i]);
    }

    printf("\n");

    // Example with array of chars (string)

    char *example2 = "hello world";
    int ascii_values[strlen(example2)];

    for (int i = 0; example2[i] != '\0'; i++) {
        ascii_values[i] = (int)example2[i];
    }

    sha256 = generateHash(ascii_values, strlen(example2));

    printf("SHA-256 Hash: ");

    for (int i = 0; i < 65; i++) {
        printf("%c", sha256[i]);
    }

    printf("\n");

    return 0;
}