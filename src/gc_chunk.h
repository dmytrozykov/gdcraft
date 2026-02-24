#pragma once

#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>

namespace godot {

class GCChunk: public Node3D {
    GDCLASS(GCChunk, Node)

public:
    static constexpr size_t SIZE = 16;
    static constexpr size_t HEIGHT = 128;
    static constexpr size_t BLOCK_COUNT = SIZE * SIZE * HEIGHT;

private:
    uint32_t blocks[BLOCK_COUNT];
    MeshInstance3D *p_mesh_instance;
    GCChunk *p_neighbours[4];

protected:
    static void _bind_methods();

public:
	GCChunk();
	~GCChunk() override = default;

    void prepare();
    int32_t get_block(int32_t x, int32_t y, int32_t z) const;
    void set_block(int32_t x, int32_t y, int32_t z, uint32_t id);
    void fill(uint32_t id);
    void fill_range(Vector3i from, Vector3i to, uint32_t id);
    void generate_mesh();

private:
    uint32_t get_block_including_neighbours(int32_t x, int32_t y, int32_t z) const;
    void free_static_bodies();
    void add_collision_shape(ArrayMesh *p_arr_mesh);
};

} // namespace godot
