#include "Logger.h"
#include <filesystem>

Logger::Logger() {}
Logger::~Logger() {
    flush();
}

bool Logger::init(const std::string &dir) {
    using namespace std;
    namespace fs = std::filesystem;
    baseDir_ = dir;
    try {
        fs::create_directories(dir);
    } catch (...) {
        // ignore
    }
    fixesPath_ = (fs::path(dir) / "fixes.log").string();
    analysisPath_ = (fs::path(dir) / "analysis.txt").string();
    fixesOut_.open(fixesPath_, ios::out | ios::app);
    analysisOut_.open(analysisPath_, ios::out | ios::app);
    if (analysisOut_.is_open()) {
        analysisOut_ << "\n==== IntelliFix++ session started ====" << "\n";
    }
    return fixesOut_.is_open() && analysisOut_.is_open();
}

void Logger::issue(size_t lineNo, const std::string &message) {
    if (fixesOut_.is_open()) {
        fixesOut_ << "[Issue] line " << lineNo << ": " << message << "\n";
    }
}

void Logger::fix(size_t lineNo, const std::string &original, const std::string &corrected, const std::string &message) {
    if (fixesOut_.is_open()) {
        fixesOut_ << "[Fix] line " << lineNo << ": " << message << "\n"
                  << "  - before: " << original << "\n"
                  << "  + after : " << corrected << "\n";
    }
}

void Logger::writeAnalysis(const std::vector<std::string> &lines) {
    if (!analysisOut_.is_open()) return;
    for (const auto &l : lines) analysisOut_ << l << "\n";
}

void Logger::flush() {
    if (fixesOut_.is_open()) fixesOut_.flush();
    if (analysisOut_.is_open()) analysisOut_.flush();
}
