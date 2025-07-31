#pragma once

#include <windows.h>

#define INJECTIONS_API __declspec(dllimport)

extern "C" {
    INJECTIONS_API BOOL InjectDllRemote(DWORD processId, LPSTR cDllFilePath);
    INJECTIONS_API BOOL InjectDllByThreadHijack(LPSTR cProcName, LPSTR cDllFilePath);
}
