#include "FPWBasePanel.h"

#include "utils.h"
#include "MessagePopup.h"

FPWBasePanel::FPWBasePanel(
        Screen& screen,
        const SYSTEM_INFO& info,
        std::wstring title,
        std::wstring enterLabel,
        std::vector<int> offsets,
        int regionsY,
        int regionsHeight
)
    : title(std::move(title))
    , enterLabel(L" <Enter> " + enterLabel + L" ")
    , offsets(std::move(offsets))
    , regionsHeight(regionsHeight)
    , selected(0)
    , editable(screen.getEditable())
    , regionsList(
            rect.moved(0, regionsY).withPadX(2).withH(regionsHeight),
            info,
            [this]() { updateText(); updateLines(); }
    )
{
    regionStart = toHex((uintptr_t) info.lpMinimumApplicationAddress, 0);
    regionSize = L"64К";
}

void FPWBasePanel::setVisible() {
    LeftPanel::setVisible();
    editable.setEnabledGetter([this]() {
        return isVisible() && !MessagePopup::isVisible() && editIsSelected();
    });
    updateLines();
    updateSelection();
}

void FPWBasePanel::registerKeys(Screen& screen) {
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
    regionsList.registerKeys(screen, canHandle);
    screen.handleKey(canHandle, VK_SPACE, 0, [this]() {
        spacePressed();
        updateLines();
    });
    screen.handleKey(canHandle, VK_RETURN, 0, [this]() {
        updateText();
        enterPressed();
    });
}

void FPWBasePanel::drawOn(Screen& s) {
    s.paintRect(rect, FG::BLACK | BG::GREY);
    s.boundedLine(rect.getLeftTop(), rect.w, title, true);

    Rect frame = rect.moved(0, 1).withH(rect.h - 1);
    s.boundedLine(frame.getLeftTop(), frame.w, std::wstring(rect.w - 2, L'═'), true);
    Rect linesRect = frame.withPadding(2, 1);
    lines.drawOn(s, linesRect);
    s.paintRect(linesRect.moved(0, offsets[1] + 1).withH(1), FG::DARK_GREY | BG::GREY, false);

    if (editIsSelected()) {
        COORD origin = linesRect.moved(2, offsets[selected]).getLeftTop();
        if (selected == 0) {
            s.paintRect({origin.X, origin.Y, 2, 1}, FG::DARK_GREY | BG::DARK_CYAN, false);
            origin.X += 2;
        }
        editable.drawOn(s, origin, !MessagePopup::isVisible());
    }

    regionsList.drawOn(s);

    s.labels(linesRect.moved(0, 19).withH(1), {enterLabel}, FG::BLACK | BG::DARK_CYAN);
}

bool FPWBasePanel::editIsSelected() const {
    return selected == 0 || selected == 1;
}

void FPWBasePanel::updateText() {
    if (selected == 0) {
        regionStart = editable.getText();
    } else if (selected == 1) {
        regionSize = editable.getText();
    } else if (selected == 2) {
        if (!regionsList.hasRegions()) {
            return;
        }
        regionStart = regionsList.selectedStart();
        regionSize = regionsList.selectedSize();
    }
    updateSelection();
}

void FPWBasePanel::updateLines() {
    std::vector<std::wstring> rows = header() + std::vector<std::wstring>(regionsHeight + 2) + footer();

    lines.setLines(styledText(std::move(rows), FG::BLACK | BG::GREY, FG::BLACK | BG::DARK_CYAN));
    lines.setSelectedIdx(offsets[selected]);
    if (selected == 0) {
        editable.setTextSizeMax(16);
        editable.setText(regionStart, 17);
    } else if (selected == 1) {
        editable.setTextSizeMax(20);
        editable.setText(regionSize, 21);
    }
    regionsList.setActive(selected == 2);
}

std::vector<std::wstring> FPWBasePanel::header() const {
    return {
            L"Адрес начала региона:",
            selected != 0 ? L"  0x" + regionStart : L"  0x",
            L"",
            L"Размер региона:",
            selected != 1 ? L"  " + regionSize : L"",
            L"  1К = 1024, 1М = 1024К, 1Г = 1024М",
    };
}

std::vector<std::wstring> FPWBasePanel::footer() const {
    return {
            check(showRegion) + L" показывать регион на карте",
    };
}

void FPWBasePanel::spacePressed() {
    if (selected == offsets.size() - 1) {
        toggle(showRegion);
        updateSelection();
    }
}
