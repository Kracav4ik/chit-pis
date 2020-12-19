#include "LeftPanel.h"

#include "regions.h"

LeftPanel* LeftPanel::visiblePanel = nullptr;

LeftPanel::LeftPanel()
    : rect({0, 0, 42, 23})
    , showRegion(true)
{
}

LeftPanel::~LeftPanel() {
    if (isVisible()) {
        visiblePanel = nullptr;
    }
}

void LeftPanel::setVisible() {
    visiblePanel = this;
    clearSelection();
}

bool LeftPanel::isVisible() const {
    return this == visiblePanel;
}

void LeftPanel::drawVisibleOn(Screen& s) {
    if (visiblePanel) {
        visiblePanel->drawOn(s);
    }
}

bool LeftPanel::parseStart(uint64_t& value) const {
    int pos = 0;
    std::swscanf(regionStart.c_str(), L"%llx%n", &value, &pos);
    return pos > 0;
}

bool LeftPanel::parseSize(uint64_t& value) const {
    static const uint64_t K = 1024;
    static const uint64_t M = K*1024;
    static const uint64_t G = M*1024;

    int pos = 0;
    std::swscanf(regionSize.c_str(), L"%lld%n", &value, &pos);
    if (pos <= 0) {
        return false;
    }
    if (pos < regionSize.size()) {
        wchar_t suffix = regionSize[pos];
        if (suffix == L'K' || suffix == L'К') {
            value *= K;
        } else if (suffix == L'M' || suffix == L'М') {
            value *= M;
        } else if (suffix == L'G' || suffix == L'Г') {
            value *= G;
        }
    }
    return true;
}

void LeftPanel::updateSelection() {
    uint64_t start, size;
    if (showRegion && parseStart(start) && parseSize(size)) {
        setSelection(start, size);
        return;
    }

    clearSelection();
}
