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
#include <chrono>
#include <random>

void _fixwcout() {
    constexpr char cp_utf16le[] = ".1200";
    setlocale( LC_ALL, cp_utf16le );
    _setmode( _fileno(stdout), _O_WTEXT );
    std::wcout << L"\r"; // need to output something for this to work
}

void ticker(int timeMs, const std::function<bool(int)>& callback) {
    using namespace std::chrono;
    int elapsed = 0;
    auto start = high_resolution_clock::now();
    while (true) {
        bool running = callback(elapsed);
        if (!running || elapsed >= timeMs) {
            break;
        }
        Sleep(100);
        elapsed = duration_cast<milliseconds>(high_resolution_clock::now() - start).count();
    }
}

int main(int argc, char* argv[]) {
    _fixwcout();

    if (argc < 4) {
        std::wcout << L"USAGE: chit-pis.exe (chit|pis) number waitMs" << std::endl;
        return 1;
    }
    bool isChit = argv[1] == std::string("chit");
    int number = std::atoi(argv[2]);
    int waitMs = std::atoi(argv[3]);

    Screen s(30, 12);
    std::wstring title = isChit ? L"ЧИТАТЕЛЬ №" : L"ПИСАТЕЛЬ №";
    title += std::to_wstring(number);
    s.setTitle(title);

    bool running = true;
    StatusScreen status(isChit, number, waitMs);

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

    auto loop = [&]() {
        if (s.hasEvent()) {
            s.processEvent();
        }
        status.tickAnim();
        repaint();
        return running;
    };

    // Waiting loop
    ticker(waitMs, [&](int elapsed) {
        status.updateWait(waitMs - elapsed);
        return loop();
    });

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> waitDistrib(500, 2500);
    std::uniform_int_distribution<> stateDistrib(1, 3);
    std::uniform_int_distribution<> pageDistrib(1, 12);

    while (running) {
        int localWait = waitDistrib(gen);
        State st = (State)stateDistrib(gen);
        int page = pageDistrib(gen);
        status.updateState(st, page);
        ticker(localWait, [&](int elapsed) {
            float progress = elapsed / (float) localWait;
            status.updateState(st, page, progress);
            return loop();
        });
    }

    // Main loop
    while (running) {
        s.processEvent();
        repaint();
    }
}
