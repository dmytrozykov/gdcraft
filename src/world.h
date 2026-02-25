#pragma once

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/variant.hpp>

#include "chunk.h"
#include "hit_payload.h"

namespace godot {
class GDC_World: public Node3D {
    GDCLASS(GDC_World, Node3D)

protected:
	static void _bind_methods();

public:
    void register_chunk(GDC_Chunk *p_chunk, Vector2i coord);

    GDC_Chunk *get_chunk(Vector2i coord);
    GDC_Chunk *get_chunk_at(Vector3 world_pos);

    int32_t get_block_at(Vector3 world_pos);
    void set_block_at(Vector3 world_pos, int32_t id);

    Variant raycast(Vector3 from, Vector3 dir, float max_dist);

    static inline Vector2i world_pos_to_chunk_coord(Vector3 world_pos) {
        return Vector2i(
            int(floorf(world_pos.x / GDC_Chunk::SIZE)),
            int(floorf(world_pos.z / GDC_Chunk::SIZE))
        );
    }

    static inline Vector3i world_to_local(Vector3 world_pos) {
        int local_x = int(floorf(world_pos.x)) % GDC_Chunk::SIZE;
        int local_y = int(floorf(world_pos.y));
        int local_z = int(floorf(world_pos.z)) % GDC_Chunk::SIZE;
        if (local_x < 0) { local_x += GDC_Chunk::SIZE; }
        if (local_z < 0) { local_z += GDC_Chunk::SIZE; }
        return Vector3i(local_x, local_y, local_z);
    }

private:
    HashMap<Vector2i, GDC_Chunk *> p_chunks;
};

}  // namespace godot