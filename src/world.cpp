#include "world.h"

#include <cmath>

#include <godot_cpp/core/class_db.hpp>

namespace godot {

void GDC_World::_bind_methods() {
	ClassDB::bind_method(D_METHOD("register_chunk", "chunk", "coord"), &GDC_World::register_chunk);
	ClassDB::bind_method(D_METHOD("get_chunk", "coord"), &GDC_World::get_chunk);
	ClassDB::bind_method(D_METHOD("get_chunk_at", "world_pos"), &GDC_World::get_chunk_at);
	ClassDB::bind_method(D_METHOD("get_block_at", "world_pos"), &GDC_World::get_block_at);
	ClassDB::bind_method(D_METHOD("set_block_at", "world_pos", "id"), &GDC_World::set_block_at);
	ClassDB::bind_method(D_METHOD("raycast", "from", "dir", "max_dist"), &GDC_World::raycast);
}

void GDC_World::register_chunk(GDC_Chunk *p_chunk, Vector2i p_coord) {
	if (p_chunk == nullptr) {
		return;
	}
	if (chunks.has(p_coord)) {
		return;
	}

	chunks[p_coord] = p_chunk;
	add_child(p_chunk);
	p_chunk->set_position(Vector3(
			p_coord.x * GDC_Chunk::SIZE, 0, p_coord.y * GDC_Chunk::SIZE));

	GDC_Chunk *px = get_chunk(p_coord + Vector2i(1, 0));
	GDC_Chunk *nx = get_chunk(p_coord + Vector2i(-1, 0));
	GDC_Chunk *pz = get_chunk(p_coord + Vector2i(0, 1));
	GDC_Chunk *nz = get_chunk(p_coord + Vector2i(0, -1));

	p_chunk->set_neighbour(GDC_Chunk::NEIGHBOUR_PX, px);
	p_chunk->set_neighbour(GDC_Chunk::NEIGHBOUR_NX, nx);
	p_chunk->set_neighbour(GDC_Chunk::NEIGHBOUR_PZ, pz);
	p_chunk->set_neighbour(GDC_Chunk::NEIGHBOUR_NZ, nz);

	if (px) {
		px->set_neighbour(GDC_Chunk::NEIGHBOUR_NX, p_chunk);
	}
	if (nx) {
		nx->set_neighbour(GDC_Chunk::NEIGHBOUR_PX, p_chunk);
	}
	if (pz) {
		pz->set_neighbour(GDC_Chunk::NEIGHBOUR_NZ, p_chunk);
	}
	if (nz) {
		nz->set_neighbour(GDC_Chunk::NEIGHBOUR_PZ, p_chunk);
	}
}

GDC_Chunk *GDC_World::get_chunk(Vector2i p_coord) {
	if (!chunks.has(p_coord)) {
		return nullptr;
	}
	return chunks.get(p_coord);
}

GDC_Chunk *GDC_World::get_chunk_at(Vector3 p_world_pos) {
	return get_chunk(world_pos_to_chunk_coord(p_world_pos));
}

int32_t GDC_World::get_block_at(Vector3 p_world_pos) {
	GDC_Chunk *chunk = get_chunk_at(p_world_pos);
	if (!chunk) {
		return -1;
	}

	Vector3i local = world_to_local(p_world_pos);
	return chunk->get_block(local.x, local.y, local.z);
}

void GDC_World::set_block_at(Vector3 p_world_pos, int32_t p_id) {
	GDC_Chunk *chunk = get_chunk_at(p_world_pos);
	if (!chunk) {
		return;
	}

	Vector3i local = world_to_local(p_world_pos);
	chunk->set_block(local.x, local.y, local.z, p_id);
	chunk->generate_mesh();

	Vector2i chunk_coord = world_pos_to_chunk_coord(p_world_pos);
	if (local.x == 0) {
		GDC_Chunk *neighbour = get_chunk(chunk_coord + Vector2i(-1, 0));
		if (neighbour) {
			neighbour->generate_mesh();
		}
	}
	if (local.x == GDC_Chunk::SIZE - 1) {
		GDC_Chunk *neighbour = get_chunk(chunk_coord + Vector2i(1, 0));
		if (neighbour) {
			neighbour->generate_mesh();
		}
	}
	if (local.z == 0) {
		GDC_Chunk *neighbour = get_chunk(chunk_coord + Vector2i(0, -1));
		if (neighbour) {
			neighbour->generate_mesh();
		}
	}
	if (local.z == GDC_Chunk::SIZE - 1) {
		GDC_Chunk *neighbour = get_chunk(chunk_coord + Vector2i(0, 1));
		if (neighbour) {
			neighbour->generate_mesh();
		}
	}
}

Variant GDC_World::raycast(Vector3 p_from, Vector3 p_dir, float p_max_dist) {
	if (p_max_dist <= 0.0f || p_dir.is_zero_approx()) {
		return Variant();
	}
	p_dir = p_dir.normalized();

	float dx = p_dir.x;
	float dy = p_dir.y;
	float dz = p_dir.z;

	int ix = int(floorf(p_from.x));
	int iy = int(floorf(p_from.y));
	int iz = int(floorf(p_from.z));

	int step_x = dx >= 0.0f ? 1 : -1;
	int step_y = dy >= 0.0f ? 1 : -1;
	int step_z = dz >= 0.0f ? 1 : -1;

	float tx_delta = dx == 0.0f ? 1e30f : fabsf(1.0f / dx);
	float ty_delta = dy == 0.0f ? 1e30f : fabsf(1.0f / dy);
	float tz_delta = dz == 0.0f ? 1e30f : fabsf(1.0f / dz);

	float tx_max = step_x > 0 ? tx_delta * (1.0f - fmodf(p_from.x - (float)ix, 1.0f)) : tx_delta * fmodf(p_from.x - ix, 1.0f);
	float ty_max = step_y > 0 ? ty_delta * (1.0f - fmodf(p_from.y - (float)iy, 1.0f)) : ty_delta * fmodf(p_from.y - iy, 1.0f);
	float tz_max = step_z > 0 ? tz_delta * (1.0f - fmodf(p_from.z - (float)iz, 1.0f)) : tz_delta * fmodf(p_from.z - iz, 1.0f);

	int stepped_index = -1;
	float t = 0.0f;
	while (t <= p_max_dist) {
		int32_t block = get_block_at(Vector3(ix, iy, iz));
		if (block > 0 && stepped_index != -1) {
			Vector3i norm(0, 0, 0);
			if (stepped_index == 0) {
				norm.x = -step_x;
			} else if (stepped_index == 1) {
				norm.y = -step_y;
			} else if (stepped_index == 2) {
				norm.z = -step_z;
			}
			Ref<GDC_HitPayload> payload;
			payload.instantiate();
			payload->set_block_pos(Vector3i(ix, iy, iz));
			payload->set_normal(norm);
			payload->set_block_id(block);
			return payload;
		}
		if (tx_max < ty_max) {
			if (tx_max < tz_max) {
				ix += step_x;
				t = tx_max;
				tx_max += tx_delta;
				stepped_index = 0;
			} else {
				iz += step_z;
				t = tz_max;
				tz_max += tz_delta;
				stepped_index = 2;
			}
		} else {
			if (ty_max < tz_max) {
				iy += step_y;
				t = ty_max;
				ty_max += ty_delta;
				stepped_index = 1;
			} else {
				iz += step_z;
				t = tz_max;
				tz_max += tz_delta;
				stepped_index = 2;
			}
		}
	}
	return Variant();
}

} // namespace godot
