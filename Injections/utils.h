#pragma once

#include <windows.h>
#include <string>
#include <iostream>

void ShowLastErrorMessageVerbose();
std::wstring ConvertToWide(const std::string& ansiStr);