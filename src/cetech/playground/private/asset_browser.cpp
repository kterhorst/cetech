#include "celib/map.inl"

#include <cetech/debugui/debugui.h>
#include <cetech/playground/asset_browser.h>
#include <cetech/debugui/private/ocornut-imgui/imgui.h>
#include <cetech/filesystem/filesystem.h>
#include <cetech/os/path.h>
#include <cetech/resource/resource.h>

#include "cetech/hashlib/hashlib.h"
#include "cetech/config/config.h"
#include "cetech/memory/memory.h"
#include "cetech/api/api_system.h"
#include "cetech/module/module.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_filesystem_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_resource_a0);

using namespace celib;

#define _G property_inspector_global
static struct _G {
    float left_column_width;
    float midle_column_width;
    char current_dir[512];
    const char *root;
    bool visible;

    Array<ct_ab_on_asset_click> on_asset_click;
    Array<ct_ab_on_asset_double_click> on_asset_double_click;
} _G;

#define _DEF_ON_CLB_FCE(type, name)                                            \
    static void register_ ## name ## _(type name) {                            \
        celib::array::push_back(_G.name, name);                                \
    }                                                                          \
    static void unregister_## name ## _(type name) {                           \
        const auto size = celib::array::size(_G.name);                         \
                                                                               \
        for(uint32_t i = 0; i < size; ++i) {                                   \
            if(_G.name[i] != name) {                                           \
                continue;                                                      \
            }                                                                  \
                                                                               \
            uint32_t last_idx = size - 1;                                      \
            _G.name[i] = _G.name[last_idx];                                    \
                                                                               \
            celib::array::pop_back(_G.name);                                   \
            break;                                                             \
        }                                                                      \
    }

_DEF_ON_CLB_FCE(ct_ab_on_asset_click, on_asset_click);

_DEF_ON_CLB_FCE(ct_ab_on_asset_double_click, on_asset_double_click);

static ct_asset_browser_a0 asset_browser_api = {
        .register_on_asset_click = register_on_asset_click_,
        .unregister_on_asset_click = unregister_on_asset_click_,
        .register_on_asset_double_click =  register_on_asset_double_click_,
        .unregister_on_asset_double_click = unregister_on_asset_double_click_,
};

static void leftColumn() {
    ImVec2 size = {_G.left_column_width, 0.0f};

    ImGui::BeginChild("left_col", size);
    ImGui::PushItemWidth(120);

    if (ImGui::TreeNode("Source")) {
        if (ImGui::Selectable("source")) {
            strcpy(_G.current_dir, "");
            _G.root = "source";
        }

        ct_filesystem_a0.listdir2(
                ct_hash_a0.id64_from_str("source"),
                "", "*",
                true, true,
                [](const char *path) {
                    const char *short_path = path +
                                             strlen(ct_filesystem_a0.root_dir(
                                                     ct_hash_a0.id64_from_str(
                                                             "source"))) + 1;
                    if (ImGui::Selectable(short_path)) {
                        strcpy(_G.current_dir, short_path);
                        _G.root = "source";
                    }
                });

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Core")) {
        if (ImGui::Selectable("core")) {
            strcpy(_G.current_dir, "");
            _G.root = "core";
        }

        ct_filesystem_a0.listdir2(
                ct_hash_a0.id64_from_str("core"),
                "", "*",
                true, true,
                [](const char *path) {
                    const char *short_path = path +
                                             strlen(ct_filesystem_a0.root_dir(
                                                     ct_hash_a0.id64_from_str(
                                                             "core"))) + 1;
                    if (ImGui::Selectable(short_path)) {
                        strcpy(_G.current_dir, short_path);
                        _G.root = "core";
                    }
                });

        ImGui::TreePop();
    }

    ImGui::PopItemWidth();
    ImGui::EndChild();
}

static void middleColumn() {
    ImVec2 size = {_G.midle_column_width, 0.0f};

    ImGui::BeginChild("middle_col", size);

    if (_G.root) {
        ct_filesystem_a0.listdir2(
                ct_hash_a0.id64_from_str(_G.root),
                _G.current_dir, "*",
                false, false,
                [](const char *path) {
                    auto root = ct_hash_a0.id64_from_str(_G.root);
                    const char *root_dir = ct_filesystem_a0.root_dir(root);

                    const char *filename = ct_path_a0.filename(path);
                    if (ImGui::Selectable(filename, false, ImGuiSelectableFlags_AllowDoubleClick)) {
                        uint64_t type, name;
                        ct_resource_a0.type_name_from_filename(root_dir, path,
                                                               &type, &name,
                                                               NULL);

                        if (ImGui::IsMouseDoubleClicked(0)) {
                            for (uint32_t i = 0;
                                 i < array::size(_G.on_asset_double_click); ++i) {
                                _G.on_asset_double_click[i](type, name, root, path + strlen(root_dir) + 1);
                            }
                        } else {
                            for (uint32_t i = 0;
                                 i < array::size(_G.on_asset_click); ++i) {
                                _G.on_asset_click[i](type, name, root, path + strlen(root_dir) + 1);
                            }
                        }
                    }
                });
    }

    ImGui::EndChild();
}


static void on_gui() {
    if (ct_debugui_a0.BeginDock("Asset browser",
                                &_G.visible,
                                DebugUIWindowFlags_(0))) {

        float content_w = ImGui::GetContentRegionAvailWidth();
        if (_G.midle_column_width < 0)
            _G.midle_column_width = content_w -
                                    _G.left_column_width -
                                    120;

        leftColumn();

        float left_size[] = {_G.left_column_width, 0.0f};
        ct_debugui_a0.SameLine(0.0f, -1.0f);
        ct_debugui_a0.VSplitter("vsplit1", left_size);
        _G.left_column_width = left_size[0];
        ct_debugui_a0.SameLine(0.0f, -1.0f);

        middleColumn();

    }

    ct_debugui_a0.EndDock();
}

static void _init(ct_api_a0 *api) {
    _G = {
            .visible = true,
            .left_column_width = 120.0f
    };

    api->register_api("ct_asset_browser_a0", &asset_browser_api);
    ct_debugui_a0.register_on_gui(on_gui);

    _G.on_asset_click.init(ct_memory_a0.main_allocator());
    _G.on_asset_double_click.init(ct_memory_a0.main_allocator());
}

static void _shutdown() {
    _G.on_asset_click.destroy();
    _G.on_asset_double_click.destroy();

    _G = {};
}

CETECH_MODULE_DEF(
        asset_browser,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_debugui_a0);
            CETECH_GET_API(api, ct_filesystem_a0);
            CETECH_GET_API(api, ct_path_a0);
            CETECH_GET_API(api, ct_resource_a0);
        },
        {
            _init(api);
        },
        {
            CEL_UNUSED(api);
            _shutdown();
        }
)