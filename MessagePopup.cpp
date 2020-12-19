#include "MessagePopup.h"

#include "Screen.h"
#include "colors.h"

static const std::wstring OK_TEXT = L" <Enter> OK ";

void MessagePopup::show(std::vector<std::wstring> text, bool clearShadow, Color bgColor) {
    MessagePopup& popup = get();
    popup.maxWidth = 0;
    for (const auto& line : text) {
        popup.maxWidth = std::max(popup.maxWidth, (int)line.size());
    }
    popup.linesCount = text.size();
    popup.lines.setLines(styledText(std::move(text), FG::WHITE | toBg(bgColor)));
    popup.clearShadow = clearShadow;
    popup.bgColor = bgColor;
    popup.visible = true;
}

void MessagePopup::registerKeys(Screen& screen) {
    MessagePopup& popup = get();
    popup.registerClosing(screen);
    screen.handleKey(&popup, VK_RETURN, 0, []() {
        MessagePopup& popup = get();
        popup.visible = false;
    });
}

void MessagePopup::drawOn(Screen& screen) {
    MessagePopup& popup = get();
    if (!popup.visible) {
        return;
    }
    SHORT w = std::max((int)OK_TEXT.size() + 2, popup.maxWidth);
    SHORT h = popup.linesCount + 2;
    auto center = screen.center();

    Rect inner{center.X, center.Y, w, h};
    Rect button = inner.moved(-w/2, h - h/2 - 1).withH(1);
    inner = inner.moved(-w/2, -h/2);
    Rect frame = inner.withPadding(-2, -1);
    Rect outer = frame.withPadding(-2, -1);
    Rect shadow(outer.moved(2, 1));

    if (popup.clearShadow) {
        screen.paintRect(shadow, FG::DARK_GREY | BG::BLACK, true, L'░');
    } else {
        screen.paintRect(shadow, FG::DARK_GREY | BG::BLACK, false);
    }
    screen.paintRect(outer, FG::WHITE | toBg(popup.bgColor));
    screen.frame(frame);

    popup.lines.drawOn(screen, inner.withH(inner.h - 2), true);
    screen.separator(inner.moved(0, inner.h - 2).withPadX(-2).withH(1));

    screen.labels(button, {OK_TEXT}, FG::BLACK | BG::GREY);
}

bool MessagePopup::isVisible() {
    return get().isPopupVisible();
}

MessagePopup& MessagePopup::get() {
    static MessagePopup popup;
    return popup;
}
