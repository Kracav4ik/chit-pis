#include "MemWritePanel.h"

#include "utils.h"
#include "MessagePopup.h"

static uint64_t mid(const SYSTEM_INFO& info) {
    return (
        (uintptr_t)info.lpMinimumApplicationAddress +
        (uintptr_t)info.lpMaximumApplicationAddress +
        1
   )/2;
}

MemWritePanel::MemWritePanel(Screen& screen, const SYSTEM_INFO& info)
    : FPWBasePanel(
        screen,
        info,
        L"Запись в память",
        L"Записать данные в память",
        {2, 5, -1, 16},
        10,
        7
    )
{
    regionStart = toHex(mid(info), 0);
    regionSize = L"16М";
}

std::vector<std::wstring> MemWritePanel::header() const {
    return std::vector<std::wstring>{
            L"",
    } + FPWBasePanel::header();
}

void MemWritePanel::enterPressed() {
    uint64_t regionStartValue, regionSizeValue;
    if (!parseStart(regionStartValue)) {
        MessagePopup::show({L"Некорректный адрес начала региона: '" + regionStart + L"'"}, true);
        return;
    }
    if (!parseSize(regionSizeValue)) {
        MessagePopup::show({L"Некорректный размер региона: '" + regionSize + L"'"}, true);
        return;
    }
    RtlSecureZeroMemory((void*) regionStartValue, regionSizeValue);
    MessagePopup::show({L"Запись успешна!"}, true, Color::DarkBlue);
}
