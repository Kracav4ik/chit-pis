#include "EditableText.h"

#include "Screen.h"
#include "MessagePopup.h"
#include "utils.h"

void EditableText::setTextSizeMax(int textMax) {
    textSizeMax = textMax;
}

std::wstring EditableText::getText() const {
    return currentText;
}

void EditableText::setText(std::wstring text, int maxLineSize, int cursor) {
    currentText = std::move(text);
    if (textSizeMax != -1 && currentText.size() > textSizeMax) {
        currentText.resize(textSizeMax);
    }
    lineSize = maxLineSize;
    if (cursor == -1) {
        cursor = currentText.size();
    }
    lineOffset = 0;
    setCursorOffset(cursor);
}

bool EditableText::isEnabled() const {
    return enabled && enabled();
}

void EditableText::setEnabledGetter(std::function<bool()> getter) {
    enabled = std::move(getter);
}

bool EditableText::consumeEvent(const KEY_EVENT_RECORD& event) {
    if (event.dwControlKeyState == 0) {
        if (event.wVirtualKeyCode == VK_BACK) {
            if (cursorOffset > 0) {
                currentText = currentText.substr(0, cursorOffset - 1) + currentText.substr(cursorOffset);
                setCursorOffset(cursorOffset - 1);
            }
            return true;
        }
        if (event.wVirtualKeyCode == VK_DELETE) {
            if (cursorOffset < currentText.size()) {
                currentText = currentText.substr(0, cursorOffset) + currentText.substr(cursorOffset + 1);
            }
            return true;
        }
        if (event.wVirtualKeyCode == VK_LEFT) {
            setCursorOffset(cursorOffset - 1);
            return true;
        }
        if (event.wVirtualKeyCode == VK_RIGHT) {
            setCursorOffset(cursorOffset + 1);
            return true;
        }
        if (event.wVirtualKeyCode == VK_HOME) {
            setCursorOffset(0);
            return true;
        }
        if (event.wVirtualKeyCode == VK_END) {
            setCursorOffset(currentText.size());
            return true;
        }
    }
    bool shiftIns = (event.dwControlKeyState & SHIFT_PRESSED) && event.wVirtualKeyCode == VK_INSERT;
    bool ctrlV = (event.dwControlKeyState & ANY_CTRL_PRESSED) && event.wVirtualKeyCode == 'V';
    if (shiftIns || ctrlV) {
        if (OpenClipboard(nullptr)) {
            HANDLE data = GetClipboardData(CF_UNICODETEXT);
            auto dataPtr = GlobalLock(data);
            if (dataPtr) {
                std::wstring text((const wchar_t*)dataPtr);
                if (textSizeMax != -1 && text.size() + currentText.size() > textSizeMax) {
                    text.resize(std::max(0, textSizeMax - (int)currentText.size()));
                }
                currentText.insert(cursorOffset, text);
                setCursorOffset(cursorOffset + text.size());
                GlobalUnlock(data);
            }
            CloseClipboard();
        }
        return true;
    }
    if (event.uChar.UnicodeChar >= L' ' && event.uChar.UnicodeChar != L'\x7f') {
        if (textSizeMax == -1 || currentText.size() < textSizeMax) {
            currentText.insert(cursorOffset, 1, event.uChar.UnicodeChar);
            setCursorOffset(cursorOffset + 1);
        }
        return true;
    }
    return false;
}

void EditableText::drawOn(Screen& screen, COORD origin, bool showCursor) {
    screen.setCursorVisible(showCursor);
    screen.textOut(origin, currentText.substr(lineOffset, lineSize));
    screen.setCursorPos(origin.X + cursorOffset - lineOffset, origin.Y);
}

void EditableText::setCursorOffset(int newOffset) {
    newOffset = clamp(0, newOffset, (int)currentText.size());
    cursorOffset = newOffset;
    if (cursorOffset < lineOffset) {
        lineOffset = cursorOffset;
    } else if (cursorOffset > lineOffset + lineSize - 1) {
        lineOffset = cursorOffset - lineSize + 1;
    }
}
