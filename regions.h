#pragma once

#include "colors.h"

#include <windows.h>
#include <cstdint>
#include <vector>

enum class RegionMode {
    PageNoAccess,
    PageReadonly,
    PageReadWrite,
    PageCopyOnWrite,
    PageExecuteNoAccess,
    PageExecuteReadonly,
    PageExecuteReadWrite,
    PageExecuteCopyOnWrite,
    PageGuard,              // MEMORY_BASIC_INFORMATION::Protect & PAGE_GUARD != 0
    ForbiddenRegion,        // MEMORY_BASIC_INFORMATION::Protect == 0
    Free,
    Uncommitted,
    UnknownValue,
};

RegionMode getModeValue(const MEMORY_BASIC_INFORMATION& basInf);

struct Region {
    uint64_t offset;
    uint64_t size;
    RegionMode mode;
};

std::vector<Region> getRegions(const SYSTEM_INFO& info);
std::vector<Region> getRegions(uintptr_t startAddr, uintptr_t endAddr);

Color getRegionColor(RegionMode mode);

void setSelection(uint64_t offset, uint64_t size);
void clearSelection();
bool hasSelection();
uint64_t getSelectionOffset();
uint64_t getSelectionSize();
