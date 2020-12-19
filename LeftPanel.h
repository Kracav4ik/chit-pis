#pragma once

#include "Screen.h"

class LeftPanel {
public:
    LeftPanel();
    LeftPanel(const LeftPanel&) = delete;
    LeftPanel& operator=(const LeftPanel&) = delete;
    virtual ~LeftPanel();

    virtual void setVisible();
    bool isVisible() const;

    static void drawVisibleOn(Screen& s);

protected:
    virtual void drawOn(Screen& s) = 0;

    bool parseStart(uint64_t& value) const;
    bool parseSize(uint64_t& value) const;
    void updateSelection();

    Rect rect;
    std::wstring regionStart;
    std::wstring regionSize;
    bool showRegion;

private:
    static LeftPanel* visiblePanel;
};
