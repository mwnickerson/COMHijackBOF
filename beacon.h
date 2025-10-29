#ifndef BEACON_H
#define BEACON_H

#include <windows.h>

#define CALLBACK_OUTPUT 0
#define CALLBACK_ERROR 0x0d

// Beacon Data Parser
typedef struct {
    char* original;
    char* buffer;
    int length;
    int size;
} datap;

// Beacon API declarations
DECLSPEC_IMPORT void BeaconDataParse(datap* parser, char* buffer, int size);
DECLSPEC_IMPORT char* BeaconDataExtract(datap* parser, int* size);
DECLSPEC_IMPORT int BeaconDataInt(datap* parser);
DECLSPEC_IMPORT short BeaconDataShort(datap* parser);
DECLSPEC_IMPORT void BeaconPrintf(int type, char* fmt, ...);

// MSVCRT functions
DECLSPEC_IMPORT size_t __cdecl MSVCRT$strlen(const char*);
DECLSPEC_IMPORT void* __cdecl MSVCRT$memset(void*, int, size_t);
DECLSPEC_IMPORT int __cdecl MSVCRT$strcmp(const char*, const char*);

#endif