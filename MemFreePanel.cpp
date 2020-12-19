#include "MemFreePanel.h"

#include "utils.h"
#include "MessagePopup.h"

MemFreePanel::MemFreePanel(Screen& screen, const SYSTEM_INFO& info)
    : FPWBasePanel(
        screen,
        info,
        L"Возврат памяти",
        L"Освободить память",
        {2, 5, -1, 14, 15, 17},
        10,
        5
    )
    , wholeRegion(true)
    , decommit(true)
{
}

void MemFreePanel::drawOn(Screen& s) {
    FPWBasePanel::drawOn(s);
    if (!decommit) {
        WORD color = FG::DARK_GREY | (selected == 3 ? BG::DARK_CYAN : BG::GREY);
        s.paintRect(rect.withPadX(2).moved(0, offsets[3] + 2).withH(1), color, false);
    }
}

std::vector<std::wstring> MemFreePanel::header() const {
    return std::vector<std::wstring>{
        L"",
    } + FPWBasePanel::header();
}

std::vector<std::wstring> MemFreePanel::footer() const {
    return std::vector<std::wstring>{
            check(wholeRegion || !decommit) + L" возврат региона целиком",
            check(decommit) + L" возврат физической памяти",
            L"",
    } + FPWBasePanel::footer();
}

void MemFreePanel::spacePressed() {
    if (selected == 3 && decommit) {
        toggle(wholeRegion);
    } else if (selected == 4) {
        toggle(decommit);
    }
    FPWBasePanel::spacePressed();
}

void MemFreePanel::enterPressed() {
    uint64_t regionStartValue, regionSizeValue;
    if (!parseStart(regionStartValue)) {
        MessagePopup::show({L"Некорректный адрес начала региона: '" + regionStart + L"'"}, true);
        return;
    }
    if (wholeRegion || !decommit) {
        regionSizeValue = 0;
    } else if (!parseSize(regionSizeValue)) {
        MessagePopup::show({L"Некорректный размер региона: '" + regionSize + L"'"}, true);
        return;
    }
    DWORD freeType = decommit ? MEM_DECOMMIT : MEM_RELEASE;
    auto success = VirtualFree((LPVOID)regionStartValue, regionSizeValue, freeType);
    if (success == FALSE) {
        DWORD error = GetLastError();
        MessagePopup::show({getLastErrorText(error), L"Код ошибки " + std::to_wstring(error)}, true);
    }
}
