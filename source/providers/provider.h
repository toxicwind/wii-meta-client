#ifndef PROVIDER_H
#define PROVIDER_H

#include <string>
#include <vector>
#include <functional>

struct GameEntry {
    std::string id;
    std::string title;
    std::string description;
    std::string cover_url;
    std::string download_url;
    std::string source;      // "vimm", "osc", "archive_org"
    std::string region;      // "USA", "EUR", "JPN"
    std::string type;        // "game", "homebrew", "cover"
    int size_mb;
    bool installed;
};

struct SearchResult {
    std::vector<GameEntry> entries;
    int total;
    bool has_more;
    std::string next_page_token;
};

class Provider {
public:
    virtual ~Provider() {}
    virtual const char* name() const = 0;
    virtual bool init() = 0;
    virtual void shutdown() = 0;

    // Search across this provider
    virtual SearchResult search(const std::string& query, int page = 0) = 0;

    // Browse by category
    virtual SearchResult browse(const std::string& category, int page = 0) = 0;

    // Get download URL for an entry
    virtual std::string resolve_download(const GameEntry& entry) = 0;

    // Check if provider is available
    virtual bool ping() = 0;

    // Progress callback: (bytes_downloaded, total_bytes, speed_kbps)
    std::function<void(int64_t, int64_t, float)> on_progress;
};

// Factory
Provider* create_vimm_provider();
Provider* create_osc_provider();
Provider* create_archive_org_provider();
Provider* create_gametdb_provider();

#endif
