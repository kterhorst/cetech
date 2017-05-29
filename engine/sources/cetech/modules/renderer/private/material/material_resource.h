#ifndef CETECH_MATERIAL_RESORCE_H
#define CETECH_MATERIAL_RESORCE_H

#include <cetech/core/os/path.h>

static const bgfx_program_handle_t null_program = {0};

void *_material_resource_loader(struct vio *input,
                                struct allocator *allocator) {
    const int64_t size = vio_api_v0.size(input);
    char *data = CETECH_ALLOCATE(allocator, char, size);
    vio_api_v0.read(input, data, 1, size);

    return data;
}

void _material_resource_unloader(void *new_data,
                                 struct allocator *allocator) {
    CETECH_DEALLOCATE(allocator, new_data);
}

void _material_resource_online(uint64_t name,
                               void *data) {
}

void _material_resource_offline(uint64_t name,
                                void *data) {
}

void *_material_resource_reloader(uint64_t name,
                                  void *old_data,
                                  void *new_data,
                                  struct allocator *allocator) {
    _material_resource_offline(name, old_data);
    _material_resource_online(name, new_data);

    CETECH_DEALLOCATE(allocator, old_data);
    return new_data;
}

static const resource_callbacks_t material_resource_callback = {
        .loader = _material_resource_loader,
        .unloader =_material_resource_unloader,
        .online =_material_resource_online,
        .offline =_material_resource_offline,
        .reloader = _material_resource_reloader
};

#endif //CETECH_MATERIAL_RESORCE_H