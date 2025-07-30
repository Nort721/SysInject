#pragma once

#include "Structs.h"
#include <Windows.h>

typedef NTSTATUS(NTAPI* fnNtQuerySystemInformation)(SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength);
typedef NTSTATUS(NTAPI* fnNtQuerySystemInformation)(SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength);

BOOL GetRemoteProcThreadViaNtQueryFunc(IN LPWSTR szProcessName, OUT PDWORD pdwProcessID, OUT PDWORD pdwThreadID, OUT OPTIONAL PHANDLE phThread);