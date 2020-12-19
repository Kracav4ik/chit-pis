#pragma once

#include "LeftPanel.h"
#include "Lines.h"

class EditableText;

class MemAllocPanel : public LeftPanel {
public:
    MemAllocPanel(Screen& screen, const SYSTEM_INFO& info);

    void setVisible() override;
    void registerKeys(Screen& screen);

protected:
    void drawOn(Screen& s) override;

private:
    bool editIsSelected() const;
    void updateText();
    void updateLines();

    int selected;
    EditableText& editable;
    Lines lines;
    bool autoAllocate;
    bool allocateCommit;
};
