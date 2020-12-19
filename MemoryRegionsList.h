#pragma once

#include "Screen.h"
#include "Lines.h"
#include "regions.h"
#include <functional>

class MemoryRegionsList {
public:
    MemoryRegionsList(Rect rect, const SYSTEM_INFO& info, std::function<void()> selectionChanged);

    void updateRegions();
    void setActive(bool active);
    bool hasRegions() const;
    Region selectedRegion() const;
    std::wstring selectedStart() const;
    std::wstring selectedSize() const;

    void registerKeys(Screen& screen, std::function<bool()> canHandle);
    void drawOn(Screen& s);

private:
    Rect linesRect() const;

    Rect rect;
    uintptr_t startAddr;
    uintptr_t endAddr;
    std::function<void()> selectionChanged;
    bool isActive;
    std::vector<Region> regions;
    Lines regionsText;
};
