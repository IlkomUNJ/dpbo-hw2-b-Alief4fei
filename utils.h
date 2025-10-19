#pragma once
#include <string>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <vector>
#include <time.h>

inline std::string now_datetime_string() {
    using namespace std::chrono;
    auto t = system_clock::to_time_t(system_clock::now());
    std::tm tm{};
#if defined(_MSC_VER)
    localtime_s(&tm, &t);
#elif defined(__unix__) || defined(__linux__) || defined(__APPLE__)
    localtime_r(&t, &tm);
#else
    {
        std::tm *pt = std::localtime(&t);
        if (pt) tm = *pt;
    }
#endif
    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

inline std::vector<std::string> split_pipe(const std::string &s) {
    std::vector<std::string> out;
    std::string cur;
    for (char c : s) {
        if (c == '|') {
            out.push_back(cur);
            cur.clear();
        } else cur.push_back(c);
    }
    out.push_back(cur);
    return out;
}

inline size_t count_nonempty_lines_in_file(const std::string &path) {
    std::ifstream in(path);
    if (!in) return 0;
    size_t count = 0;
    std::string line;
    while (std::getline(in, line)) {
        if (line.size() && line[0] != '#') ++count;
    }
    return count;
}

inline void ensure_data_folder() {
    std::filesystem::path d("data");
    if (!std::filesystem::exists(d)) std::filesystem::create_directory(d);
    std::vector<std::string> files = {"data/buyers.txt","data/sellers.txt","data/items.txt","data/transactions.txt"};
    for (auto &f: files) {
        std::ofstream ofs(f, std::ios::app);
    }
}
