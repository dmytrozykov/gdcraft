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

void GDC_World::register_chunk(GDC_Chunk *p_chunk, Vector2i coord) {
    p_chunks[coord] = p_chunk;
    add_child(p_chunk);
    p_chunk->set_position(Vector3(
        coord.x * GDC_Chunk::SIZE, 0, coord.y * GDC_Chunk::SIZE
    ));

    GDC_Chunk *p_px = get_chunk(coord + Vector2i(1, 0));
    GDC_Chunk *p_nx = get_chunk(coord + Vector2i(-1, 0));
    GDC_Chunk *p_pz = get_chunk(coord + Vector2i(0, 1));
    GDC_Chunk *p_nz = get_chunk(coord + Vector2i(0, -1));

    p_chunk->set_neighbour(GDC_Chunk::NEIGHBOUR_PX, p_px);
    p_chunk->set_neighbour(GDC_Chunk::NEIGHBOUR_NX, p_nx);
    p_chunk->set_neighbour(GDC_Chunk::NEIGHBOUR_PZ, p_pz);
    p_chunk->set_neighbour(GDC_Chunk::NEIGHBOUR_NZ, p_nz);

    if (p_px) p_px->set_neighbour(GDC_Chunk::NEIGHBOUR_NX, p_chunk);
    if (p_nx) p_nx->set_neighbour(GDC_Chunk::NEIGHBOUR_PX, p_chunk);
    if (p_pz) p_pz->set_neighbour(GDC_Chunk::NEIGHBOUR_NZ, p_chunk);
    if (p_nz) p_nz->set_neighbour(GDC_Chunk::NEIGHBOUR_PZ, p_chunk);
}

GDC_Chunk *GDC_World::get_chunk(Vector2i coord) {
    if (!p_chunks.has(coord)) { return nullptr; }
	return p_chunks.get(coord);
}

GDC_Chunk *GDC_World::get_chunk_at(Vector3 world_pos) {
	return get_chunk(world_pos_to_chunk_coord(world_pos));
}

int32_t GDC_World::get_block_at(Vector3 world_pos) {
	GDC_Chunk *p_chunk = get_chunk_at(world_pos);
    if (!p_chunk) { return -1; }

    Vector3i local = world_to_local(world_pos);
    return p_chunk->get_block(local.x, local.y, local.z);
}

void GDC_World::set_block_at(Vector3 world_pos, int32_t id) {
    GDC_Chunk *p_chunk = get_chunk_at(world_pos);
    if (!p_chunk) { return; }

    Vector3i local = world_to_local(world_pos);
    p_chunk->set_block(local.x, local.y, local.z, id);
    p_chunk->generate_mesh();

    Vector2i chunk_coord = world_pos_to_chunk_coord(world_pos);
    if (local.x == 0) {
        GDC_Chunk *p_neighbour = get_chunk(chunk_coord + Vector2i(-1, 0));
        if (p_neighbour) { p_neighbour->generate_mesh(); }
    }
    if (local.x == GDC_Chunk::SIZE - 1) {
        GDC_Chunk *p_neighbour = get_chunk(chunk_coord + Vector2i(1, 0));
        if (p_neighbour) { p_neighbour->generate_mesh(); }
    }
    if (local.z == 0) {
        GDC_Chunk *p_neighbour = get_chunk(chunk_coord + Vector2i(0, -1));
        if (p_neighbour) { p_neighbour->generate_mesh(); }
    }
    if (local.z == GDC_Chunk::SIZE - 1) {
        GDC_Chunk *p_neighbour = get_chunk(chunk_coord + Vector2i(0, 1));
        if (p_neighbour) { p_neighbour->generate_mesh(); }
    }
}

Variant GDC_World::raycast(Vector3 from, Vector3 dir, float max_dist) {
	float dx = dir.x;
    float dy = dir.y;
    float dz = dir.z;

    int ix = int(floorf(from.x));
    int iy = int(floorf(from.y));
    int iz = int(floorf(from.z));

    int step_x = dx >= 0.0f ? 1 : -1;
    int step_y = dy >= 0.0f ? 1 : -1;
    int step_z = dz >= 0.0f ? 1 : -1;

    float tx_delta = dx == 0.0f ? 1e30f : fabsf(1.0f / dx);
    float ty_delta = dy == 0.0f ? 1e30f : fabsf(1.0f / dy);
    float tz_delta = dz == 0.0f ? 1e30f : fabsf(1.0f / dz);

    float tx_max = step_x > 0 ? tx_delta * (1.0f - fmodf(from.x - (float)ix, 1.0f)) : tx_delta * fmodf(from.x - ix, 1.0f);
    float ty_max = step_y > 0 ? ty_delta * (1.0f - fmodf(from.y - (float)iy, 1.0f)) : ty_delta * fmodf(from.y - iy, 1.0f);
    float tz_max = step_z > 0 ? tz_delta * (1.0f - fmodf(from.z - (float)iz, 1.0f)) : tz_delta * fmodf(from.z - iz, 1.0f);

    int stepped_index = -1;
    float t = 0.0f;
    while (t <= max_dist) {
        int32_t block = get_block_at(Vector3(ix, iy, iz));
        if (block > 0 && stepped_index != -1) {
            Vector3i norm(0, 0, 0);
            if (stepped_index == 0) norm.x = -step_x;
            else if (stepped_index == 1) norm.y = -step_y;
            else if (stepped_index == 2) norm.z = -step_z;
            Ref<GDC_HitPayload> payload;
            payload.instantiate();
            payload->set_block_pos(Vector3i(ix, iy, iz));
            payload->set_normal(norm);
            payload->set_block_id(block);
            return payload;
        }
        if (tx_max < ty_max) {
            if (tx_max < tz_max) {
                ix += step_x; t = tx_max; tx_max += tx_delta; stepped_index = 0;
            } else {
                iz += step_z; t = tz_max; tz_max += tz_delta; stepped_index = 2;
            }
        } else {
            if (ty_max < tz_max) {
                iy += step_y; t = ty_max; ty_max += ty_delta; stepped_index = 1;
            } else {
                iz += step_z; t = tz_max; tz_max += tz_delta; stepped_index = 2;
            }
        }
    }
    return Variant();
}

} // namespace godot
