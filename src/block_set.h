#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/typed_array.hpp>

#include "block_data.h"

namespace godot {

class GDC_BlockSet : public Resource {
    GDCLASS(GDC_BlockSet, Resource)

    TypedArray<GDC_BlockData> blocks;

protected:
    static void _bind_methods();

public:
    GDC_BlockSet() = default;
    ~GDC_BlockSet() override = default;

    TypedArray<GDC_BlockData> get_blocks() const;
    void set_blocks(const TypedArray<GDC_BlockData> &p_blocks);
};

} // namespace godot
