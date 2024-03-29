#include <stdbool.h>

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>

bool openFileDialog(char* filePathOut, size_t filePathMaxLen) {
    OPENFILENAME ofn;  // common dialog box structure
    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetActiveWindow();
    ofn.lpstrFile = filePathOut;
    ofn.nMaxFile = filePathMaxLen;
    ofn.lpstrFilter = "STereoLithography(*.stl)\0*.stl\0All(*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn)) {
        printf("Open file: %s\n", ofn.lpstrFile);
        return true;
    }
    return false;
}
#else
bool openFileDialog(char* filePathOut, size_t filePathMaxLen) {
    return false;
}
#endif