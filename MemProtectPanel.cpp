#include "MemProtectPanel.h"

#include "utils.h"
#include "MessagePopup.h"

MemProtectPanel::MemProtectPanel(Screen& screen, const SYSTEM_INFO& info)
    : FPWBasePanel(
        screen,
        info,
        L"Защита памяти",
        L"Изменить защиту региона",
        {1, 4, -1, 13, 13, 14, 14, 15, 15, 17},
        9,
        5
    )
    , flagNoAccess(true)
    , flagReadonly(false)
    , flagReadWrite(false)
    , flagCopyOnWrite(false)
    , flagExecute(false)
    , flagGuard(false)
{
}

void MemProtectPanel::drawOn(Screen& s) {
    FPWBasePanel::drawOn(s);

    if (3 <= selected && selected <= 8) {
        Rect overpaint = rect.moved(2, offsets[selected] + 2).withH(1);
        if (selected % 2 == 0) {
            overpaint.w = 18;
        } else {
            overpaint = overpaint.moved(18, 0).withW(20);
        }
        s.paintRect(overpaint, FG::BLACK | BG::GREY, false);
    }
}

void MemProtectPanel::updateText() {
    FPWBasePanel::updateText();
    if (selected == 2) {
        if (!regionsList.hasRegions()) {
            return;
        }
        auto region = regionsList.selectedRegion();
        flagNoAccess = false;
        flagReadonly = false;
        flagReadWrite = false;
        flagCopyOnWrite = false;
        flagExecute = false;
        flagGuard = false;
        RegionMode mode = region.mode;
        if (mode == RegionMode::PageGuard) {
            flagGuard = true;
            MEMORY_BASIC_INFORMATION basInf;
            VirtualQuery((LPCVOID)region.offset, &basInf, sizeof(MEMORY_BASIC_INFORMATION));
            basInf.Protect &= ~((DWORD)PAGE_GUARD);
            mode = getModeValue(basInf);
        }
        switch (mode) {
            case RegionMode::PageNoAccess:
                flagNoAccess = true;
                break;
            case RegionMode::PageReadonly:
                flagReadonly = true;
                break;
            case RegionMode::PageReadWrite:
                flagReadWrite = true;
                break;
            case RegionMode::PageCopyOnWrite:
                flagCopyOnWrite = true;
                break;
            case RegionMode::PageExecuteNoAccess:
                flagNoAccess = true;
                flagExecute = true;
                break;
            case RegionMode::PageExecuteReadonly:
                flagReadonly = true;
                flagExecute = true;
                break;
            case RegionMode::PageExecuteReadWrite:
                flagReadWrite = true;
                flagExecute = true;
                break;
            case RegionMode::PageExecuteCopyOnWrite:
                flagCopyOnWrite = true;
                flagExecute = true;
                break;
            default:
                flagNoAccess = true;
        }
    }
}

std::vector<std::wstring> MemProtectPanel::footer() const {
    return std::vector<std::wstring>{
            radio(flagNoAccess) + L" нет доступа   " + radio(flagReadonly) + L" только чтение",
            radio(flagReadWrite) + L" чтение/запись " + radio(flagCopyOnWrite) + L" копия при записи",
            check(flagExecute) + L" исполняемая   " + check(flagGuard) + L" защищенная",
            L"",
    } + FPWBasePanel::footer();
}

void MemProtectPanel::spacePressed() {
    if (selected == 3) {
        flagNoAccess = true;
        flagReadonly = false;
        flagReadWrite = false;
        flagCopyOnWrite = false;
    } else if (selected == 4) {
        flagNoAccess = false;
        flagReadonly = true;
        flagReadWrite = false;
        flagCopyOnWrite = false;
    } else if (selected == 5) {
        flagNoAccess = false;
        flagReadonly = false;
        flagReadWrite = true;
        flagCopyOnWrite = false;
    } else if (selected == 6) {
        flagNoAccess = false;
        flagReadonly = false;
        flagReadWrite = false;
        flagCopyOnWrite = true;
    } else if (selected == 7) {
        toggle(flagExecute);
    } else if (selected == 8) {
        toggle(flagGuard);
    }
    FPWBasePanel::spacePressed();
}

void MemProtectPanel::enterPressed() {
    uint64_t regionStartValue, regionSizeValue;
    if (!parseStart(regionStartValue)) {
        MessagePopup::show({L"Некорректный адрес начала региона: '" + regionStart + L"'"}, true);
        return;
    }
    if (!parseSize(regionSizeValue)) {
        MessagePopup::show({L"Некорректный размер региона: '" + regionSize + L"'"}, true);
        return;
    }

    DWORD protect;
    if (flagNoAccess) {
        protect = flagExecute ? PAGE_EXECUTE : PAGE_NOACCESS;
    } else if (flagReadonly) {
        protect = flagExecute ? PAGE_EXECUTE_READ : PAGE_READONLY;
    } else if (flagReadWrite) {
        protect = flagExecute ? PAGE_EXECUTE_READWRITE : PAGE_READWRITE;
    } else if (flagCopyOnWrite) {
        protect = flagExecute ? PAGE_EXECUTE_WRITECOPY : PAGE_WRITECOPY;
    }
    if (flagGuard) {
        protect |= PAGE_GUARD;
    }
    DWORD oldProtect;
    auto success = VirtualProtect((LPVOID)regionStartValue, regionSizeValue, protect, &oldProtect);
    if (success == FALSE) {
        DWORD error = GetLastError();
        MessagePopup::show({getLastErrorText(error), L"Код ошибки " + std::to_wstring(error)}, true);
    }
}
