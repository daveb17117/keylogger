#ifndef KEYLOGGER_H
#define KEYLOGGER_H

#include <windows.h>
#include <stdbool.h>

// Initialize and start the keylogger
bool startKeylogger(void);

// Stop the keylogger and cleanup
void stopKeylogger(void);

// Manually trigger a buffer flush
void flushKeyloggerBuffer(void);

#endif