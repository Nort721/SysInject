#include "procutil.h"
#include "utils.h"
#include <stdio.h>

#define NT_SUCCESS(Status)				((NTSTATUS)(Status) >= 0)

static void PrintLastError(const char* context) {
    DWORD err = GetLastError();
    if (err == 0) {
        printf("%s: no error\n", context);
        return;
    }
    char* msg = NULL;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&msg, 0, NULL);
    printf("%s failed: GetLastError=%lu : %s\n", context, err, msg ? msg : "(no message)");
    if (msg) LocalFree(msg);
}

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

typedef NTSTATUS
(*pWindLoadDriver_t)(
    _In_ PWCHAR LoaderName,
    _In_ PWCHAR DriverName,
    _In_ BOOLEAN Hidden
);

typedef NTSTATUS
(*pWindUnloadDriver_t)(
    _In_ PWCHAR DriverName,
    _In_ BOOLEAN Hidden
);

static HMODULE g_hSwin2 = NULL;
static pWindLoadDriver_t  g_pWindLoadDriver = NULL;
static pWindUnloadDriver_t g_pWindUnloadDriver = NULL;

BOOL LoadWindLoader(void) {
    // Already loaded
    if (g_hSwin2 && g_pWindLoadDriver && g_pWindUnloadDriver) {
        return TRUE;
    }
    
    HMODULE g_hSwin2 = LoadLibraryA("swind2.dll");
    if (!g_hSwin2) {
        printf("Failed to load swind2.dll\n");
        MessageBoxA(NULL, "Failed to load swind2.dll", "Aborting", MB_ICONERROR);
        return FALSE;
    }

    g_pWindLoadDriver = (pWindLoadDriver_t)GetProcAddress(g_hSwin2, "WindLoadDriver");
    g_pWindUnloadDriver = (pWindUnloadDriver_t)GetProcAddress(g_hSwin2, "WindUnloadDriver");

    if (!g_pWindLoadDriver || !g_pWindUnloadDriver) {
        PrintLastError("GetProcAddress");
        MessageBoxA(NULL, "Failed to find swind2.dll's driver functions", "Aborting", MB_ICONERROR);
        FreeLibrary(g_hSwin2);
        g_hSwin2 = NULL;
        g_pWindLoadDriver = NULL;
        g_pWindUnloadDriver = NULL;
        return FALSE;
    }

    return TRUE;
}

void UnloadWindLoader(void) {
    if (g_hSwin2) {
        FreeLibrary(g_hSwin2);
        g_hSwin2 = NULL;
    }
    g_pWindLoadDriver = NULL;
    g_pWindUnloadDriver = NULL;
}

BOOL Ring0TerminateProcess(DWORD dwProcessId) {
    int selection = MessageBoxA(NULL, "Escalating privileges to NT Authority\\System to terminate process.", "Higher privileges required", MB_OKCANCEL);

    if (selection == IDCANCEL) {
        return FALSE;
    }

    EnableDebugPrivilege();

    if (!LoadWindLoader()) {
        MessageBoxA(NULL, "Failed to load windloader.", "Aborting", MB_ICONERROR);
        return FALSE;
    }

    WCHAR vulndrv[] = L"gdrv.sys";
    WCHAR ksysinject[] = L"ksysinject.sys";

    NTSTATUS status = g_pWindLoadDriver(vulndrv, ksysinject, FALSE);

    if (!NT_SUCCESS(status)) {
        char errbuf[128];
        sprintf_s(errbuf, sizeof(errbuf), "WindLoadDriver failed: 0x%08X", (unsigned)status);
        MessageBoxA(NULL, errbuf, "Driver load failed", MB_ICONERROR);
        //g_pWindUnloadDriver(ksysinject, FALSE);
        return FALSE;
    }

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