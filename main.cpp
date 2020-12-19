#include <iostream>
#include "Lines.h"
#include "Screen.h"

#include "colors.h"
#include "StatusScreen.h"
#include "MessagePopup.h"
#include "utils.h"

#include <memory>
#include <cstdio>
#include <io.h>
#include <fcntl.h>

void _fixwcout() {
    constexpr char cp_utf16le[] = ".1200";
    setlocale( LC_ALL, cp_utf16le );
    _setmode( _fileno(stdout), _O_WTEXT );
    std::wcout << L"\r"; // need to output something for this to work
}

int main(int argc, char* argv[]) {
    _fixwcout();

    if (argc < 4) {
        std::wcout << L"USAGE: chit-pis.exe (chit|pis) number delay" << std::endl;
        return 1;
    }
    bool isChit = argv[1] == std::string("chit");
    int number = std::atoi(argv[2]);
    int delay = std::atoi(argv[3]);

    Screen s(30, 12);
    s.setTitle(isChit ? L"ЧИТАТЕЛЬ" : L"ПИСАТЕЛЬ");

    bool running = true;
    StatusScreen status(isChit, number);

    // Drawing
    auto repaint = [&]() {
        s.clear(FG::GREY | BG::BLACK);

        status.drawOn(s);
        MessagePopup::drawOn(s);

        s.flip();
    };

    // Global exit
    s.handlePriorityKey(VK_F10, 0, [&]() {
        running = false;
    });

    // Global message popup
    MessagePopup::registerKeys(s);

    // Initial state
    repaint();

    // Main loop
    while (running) {
        s.processEvent();
        repaint();
    }
}
