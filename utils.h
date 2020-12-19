#pragma once

#include <windows.h>

#include <string>
#include <iomanip>
#include <vector>
#include <cmath>

bool isDir(const std::wstring& path);
std::wstring getLastErrorText();
std::wstring getLastErrorText(DWORD lastError);

std::wstring getTimeStr(const SYSTEMTIME& tl);
std::wstring getTimeStr(const FILETIME& time);
bool sameTime(const FILETIME& t1, const FILETIME& t2);

std::wstring align(const std::wstring& s, int size);
std::wstring trim(const std::wstring& s);

template<typename T>
std::wstring toHex(T num, int width = 2*sizeof(T)) {
    std::wstringstream s;
    if (width > 0) {
        s << std::setfill(L'0') << std::setw(width);
    }
    s << std::hex << num;
    return s.str();
}

template <typename T>
T clamp(const T& left, const T& value, const T& right) {
    return std::max(left, std::min(value, right));
}

template <typename T>
int roundI(const T& val) {
    return (int)std::round(val);
}

template <typename T, int Size>
int arraySize(T (& _unused)[Size]) {
    return Size;
}

template <typename T, int Size>
int nextIdx(int idx, T (& array)[Size]) {
    return (idx + 1) % arraySize(array);
}
int nextIdx(int idx, const std::vector<int>& array);

template <typename T, int Size>
int prevIdx(int idx, T (& array)[Size]) {
    return (idx + arraySize(array) - 1) % arraySize(array);
}
int prevIdx(int idx, const std::vector<int>& array);

template <typename T>
std::vector<T> operator+(std::vector<T> left, const std::vector<T>& right) {
    left.insert(left.end(), right.begin(), right.end());
    return left;
}

void toggle(bool& flag);

std::wstring check(bool value);

std::wstring radio(bool value);
