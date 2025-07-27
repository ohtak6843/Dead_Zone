#include "pch.h"
#include "Framework.h"


unique_ptr<Framework> gameFramework = make_unique<Framework>();

wstring s2ws(const string& s)
{
	int32 len;
	int32 slength = static_cast<int32>(s.length()) + 1;
	len = ::MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	::MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	wstring ret(buf);
	delete[] buf;
	return ret;
}

string ws2s(const wstring& s)
{
	int32 len;
	int32 slength = static_cast<int32>(s.length()) + 1;
	len = ::WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, 0, 0);
	string r(len, '\0');
	::WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, &r[0], len, 0, 0);
	return r;
}


void EnableConsole()
{
	AllocConsole();

	// C 표준 입출력 리디렉션
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);   // 표준 출력 리디렉션
	freopen_s(&fp, "CONOUT$", "w", stderr);   // 표준 에러 리디렉션
	freopen_s(&fp, "CONIN$", "r", stdin);     // 표준 입력 리디렉션

	// C++ 스트림 동기화
	std::ios::sync_with_stdio(); // C++ <-> C 스트림 동기화
	std::wcout.clear();
	std::cout.clear();
	std::wcerr.clear();
	std::cerr.clear();
	std::wcin.clear();
	std::cin.clear();
}