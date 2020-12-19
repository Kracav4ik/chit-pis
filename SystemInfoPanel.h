#pragma once

#include "LeftPanel.h"
#include "Lines.h"

class SystemInfoPanel : public LeftPanel {
public:
    void setVisible() override;

protected:
    void drawOn(Screen& s) override;

private:
    void updateLines();

    Lines lines;
};
