#pragma once

#include "Lines.h"
#include "Popup.h"
#include "colors.h"

#include <string>
#include <vector>

class Screen;

class MessagePopup : Popup {
public:
    static void show(std::vector<std::wstring> text, bool clearShadow = false, Color bgColor = Color::DarkRed);
    static void registerKeys(Screen& screen);
    static void drawOn(Screen& screen);
    static bool isVisible();

private:
    MessagePopup() = default;
    static MessagePopup& get();

    Lines lines;
    int maxWidth = 0;
    int linesCount = 0;
    bool clearShadow = false;
    Color bgColor = Color::DarkRed;
};
