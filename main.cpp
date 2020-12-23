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
#include <fstream>
#include <cstdint>

int randInt(int a, int b) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution distrib(a, b);
    return distrib(gen);
}

struct Page {
    volatile char data[4*1024];
};

struct Pages {
    Page pages[12];
};

enum class PageState : uint8_t {
    CanWrite = 0,
    Busy,
    CanRead
};

struct SharedObject {
    volatile PageState pageStates[12];

    int takePage(bool isChit) {
        PageState searchState = isChit ? PageState::CanRead : PageState::CanWrite;
        for (int i = 0; i < 12; ++i) {
            if (pageStates[i] == searchState) {
                pageStates[i] = PageState::Busy;
                return i;
            }
        }
        return -1;
    }

    void returnPage(int page, bool isChit) {
        PageState newState = isChit ? PageState::CanWrite : PageState::CanRead;
        pageStates[page] = newState;
    }
};

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

class Semaphore {
public:
    Semaphore(const std::wstring& name, int initial, int max) {
        sem = CreateSemaphoreW(nullptr, initial, max, name.c_str());
    }

    ~Semaphore() {
        CloseHandle(sem);
    }

    void acquire() {
        WaitForSingleObject(sem, INFINITE);
    }

    void release() {
        ReleaseSemaphore(sem, 1, nullptr);
    }

private:
    HANDLE sem;
};

class Mutex {
friend class MutexLock;
public:
    explicit Mutex(const std::wstring& name) {
        mutex = CreateMutexW(nullptr, FALSE, name.c_str());
    }

    ~Mutex() {
        CloseHandle(mutex);
    }

private:
    void lock() {
        WaitForSingleObject(mutex, INFINITE);
    }

    void unlock() {
        ReleaseMutex(mutex);
    }

    HANDLE mutex;
};

class MutexLock {
public:
    explicit MutexLock(Mutex& mut) : mut(mut) {
        mut.lock();
    }

    ~MutexLock() {
        mut.unlock();
    }
private:
    Mutex& mut;
};

template<typename T>
class MemMapping {
public:
    explicit MemMapping(const std::wstring& name) {
        mapFile = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE | SEC_COMMIT, 0, sizeof(T), name.c_str());
        mapView = MapViewOfFile(mapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        VirtualLock(mapView, sizeof(T));
    }

    ~MemMapping() {
        VirtualUnlock(mapView, sizeof(T));
        UnmapViewOfFile(mapView);
        CloseHandle(mapFile);
    }

    T* data() {
        return (T*)mapView;
    }
private:
    void* mapView;
    HANDLE mapFile;
};

class LogFile {
public:
    explicit LogFile(const std::string& name)
            : log(name, std::ios_base::app)
    {}

    void write(const std::string& toWrite) {
        using namespace std::chrono;
        auto now = high_resolution_clock::now();
        log << now.time_since_epoch().count() << ": " << toWrite << std::endl;
    }

private:
    std::ofstream log;
};

class Worker {
public:
    void singleRun(const std::function<bool()>& process);

    virtual bool isChit() const = 0;
    virtual Semaphore& inputSem() = 0;
    virtual Semaphore& outputSem() = 0;
    virtual void processBuf(Page& page) = 0;

    virtual ~Worker();

protected:
    Worker(StatusScreen& status, LogFile& log);

    Mutex mutex;
    Semaphore pagesToWriteSemaphore;
    Semaphore pagesToReadSemaphore;
    MemMapping<SharedObject> mapShared;
    MemMapping<Pages> mapPages;
    StatusScreen& status;
    LogFile& log;

    char buf[sizeof(Page)];
};

void Worker::singleRun(const std::function<bool()>& process) {
    log.write("WAIT");
    status.updateState(State::Waiting);
    if (!process()) {
        return;
    }

    inputSem().acquire();

    int page;
    {
        MutexLock lock(mutex);
        page = mapShared.data()->takePage(isChit());
    }

    log.write(isChit() ? "READ" : "WRITE");
    State st = isChit() ? State::Reading : State::Writing;
    status.updateState(st, page);
    if (process()) {
        processBuf(mapPages.data()->pages[page]);

        int localWait = randInt(500, 1500);
        ticker(localWait, [&](int elapsed) {
            float progress = elapsed / (float) localWait;
            status.updateState(st, page, progress);
            return process();
        });
    }

    log.write("WAIT");
    status.updateState(State::Waiting);
    process();
    {
        MutexLock lock(mutex);
        mapShared.data()->returnPage(page, isChit());
    }

    outputSem().release();
}

Worker::~Worker() = default;

Worker::Worker(StatusScreen& status, LogFile& log)
        : mutex(L"SharedMemMutex")
        , pagesToWriteSemaphore(L"PagesToWriteSemaphore", 12, 12)
        , pagesToReadSemaphore(L"PagesToReadSemaphore", 0, 12)
        , mapShared(L"MapShared")
        , mapPages(L"MapPages")
        , status(status)
        , log(log)
{}

class Chitatel : public Worker {
public:
    explicit Chitatel(StatusScreen& status, LogFile& log) : Worker(status, log) {}

    bool isChit() const override {
        return true;
    }

    Semaphore& inputSem() override {
        return pagesToReadSemaphore;
    }

    Semaphore& outputSem() override {
        return pagesToWriteSemaphore;
    }

    void processBuf(Page& page) override {
        for (int i = 0; i < sizeof(buf); ++i) {
            buf[i] = page.data[i];
        }
    }
};

class Pisatel : public Worker {
public:
    explicit Pisatel(StatusScreen& status, LogFile& log) : Worker(status, log) {}

    bool isChit() const override {
        return false;
    }

    Semaphore& inputSem() override {
        return pagesToWriteSemaphore;
    }

    Semaphore& outputSem() override {
        return pagesToReadSemaphore;
    }

    void processBuf(Page& page) override {
        for (int i = 0; i < sizeof(buf); ++i) {
            page.data[i] = buf[i];
        }
    }
};

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

    LogFile log(std::string("logfile_") + (isChit ? "_chit_" : "_pis_") + std::to_string(number) + ".log");
    std::unique_ptr<Worker> worker;
    if (isChit) {
        worker = std::make_unique<Chitatel>(status, log);
    } else {
        worker = std::make_unique<Pisatel>(status, log);
    }

    // Main loop
    log.write("START");
    while (running) {
        worker->singleRun(loop);
    }
    log.write("STOP");
}
