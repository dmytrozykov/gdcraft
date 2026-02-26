#pragma once

#include <array>

#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/node3d.hpp>

namespace godot {

class GDC_Chunk : public Node3D {
	GDCLASS(GDC_Chunk, Node3D)

public:
	static const int32_t SIZE = 16;
	static const int32_t HEIGHT = 128;
	static const int32_t BLOCK_COUNT = SIZE * SIZE * HEIGHT;

	static const int32_t NEIGHBOUR_PX = 0; // +X neighbour
	static const int32_t NEIGHBOUR_NX = 1; // -X neighbour
	static const int32_t NEIGHBOUR_PZ = 2; // +Z neighbour
	static const int32_t NEIGHBOUR_NZ = 3; // -Z neighbour

private:
	std::array<int32_t, BLOCK_COUNT> blocks;
	MeshInstance3D *mesh_instance;
	std::array<GDC_Chunk *, 4> neighbours;

protected:
	static void _bind_methods();

public:
	GDC_Chunk();
	~GDC_Chunk() override = default;

	int32_t get_block(int32_t p_x, int32_t p_y, int32_t p_z) const;
	void set_block(int32_t x, int32_t p_y, int32_t p_z, int32_t id);

	void fill(int32_t p_id);
	void fill_range(Vector3i p_from, Vector3i p_to, int32_t p_id);

	GDC_Chunk *get_neighbour(int32_t p_index) const;
	void set_neighbour(int32_t p_index, GDC_Chunk *p_neighbour);

	void generate_mesh();

private:
	int32_t get_block_including_neighbours(int32_t p_x, int32_t p_y, int32_t p_z) const;
	void free_static_bodies();
	void add_collision_shape(ArrayMesh *p_arr_mesh);
};

} // namespace godot
