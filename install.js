// ============================================================================
// Keyboard Blocker - Windows application to block keyboard input
// Copyright (c) 2025 Pavel Bashkardin

// This file is part of Keyboard Blocker project, released under MIT license.

// Description: Installation script for Keyboard Blocker
// ============================================================================

var shell = new ActiveXObject("WScript.Shell");
var fso = new ActiveXObject("Scripting.FileSystemObject");

var appDir = fso.GetParentFolderName(WScript.ScriptFullName);
var appName = "Keyboard Blocker";

var exePath = fso.BuildPath(appDir, "keyblock.exe");
var tmpPath = fso.BuildPath(appDir, "keyblock.tmp");
var iconPath = fso.BuildPath(appDir, "icon32.ico");

// ===========================================
// Replace keyblock.exe safely
// ===========================================

try {
    shell.Run("taskkill /F /IM keyblock.exe", 0, true);
} catch (e) {}

var wmi = GetObject("winmgmts:\\\\.\\root\\cimv2");
var i, list;

// wait process exit (~10 sec)
for (i = 0; i < 50; i++) {
    list = wmi.ExecQuery("Select * From Win32_Process Where Name='keyblock.exe'");
    if (list.Count === 0) break;
    WScript.Sleep(200);
}

// retry replace
var replaced = false;

if (fso.FileExists(tmpPath)) {
    for (i = 0; i < 20; i++) {
        try {
            if (fso.FileExists(exePath)) {
                fso.DeleteFile(exePath, true);
            }

            fso.MoveFile(tmpPath, exePath);

            replaced = true;
            break;
        } catch (e) {
            WScript.Sleep(200);
        }
    }
}

// fallback
if (!replaced) {
    try {
        fso.CopyFile(tmpPath, exePath, true);
        fso.DeleteFile(tmpPath, true);
    } catch (e) {}
}

// ===========================================
// Registry init
// ===========================================

try {
    shell.RegDelete("HKCU\\Software\\Keyblock\\");
} catch (e) {}

shell.RegWrite("HKCU\\Software\\Keyblock\\State", 0, "REG_DWORD");

// ===========================================
// Startup
// ===========================================

try {
    shell.RegDelete("HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run\\Keyboard Blocker");
} catch (e) {}

shell.RegWrite(
    "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run\\Keyboard Blocker",
    "\"" + exePath + "\"",
    "REG_SZ"
);

// ===========================================
// Uninstall
// ===========================================

var unregKey = "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Keyboard Blocker";

shell.RegWrite(unregKey + "\\", "", "REG_SZ");
shell.RegWrite(unregKey + "\\DisplayName", appName, "REG_SZ");
shell.RegWrite(unregKey + "\\UninstallString", "\"" + fso.BuildPath(appDir, "uninstall.exe") + "\"", "REG_SZ");
shell.RegWrite(unregKey + "\\Publisher", "Pavel Bashkardin", "REG_SZ");

if (fso.FileExists(iconPath)) {
    shell.RegWrite(unregKey + "\\DisplayIcon", iconPath, "REG_SZ");
}

shell.RegWrite(unregKey + "\\NoModify", 1, "REG_DWORD");
shell.RegWrite(unregKey + "\\NoRepair", 1, "REG_DWORD");

// ===========================================
// Shortcuts
// ===========================================

function createShortcut(folder, name, target, workdir, icon) {
    var path = fso.BuildPath(folder, name);
    var shortcut = shell.CreateShortcut(path);

    shortcut.TargetPath = target;
    shortcut.WorkingDirectory = workdir;

    if (icon) {
        shortcut.IconLocation = icon + ",0";
    }

    shortcut.Save();
}

var desktop = shell.SpecialFolders("Desktop");
var startMenu = shell.SpecialFolders("StartMenu") + "\\Programs";

if (!fso.FolderExists(startMenu)) {
    fso.CreateFolder(startMenu);
}

createShortcut(desktop, "Keyboard Blocker.lnk", exePath, appDir, iconPath);
createShortcut(startMenu, "Keyboard Blocker.lnk", exePath, appDir, iconPath);

// ===========================================
// Run program
// ===========================================

shell.Run("\"" + exePath + "\"", 0, false);

// ===========================================
// Self delete
// ===========================================

(function selfDelete() {
    var temp = fso.GetSpecialFolder(2);
    var bat = fso.BuildPath(temp, "delete_me.bat");

    var file = fso.CreateTextFile(bat, true);

    file.WriteLine("@echo off");
    file.WriteLine("ping 127.0.0.1 -n 2 >nul");
    file.WriteLine(":loop");
    file.WriteLine("del \"" + WScript.ScriptFullName + "\"");
    file.WriteLine("if exist \"" + WScript.ScriptFullName + "\" goto loop");
    file.WriteLine("del \"" + bat + "\"");

    file.Close();

    shell.Run("\"" + bat + "\"", 0, true);
})();
