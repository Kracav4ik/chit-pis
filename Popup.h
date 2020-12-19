#pragma once

class Screen;

class Popup {
public:
    bool isPopupVisible() const;

protected:
    void registerClosing(Screen& screen);

    bool visible = false;
};
