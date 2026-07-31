#include "provider.h"
#include "../network/http.h"

// GameTDB cover art fetcher
// No API needed — direct URL construction

class GameTDBProvider : public Provider {
    static constexpr const char* COVER_BASE = "https://www.gametdb.com/Wii/cover";
    HttpClient http;

public:
    const char* name() const override { return "GameTDB"; }

    bool init() override { return http.init(); }
    void shutdown() override { http.shutdown(); }

    SearchResult search(const std::string& query, int page) override {
        // GameTDB has no search API — we need title_id
        return {};
    }

    SearchResult browse(const std::string& category, int page) override {
        return {};
    }

    std::string resolve_download(const GameEntry& entry) override {
        // Construct cover URL: /cover/<region>/<title_id>.png
        std::string region = entry.region.empty() ? "US" : entry.region;
        return std::string(COVER_BASE) + "/" + region + "/" + entry.id + ".png";
    }

    bool ping() override {
        return http.head("https://www.gametdb.com");
    }

    // Specialized: fetch cover for a given title_id
    std::string get_cover_url(const std::string& title_id, const std::string& region = "US") {
        return std::string(COVER_BASE) + "/" + region + "/" + title_id + ".png";
    }
};

Provider* create_gametdb_provider() {
    return new GameTDBProvider();
}
