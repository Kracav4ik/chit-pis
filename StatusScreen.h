#pragma once

#include "Lines.h"

enum class State {
    Inactive,
    Reading,
    Writing,
    Waiting,
};

class StatusScreen {
public:
    StatusScreen(bool isChit, int number, int waitMs);

    void updateState(State s, int page, float progress = 0);
    void updateWait(int wait);
    void tickAnim();

    void drawOn(Screen& s);

private:
    void updateLines();

    bool isChit;
    int number;
    State state;
    int activePage;
    float stateProgress;
    int waitMs;
    int arrowTick;
    Lines lines;
};
