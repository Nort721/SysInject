#pragma once

#include <Windows.h>

BOOL HijackTargetThread(IN HANDLE hThread, IN PVOID pFunctionAddress, IN PVOID pArgument);