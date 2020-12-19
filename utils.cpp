#include "utils.h"

#include <vector>
#include <sstream>
#include <iomanip>

bool isDir(const std::wstring& path) {
    return GetFileAttributesW(path.c_str()) & FILE_ATTRIBUTE_DIRECTORY;
}

std::wstring getLastErrorText() {
    return getLastErrorText(GetLastError());
}

std::wstring getLastErrorText(DWORD lastError) {
    wchar_t* buf;
    DWORD messageSize = FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            lastError,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPWSTR)(&buf),
            0,
            nullptr
    );
    if (messageSize == 0) {
        return L"Не удалось получить текст ошибки " + std::to_wstring(lastError);
    }
    std::wstring error(buf, messageSize);
    LocalFree(buf);
    return trim(error);
}

std::wstring getTimeStr(const SYSTEMTIME& tl) {
    std::vector<wchar_t> buf(100);
    std::swprintf(buf.data(), buf.size(), L"%02d.%02d.%04d %02d:%02d:%02d.%03d", tl.wDay, tl.wMonth, tl.wYear, tl.wHour, tl.wMinute, tl.wSecond, tl.wMilliseconds);
    return buf.data();
}

std::wstring getTimeStr(const FILETIME& time) {
    SYSTEMTIME ts;
    FileTimeToSystemTime(&time, &ts);
    SYSTEMTIME tl;
    SystemTimeToTzSpecificLocalTime(nullptr, &ts, &tl);
    return getTimeStr(tl);
}

bool sameTime(const FILETIME& t1, const FILETIME& t2) {
    return getTimeStr(t1) == getTimeStr(t2);
}

std::wstring align(const std::wstring& s, int size) {
    if (s.size() > size) {
        s.substr(0, size);
    }
    return s + std::wstring(size - s.size(), L' ');
}

std::wstring trim(const std::wstring& s) {
    int start = 0;
    for (; start < s.size(); ++start) {
        if (!iswspace(s[start])) {
            break;
        }
    }
    int size = s.size() - start;
    for (; size > 0; --size) {
        if (!iswspace(s[start + size - 1])) {
            break;
        }
    }
    return s.substr(start, size);
}

int nextIdx(int idx, const std::vector<int>& array) {
    return (idx + 1) % (int)array.size();
}

int prevIdx(int idx, const std::vector<int>& array) {
    return (idx + (int)array.size() - 1) % (int)array.size();
}

void toggle(bool& flag) {
    flag = !flag;
}

std::wstring check(bool value) {
    return value ? L"[x]" : L"[ ]";
}

std::wstring radio(bool value) {
    return value ? L"(*)" : L"( )";
}
