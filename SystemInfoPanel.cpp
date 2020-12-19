#include "SystemInfoPanel.h"

#include "utils.h"

static std::wstring getArchName(WORD arch) {
    switch (arch) {
        case PROCESSOR_ARCHITECTURE_AMD64: return L"x64";
        case PROCESSOR_ARCHITECTURE_ARM: return L"ARM";
        case PROCESSOR_ARCHITECTURE_IA64: return L"IA64";
        case PROCESSOR_ARCHITECTURE_INTEL: return L"x86";
        default: return L"неизвестно";
    }
}

void SystemInfoPanel::setVisible() {
    LeftPanel::setVisible();
    updateLines();
}

void SystemInfoPanel::drawOn(Screen& s) {
    s.paintRect(rect, FG::WHITE | BG::BLACK);
    s.boundedLine(rect.getLeftTop(), rect.w, L"Информация о системе", true);
    Rect frame = rect.moved(0, 1).withH(rect.h - 1);
    s.boundedLine(frame.getLeftTop(), frame.w, std::wstring(frame.w - 2, L'═'), true);
    lines.drawOn(s, frame.withPadding(1, 1));
}

void SystemInfoPanel::updateLines() {
    SYSTEM_INFO info;
    MEMORYSTATUSEX memstat;
    memstat.dwLength = sizeof(MEMORYSTATUSEX);
    GetSystemInfo(&info);
    GlobalMemoryStatusEx(&memstat);

    std::vector<std::wstring> rows = {
            L"Информация о процессоре:",
            L"  Архитектура:     " + getArchName(info.wProcessorArchitecture),
            L"  Количество ядер: " + std::to_wstring(info.dwNumberOfProcessors),
            L"  Маска:           " + toHex(info.dwActiveProcessorMask),
            L"  Ревизия:         " + toHex(info.wProcessorRevision),
            L"",
            L"Информация о памяти:",
            L"  Использование памяти:  " + std::to_wstring(memstat.dwMemoryLoad) + L"%",
            L"  Физ. памяти всего:     " + std::to_wstring(memstat.ullTotalPhys),
            L"  Физ. памяти доступно:  " + std::to_wstring(memstat.ullAvailPhys),
            L"  Полн. памяти всего:    " + std::to_wstring(memstat.ullTotalPageFile),
            L"  Полн. памяти доступно: " + std::to_wstring(memstat.ullAvailPageFile),
            L"  Вирт. памяти всего:    " + std::to_wstring(memstat.ullTotalVirtual),
            L"  Вирт. памяти доступно: " + std::to_wstring(memstat.ullAvailVirtual),
            L"",
            L"Информация об аллокациях:",
            L"  Минимальный адрес:  " + toHex((uintptr_t) info.lpMinimumApplicationAddress),
            L"  Максимальный адрес: " + toHex((uintptr_t) info.lpMaximumApplicationAddress),
            L"  Размер страницы:         " + std::to_wstring(info.dwPageSize),
            L"  Гранулярность аллокаций: " + std::to_wstring(info.dwAllocationGranularity),
    };
    lines.setLines(styledText(std::move(rows), FG::WHITE | BG::BLACK));
}
