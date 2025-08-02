#pragma once

#include <windows.h>

#define INJECTIONS_API __declspec(dllimport)

extern "C" {
    INJECTIONS_API BOOL TerminateTargetProcess(DWORD dwProcessId);
}