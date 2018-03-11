#ifndef CETECH_ASSET_BROWSER_H
#define CETECH_ASSET_BROWSER_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>

struct ct_resource_id;

//==============================================================================
// Typedefs
//==============================================================================

struct ct_asset_browser_click_ev {
    uint64_t asset;
    uint64_t root;
    const char *path;
};

#define ASSET_BROWSER_EBUS_NAME "asset_browser"
#define ASSET_BROWSER_EBUS CT_ID64_0(ASSET_BROWSER_EBUS_NAME)

enum {
    ASSET_INAVLID_EVENT = 0,
    ASSET_CLICK_EVENT,
    ASSET_DCLICK_EVENT,
};

//==============================================================================
// Api
//==============================================================================

struct ct_asset_browser_a0 {
    uint64_t (*get_selected_asset_type)();
    void (*get_selected_asset_name)(char *asset_name);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_ASSET_BROWSER_H
