#pragma once
#include <fstream>
#include <optional>
#include <string>
#include <unordered_map>

namespace cpp_commons::config {

// Parses a .env file (KEY=value lines) for use in development.
class DotenvSource {
public:
    explicit DotenvSource(const std::string& path) { load(path); }

    [[nodiscard]] std::optional<std::string> get(const std::string& key) const {
        auto it = values_.find(key);
        if (it == values_.end())
            return std::nullopt;
        return it->second;
    }

private:
    void load(const std::string& path) {
        std::ifstream file{path};
        if (!file.is_open())
            return;  // missing .env is not an error
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#')
                continue;
            auto eq = line.find('=');
            if (eq == std::string::npos)
                continue;
            auto key = line.substr(0, eq);
            auto val = line.substr(eq + 1);
            if (!val.empty() && val.front() == '"')
                val = val.substr(1);
            if (!val.empty() && val.back() == '"')
                val.pop_back();
            values_[key] = val;
        }
    }

    std::unordered_map<std::string, std::string> values_;
};

}  // namespace cpp_commons::config
