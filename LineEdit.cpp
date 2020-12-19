#include "LineEdit.h"

#include "Screen.h"
#include "colors.h"
#include "Popup.h"
#include "MessagePopup.h"

LineEdit::LineEdit(Screen& screen, Popup* owner, SHORT w, int textSizeMax)
    : editable(screen.getEditable())
    , owner(owner)
    , w(w)
    , textSizeMax(textSizeMax)
{
}

void LineEdit::setText(std::wstring newText, std::function<bool()> editEnabled) {
    editable.setTextSizeMax(textSizeMax);
    editable.setText(std::move(newText), w);
    editable.setEnabledGetter([this, editEnabled = std::move(editEnabled)]() {
        return !MessagePopup::isVisible() && owner->isPopupVisible() && (!editEnabled || editEnabled());
    });
}

std::wstring LineEdit::getText() const {
    return editable.getText();
}

void LineEdit::drawOn(Screen& screen, COORD pos, WORD colorAttr, bool clearChars) {
    screen.paintRect({pos.X, pos.Y, w, 1}, colorAttr, clearChars);
    editable.drawOn(screen, pos, !MessagePopup::isVisible());
}
