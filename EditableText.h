#pragma once

#include <windows.h>

#include <string>
#include <functional>

class Screen;

class EditableText {
public:
    void setTextSizeMax(int textMax);

    std::wstring getText() const;
    void setText(std::wstring text, int maxLineSize, int cursor = -1);

    bool isEnabled() const;
    void setEnabledGetter(std::function<bool()> getter);

    bool consumeEvent(const KEY_EVENT_RECORD& event);
    void drawOn(Screen& screen, COORD origin, bool showCursor);

private:
    void setCursorOffset(int newOffset);

    std::function<bool()> enabled;
    std::wstring currentText;
    int lineSize;
    int cursorOffset;
    int lineOffset;
    int textSizeMax = -1;
};
