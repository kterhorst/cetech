local ffi = require("ffi")
local api_system = require("cetech/api_system")

ffi.cdef [[
struct ct_transform {
    struct ct_world world;
    uint32_t idx;
};

struct ct_transform_a0 {
    int (*is_valid)(struct ct_transform transform);
    void (*transform)(struct ct_transform transform,
                      mat44f_t *parent);

    void (*get_position)(struct ct_transform transform, float *value);

    void (*get_rotation)(struct ct_transform transform, float *value);

    void (*get_scale)(struct ct_transform transform, float *value);

    void *(*get_world_matrix)(struct ct_transform transform, float *value);

    void (*set_position)(struct ct_transform transform,
                         float* pos);

    void (*set_rotation)(struct ct_transform transform,
                         float* rot);

    void (*set_scale)(struct ct_transform transform,
                      float* scale);

    int (*has)(struct ct_world world,
               struct ct_entity entity);

    struct ct_transform (*get)(struct ct_world world,
                               struct ct_entity entity);

    struct ct_transform (*create)(struct ct_world world,
                                  struct ct_entity entity,
                                  struct ct_entity parent,
                                  float* position,
                                  float* rotation,
                                  float* scale);

    void (*link)(struct ct_world world,
                 struct ct_entity parent,
                 struct ct_entity child);
};
]]

local C = ffi.C
local api = api_system.load("ct_transform_a0")

--! dsadas
--! dasdasda
Transform = Transform or {}

--! Get transformation component
--! \param world lightuserdata Word
--! \param entity lightuserdata Entity
--! \return lightuserdata Transformation component
function Transform.get(world, entity)
    return api.get(world, entity)
end

--! Is transformation component valid?
--! \param transform lightuserdata Transformation component
--! \return boolean True if is valid.
function Transform.is_valid(transform)
    return api.is_valid(transform) > 0
end

--! Has entity transformation component?
--! \param world lightuserdata Word
--! \param entity lightuserdata Entity
--! \return boolean True if has.
function Transform.has(world, entity)
    return api.has(world, entity) > 0
end

--! Get position.
--! \param transform lightuserdata Transformation component.
--! \return cetech.Vec3f Position.
function Transform.get_position(transform)
    local p = ffi.new("float[3]")

    api.get_position(transform, p)

    return cetech.Vec3f.make(p[0], p[1], p[2])
end

--! Get rotation.
--! \param transform lightuserdata Transformation component.
--! \return cetech.Quatf Rotation.
function Transform.get_rotation(transform)
    local p = ffi.new("float[4]")

    api.get_rotation(transform, p)

    return cetech.Quatf.make(p[0], p[1], p[2], p[3])
end

--! Get scale.
--! \param transform lightuserdata Transformation component.
--! \return cetech.Vec3f Scale.
function Transform.get_scale(transform)
    local p = ffi.new("float[3]")

    api.get_scale(transform, p)

    return cetech.Vec3f.make(p[0], p[1], p[2])
end

--! Get world matrix.
--! \param transform lightuserdata Transformation component.
--! \return cetech.Mat44f World matrix.
function Transform.get_world_matrix(transform)
    local p = ffi.new("float[16]")

    api.get_world_matrix(transform, p)

    return cetech.Mat44f.make(p[0], p[1], p[2], p[3],
                              p[4], p[5], p[6], p[7],
                              p[8], p[9], p[10], p[11],
                              p[12], p[13], p[14], p[15])
end

--! Set position.
--! \param transform lightuserdata Transformation component.
--! \param position cetech.Vec3f New position.
function Transform.set_position(transform, position)
    local p = ffi.new("float[3]", { position.x, position.y, position.z })

    api.set_position(transform, p)
end

--! Set rotation.
--! \param transform lightuserdata Transformation component.
--! \param rotation cetech.Quatf New rotation.
function Transform.set_rotation(transform, rotation)
    local r = ffi.new("float[4]", { rotation.x, rotation.y, rotation.z, rotation.w })

    api.set_rotation(transform, r)
end

--! Set scale.
--! \param transform lightuserdata Transformation component.
--! \param scale cetech.Vec3f New scale.
function Transform.set_scale(transform, scale)
    local s = ffi.new("float[3]", { scale.x, scale.y, scale.z })

    api.set_scale(transform, s)
end

return Transform