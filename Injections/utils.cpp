#include "utils.h"
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

std::wstring ConvertToWide(const std::string& ansiStr) {
	int size_needed = MultiByteToWideChar(CP_ACP, 0, ansiStr.c_str(), -1, nullptr, 0);
	std::wstring wideStr(size_needed, 0);
	MultiByteToWideChar(CP_ACP, 0, ansiStr.c_str(), -1, &wideStr[0], size_needed);
	return wideStr;
}