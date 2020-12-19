#pragma once

#include "FPWBasePanel.h"

class MemFreePanel : public FPWBasePanel {
public:
    MemFreePanel(Screen& screen, const SYSTEM_INFO& info);

protected:
    void drawOn(Screen& s) override;

    std::vector<std::wstring> header() const override;
    std::vector<std::wstring> footer() const override;
    void spacePressed() override;
    void enterPressed() override;

private:
    bool wholeRegion;
    bool decommit;
};
