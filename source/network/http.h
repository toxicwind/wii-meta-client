#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <string>
#include <functional>

class HttpClient {
public:
    bool init();
    void shutdown();

    // Simple GET
    std::string get(const std::string& url);

    // HEAD check
    bool head(const std::string& url);

    // Chunked download with progress
    bool download(const std::string& url, 
                  const std::string& path,
                  std::function<void(int64_t, int64_t)> progress = nullptr);

    // URL encode
    std::string url_encode(const std::string& str);

    // Range request (resume)
    std::string get_range(const std::string& url, int64_t start, int64_t end);

private:
    int socket_fd;
    bool ssl_enabled;

    bool connect(const std::string& host, int port);
    std::string request(const std::string& method, 
                        const std::string& path,
                        const std::string& host,
                        const std::string& extra_headers = "");
};

#endif
