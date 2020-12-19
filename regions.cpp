#include "regions.h"

#include <map>

static uint64_t selectionOffset = 0;
static uint64_t selectionSize = 0;

RegionMode getModeValue(const MEMORY_BASIC_INFORMATION& basInf) {
    if (basInf.State == MEM_FREE) {
        return RegionMode::Free;
    }
    if (basInf.State == MEM_RESERVE) {
        return RegionMode::Uncommitted;
    }
    DWORD mode = basInf.Protect;
    if (mode & PAGE_GUARD) {
        return RegionMode::PageGuard;
    }
    switch (mode) {
        case 0: return RegionMode::ForbiddenRegion;
        case PAGE_NOACCESS: return RegionMode::PageNoAccess;
        case PAGE_READONLY: return RegionMode::PageReadonly;
        case PAGE_READWRITE: return RegionMode::PageReadWrite;
        case PAGE_WRITECOPY: return RegionMode::PageCopyOnWrite;
        case PAGE_EXECUTE: return RegionMode::PageExecuteNoAccess;
        case PAGE_EXECUTE_READ: return RegionMode::PageExecuteReadonly;
        case PAGE_EXECUTE_READWRITE: return RegionMode::PageExecuteReadWrite;
        case PAGE_EXECUTE_WRITECOPY: return RegionMode::PageExecuteCopyOnWrite;
        default: return RegionMode::UnknownValue;
    }
}

std::vector<Region> getRegions(const SYSTEM_INFO& info) {
    return getRegions((uintptr_t)info.lpMinimumApplicationAddress, (uintptr_t)info.lpMaximumApplicationAddress);
}

std::vector<Region> getRegions(uintptr_t startAddr, uintptr_t endAddr) {
    std::vector<Region> result;

    MEMORY_BASIC_INFORMATION basInf;
    while (startAddr <= endAddr) {
        VirtualQuery((LPCVOID)startAddr, &basInf, sizeof(MEMORY_BASIC_INFORMATION));

        RegionMode mode = getModeValue(basInf);
        result.push_back({startAddr, basInf.RegionSize, mode});
        startAddr += basInf.RegionSize;
    }

    return result;
}

Color getRegionColor(RegionMode mode) {
    switch (mode) {
        case RegionMode::PageNoAccess: return Color::DarkGrey;
        case RegionMode::PageReadonly: return Color::DarkBlue;
        case RegionMode::PageReadWrite: return Color::DarkGreen;
        case RegionMode::PageCopyOnWrite: return Color::DarkCyan;
        case RegionMode::PageExecuteNoAccess: return Color::White;
        case RegionMode::PageExecuteReadonly: return Color::Blue;
        case RegionMode::PageExecuteReadWrite: return Color::Green;
        case RegionMode::PageExecuteCopyOnWrite: return Color::Cyan;
        case RegionMode::PageGuard: return Color::DarkRed;
        case RegionMode::ForbiddenRegion: return Color::Red;
        case RegionMode::Free: return Color::Black;
        case RegionMode::Uncommitted: return Color::DarkYellow;
        default: return Color::Magenta;
    }
}

void setSelection(uint64_t offset, uint64_t size) {
    selectionOffset = offset;
    selectionSize = size;
}

void clearSelection() {
    setSelection(0, 0);
}

bool hasSelection() {
    return selectionSize > 0;
}

uint64_t getSelectionOffset() {
    return selectionOffset;
}

uint64_t getSelectionSize() {
    return selectionSize;
}
