# Educational Windows Keylogger

A demonstration project showcasing Windows system programming and cryptography implementations. This multi-phase keylogger project illustrates advanced Windows programming techniques.

## ⚠️ Educational Purpose Only
This code demonstrates techniques that could be misused and should only be studied in accordance with applicable laws and ethical guidelines. The purpose is to understand Windows internals, cryptography, and system programming concepts.

## Current Features (Phase 1)
- Keyboard event hooking and logging
- AES-256 encryption with CBC mode
- Real-time buffer management
- Secure data storage

## Planned Features
- Phase 2: Persistence
 - Service installation
 - System startup integration
 - Privilege handling
- Phase 3: Command & Control
 - Secure C2 communications
 - Encrypted data transmission
 - Connection resilience

## Prerequisites
- Windows 10/11
- MSYS2 with UCRT64 environment
- GCC (MinGW-w64)
- make

## Building
```bash
make debug    # Debug build
make release  # Release build