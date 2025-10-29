# COM Hijack BOF
Automates COM hijacking of `msedgewebview2.exe` for persistence and code execution.

## Compilation
In x64 Native Tools Command Prompt for VS:
```
cl.exe /c /GS- /O2 comhijack.c /Focomhijack.x64.o
```

## Usage
1. Upload the DLLs to the target machine
2. Run the command to setup the COM hijacking
```
beacon> comhijack C:\path\to\hijack.dll
```
3. Clean up the COM hijacking Registry keys
```
beacon> comhijack_cleanup
```





