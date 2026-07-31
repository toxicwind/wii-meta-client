#include "provider.h"
#include "../network/http.h"
#include "../utils/json.h"

// Open Shop Channel JSON API client
// Based on libreshop-client and osc-dl

class OSCProvider : public Provider {
    static constexpr const char* REPO_URL = "https://hbb1.oscwii.org/hbb/apps.json";
    static constexpr const char* DL_BASE = "https://hbb1.oscwii.org/hbb/";
    HttpClient http;
    JsonParser json;

public:
    const char* name() const override { return "Open Shop Channel"; }

    bool init() override {
        return http.init();
    }

    void shutdown() override {
        http.shutdown();
    }

    SearchResult search(const std::string& query, int page) override {
        SearchResult result;

        std::string data = http.get(REPO_URL);
        if (data.empty()) return result;

        auto apps = json.parse_array(data);
        for (const auto& app : apps) {
            std::string name = json.get_string(app, "name");
            std::string desc = json.get_string(app, "short_description");

            if (query.empty() || 
                name.find(query) != std::string::npos ||
                desc.find(query) != std::string::npos) {

                GameEntry entry;
                entry.id = json.get_string(app, "internal_name");
                entry.title = name;
                entry.description = desc;
                entry.source = "osc";
                entry.type = "homebrew";
                entry.download_url = std::string(DL_BASE) + entry.id + "/" + entry.id + ".zip";
                entry.cover_url = std::string(DL_BASE) + entry.id + "/icon.png";

                result.entries.push_back(entry);
            }
        }

        result.total = result.entries.size();
        return result;
    }

    SearchResult browse(const std::string& category, int page) override {
        // OSC has categories in apps.json: "category": "utilities", "games", etc.
        SearchResult result;

        std::string data = http.get(REPO_URL);
        if (data.empty()) return result;

        auto apps = json.parse_array(data);
        for (const auto& app : apps) {
            std::string cat = json.get_string(app, "category");
            if (cat == category || category.empty()) {
                GameEntry entry;
                entry.id = json.get_string(app, "internal_name");
                entry.title = json.get_string(app, "name");
                entry.description = json.get_string(app, "short_description");
                entry.source = "osc";
                entry.type = "homebrew";
                entry.download_url = std::string(DL_BASE) + entry.id + "/" + entry.id + ".zip";
                entry.cover_url = std::string(DL_BASE) + entry.id + "/icon.png";

                result.entries.push_back(entry);
            }
        }

        result.total = result.entries.size();
        return result;
    }

    std::string resolve_download(const GameEntry& entry) override {
        return entry.download_url;
    }

    bool ping() override {
        return http.head("https://hbb1.oscwii.org");
    }
};

Provider* create_osc_provider() {
    return new OSCProvider();
}
