#pragma once

#include "FPWBasePanel.h"

class MemWritePanel : public FPWBasePanel {
public:
    MemWritePanel(Screen& screen, const SYSTEM_INFO& info);

protected:
    std::vector<std::wstring> header() const override;
    void enterPressed() override;
};
