#pragma once

#include <array>

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>

namespace godot {

class GDC_Chunk : public Node3D {
	GDCLASS(GDC_Chunk, Node3D)

public:
    static const size_t SIZE = 16;
    static const size_t HEIGHT = 128;
    static const size_t BLOCK_COUNT = SIZE * SIZE * HEIGHT;

    static const int32_t NEIGHBOUR_PX = 0; // +X neighbour
    static const int32_t NEIGHBOUR_NX = 1; // -X neighbour
    static const int32_t NEIGHBOUR_PZ = 2; // +Z neighbour
    static const int32_t NEIGHBOUR_NZ = 3; // -Z neighbour

	std::array<GDC_Chunk *, 4> p_neighbours;

private:
	std::array<int32_t, BLOCK_COUNT> blocks;
	MeshInstance3D *p_mesh_instance;

protected:
	static void _bind_methods();

public:
	GDC_Chunk();
	~GDC_Chunk() override = default;

	int32_t get_block(int32_t x, int32_t y, int32_t z) const;
    void set_block(int32_t x, int32_t y, int32_t z, int32_t id);

    void fill(int32_t id);
    void fill_range(Vector3i from, Vector3i to, int32_t id);
	
    GDC_Chunk *get_neighbour(int32_t index) const;
    void set_neighbour(int32_t index, GDC_Chunk *neighbour);

	void generate_mesh();

private:
	int32_t get_block_including_neighbours(int32_t x, int32_t y, int32_t z) const;
    void free_static_bodies();
    void add_collision_shape(ArrayMesh *p_arr_mesh);
};

} // namespace godot
