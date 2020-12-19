#include "MemoryRegionsList.h"

#include "utils.h"

MemoryRegionsList::MemoryRegionsList(Rect rect, const SYSTEM_INFO& info, std::function<void()> selectionChanged)
    : rect(rect)
    , startAddr((uintptr_t)info.lpMinimumApplicationAddress)
    , endAddr((uintptr_t)info.lpMaximumApplicationAddress)
    , selectionChanged(std::move(selectionChanged))
    , isActive(false)
{
    updateRegions();
}

void MemoryRegionsList::updateRegions() {
    regions = getRegions(startAddr, endAddr);
    std::vector<std::wstring> rows;
    rows.reserve(regions.size());
    for (const auto& region : regions) {
        rows.emplace_back(L"0x" + toHex(region.offset, 12) + L" " + std::to_wstring(region.size));
    }
    WORD selectedColor = isActive ? FG::BLACK | BG::DARK_CYAN : FG::BLACK | BG::DARK_GREY;
    regionsText.setLines(styledText(std::move(rows), FG::BLACK | BG::GREY, selectedColor));
    if (hasRegions() && !regionsText.hasSelection()) {
        regionsText.setSelectedIdx(0);
    }
}

void MemoryRegionsList::setActive(bool active) {
    if (isActive == active) {
        return;
    }
    isActive = active;
    updateRegions();
    selectionChanged();
}

bool MemoryRegionsList::hasRegions() const {
    return !regions.empty();
}

Region MemoryRegionsList::selectedRegion() const {
    return regions[regionsText.getSelectedIdx()];
}

std::wstring MemoryRegionsList::selectedStart() const {
    if (!hasRegions()) {
        return L"";
    }
    auto region = selectedRegion();
    return toHex(region.offset, 0);
}

std::wstring MemoryRegionsList::selectedSize() const {
    if (!hasRegions()) {
        return L"";
    }
    static const uint64_t K = 1024;
    static const uint64_t M = K*1024;
    static const uint64_t G = M*1024;
    auto region = selectedRegion();
    if (region.size % G == 0) {
        return std::to_wstring(region.size/G) + L"Г";
    }
    if (region.size % M == 0) {
        return std::to_wstring(region.size/M) + L"М";
    }
    if (region.size % K == 0) {
        return std::to_wstring(region.size/K) + L"К";
    }
    return std::to_wstring(region.size);
}

void MemoryRegionsList::registerKeys(Screen& screen, std::function<bool()> canHandle) {
    auto canHandleKeys = [this, canHandle = std::move(canHandle)]() {
        return isActive && canHandle();
    };
    screen.handleKey(canHandleKeys, VK_UP, 0, [this]() {
        regionsText.selectPrev();
        regionsText.scrollToSelection(linesRect().h);
        selectionChanged();
    });
    screen.handleKey(canHandleKeys, VK_DOWN, 0, [this]() {
        regionsText.selectNext();
        regionsText.scrollToSelection(linesRect().h);
        selectionChanged();
    });
    screen.handleKey(canHandleKeys, VK_PRIOR, 0, [this]() {
        regionsText.moveSelection(-linesRect().h + 1);
        regionsText.scrollToSelection(linesRect().h);
        selectionChanged();
    });
    screen.handleKey(canHandleKeys, VK_NEXT, 0, [this]() {
        regionsText.moveSelection(linesRect().h - 1);
        regionsText.scrollToSelection(linesRect().h);
        selectionChanged();
    });
    screen.handleKey(canHandleKeys, VK_HOME, 0, [this]() {
        regionsText.selectFirst();
        regionsText.scrollToSelection(linesRect().h);
        selectionChanged();
    });
    screen.handleKey(canHandleKeys, VK_END, 0, [this]() {
        regionsText.selectLast();
        regionsText.scrollToSelection(linesRect().h);
        selectionChanged();
    });
}

void MemoryRegionsList::drawOn(Screen& s) {
    updateRegions();
    s.frame(rect, false);
    s.textOut(rect.moved(1, 0).getLeftTop(), L" Регионы виртуальной памяти ");
    regionsText.drawOn(s, linesRect());
    Rect regionMini = linesRect().moved(-3, 0).withW(2).withH(1);
    for (int i = 0; i < linesRect().h; ++i) {
        const auto& region = regions[regionsText.getScrollOffset() + i];
        s.paintRect(regionMini.moved(0, i), toBg(getRegionColor(region.mode)));
    }
}

Rect MemoryRegionsList::linesRect() const {
    return rect.withW(rect.w - 1).withPadding(4, 1).moved(2, 0);
}
