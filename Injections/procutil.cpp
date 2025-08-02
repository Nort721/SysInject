#include "procutil.h"
#include "utils.h"
#include <stdio.h>

BOOL EnableDebugPrivilege() {
    HANDLE hToken;
    LUID luid;
    TOKEN_PRIVILEGES tp;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        return false;

    if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid)) {
        CloseHandle(hToken);
        return false;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    BOOL success = AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
    CloseHandle(hToken);
    return success && GetLastError() == ERROR_SUCCESS;
}

BOOL Ring0TerminateProcess(DWORD dwProcessId) {
    MessageBoxA(NULL, "Escalating privileges to NT Authority\\System to terminate process.", "Higher privileges required", MB_OKCANCEL);
    EnableDebugPrivilege();

    // ToDo => load kernel driver

    // ToDo => send IOCTL to trigger process termination from ring0

    return TRUE;
}

BOOL TerminateTargetProcess(DWORD dwProcessId) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION, FALSE, dwProcessId);
    if (!hProcess) {
        DWORD err = GetLastError();
        if (err == ERROR_ACCESS_DENIED) {
            return Ring0TerminateProcess(dwProcessId);
        }
        else {
            printf("[!] OpenProcess failed. Error: %d\n", err);
            ShowLastErrorMessageVerbose();
            return false;
        }
    }

    // Try to terminate the process
    BOOL result = TerminateProcess(hProcess, 1);
    DWORD terminateErr = GetLastError();

    CloseHandle(hProcess);

    if (!result) {
        if (terminateErr == ERROR_ACCESS_DENIED) {
            return Ring0TerminateProcess(dwProcessId);
        }
        else {
            printf("[!] TerminateProcess failed. Error: %d\n", terminateErr);
            ShowLastErrorMessageVerbose();
            return false;
        }
    }

    printf("[+] Process terminated successfully.\n");
    return true;
}