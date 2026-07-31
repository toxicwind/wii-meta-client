#include "provider.h"
#include "../network/http.h"
#include "../utils/html.h"
#include <cstring>
#include <sstream>

// Vimm's Lair HTML scraper
// Based on X-Store (Xbox 360) and OctoLair (TrimUI) implementations

class VimmProvider : public Provider {
    static constexpr const char* BASE_URL = "https://vimm.net/vault";
    static constexpr const char* DL_DOMAIN = "https://dl2.vimm.net";
    HttpClient http;

public:
    const char* name() const override { return "Vimm's Lair"; }

    bool init() override {
        return http.init();
    }

    void shutdown() override {
        http.shutdown();
    }

    SearchResult search(const std::string& query, int page) override {
        SearchResult result;

        // Build search URL: vimm.net/vault/Wii?q=<query>
        std::string url = std::string(BASE_URL) + "/Wii";
        if (!query.empty()) {
            url += "?q=" + http.url_encode(query);
        }

        std::string html = http.get(url);
        if (html.empty()) return result;

        // Parse HTML table rows
        // Vimm's uses: <tr><td><a href="/vault/<id>">Title</a></td>...</tr>
        HtmlParser parser(html);
        auto rows = parser.find_all("tr");

        for (const auto& row : rows) {
            auto links = parser.find_in(row, "a");
            for (const auto& link : links) {
                std::string href = parser.attr(link, "href");
                std::string text = parser.text(link);

                if (href.find("/vault/") == 0 && !text.empty()) {
                    GameEntry entry;
                    entry.id = href.substr(7); // Remove "/vault/"
                    entry.title = text;
                    entry.source = "vimm";
                    entry.type = "game";

                    // Extract mediaId from game page (second request)
                    entry.download_url = resolve_mediaId(entry.id);

                    result.entries.push_back(entry);
                }
            }
        }

        result.total = result.entries.size();
        return result;
    }

    SearchResult browse(const std::string& category, int page) override {
        // Vimm's has console categories: Wii, GameCube, N64, etc.
        return search("", page);
    }

    std::string resolve_download(const GameEntry& entry) override {
        return entry.download_url;
    }

    bool ping() override {
        return http.head("https://vimm.net");
    }

private:
    std::string resolve_mediaId(const std::string& vault_id) {
        // Fetch game page and extract mediaId from JavaScript
        // Page contains: var mediaId = [12345, 67890];
        std::string url = std::string(BASE_URL) + "/" + vault_id;
        std::string html = http.get(url);

        size_t pos = html.find("var mediaId = [");
        if (pos == std::string::npos) return "";

        pos += 15;
        size_t end = html.find("]", pos);
        if (end == std::string::npos) return "";

        std::string ids = html.substr(pos, end - pos);
        // Take first mediaId
        size_t comma = ids.find(",");
        std::string mediaId = (comma != std::string::npos) ? ids.substr(0, comma) : ids;

        // Build download URL
        return std::string(DL_DOMAIN) + "?mediaId=" + mediaId;
    }
};

Provider* create_vimm_provider() {
    return new VimmProvider();
}
