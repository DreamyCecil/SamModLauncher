/* Copyright (c) 2023 Dreamy Cecil
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <string>
#include <algorithm>
#include <direct.h>
#include <windows.h>

using namespace std;

// Path to this application and its executable filename
wstring _strPath;
wstring _strExe;

// Go up the path until a certain directory
size_t GoUpUntilDir(wstring strPath, wstring strDirName) {
  // Make both strings lowercase and make consistent path slashes
  transform(strPath.begin(), strPath.end(), strPath.begin(), towlower);
  transform(strDirName.begin(), strDirName.end(), strDirName.begin(), towlower);

  replace(strPath.begin(), strPath.end(), L'\\', L'/');

  // Absolute path (e.g. "path-with/strDirName/in-between")
  size_t iDir(strPath.rfind(L"/" + strDirName + L"/"));
  if (iDir != wstring::npos) return iDir + 1;

  // Relative down to the desired directory (e.g. "end/should-be/strDirName")
  iDir = strPath.rfind(L"/" + strDirName) + 1;
  if (iDir == strPath.length() - strDirName.length()) return iDir;

  // Relative up to the desired directory (e.g. "strDirName/in-the-beginning")
  iDir = strPath.find(strDirName + L"/");
  if (iDir == 0) return iDir;

  // No extra directories up or down the path, must be the same
  if (strPath == strDirName) return 0;

  return wstring::npos;
};

// Display message box with an error
void ShowError(wstring str) {
  MessageBoxW(NULL, str.c_str(), L"Error", MB_ICONERROR|MB_OK);
};

// Terminate the application if the directory is invalid
void AssertDir(size_t iDir) {
  if (iDir != wstring::npos) return;

  ShowError(L"Please place '" + _strExe + L"' inside any folder under 'Mods' to launch that specific mod!\n"
            L"For example: 'C:\\SeriousSam\\Mods\\MyMod\\'\n\n"
            L"NOTE: This is simply a mod launcher! Make sure you aren't mistaking it for the game's executables, "
            L"which all have distinct icons on them!\n\n"
            L"This executable should always be placed under\n'Mods/<any mod folder>' and nowhere else.\n");

  exit(EXIT_SUCCESS);
};

// Get descriptive Windows error
wstring GetWindowsError(void) {
  LPVOID lpMsgBuf;
  FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);

  wstring strResultMessage = (wchar_t *)lpMsgBuf;
  LocalFree(lpMsgBuf);

  return strResultMessage;
};

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
  // Get command line arguments
  int ctArgs = 0;
  LPWSTR *astrArgs = CommandLineToArgvW(GetCommandLineW(), &ctArgs);

  if (ctArgs < 1) {
    ShowError(L"Cannot find a path to this executable!");
    return 0;
  }

  _strPath = astrArgs[0];
  _strExe = _strPath.substr(_strPath.find_last_of(L"/\\") + 1); // Everything after the last directory separator

  // Determine custom Bin directory using a separator in the EXE name
  wstring strBin = L"Bin\\";

  // Example: "Bin_1.10@SeriousSam.exe"
  size_t iSep = _strExe.find('@');

  if (iSep != wstring::npos) {
    strBin = _strExe.substr(0, iSep) + L"\\"; // "Bin_1.10"
    _strExe = _strExe.erase(0, iSep + 1); // "SeriousSam.exe"
  }

  // Check if residing within the "Mods" directory
  size_t iModsDir = GoUpUntilDir(_strPath, L"Mods");
  AssertDir(iModsDir);

  // Get mod name
  wstring strMod = _strPath.substr(iModsDir + 5); // + "Mods/"

  size_t iModDirEnd = strMod.find_first_of(L"/\\");
  AssertDir(iModDirEnd);

  strMod = strMod.erase(iModDirEnd);

  // Path to the desired game executable and mod launch arguments
  wstring strGameBin = _strPath.substr(0, iModsDir) + strBin;
  wstring strGameExe = strGameBin + _strExe;
  wstring strCmd = L" +game \"" + strMod + L"\"";

  // Change working directory to the game's one
  _wchdir(strGameBin.c_str());

  // Start application with the mod
  STARTUPINFOW cif;
  ZeroMemory(&cif, sizeof(cif));
  PROCESS_INFORMATION pi;

  if (!CreateProcessW(strGameExe.c_str(), &strCmd[0], 0, 0, 0, CREATE_DEFAULT_ERROR_MODE, 0, 0, &cif, &pi)) {
    ShowError(L"Cannot start '" + strMod + L"' mod:\n" + GetWindowsError());
  }

  return 0;
};
