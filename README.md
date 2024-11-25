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

```

# Keylogger Setup and Execution with Persistence

## Step 1: Build the Keylogger Service
1. Open the project directory in the terminal.
2. Run the following command to create the `keylogger.exe` file:
```bash
make release
```

## Step 2: Add Persistence
The program now automatically adds itself to the Startup folder during execution to ensure it starts automatically with the system. The following steps are performed by the program:
1. Check Admin Privileges:
- If the user has admin rights, the program copies itself to:
```bash
C:\ProgramData\Microsoft\Windows\Start Menu\Programs\Startup
```
- If not, it copies itself to the user-level startup folder:
```bash
C:\Users\Name\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup
```
2. How to test:
- Run the keylogger.exe
- Run the persist.exe
```bash
gcc -o persist persist.c
```
- Check the appropriate Startup folder for the copied executable.
- Reboot the system and verify that the keylogger starts automatically.
- Type something to simulate keystrokes.

## Step 4: Build the Decryptor
1. Compile the decryptor program:
```bash
gcc decrypt.c -o decrypt -lcrypt32
```

## Step 5: Decrypt the Captured Data
```bash
./src/decrypt/decrypt -k path\to\key.bin -i path\to\keylog.encrypted -o path\to\decrypted.txt
```