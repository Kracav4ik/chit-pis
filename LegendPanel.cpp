#include "LegendPanel.h"

LegendPanel::LegendPanel() {
    std::vector<std::wstring> rows = {
            L"Управление (с зажатым Ctrl):",
            L"  стрелки ← и →: сдвиг на одну клетку",
            L"  стрелки ↑ и ↓: сдвиг на строчку",
            L"  PgUp и PgDn: сдвиг на полэкрана",
            L"  Home и End: сдвиг в начало/конец",
            L"  + и -: смена масштаба клетки",
            L"Легенда:",
            L" Защита неисполняемых страниц:",
            L"  ?? нет доступа     ?? только чтение",
            L"  ?? чтение/запись   ?? копия при записи",
            L" Защита исполняемых страниц:",
            L"  ?? нет доступа     ?? только чтение",
            L"  ?? чтение/запись   ?? копия при записи",
            L" Страницы без физической памяти:",
            L"  ?? зарезервированные   ?? свободные",
            L" Особые страницы:",
            L"  ?? защищенная      ?? нет информации",
            L" Специальные цвета:",
            L"  ?? клетка содержит различные страницы",
            L"  ?? текущий выделенный регион",
    };
    lines.setLines(styledText(std::move(rows), FG::BLACK | BG::GREY));
}

void LegendPanel::drawOn(Screen& s) {
    s.paintRect(rect, FG::BLACK | BG::GREY);
    s.boundedLine(rect.getLeftTop(), rect.w, L"Управление и легенда карты памяти", true);
    Rect frame = rect.moved(0, 1).withH(rect.h - 1);
    s.boundedLine(frame.getLeftTop(), frame.w, std::wstring(frame.w - 2, L'═'), true);
    lines.drawOn(s, frame.withPadding(1, 1));

    s.paintRect({3, 10, 2, 1}, FG::BLACK | BG::DARK_GREY);
    s.paintRect({22, 10, 2, 1}, FG::BLACK | BG::DARK_BLUE);
    s.paintRect({3, 11, 2, 1}, FG::BLACK | BG::DARK_GREEN);
    s.paintRect({22, 11, 2, 1}, FG::BLACK | BG::DARK_CYAN);
    s.paintRect({3, 13, 2, 1}, FG::BLACK | BG::WHITE);
    s.paintRect({22, 13, 2, 1}, FG::BLACK | BG::BLUE);
    s.paintRect({3, 14, 2, 1}, FG::BLACK | BG::GREEN);
    s.paintRect({22, 14, 2, 1}, FG::BLACK | BG::CYAN);
    s.paintRect({3, 16, 2, 1}, FG::BLACK | BG::DARK_YELLOW);
    s.paintRect({26, 16, 2, 1}, FG::BLACK | BG::BLACK);
    s.paintRect({3, 18, 2, 1}, FG::BLACK | BG::DARK_RED);
    s.paintRect({22, 18, 2, 1}, FG::BLACK | BG::RED);
    s.paintRect({3, 20, 2, 1}, FG::BLACK | BG::DARK_MAGENTA);
    s.paintRect({3, 21, 2, 1}, FG::BLACK | BG::YELLOW);
}
