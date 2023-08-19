#ifndef __BYTES_H__
#define __BYTES_H__

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <cstring>

uint8_t* uint_to_little_endian_bytearray(unsigned int number, int size) {
    // allocate size * sizeof(uint8) to variable nle
    uint8_t* nle = (uint8_t*)malloc(size * sizeof(uint8_t));

    // loop through nle
    for(int i = 0; i < size; i++) {
        // set nle[i] to number shifted right by ((i * 8) and 0xFF)
        nle[i] = number >> i * 8 & 0xFF;
    }

    // return bytearray
    return nle;
}

void merge_bytes(uint8_t* bytes, int bytesLen, uint8_t* bytes2, int bytesLen2, uint8_t** outBytes, int* outBytesLen) {
    // set outbytes to length of bytes + length of bytes2 * sizeof uint8
    *outBytes = (uint8_t*)malloc((bytesLen + bytesLen2) * sizeof(uint8_t));
    // set outbyteslen to byteslen + byteslen2
    *outBytesLen = bytesLen + bytesLen2;

    // append bytes to outbytes
    memcpy(*outBytes, bytes, bytesLen);
    // append bytes2 to outbytes
    memcpy(*outBytes + bytesLen, bytes2, bytesLen2);
}

void printBytes(uint8_t* bytes, int bytesLen) {
    for(int i = 0; i < bytesLen; i++) {
        printf("\\x%02x", bytes[i]);
    } 

    printf("\n");    
}
#endif