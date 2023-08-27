#ifndef __BYTES_H__
#define __BYTES_H__

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <cstring>

uint8_t* uint_to_little_endian_bytearray(unsigned int number, size_t size) {
    // allocate size * sizeof(uint8) to variable nle
    uint8_t* nle = (uint8_t*)malloc(size * sizeof(uint8_t));

    // loop through nle
    for(size_t i = 0; i < size; i++) {
        // set nle[i] to number shifted right by ((i * 8) and 0xFF)
        nle[i] = number >> i * 8 & 0xFF;
    }

    // return bytearray
    return nle;
}

size_t merge_bytes(uint8_t* bytes, size_t bytesLen, uint8_t* bytes2, size_t bytesLen2, uint8_t* outBytes) {
    // set outbyteslen to byteslen + byteslen2
    size_t outSize = bytesLen + bytesLen2;

    // append bytes to outbytes
    memcpy(outBytes, bytes, bytesLen);
    // append bytes2 to outbytes
    memcpy(outBytes + bytesLen, bytes2, bytesLen2);

    return outSize;
}

void printBytes(uint8_t* bytes, size_t bytesLen) {
    for(size_t i = 0; i < bytesLen; i++) {
        printf("\\x%02x", bytes[i]);
    }

    printf("\n");
}
#endif
