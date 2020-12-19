#include "MemAllocPanel.h"

#include "utils.h"
#include "MessagePopup.h"

static const int offsets[] = {2, 4, 9, 14, 16};

static uint64_t mid(const SYSTEM_INFO& info) {
    return (
        (uintptr_t)info.lpMinimumApplicationAddress +
        (uintptr_t)info.lpMaximumApplicationAddress +
        1
    )/2;
}

MemAllocPanel::MemAllocPanel(Screen& screen, const SYSTEM_INFO& info)
    : selected(0)
    , editable(screen.getEditable())
    , autoAllocate(false)
    , allocateCommit(false)
{
    regionStart = toHex(mid(info), 0);
    regionSize = L"16М";
}

void MemAllocPanel::setVisible() {
    LeftPanel::setVisible();
    editable.setEnabledGetter([this]() {
        return isVisible() && !MessagePopup::isVisible() && editIsSelected();
    });
    updateLines();
    updateSelection();
}

void MemAllocPanel::registerKeys(Screen& screen) {
    auto canHandle = [this]() {
        return isVisible() && !MessagePopup::isVisible();
    };
    screen.handleKey(canHandle, VK_TAB, SHIFT_PRESSED, [this]() {
        updateText();
        selected = prevIdx(selected, offsets);
        updateLines();
    });
    screen.handleKey(canHandle, VK_TAB, 0, [this]() {
        updateText();
        selected = nextIdx(selected, offsets);
        updateLines();
    });
    screen.handleKey(canHandle, VK_SPACE, 0, [this]() {
        if (selected == 1) {
            toggle(autoAllocate);
        } else if (selected == 3) {
            toggle(allocateCommit);
        } else if (selected == 4) {
            toggle(showRegion);
            updateSelection();
        }
        updateLines();
    });
    screen.handleKey(canHandle, VK_RETURN, 0, [this]() {
        updateText();
        uint64_t regionStartValue, regionSizeValue;
        if (autoAllocate) {
            regionStartValue = 0;
        } else if (!parseStart(regionStartValue)) {
            MessagePopup::show({L"Некорректный адрес начала региона: '" + regionStart + L"'"}, true);
            return;
        }
        if (!parseSize(regionSizeValue)) {
            MessagePopup::show({L"Некорректный размер региона: '" + regionSize + L"'"}, true);
            return;
        }
        DWORD allocType = allocateCommit ? MEM_COMMIT : MEM_RESERVE;
        LPVOID addr = VirtualAlloc((LPVOID) regionStartValue, regionSizeValue, allocType, PAGE_READWRITE);
        if (!addr) {
            DWORD error = GetLastError();
            MessagePopup::show({getLastErrorText(error), L"Код ошибки " + std::to_wstring(error)}, true);
        } else if (autoAllocate) {
            MessagePopup::show({L"Память выделена по адресу 0x" + toHex((intptr_t)addr, 12)}, true);
        }
    });
}

void MemAllocPanel::drawOn(Screen& s) {
    s.paintRect(rect, FG::BLACK | BG::GREY);
    s.boundedLine(rect.getLeftTop(), rect.w, L"Резервирование памяти", true);

    Rect frame = rect.moved(0, 1).withH(rect.h - 1);
    s.boundedLine(frame.getLeftTop(), frame.w, std::wstring(rect.w - 2, L'═'), true);
    Rect linesRect = frame.withPadding(2, 1);
    lines.drawOn(s, linesRect);
    s.paintRect(linesRect.moved(0, 10).withH(1), FG::DARK_GREY | BG::GREY, false);

    if (autoAllocate) {
        WORD bgColor = selected == 0 ? BG::DARK_CYAN : BG::GREY;
        s.paintRect(linesRect.moved(0, offsets[0]).withH(1), FG::DARK_GREY | bgColor, false);
    }

    if (editIsSelected()) {
        COORD origin = linesRect.moved(2, offsets[selected]).getLeftTop();
        if (selected == 0) {
            s.paintRect({origin.X, origin.Y, 2, 1}, FG::DARK_GREY | BG::DARK_CYAN, false);
            origin.X += 2;
        }
        editable.drawOn(s, origin, !MessagePopup::isVisible());
    }

    s.labels(linesRect.moved(0, 19).withH(1), {L" <Enter> Зарезервировать память "}, FG::BLACK | BG::DARK_CYAN);
}

bool MemAllocPanel::editIsSelected() const {
    return (selected == 0 && !autoAllocate) || selected == 2;
}

void MemAllocPanel::updateText() {
    if (selected == 0) {
        regionStart = editable.getText();
    } else if (selected == 2) {
        regionSize = editable.getText();
    }
    updateSelection();
}

void MemAllocPanel::updateLines() {
    std::wstring startLabel = L"  0x";
    if (selected != 0 || autoAllocate) {
        startLabel += regionStart;
    }
    std::vector<std::wstring> rows = {
            L"",
            L"Адрес начала резервирования:",
            startLabel,
            L"",
            check(autoAllocate) + L" автоматическое резервирование",
            L"",
            std::wstring(rect.w - 6, L'─'),
            L"",
            L"Размер резервируемого региона:",
            selected != 2 ? L"  " + regionSize : L"",
            L"  1К = 1024, 1М = 1024К, 1Г = 1024М",
            L"",
            std::wstring(rect.w - 6, L'─'),
            L"",
            check(allocateCommit) + L" передача физической памяти",
            L"",
            check(showRegion) + L" показывать регион на карте",
    };
    lines.setLines(styledText(std::move(rows), FG::BLACK | BG::GREY, FG::BLACK | BG::DARK_CYAN));
    lines.setSelectedIdx(offsets[selected]);
    if (selected == 0) {
        editable.setTextSizeMax(16);
        editable.setText(regionStart, 17);
    } else if (selected == 2) {
        editable.setTextSizeMax(20);
        editable.setText(regionSize, 21);
    }
}
