#include "StatusScreen.h"

#include "colors.h"
#include "Screen.h"

std::wstring arrow(int i, int w = 3) {
    std::wstring result(w, L' ');
    result[i % result.size()] = L'>';
    return result;
}

StatusScreen::StatusScreen(bool isChit, int number)
    : isChit(isChit)
    , number(number)
{
    std::wstring chitPis = isChit ? L"ЧИТАТЕЛЬ" : L"ПИСАТЕЛЬ";

    std::vector<std::wstring> rows = {
            L"              ┌01 [          ]",
            L"              │02 [          ]",
            L" " + chitPis + L" №?? │03 [ЗАПИСЬ >>>]",
            L"              │04 [          ]",
            L" старт через: │05 [>>> ЧТЕНИЕ]",
            L"   1234 мсек  │06 [          ]",
            L"              │07 [          ]",
            L"   СТРАНИЦА ──┤08 [ ОЖИДАНИЕ ]",
            L"    ФАЙЛА     │09 [          ]",
            L"              │10 [          ]",
            L"  F10  Выход  │11 [          ]",
            L"              └12 [          ]",
    };
    lines.setLines(styledText(std::move(rows), FG::GREY | BG::BLACK));
}

void StatusScreen::drawOn(Screen& s) {
    lines.drawOn(s, {0, 0, s.w(), s.h()});
}
