#pragma once

#include "Lines.h"

class StatusScreen {
public:
    StatusScreen(bool isChit, int number);

    void drawOn(Screen& s);

private:
    bool isChit;
    int number;
    Lines lines;
};
