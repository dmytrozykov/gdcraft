#include "hit_payload.h"

#include <godot_cpp/core/class_db.hpp>

namespace godot {

void GDC_HitPayload::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_block_pos"), &GDC_HitPayload::get_block_pos);
	ClassDB::bind_method(D_METHOD("set_block_pos", "pos"), &GDC_HitPayload::set_block_pos);
	ClassDB::bind_method(D_METHOD("get_normal"), &GDC_HitPayload::get_normal);
	ClassDB::bind_method(D_METHOD("set_normal", "normal"), &GDC_HitPayload::set_normal);
	ClassDB::bind_method(D_METHOD("get_block_id"), &GDC_HitPayload::get_block_id);
	ClassDB::bind_method(D_METHOD("set_block_id", "id"), &GDC_HitPayload::set_block_id);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3I, "block_pos"), "set_block_pos", "get_block_pos");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3I, "normal"), "set_normal", "get_normal");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "block_id"), "set_block_id", "get_block_id");
}

Vector3i GDC_HitPayload::get_block_pos() const {
	return block_pos;
}
void GDC_HitPayload::set_block_pos(const Vector3i &p_pos) {
	block_pos = p_pos;
}

Vector3i GDC_HitPayload::get_normal() const {
	return normal;
}
void GDC_HitPayload::set_normal(const Vector3i &p_normal) {
	normal = p_normal;
}

int32_t GDC_HitPayload::get_block_id() const {
	return block_id;
}
void GDC_HitPayload::set_block_id(int32_t p_id) {
	block_id = p_id;
}

} // namespace godot