#pragma once
#include <string>
#include <fstream>
#include <vector>

class Logger {
public:
    Logger();
    ~Logger();

    // Initialize log files under a directory (creates/overwrites)
    bool init(const std::string &dir);

    // Log a detected issue without change
    void issue(size_t lineNo, const std::string &message);

    // Log an applied fix with before/after
    void fix(size_t lineNo, const std::string &original, const std::string &corrected, const std::string &message);

    // Write rich analysis text (for interactive window)
    void writeAnalysis(const std::vector<std::string> &lines);

    // Path to analysis file for external viewer
    std::string analysisPath() const { return analysisPath_; }
    std::string fixesPath() const { return fixesPath_; }

    void flush();

private:
    std::ofstream fixesOut_;
    std::ofstream analysisOut_;
    std::string baseDir_;
    std::string analysisPath_;
    std::string fixesPath_;
};
