#include "block_data.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void GDC_BlockData::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_block_name"), &GDC_BlockData::get_block_name);
    ClassDB::bind_method(D_METHOD("set_block_name", "name"), &GDC_BlockData::set_block_name);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "name"), "set_block_name", "get_block_name");

    ClassDB::bind_method(D_METHOD("get_color"), &GDC_BlockData::get_color);
    ClassDB::bind_method(D_METHOD("set_color", "color"), &GDC_BlockData::set_color);
    ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color"), "set_color", "get_color");

    ClassDB::bind_method(D_METHOD("get_id"), &GDC_BlockData::get_id);
    ClassDB::bind_method(D_METHOD("set_id", "id"), &GDC_BlockData::set_id);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "id"), "set_id", "get_id");
}

String GDC_BlockData::get_block_name() const {
    return block_name;
}

void GDC_BlockData::set_block_name(const String &p_name) {
    block_name = p_name;
}

Color GDC_BlockData::get_color() const {
    return color;
}

void GDC_BlockData::set_color(const Color &p_color) {
    color = p_color;
}

int32_t GDC_BlockData::get_id() const {
    return id;
}

void GDC_BlockData::set_id(int32_t p_id) {
    id = p_id;
}
