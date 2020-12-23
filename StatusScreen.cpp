#include "StatusScreen.h"

#include "colors.h"
#include "Screen.h"
#include "utils.h"

static const int PROGRESS_MAX = 12;

std::wstring arrow(int i, int w) {
    std::wstring result(w, L' ');
    int extra = 3;
    i %= (result.size() + extra);
    for (int k = 0; k < extra; ++k) {
        if (0 <= i - k && i - k < result.size()) {
            result[i - k] = L'>';
        }
    }
    return result;
}

StatusScreen::StatusScreen(bool isChit, int number, int waitMs)
    : isChit(isChit)
    , number(number)
    , state(State::Inactive)
    , activePage(0)
    , stateProgress(0)
    , waitMs(waitMs)
    , arrowTick(0)
{
    updateLines();
}

void StatusScreen::updateState(State s, int page, float progress) {
    state = s;
    activePage = page;
    stateProgress = progress;
    waitMs = 0;
    if (progress == 0) {
        arrowTick = 0;
    }
    updateLines();
}

void StatusScreen::updateWait(int wait) {
    waitMs = wait;
    updateLines();
}

void StatusScreen::tickAnim() {
    ++arrowTick;
}

void StatusScreen::drawOn(Screen& s) {
    lines.drawOn(s, {0, 0, s.w(), s.h()});
    Rect lineNum{15, 0, 2, 1};
    for (int page = 0; page < 12; ++page) {
        WORD fg = FG::DARK_GREY;
        if (page == activePage) {
            fg = FG::WHITE;
            int progressValue = roundI(stateProgress * PROGRESS_MAX);
            progressValue = clamp(0, progressValue, PROGRESS_MAX);
            WORD progressColor = BG::DARK_GREEN;
            s.paintRect(lineNum.moved(3, page).withW(PROGRESS_MAX), FG::WHITE | BG::DARK_GREY, false);
            s.paintRect(lineNum.moved(3, page).withW(progressValue), FG::WHITE | progressColor, false);
        }
        s.paintRect(lineNum.moved(0, page), fg | BG::BLACK, false);
    }
    if (state == State::Waiting) {
        s.paintRect({2, 3, 10, 1}, FG::WHITE | BG::DARK_RED, false);
    }
    s.paintRect({2, 10, 10, 1}, FG::BLACK | BG::GREY, false);
}

void StatusScreen::updateLines() {
    std::wstring chitPis = isChit ? L"ЧИТАТЕЛЬ" : L"ПИСАТЕЛЬ";
    std::wstring num = align(std::to_wstring(number), 2);

    std::wstring empty(14, L' ');
    std::wstring emptyMid = empty + L"│";

    std::wstring waitComment = emptyMid;
    std::wstring waitTime = emptyMid;
    if (state == State::Inactive) {
        waitComment = L" старт через: │";
        if (waitMs > 0) {
            waitTime = align(std::to_wstring(waitMs), 7, false) + L" мсек  │";
        }
    } else if (state == State::Waiting) {
        waitComment = L"   ОЖИДАНИЕ   │";
    }

    std::vector<std::wstring> rows = {
            empty + L"┌",
            L" " + chitPis + L" №" + num + L" │",
            emptyMid,
            waitComment,
            waitTime,
            emptyMid,
            L"   СТРАНИЦА ──┤",
            L"     ФАЙЛА    │",
            emptyMid,
            emptyMid,
            L"  F10  Выход  │",
            empty + L"└",
    };
    for (int page = 0; page < 12; ++page) {
        rows[page] += align(std::to_wstring(page), 2, false, L'0');
        if (activePage == page) {
            if (state == State::Reading) {
                rows[page] += L" " + arrow(arrowTick, 5) + L"ЧТЕНИЕ";
                continue;
            } else if (state == State::Writing) {
                rows[page] += L"  ЗАПИСЬ" + arrow(arrowTick, 5);
                continue;
            }
        }
    }
    lines.setLines(styledText(std::move(rows), FG::GREY | BG::BLACK));
}
