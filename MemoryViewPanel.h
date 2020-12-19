#pragma once

#include "Screen.h"
#include "regions.h"
#include <vector>

class MemoryViewPanel {
public:
    MemoryViewPanel(Rect rect, const SYSTEM_INFO& info);

    bool zoomIn();
    bool zoomOut();
    void upCell();
    void downCell();
    void upLine();
    void downLine();
    void upPage();
    void downPage();
    void toBegin();
    void toEnd();

    void registerKeys(Screen& screen);
    void drawOn(Screen& s);
    void updateRegions();

private:
    Rect frameRect() const;
    Rect pixelRect() const;
    int getPixelArea() const;
    int getLinesCount() const;
    uint64_t getLineBytes() const;
    uint64_t getAreaBytes() const;
    std::wstring getTitle() const;
    Color getCellColor(uint64_t startAddress) const;
    std::vector<Color> getPixels() const;
    std::vector<Color> getScrollbar() const;

    Rect rect;
    uint64_t beginAddr;
    uint64_t endAddr;
    uint64_t pageSize;
    uint64_t curOffset;
    uint64_t cellSize;
    std::map<uint64_t, Region> regionOffsets;
};
