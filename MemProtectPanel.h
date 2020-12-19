#pragma once

#include "FPWBasePanel.h"

class MemProtectPanel : public FPWBasePanel {
public:
    MemProtectPanel(Screen& screen, const SYSTEM_INFO& info);

protected:
    void drawOn(Screen& s) override;
    void updateText() override;

    std::vector<std::wstring> footer() const override;
    void spacePressed() override;
    void enterPressed() override;

private:
    bool flagNoAccess;
    bool flagReadonly;
    bool flagReadWrite;
    bool flagCopyOnWrite;
    bool flagExecute;
    bool flagGuard;
};
