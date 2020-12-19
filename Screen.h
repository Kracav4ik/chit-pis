#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <functional>
#include <map>

#include "EditableText.h"
#include "colors.h"

#define ANY_ALT_PRESSED (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)
#define ANY_CTRL_PRESSED (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)

struct Rect {
    SHORT x;
    SHORT y;
    SHORT w;
    SHORT h;

    COORD getLeftTop() const { return {x, y}; }
    int getArea() const { return w * h; }

    Rect withPadX(SHORT padX) const {
        Rect copy = *this;
        copy.x += padX;
        copy.w -= 2 * padX;
        return copy;
    }
    Rect withPadY(SHORT padY) const {
        Rect copy = *this;
        copy.y += padY;
        copy.h -= 2 * padY;
        return copy;
    }
    Rect withPadding(SHORT padX, SHORT padY) const {
        Rect copy = *this;
        copy.x += padX;
        copy.y += padY;
        copy.w -= 2 * padX;
        copy.h -= 2 * padY;
        return copy;
    }
    Rect moved(SHORT dx, SHORT dy) const {
        Rect copy = *this;
        copy.x += dx;
        copy.y += dy;
        return copy;
    }
    Rect withW(SHORT newW) const {
        Rect copy = *this;
        copy.w = newW;
        return copy;
    }
    Rect withH(SHORT newH) const {
        Rect copy = *this;
        copy.h = newH;
        return copy;
    }
};

class Popup;

class Screen {
public:
    Screen(SHORT width, SHORT height);
    ~Screen();

    void clear(WORD colorAttr);
    void paintRect(Rect rect, WORD colorAttr, bool clearChars = true, wchar_t fillChar = L' ');
    void textOut(COORD pos, const std::wstring& text);
    void textOut(COORD pos, const std::wstring& text, DWORD size);
    void boundedLine(COORD pos, SHORT w, const std::wstring& text, bool centered = false);
    void frame(Rect rect, bool fat = true);
    void separator(Rect rect, bool fatLine = false, bool fatEnds = true);
    void labelsFill(const Rect& rect, const std::vector<std::wstring>& labelsList, WORD colorAttr);
    void labels(Rect rect, const std::vector<std::wstring>& labelsList, WORD colorAttr, int separator = 2);
    void pixelMap(Rect rect, const std::vector<Color>& pixels, Color bgColor, bool doubleHeight = true);
    void flip();

    void setTitle(const std::wstring& title);
    void setCursorVisible(bool visible);
    void setCursorPos(SHORT x, SHORT y);

    SHORT w() const;
    SHORT h() const;
    COORD center() const;

    EditableText& getEditable();

    void processEvent();
    void handlePriorityKey(WORD virtualKey, WORD modifiers, std::function<void()> callback);
    void handleKey(WORD virtualKey, WORD modifiers, std::function<void()> callback);
    void handleKey(Popup* owner, WORD virtualKey, WORD modifiers, std::function<void()> callback);
    void handleKey(std::function<bool()> canHandle, WORD virtualKey, WORD modifiers, std::function<void()> callback);

private:
    Rect adjust(Rect rect);
    COORD adjust(COORD rect);
    static HANDLE createBuffer(SHORT width, SHORT height);

    struct HandleRecord {
        DWORD key;
        std::function<bool()> canHandle;
        std::function<void()> callback;
    };

    SHORT width;
    SHORT height;
    HANDLE stdinHandle;
    HANDLE origConsole;
    HANDLE currConsole;
    HANDLE nextConsole;

    EditableText editable;

    std::map<DWORD, std::function<void()>> priorityHandlers;
    std::vector<HandleRecord> handlersWithCheck;
    std::map<DWORD, std::function<void()>> globalHandlers;
};
