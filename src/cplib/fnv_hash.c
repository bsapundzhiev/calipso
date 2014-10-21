#include <stdio.h>
#include <stdint.h>

#define FNV_prime_32 			16777619U
#define FNV_prime_64 			1099511628211U
#define FNV_prime_128 			309485009821345068724781371U
#define FNV_offset_basis_32 	2166136261U
#define FNV_offset_basis_64 	14695981039346656037U
#define FNV_offset_basis_128 	144066263297769815596495629667062367629U

uint32_t FNV_hash32(uint32_t key)
{
    uint8_t i, *bytes = (uint8_t*) (&key);
    uint32_t hash = FNV_offset_basis_32;

    for (i = 0; i < sizeof(key); i++) {
        hash = (FNV_prime_32 * hash) ^ bytes[i];
    }

    return hash;
}

uint64_t FNV_hash64(uint64_t key)
{
    uint8_t i, *bytes = (uint8_t*) (&key);
    uint32_t hash = FNV_offset_basis_32;

    for (i = 0; i < sizeof(key); i++) {
        hash = (FNV_prime_32 * hash) ^ bytes[i];
    }

    return hash;
}

uint32_t FNV_hashStr(const char* data)
{
    unsigned char *s = (unsigned char *) data;
    uint32_t hash = FNV_offset_basis_32;
    while (*s) {
        hash = (FNV_prime_32 * hash) ^ (uint8_t) *s++;
    }

    return hash;
}

/*int main()
 {
 int x = 2034;
 uint32_t t = FNV_hash32(x);
 uint32_t t2 = FNV_hashStr("file1");
 printf("test %x-%x \n", t, t2);
 }*/

