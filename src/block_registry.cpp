#include "block_registry.h"

#include <godot_cpp/core/class_db.hpp>

namespace godot {

GDC_BlockRegistry *GDC_BlockRegistry::singleton = nullptr;

void GDC_BlockRegistry::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_block_set"), &GDC_BlockRegistry::get_block_set);
	ClassDB::bind_method(D_METHOD("set_block_set", "block_set"), &GDC_BlockRegistry::set_block_set);
	ADD_PROPERTY(
			PropertyInfo(Variant::OBJECT, "block_set", PROPERTY_HINT_RESOURCE_TYPE, "GDC_BlockSet"),
			"set_block_set", "get_block_set");

	ClassDB::bind_method(D_METHOD("get_block_by_id", "id"), &GDC_BlockRegistry::get_block_by_id);
	ClassDB::bind_method(D_METHOD("get_block_by_name", "name"), &GDC_BlockRegistry::get_block_by_name);
	ClassDB::bind_method(D_METHOD("get_block_count"), &GDC_BlockRegistry::get_block_count);
}

GDC_BlockRegistry *GDC_BlockRegistry::get_singleton() {
	return singleton;
}

void GDC_BlockRegistry::_enter_tree() {
	singleton = this;
	reload();
}

void GDC_BlockRegistry::_exit_tree() {
	if (singleton == this) {
		singleton = nullptr;
	}
}

Ref<GDC_BlockSet> GDC_BlockRegistry::get_block_set() const {
	return block_set;
}

void GDC_BlockRegistry::set_block_set(const Ref<GDC_BlockSet> &p_block_set) {
	block_set = p_block_set;
	if (is_inside_tree()) {
		reload();
	}
}

Ref<GDC_BlockData> GDC_BlockRegistry::get_block_by_id(int32_t id) const {
	const size_t idx = static_cast<size_t>(id) - 1;
	if (idx < blocks_by_id.size()) {
		return blocks_by_id[idx];
	}
	return Ref<GDC_BlockData>();
}

Ref<GDC_BlockData> GDC_BlockRegistry::get_block_by_name(const String &p_name) const {
	const String key = p_name.to_lower();
	const Ref<GDC_BlockData> *found = blocks_by_name.getptr(key);
	if (found != nullptr) {
		return *found;
	}
	return Ref<GDC_BlockData>();
}

int32_t GDC_BlockRegistry::get_block_count() const {
	return static_cast<int32_t>(blocks_by_id.size());
}

void GDC_BlockRegistry::reload() {
	blocks_by_id.clear();
	blocks_by_name.clear();

	if (block_set.is_null()) {
		return;
	}

	const TypedArray<GDC_BlockData> blocks = block_set->get_blocks();
	for (int i = 0; i < blocks.size(); ++i) {
		Ref<GDC_BlockData> block = blocks[i];
		if (block.is_null()) {
			continue;
		}
		const int32_t id = static_cast<int>(blocks_by_id.size()) + 1;
		block->set_id(id);
		blocks_by_id.push_back(block);
		blocks_by_name.insert(block->get_block_name().to_lower(), block);
	}
}

} // namespace godot
