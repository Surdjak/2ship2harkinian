#include <atomic>
#include <chrono>
#include <cstring>
#include <fstream>
#include <thread>

extern "C" int zapd_report(int argc, char* argv[], std::atomic<size_t>* extractCount, std::atomic<size_t>* totalExtract);

int main(int argc, char* argv[]) {
    if (argc < 2 || std::strcmp(argv[1], "--report-progress") != 0) {
        return zapd_report(argc, argv, nullptr, nullptr);
    }
    std::atomic<size_t> count = 0, total = 0;
    std::jthread reporter([&](std::stop_token stop) {
        size_t lastCount = 0, lastTotal = 0;
        while (!stop.stop_requested()) {
            const size_t currentTotal = total.load();
            const size_t currentCount = count.load();
            if (currentTotal != lastTotal || currentCount != lastCount) {
                // ponytail: Poll a small text file. A partial read is retried on the next poll.
                std::ofstream("progress.txt") << currentCount << ' ' << currentTotal << '\n';
                lastCount = currentCount;
                lastTotal = currentTotal;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
    argv[1] = argv[0];
    return zapd_report(argc - 1, argv + 1, &count, &total);
}
