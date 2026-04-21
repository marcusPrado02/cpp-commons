#pragma once
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <yaml-cpp/yaml.h>

namespace cpp_commons::config {

// Reads configuration from a YAML file.
// Only top-level scalar key-value pairs are exposed (flat map).
// Nested keys use dot notation: "database.host" maps to yaml["database"]["host"].
class YamlSource {
public:
    // Load from file path. Throws std::runtime_error if the file cannot be parsed.
    explicit YamlSource(const std::string& path) {
        try {
            root_ = YAML::LoadFile(path);
        } catch (const YAML::Exception& e) {
            throw std::runtime_error{std::string{"YamlSource: cannot load '"} + path +
                                     "': " + e.what()};
        }
    }

    // Load from a YAML string (useful in tests).
    static YamlSource from_string(const std::string& yaml_text) {
        YamlSource src;
        try {
            src.root_ = YAML::Load(yaml_text);
        } catch (const YAML::Exception& e) {
            throw std::runtime_error{std::string{"YamlSource: parse error: "} + e.what()};
        }
        return src;
    }

    // Lookup a key. Supports dot-separated nested access ("db.host").
    [[nodiscard]] std::optional<std::string> get(const std::string& key) const {
        // YAML::Node is a reference alias; Clone() makes an independent copy for traversal.
        YAML::Node node = YAML::Clone(root_);
        std::size_t start = 0;
        while (start < key.size()) {
            auto dot = key.find('.', start);
            auto part =
                dot == std::string::npos ? key.substr(start) : key.substr(start, dot - start);
            if (!node.IsMap() || !node[part])
                return std::nullopt;
            node = node[part];
            start = dot == std::string::npos ? key.size() : dot + 1;
        }
        if (!node.IsScalar())
            return std::nullopt;
        return node.as<std::string>();
    }

private:
    YamlSource() = default;
    YAML::Node root_;
};

}  // namespace cpp_commons::config
