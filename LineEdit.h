#pragma once

#include <windows.h>

#include <string>
#include <functional>

class Screen;
class Popup;
class EditableText;

class LineEdit {
public:
    LineEdit(Screen& screen, Popup* owner, SHORT w, int textSizeMax = -1);

    void setText(std::wstring newText, std::function<bool()> editEnabled = {});
    std::wstring getText() const;
    void setReadOnly(bool newReadOnly);

    void drawOn(Screen& screen, COORD pos, WORD colorAttr, bool clearChars = true);

private:
    EditableText& editable;
    Popup* owner;
    SHORT w;
    int textSizeMax;
};
