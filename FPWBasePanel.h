#pragma once

#include "LeftPanel.h"
#include "Lines.h"
#include "MemoryRegionsList.h"

#include <vector>

class EditableText;

class FPWBasePanel : public LeftPanel {
public:
    FPWBasePanel(
            Screen& screen,
            const SYSTEM_INFO& info,
            std::wstring title,
            std::wstring enterLabel,
            std::vector<int> offsets,
            int regionsY,
            int regionsHeight
    );

    void setVisible() override;
    void registerKeys(Screen& screen);

protected:
    void drawOn(Screen& s) override;

    bool editIsSelected() const;
    virtual void updateText();
    void updateLines();

    virtual std::vector<std::wstring> header() const;
    virtual std::vector<std::wstring> footer() const;

    virtual void spacePressed();
    virtual void enterPressed() = 0;

    std::wstring title;
    std::wstring enterLabel;
    const std::vector<int> offsets;
    int regionsHeight;

    int selected;
    EditableText& editable;
    Lines lines;
    MemoryRegionsList regionsList;
};
