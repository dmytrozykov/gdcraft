#include "world_generator.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/templates/local_vector.hpp>
#include <godot_cpp/variant/variant.hpp>

#include "block_registry.h"

namespace godot {

void GDC_WorldGenerator::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_world", "world"), &GDC_WorldGenerator::set_world);
	ClassDB::bind_method(D_METHOD("get_world"), &GDC_WorldGenerator::get_world);

	ADD_PROPERTY(
			PropertyInfo(
					Variant::OBJECT,
					"world",
					PROPERTY_HINT_NODE_TYPE,
					"GDC_World"),
			"set_world",
			"get_world");
}

void GDC_WorldGenerator::_ready() {
	generate();
}

void GDC_WorldGenerator::set_world(GDC_World *p_world) {
	world = p_world;
}

GDC_World *GDC_WorldGenerator::get_world() const {
	return world;
}

void GDC_WorldGenerator::generate() {
	if (world == nullptr) {
		ERR_PRINT("WorldGenerator: world node is not set.");
		return;
	}

	Ref<GDC_BlockData> stone = GDC_BlockRegistry::get_singleton()->get_block_by_name("stone");
	Ref<GDC_BlockData> dirt = GDC_BlockRegistry::get_singleton()->get_block_by_name("dirt");
	Ref<GDC_BlockData> grass = GDC_BlockRegistry::get_singleton()->get_block_by_name("grass");

	if (stone == nullptr || dirt == nullptr || grass == nullptr) {
		ERR_PRINT("WorldGenerator: required blocks (stone/dirt/grass) are missing in BlockRegistry.");
		return;
	}

	LocalVector<GDC_Chunk *> chunks;
	for (int32_t z = 0; z < 4; ++z) {
		for (int32_t x = 0; x < 4; ++x) {
			GDC_Chunk *chunk = memnew(GDC_Chunk);
			chunk->fill_range(Vector3i(0, 0, 0), Vector3i(16, 2, 16), stone->get_id());
			chunk->fill_range(Vector3i(0, 2, 0), Vector3i(16, 4, 16), dirt->get_id());
			chunk->fill_range(Vector3i(0, 4, 0), Vector3i(16, 5, 16), grass->get_id());
			chunk->set_block(8, 6, 8, stone->get_id());
			world->register_chunk(chunk, Vector2i(x, z));
			chunks.push_back(chunk);
		}
	}

	for (GDC_Chunk *chunk : chunks) {
		chunk->generate_mesh();
	}
}

} // namespace godot