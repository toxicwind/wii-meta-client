#include <string>
#include <vector>

class HtmlParser {
    std::string html;
public:
    HtmlParser(const std::string& h) : html(h) {}

    std::vector<std::string> find_all(const std::string& tag) {
        return {};
    }

    std::vector<std::string> find_in(const std::string& context, const std::string& tag) {
        return {};
    }

    std::string attr(const std::string& element, const std::string& name) {
        return "";
    }

    std::string text(const std::string& element) {
        return "";
    }
};
