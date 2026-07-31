#include <grrlib.h>
#include <wiiuse/wpad.h>
#include <network.h>
#include <fat.h>
#include "providers/provider.h"
#include "network/download.h"
#include <vector>
#include <string>

// GRRLIB colors
#define COLOR_BG      0x000000FF
#define COLOR_TEXT    0xFFFFFFFF
#define COLOR_HIGHLIGHT 0x00FF00FF
#define COLOR_ERROR   0xFF0000FF

// Screen dimensions
#define SCREEN_W 640
#define SCREEN_H 480

class MetaClient {
    std::vector<Provider*> providers;
    DownloadManager* downloads;
    GRRLIB_texImg* bg_tex;
    int selected_provider;
    int selected_item;
    std::vector<GameEntry> current_results;
    bool show_queue;

public:
    bool init() {
        // Init GRRLIB
        GRRLIB_Init();

        // Init Wiimote
        WPAD_Init();
        WPAD_SetDataFormat(WPAD_CHAN_ALL, WPAD_FMT_BTNS_ACC_IR);

        // Init network
        if (if_config(NULL, NULL, NULL, true, 20) < 0) {
            return false;
        }

        // Init FAT (SD/USB)
        fatInitDefault();

        // Init providers
        providers.push_back(create_osc_provider());
        providers.push_back(create_vimm_provider());
        providers.push_back(create_gametdb_provider());

        for (auto p : providers) {
            p->init();
        }

        // Init download manager
        downloads = new DownloadManager(2);
        downloads->start();

        selected_provider = 0;
        selected_item = 0;
        show_queue = false;

        return true;
    }

    void run() {
        while (true) {
            WPAD_ScanPads();
            u32 buttons = WPAD_ButtonsDown(WPAD_CHAN_0);

            if (buttons & WPAD_BUTTON_HOME) break;

            handle_input(buttons);
            render();

            VIDEO_WaitVSync();
        }
    }

    void shutdown() {
        downloads->stop();
        delete downloads;

        for (auto p : providers) {
            p->shutdown();
            delete p;
        }

        GRRLIB_Exit();
    }

private:
    void handle_input(u32 buttons) {
        if (show_queue) {
            if (buttons & WPAD_BUTTON_B) show_queue = false;
            return;
        }

        if (buttons & WPAD_BUTTON_UP) {
            selected_item--;
            if (selected_item < 0) selected_item = current_results.size() - 1;
        }
        if (buttons & WPAD_BUTTON_DOWN) {
            selected_item++;
            if (selected_item >= (int)current_results.size()) selected_item = 0;
        }
        if (buttons & WPAD_BUTTON_LEFT) {
            selected_provider--;
            if (selected_provider < 0) selected_provider = providers.size() - 1;
            refresh_results();
        }
        if (buttons & WPAD_BUTTON_RIGHT) {
            selected_provider++;
            if (selected_provider >= (int)providers.size()) selected_provider = 0;
            refresh_results();
        }
        if (buttons & WPAD_BUTTON_A) {
            if (selected_item < (int)current_results.size()) {
                download_selected();
            }
        }
        if (buttons & WPAD_BUTTON_1) {
            show_queue = true;
        }
    }

    void render() {
        GRRLIB_FillScreen(COLOR_BG);

        // Header
        GRRLIB_Printf(20, 20, NULL, COLOR_TEXT, 1, 
                      "Wii Meta-Client | %s", 
                      providers[selected_provider]->name());

        if (show_queue) {
            render_queue();
        } else {
            render_browse();
        }

        GRRLIB_Render();
    }

    void render_browse() {
        int y = 60;
        for (int i = 0; i < (int)current_results.size() && y < 440; i++) {
            u32 color = (i == selected_item) ? COLOR_HIGHLIGHT : COLOR_TEXT;
            GRRLIB_Printf(40, y, NULL, color, 0.8,
                          "%s [%s]", 
                          current_results[i].title.c_str(),
                          current_results[i].source.c_str());
            y += 25;
        }

        // Controls hint
        GRRLIB_Printf(20, 450, NULL, 0x888888FF, 0.6,
                      "A=Download | 1=Queue | HOME=Exit | D-Pad=Navigate");
    }

    void render_queue() {
        GRRLIB_Printf(20, 60, NULL, COLOR_TEXT, 1, "Download Queue");

        int y = 90;
        GRRLIB_Printf(20, y, NULL, COLOR_TEXT, 0.8,
                      "Pending: %d | Active: %d | Done: %d",
                      downloads->pending_count(),
                      downloads->active_count(),
                      downloads->completed_count());
    }

    void refresh_results() {
        current_results.clear();
        selected_item = 0;

        auto result = providers[selected_provider]->browse("", 0);
        current_results = result.entries;
    }

    void download_selected() {
        const GameEntry& entry = current_results[selected_item];
        std::string url = providers[selected_provider]->resolve_download(entry);

        std::string dest = "/apps/" + entry.id + "/" + entry.id + ".zip";
        downloads->enqueue(url, dest, entry.title);
    }
};

int main(int argc, char** argv) {
    MetaClient client;

    if (!client.init()) {
        return 1;
    }

    client.run();
    client.shutdown();

    return 0;
}
