#include "MemoryViewPanel.h"

#include "utils.h"

#include <sstream>
#include <cmath>

static const uint64_t ZOOM_STEP = 2;

static const Color BACKGROUND = Color::Grey;
static const Color MIXED_REGION = Color::DarkMagenta;
static const Color SELECTION_REGION = Color::Yellow;

static std::wstring getCellStr(uint64_t cell) {
    static const uint64_t K = 1024;
    static const uint64_t M = K*1024;
    static const uint64_t G = M*1024;
    std::wstringstream result;
    if (cell < K) {
        result << cell << L"Б";
    } else  if (cell < M) {
        result << cell / K << L"К";
    } else if (cell < G) {
        result << cell / M << L"М";
    } else {
        result << cell / G << L"Г";
    }
    return result.str();
}

MemoryViewPanel::MemoryViewPanel(Rect rect, const SYSTEM_INFO& info)
    : rect(rect)
    , beginAddr((uintptr_t)info.lpMinimumApplicationAddress)
    , endAddr((uintptr_t)info.lpMaximumApplicationAddress + 1)
    , pageSize(info.dwPageSize)
    , curOffset(0)
    , cellSize(pageSize)
{
    while (zoomOut()) {
        // do nothing
    }
}

bool MemoryViewPanel::zoomIn() {
    if (cellSize == pageSize) {
        return false;
    }
    cellSize = std::max(pageSize, cellSize / ZOOM_STEP);
    return true;
}

bool MemoryViewPanel::zoomOut() {
    if (getAreaBytes() >= endAddr - beginAddr) {
        return false;
    }
    cellSize *= ZOOM_STEP;
    if (curOffset + getAreaBytes() >= endAddr) {
        toEnd();
    }
    return true;
}

void MemoryViewPanel::upCell() {
    if (curOffset < cellSize) {
        curOffset = 0;
    } else {
        curOffset -= cellSize;
    }
}

void MemoryViewPanel::downCell() {
    if (curOffset + getAreaBytes() + cellSize < endAddr + getLineBytes()) {
        curOffset += cellSize;
    }
}

void MemoryViewPanel::upLine() {
    if (curOffset < getLineBytes()) {
        curOffset = 0;
    } else {
        curOffset -= getLineBytes();
    }
}

void MemoryViewPanel::downLine() {
    if (curOffset + getAreaBytes() < endAddr) {
        curOffset += getLineBytes();
    }
}

void MemoryViewPanel::upPage() {
    for (int i = 0; i < getLinesCount()/2; ++i) {
        upLine();
    }
}

void MemoryViewPanel::downPage() {
    for (int i = 0; i < getLinesCount()/2; ++i) {
        downLine();
    }
}

void MemoryViewPanel::toBegin() {
    curOffset = 0;
}

void MemoryViewPanel::toEnd() {
    uint64_t linesCount = (uint64_t)ceil(endAddr / (double)getLineBytes());
    uint64_t line = 0;
    if (linesCount > getLinesCount()) {
        line = linesCount - getLinesCount();
    }
    curOffset = line * getLineBytes();
}

void MemoryViewPanel::registerKeys(Screen& screen) {
    screen.handleKey(VK_LEFT, ANY_CTRL_PRESSED, [this]() {
        upCell();
    });
    screen.handleKey(VK_RIGHT, ANY_CTRL_PRESSED, [this]() {
        downCell();
    });
    screen.handleKey(VK_UP, ANY_CTRL_PRESSED, [this]() {
        upLine();
    });
    screen.handleKey(VK_DOWN, ANY_CTRL_PRESSED, [this]() {
        downLine();
    });
    screen.handleKey(VK_PRIOR, ANY_CTRL_PRESSED, [this]() {
        upPage();
    });
    screen.handleKey(VK_NEXT, ANY_CTRL_PRESSED, [this]() {
        downPage();
    });
    screen.handleKey(VK_HOME, ANY_CTRL_PRESSED, [this]() {
        toBegin();
    });
    screen.handleKey(VK_END, ANY_CTRL_PRESSED, [this]() {
        toEnd();
    });
    screen.handleKey(VK_ADD, ANY_CTRL_PRESSED, [this]() {
        zoomIn();
    });
    screen.handleKey(VK_SUBTRACT, ANY_CTRL_PRESSED, [this]() {
        zoomOut();
    });
}

void MemoryViewPanel::drawOn(Screen& s) {
    updateRegions();
    s.paintRect(rect, FG::WHITE | BG::GREY);
    s.paintRect(rect.withH(1), FG::BLACK | BG::GREY);
    s.boundedLine(rect.getLeftTop(), rect.w, L"Карта виртуальной памяти", true);
    Rect frame = frameRect();
    s.frame(frame, false);
    s.labels(frame.withH(1), {getTitle()}, FG::BLACK | BG::GREY);
    Rect pixels = pixelRect();
    s.pixelMap(pixels, getPixels(), BACKGROUND);
    s.pixelMap(pixels.moved(pixels.w, 0).withW(1), getScrollbar(), BACKGROUND);
}

void MemoryViewPanel::updateRegions() {
    regionOffsets.clear();
    for (const auto& region : getRegions(beginAddr, endAddr - 1)) {
        regionOffsets.emplace(region.offset, region);
    }
}

Rect MemoryViewPanel::frameRect() const {
    return rect.moved(0, 1).withH(rect.h - 1);
}

Rect MemoryViewPanel::pixelRect() const {
    return frameRect().withPadding(1, 1);
}

int MemoryViewPanel::getPixelArea() const {
    return pixelRect().getArea() * 2;
}

int MemoryViewPanel::getLinesCount() const {
    return 2*pixelRect().h;
}

uint64_t MemoryViewPanel::getLineBytes() const {
    return pixelRect().w * cellSize;
}

uint64_t MemoryViewPanel::getAreaBytes() const {
    return getPixelArea() * cellSize;
}

std::wstring MemoryViewPanel::getTitle() const {
    std::wstringstream result;
    result << std::hex << L"Смещение 0x" << curOffset << L", клетка " << getCellStr(cellSize);
    return result.str();
}

Color MemoryViewPanel::getCellColor(uint64_t startAddress) const {
    if (hasSelection()) {
        if (startAddress < getSelectionOffset() + getSelectionSize() && startAddress + cellSize > getSelectionOffset()) {
            return SELECTION_REGION;
        }
    }
    if (startAddress < beginAddr) {
        if (startAddress + cellSize <= beginAddr) {
            return BACKGROUND;
        } else {
            return MIXED_REGION;
        };
    }
    auto beginIter = regionOffsets.lower_bound(startAddress);
    if (beginIter != regionOffsets.begin()) {
        auto prev = beginIter;
        --prev;
        if (prev->second.offset + prev->second.size > startAddress) {
            beginIter = prev;
        }
    }
    uint64_t endAddress = startAddress + cellSize;
    auto endIter = regionOffsets.lower_bound(endAddress);
    std::optional<RegionMode> commonMode;
    for (auto iter = beginIter; iter != endIter; ++iter) {
        RegionMode mode = iter->second.mode;
        if (commonMode && *commonMode != mode) {
            return MIXED_REGION;
        }
        commonMode = mode;
    }
    if (commonMode) {
        return getRegionColor(*commonMode);
    }
    return getRegionColor(RegionMode::UnknownValue);
}

std::vector<Color> MemoryViewPanel::getPixels() const {
    std::vector<Color> result;
    uint64_t offset = curOffset;
    for (int i = 0; i < getPixelArea() && offset < endAddr; ++i, offset += cellSize) {
        result.push_back(getCellColor(offset));
    }
    return result;
}

std::vector<Color> MemoryViewPanel::getScrollbar() const {
    int linesCount = getLinesCount();
    std::vector<Color> result(linesCount, BACKGROUND);
    double addrDist = endAddr;
    int scrollBegin = roundI(linesCount * curOffset / addrDist);
    int scrollSize = clamp(1, roundI(linesCount * getAreaBytes() / addrDist), (int)result.size());
    scrollBegin = std::min(scrollBegin, linesCount - scrollSize);
    for (int i = scrollBegin; i < scrollBegin + scrollSize; ++i) {
        result[i] = Color::White;
    }
    return result;
}
