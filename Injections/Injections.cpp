#include "Injections.h"
#include <iostream>
#include <windows.h>
#include <string.h>
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <string>
#include <sstream>

void ShowLastErrorMessageVerbose() {
	DWORD error = GetLastError();

	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		error,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&lpMsgBuf,
		0, nullptr);

	std::stringstream ss;
	ss << "Execution Failed With Error: " << error << "\n" << (char*)lpMsgBuf;

	MessageBoxA(nullptr, ss.str().c_str(), "Error", MB_OK | MB_ICONERROR);

	LocalFree(lpMsgBuf);
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
