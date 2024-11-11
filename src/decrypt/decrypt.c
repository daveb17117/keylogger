#include <windows.h>
#include <wincrypt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KEY_SIZE 32
#define IV_SIZE 16
#define BUFFER_SIZE 1024

void printUsage(const char* programName) {
    printf("Usage: %s [options]\n", programName);
    printf("Options:\n");
    printf("  -k, --keyfile <path>     Path to the key file (default: key.bin)\n");
    printf("  -i, --input <path>       Path to encrypted input file (default: keylog.encrypted)\n");
    printf("  -o, --output <path>      Path to decrypted output file (default: keylog_decrypted.txt)\n");
    printf("  -h, --help              Show this help message\n");
    printf("\nExample:\n");
    printf("  %s -k C:\\keys\\key.bin -i C:\\logs\\encrypted.dat -o C:\\logs\\decrypted.txt\n", programName);
}

typedef struct {
    char keyFilePath[MAX_PATH];
    char inputFilePath[MAX_PATH];
    char outputFilePath[MAX_PATH];
} Config;

BOOL parseArguments(int argc, char* argv[], Config* config) {
    // Set defaults
    strcpy(config->keyFilePath, "key.bin");
    strcpy(config->inputFilePath, "keylog.encrypted");
    strcpy(config->outputFilePath, "keylog_decrypted.txt");

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printUsage(argv[0]);
            return FALSE;
        }
        else if ((strcmp(argv[i], "-k") == 0 || strcmp(argv[i], "--keyfile") == 0) && i + 1 < argc) {
            strcpy(config->keyFilePath, argv[++i]);
        }
        else if ((strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--input") == 0) && i + 1 < argc) {
            strcpy(config->inputFilePath, argv[++i]);
        }
        else if ((strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) && i + 1 < argc) {
            strcpy(config->outputFilePath, argv[++i]);
        }
        else {
            printf("Unknown or incomplete argument: %s\n", argv[i]);
            printUsage(argv[0]);
            return FALSE;
        }
    }
    return TRUE;
}

BOOL initializeDecryption(HCRYPTPROV* hProv, HCRYPTKEY* hKey, const char* keyFilePath) {
    if (!CryptAcquireContext(hProv, NULL, MS_ENH_RSA_AES_PROV, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        printf("Failed to acquire crypto context. Error: %lu\n", GetLastError());
        return FALSE;
    }

    FILE* keyFile = fopen(keyFilePath, "rb");
    if (!keyFile) {
        printf("Could not open key file: %s\n", keyFilePath);
        CryptReleaseContext(*hProv, 0);
        return FALSE;
    }

    BYTE keyData[KEY_SIZE];
    if (fread(keyData, 1, KEY_SIZE, keyFile) != KEY_SIZE) {
        printf("Failed to read key data from: %s\n", keyFilePath);
        fclose(keyFile);
        CryptReleaseContext(*hProv, 0);
        return FALSE;
    }
    fclose(keyFile);

    BLOBHEADER blobHeader;
    blobHeader.bType = PLAINTEXTKEYBLOB;
    blobHeader.bVersion = CUR_BLOB_VERSION;
    blobHeader.reserved = 0;
    blobHeader.aiKeyAlg = CALG_AES_256;

    DWORD keyBlobLen = sizeof(BLOBHEADER) + sizeof(DWORD) + KEY_SIZE;
    BYTE* keyBlob = (BYTE*)malloc(keyBlobLen);
    memcpy(keyBlob, &blobHeader, sizeof(BLOBHEADER));
    DWORD* keySize = (DWORD*)(keyBlob + sizeof(BLOBHEADER));
    *keySize = KEY_SIZE;
    memcpy(keyBlob + sizeof(BLOBHEADER) + sizeof(DWORD), keyData, KEY_SIZE);

    if (!CryptImportKey(*hProv, keyBlob, keyBlobLen, 0, 0, hKey)) {
        printf("Failed to import key. Error: %lu\n", GetLastError());
        free(keyBlob);
        CryptReleaseContext(*hProv, 0);
        return FALSE;
    }

    free(keyBlob);
    return TRUE;
}

BOOL decryptFile(const char* inputPath, const char* outputPath, HCRYPTPROV hProv, HCRYPTKEY hKey) {
    FILE* inFile = fopen(inputPath, "rb");
    if (!inFile) {
        printf("Could not open input file: %s\n", inputPath);
        return FALSE;
    }

    FILE* outFile = fopen(outputPath, "w");
    if (!outFile) {
        printf("Could not create output file: %s\n", outputPath);
        fclose(inFile);
        return FALSE;
    }

    BYTE buffer[BUFFER_SIZE];
    BYTE iv[IV_SIZE];
    DWORD processedBlocks = 0;
    size_t bytesRead;
    BOOL success = TRUE;

    // Get file size
    fseek(inFile, 0, SEEK_END);
    long fileSize = ftell(inFile);
    fseek(inFile, 0, SEEK_SET);

    printf("Input file size: %ld bytes\n", fileSize);

    while (!feof(inFile) && ftell(inFile) < fileSize) {
        // Read IV
        bytesRead = fread(iv, 1, IV_SIZE, inFile);
        if (bytesRead != IV_SIZE) {
            if (bytesRead == 0 && feof(inFile)) {
                // Normal end of file
                break;
            }
            printf("Failed to read IV (read %zu bytes)\n", bytesRead);
            break;
        }

        // Read original data length
        DWORD originalLen;
        if (fread(&originalLen, sizeof(DWORD), 1, inFile) != 1) {
            printf("Failed to read original length\n");
            break;
        }

        printf("Block %lu: Original length: %lu\n", processedBlocks, originalLen);

        // Set IV for this block
        struct {
            BLOBHEADER hdr;
            DWORD keySize;
            BYTE IV[IV_SIZE];
        } ivBlob = {
            {PLAINTEXTKEYBLOB, CUR_BLOB_VERSION, 0, 0},
            IV_SIZE,
        };
        memcpy(ivBlob.IV, iv, IV_SIZE);
        
        if (!CryptSetKeyParam(hKey, KP_IV, (BYTE*)&ivBlob.IV, 0)) {
            printf("Failed to set IV for block %lu. Error: %lu\n", processedBlocks, GetLastError());
            success = FALSE;
            break;
        }

        // Read encrypted data (rounded up to 16-byte boundary)
        DWORD paddedSize = ((originalLen + 15) / 16) * 16;
        bytesRead = fread(buffer, 1, paddedSize, inFile);
        if (bytesRead != paddedSize) {
            printf("Failed to read encrypted data (expected %lu, got %zu)\n", paddedSize, bytesRead);
            break;
        }

        printf("Block %lu: Read %zu bytes of encrypted data\n", processedBlocks, bytesRead);

        // Decrypt the data
        DWORD dataLen = bytesRead;
        if (!CryptDecrypt(hKey, 0, TRUE, 0, buffer, &dataLen)) {
            printf("Decryption failed for block %lu. Error: %lu\n", processedBlocks, GetLastError());
            success = FALSE;
            break;
        }

        // Write only the original length of data
        fwrite(buffer, 1, originalLen, outFile);
        processedBlocks++;

        printf("Block %lu: Decrypted and written %lu bytes\n", processedBlocks, originalLen);
    }

    printf("\nProcessed %lu blocks in total\n", processedBlocks);
    
    fclose(inFile);
    fclose(outFile);
    return success;
}

int main(int argc, char* argv[]) {
    Config config;
    
    // Parse command line arguments
    if (!parseArguments(argc, argv, &config)) {
        return 1;
    }

    printf("Using configuration:\n");
    printf("Key file: %s\n", config.keyFilePath);
    printf("Input file: %s\n", config.inputFilePath);
    printf("Output file: %s\n", config.outputFilePath);

    HCRYPTPROV hProv;
    HCRYPTKEY hKey;

    // Initialize crypto
    if (!initializeDecryption(&hProv, &hKey, config.keyFilePath)) {
        printf("Failed to initialize decryption\n");
        return 1;
    }

    printf("Decrypting file...\n");
    if (!decryptFile(config.inputFilePath, config.outputFilePath, hProv, hKey)) {
        printf("Decryption failed\n");
        CryptDestroyKey(hKey);
        CryptReleaseContext(hProv, 0);
        return 1;
    }

    printf("Decryption successful!\n");
    printf("Decrypted data written to: %s\n", config.outputFilePath);

    // Cleanup
    CryptDestroyKey(hKey);
    CryptReleaseContext(hProv, 0);
    return 0;
}