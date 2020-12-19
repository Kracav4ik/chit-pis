#pragma once

#include "LeftPanel.h"
#include "Lines.h"

class LegendPanel : public LeftPanel {
public:
    LegendPanel();

protected:
    void drawOn(Screen& s) override;

private:
    Lines lines;
};
