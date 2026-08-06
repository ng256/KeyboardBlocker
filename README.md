# Keyboard Blocker
Copyright © 2025 Pavel Bashkardin

<p align="left">
  <img width="64" height="64" alt="image" src="https://github.com/user-attachments/assets/3a73d802-ff6d-4e24-b785-cb6df88fd3dd" />
</p>
<p align="left">
  <a href="LICENSE">
    <img src="https://img.shields.io/badge/License-MIT-green.svg" height="64" alt="MIT License">
  </a>
  <a href="https://sourceforge.net/projects/keyboard-blocker/?pk_campaign=badge&pk_source=vendor">
    <img src="https://b.sf-syn.com/badge_img/4069603/oss-rising-star-white?achievement=oss-rising-star" height="64" alt="SourceForge Rising Star">
  </a>
  <a href="https://sourceforge.net/projects/keyboard-blocker/?pk_campaign=badge&pk_source=vendor">
    <img src="https://b.sf-syn.com/badge_img/4069603/oss-users-love-us-white" height="64" alt="SourceForge Users Love Us">
  </a>
</p>

## Description

**Keyboard Blocker** is a lightweight Windows utility designed to completely block keyboard input. It runs silently in the background with a system tray icon (near the clock), making it ideal for preventing accidental keystrokes (e.g., during video playback or when a child is using the computer).

The program uses a low‑level keyboard hook (`WH_KEYBOARD_LL`) to intercept and suppress all keyboard messages before it reaches applications. It is intended for situations where you want to prevent a child (or anyone) from accidentally pressing keys while watching a video.

### Key Features
- **System Tray Integration**: Runs in the background with a tray icon.
- **Keyword Unblocking**: Type `unblock` to unblock the keyboard at any time.
- **Manual Control**: Right-click the tray icon to block/unblock or exit.
- **Registry Persistence**: Remembers the last blocking state.
- **Self-Contained Installer**: Easy installation and uninstallation via `Setup.exe`.
- **No Admin Rights Required**: Works without administrator privileges.
- **Native WinAPI Application**: Lightweight standalone executable with no external frameworks or runtime dependencies.

## Usage

<img width="257" height="72" alt="image" src="https://github.com/user-attachments/assets/c389bf4f-1008-4945-88fb-02260f01a79f" />

- **Block/Unblock**:
  - Right-click the tray icon and choose **"Block"** or **"Unblock".**
  - Or, simply type `unblock` on your keyboard to unblock the keyboard instantly.
- **Exit**:
  - Right-click the tray icon and select **"Exit"**.

<img width="134" height="79" alt="image" src="https://github.com/user-attachments/assets/0dbcb85e-38f0-4d47-9045-b40f4f6da7cb" /> 
    
- **Notifications**:
  - Left-click the tray icon to show the current blocking state.

<img width="361" height="119" alt="image" src="https://github.com/user-attachments/assets/1107608e-2b9a-4a57-aefb-ced1c8312608" />

The program is designed to be as unobtrusive as possible: no windows, only a tray icon and occasional balloons.

## Installation

### Using the Installer
- Run [Setup.exe](https://github.com/ng256/KeyboardBlocker/releases/download/keyblock/Setup.exe) to install Keyboard Blocker.
- The program will be extracted to `%APPDATA%\Keyblock` and added to Windows startup.

If you want, you can modify the source code and create your own binaries.

## Uninstallation

- **Via Installer**:
  - Open **Apps & Features** (or **Programs and Features**).
  - Find **Keyboard Blocker** and click **Uninstall**.
- **Manual Removal**:
  - Delete the installation folder (`%APPDATA%\Keyblock`).
  - Remove the **"Keyboard Blocker"** entry from Windows startup (via Task Manager or Registry Editor).

## Building from Source

### Compilation Requirements

- Windows operating system.

- **MinGW‑w64** compiler (TDM‑GCC recommended) with `windres` (resource compiler) and `objdump` (usually included). The provided build script assumes the compiler is installed at: `C:\Program Files\TDM-GCC-64\bin`. If your compiler is located elsewhere, edit the `set PATH=` line in `build.bat`.
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

## License

This project is licensed under the **MIT License**. See the [LICENSE](LICENSE) file or the source code headers for details. In short, you may use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the software, provided that the copyright notice and permission notice appear in all copies.

## Author

**Pavel Bashkardin**

For questions or suggestions, feel free to contact the author via GitHub.
