#include <string>
#include <vector>

class JsonValue {
public:
    std::string raw;
};

class JsonParser {
public:
    std::vector<JsonValue> parse_array(const std::string& json) {
        return {};
    }

    std::string get_string(const JsonValue& val, const std::string& key) {
        return "";
    }
};
