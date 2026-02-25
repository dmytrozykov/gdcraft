#pragma once

#include <cstdint>

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/vector3i.hpp>

namespace godot {

class GDC_HitPayload: public RefCounted {
    GDCLASS(GDC_HitPayload, RefCounted)

    Vector3i block_pos;
    Vector3i normal;
    int32_t block_id = 0;

protected:
    static void _bind_methods();

public:
    GDC_HitPayload() = default;
    ~GDC_HitPayload() override = default;

    Vector3i get_block_pos() const;
    void set_block_pos(const Vector3i &p_pos);

    Vector3i get_normal() const;
    void set_normal(const Vector3i &p_normal);

    int32_t get_block_id() const;
    void set_block_id(int32_t p_id);
};

} // namespace godot
