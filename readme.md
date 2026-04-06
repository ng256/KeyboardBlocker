# Keyboard Blocker

Copyright © 2025 Pavel Bashkardin

## Description

**Keyboard Blocker** is a lightweight Windows utility designed to completely block keyboard input. It runs silently in the background with a system tray icon (near the clock), making it ideal for preventing accidental keystrokes (e.g., during video playback or when a child is using the computer).

The program uses a low‑level keyboard hook (`WH_KEYBOARD_LL`) to intercept and discard all keyboard messages. It is intended for situations where you want to prevent a child (or anyone) from accidentally pressing keys while watching a video.

### Key Features
- **System Tray Integration**: Runs in the background with a tray icon.
- **Keyword Unblocking**: Type `unblock` to unblock the keyboard at any time.
- **Manual Control**: Right-click the tray icon to block/unblock or exit.
- **Registry Persistence**: Remembers the last blocking state.
- **Self-Contained Installer**: Easy installation and uninstallation via `Setup.exe`.
- **No Admin Rights Required**: Works without administrator privileges.

## Installation

### Compilation Requirements

- Windows operating system.

- **MinGW‑w64** compiler (TDM‑GCC recommended) with `windres` (resource compiler) and `objdump` (usually included). The provided build script assumes the compiler is installed at: `C:\Program Files (x86)\Embarcadero\Dev-Cpp\TDM-GCC-64\bin`. If your compiler is located elsewhere, edit the `set PATH=` line in `build.bat`.
  - [TDM-GCC (MinGW-w64)](https://jmeubank.github.io/tdm-gcc/)
  - [Official MinGW-w64](https://www.mingw-w64.org/)

- **Optional:** 
  - [UPX](https://upx.github.io/) or [UPX on SourceForge](https://upx.sourceforge.net/) (Ultimate Packer for eXecutables) for further size reduction.
  - [WinRAR](https://www.win-rar.com/) Default path: `C:\Program Files\WinRAR\WinRAR.exe`. Change the `set PATH=` line in `buildsfx.bat` if it necessary.

### How to Build

Simply double‑click `build.bat` or run it from the command prompt.  

The script will:

1. Delete old object files and the previous executable.
2. Compile all the resources script.
3. Compiles the main application and uninstaller.
4. Link everything to executable file with optimizations for small size.
5. Display the list of imported DLLs (to verify no unexpected dependencies).
6. If UPX is found, compress the executable (optional).
7. Create an SFX archive that includes all necessary files and an installation script. Ensure WinRAR is installed and the path in `buildsfx.bat` is correctly set to the default or your custom location.

After a successful build you will find `keyblock.exe`, `uninstall.exe` and `setup.exe` in the same folder.

### Using the Installer
- Run `Setup.exe` to install Keyboard Blocker.
- The program will be extracted to `%APPDATA%\Keyblock` and added to Windows startup.

## Usage

- **Block/Unblock**:
  - Right-click the tray icon and select **"Block"** or **"Unblock"**.
  - Type `unblock` to unblock the keyboard at any time.
- **Exit**:
  - Right-click the tray icon and select **"Exit"**.
- **Notifications**:
  - Left-click the tray icon to show the current blocking state.

The program is designed to be as unobtrusive as possible: no windows, only a tray icon and occasional balloons.

## Uninstallation

- **Via Installer**:
  - Open **Apps & Features** (or **Programs and Features**).
  - Find **Keyboard Blocker** and click **Uninstall**.
- **Manual Removal**:
  - Delete the installation folder (`%APPDATA%\Keyblock`).
  - Remove the **"Keyboard Blocker"** entry from Windows startup (via Task Manager or Registry Editor).

## License

This project is licensed under the **MIT License**. See the [LICENSE](LICENSE) file or the source code headers for details. In short, you may use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the software, provided that the copyright notice and permission notice appear in all copies.

## Author

**Pavel Bashkardin**

For questions or suggestions, feel free to contact the author via GitHub.
