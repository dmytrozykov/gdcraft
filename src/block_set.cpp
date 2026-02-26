#include "block_set.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void GDC_BlockSet::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_blocks"), &GDC_BlockSet::get_blocks);
    ClassDB::bind_method(D_METHOD("set_blocks", "blocks"), &GDC_BlockSet::set_blocks);
    ADD_PROPERTY(
        PropertyInfo(Variant::ARRAY, "blocks", PROPERTY_HINT_ARRAY_TYPE, "GDC_BlockData"),
        "set_blocks", "get_blocks"
    );
}

TypedArray<GDC_BlockData> GDC_BlockSet::get_blocks() const {
    return blocks;
}

void GDC_BlockSet::set_blocks(const TypedArray<GDC_BlockData> &p_blocks) {
    blocks = p_blocks;
}
