#include "chunk.h"

#include <algorithm>
#include <vector>

#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/concave_polygon_shape3d.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/static_body3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/packed_color_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "block_registry.h"

namespace godot {

enum FaceDir {
	FACE_UP = 0,
	FACE_BACK,
	FACE_LEFT,
	FACE_RIGHT,
	FACE_TOP,
	FACE_BOTTOM,
	FACE_LAST = FACE_BOTTOM
};

using FaceVertices = std::array<Vector3, 4>;

const std::array<FaceVertices, FACE_LAST + 1> FACES = { { // Front
		{ Vector3(0, 0, 1), Vector3(0, 1, 1), Vector3(1, 1, 1), Vector3(1, 0, 1) },
		// Back
		{ Vector3(1, 0, 0), Vector3(1, 1, 0), Vector3(0, 1, 0), Vector3(0, 0, 0) },
		// Left
		{ Vector3(0, 0, 0), Vector3(0, 1, 0), Vector3(0, 1, 1), Vector3(0, 0, 1) },
		// Right
		{ Vector3(1, 0, 1), Vector3(1, 1, 1), Vector3(1, 1, 0), Vector3(1, 0, 0) },
		// Top
		{ Vector3(0, 1, 1), Vector3(0, 1, 0), Vector3(1, 1, 0), Vector3(1, 1, 1) },
		// Bottom
		{ Vector3(0, 0, 0), Vector3(0, 0, 1), Vector3(1, 0, 1), Vector3(1, 0, 0) } } };

const std::array<Vector3, FACE_LAST + 1> FACE_NORMALS = {
	Vector3(0, 0, 1),
	Vector3(0, 0, -1),
	Vector3(-1, 0, 0),
	Vector3(1, 0, 0),
	Vector3(0, 1, 0),
	Vector3(0, -1, 0),
};

constexpr std::array<float, FACE_LAST + 1> FACE_BRIGHTNESS = {
	1.0f, 0.6f, 0.85f, 0.75f, 0.9f, 0.8f
};

const std::array<Vector2, 4> FACE_UVS = {
	Vector2(0, 0), Vector2(1, 0), Vector2(1, 1), Vector2(0, 1)
};

void GDC_Chunk::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_block", "x", "y", "z"), &GDC_Chunk::get_block);
	ClassDB::bind_method(D_METHOD("set_block", "x", "y", "z", "id"), &GDC_Chunk::set_block);

	ClassDB::bind_method(D_METHOD("fill", "id"), &GDC_Chunk::fill);
	ClassDB::bind_method(D_METHOD("fill_range", "from", "to", "id"), &GDC_Chunk::fill_range);

	ClassDB::bind_method(D_METHOD("generate_mesh"), &GDC_Chunk::generate_mesh);

	ClassDB::bind_method(D_METHOD("get_neighbour", "index"), &GDC_Chunk::get_neighbour);
	ClassDB::bind_method(D_METHOD("set_neighbour", "index", "neighbour"), &GDC_Chunk::set_neighbour);

	ClassDB::bind_integer_constant(get_class_static(), StringName(), "SIZE", SIZE);
	ClassDB::bind_integer_constant(get_class_static(), StringName(), "HEIGHT", HEIGHT);
	ClassDB::bind_integer_constant(get_class_static(), StringName(), "BLOCK_COUNT", BLOCK_COUNT);

	ClassDB::bind_integer_constant(get_class_static(), StringName(), "NEIGHBOUR_PX", NEIGHBOUR_PX);
	ClassDB::bind_integer_constant(get_class_static(), StringName(), "NEIGHBOUR_NX", NEIGHBOUR_NX);
	ClassDB::bind_integer_constant(get_class_static(), StringName(), "NEIGHBOUR_PZ", NEIGHBOUR_PZ);
	ClassDB::bind_integer_constant(get_class_static(), StringName(), "NEIGHBOUR_NZ", NEIGHBOUR_NZ);
}

GDC_Chunk::GDC_Chunk() {
	std::fill(blocks.begin(), blocks.end(), 0);
	std::fill(neighbours.begin(), neighbours.end(), nullptr);

	mesh_instance = memnew(MeshInstance3D);
	mesh_instance->set_gi_mode(GeometryInstance3D::GI_MODE_DYNAMIC);

	StandardMaterial3D *mat = memnew(StandardMaterial3D);
	mat->set("vertex_color_use_as_albedo", true);
	mesh_instance->set_material_override(mat);

	add_child(mesh_instance);
}

int32_t GDC_Chunk::get_block(const int32_t p_x, const int32_t p_y, const int32_t p_z) const {
	if (p_x >= 0 && p_y >= 0 && p_z >= 0 && p_x < SIZE && p_y < HEIGHT && p_z < SIZE) {
		return blocks[(p_y * SIZE * SIZE) + (p_z * SIZE) + p_x];
	}
	return -1;
}

void GDC_Chunk::set_block(const int32_t p_x, const int32_t p_y, const int32_t p_z, int32_t p_id) {
	if (p_x >= 0 && p_y >= 0 && p_z >= 0 && p_x < SIZE && p_y < HEIGHT && p_z < SIZE) {
		blocks[(p_y * SIZE * SIZE) + (p_z * SIZE) + p_x] = p_id;
	}
}

void GDC_Chunk::fill(int32_t p_id) {
	std::fill(blocks.begin(), blocks.end(), p_id);
}

void GDC_Chunk::fill_range(Vector3i p_from, Vector3i p_to, int32_t p_id) {
	const int32_t start_x = std::clamp(p_from.x, 0, SIZE);
	const int32_t start_y = std::clamp(p_from.y, 0, HEIGHT);
	const int32_t start_z = std::clamp(p_from.z, 0, SIZE);

	const int32_t end_x = std::clamp(p_to.x, 0, SIZE);
	const int32_t end_y = std::clamp(p_to.y, 0, HEIGHT);
	const int32_t end_z = std::clamp(p_to.z, 0, SIZE);

	if (start_x >= end_x || start_y >= end_y || start_z >= end_z) {
		return;
	}

	for (int32_t y = start_y; y < end_y; ++y) {
		for (int32_t z = start_z; z < end_z; ++z) {
			for (int32_t x = start_x; x < end_x; ++x) {
				set_block(x, y, z, p_id);
			}
		}
	}
}

void GDC_Chunk::generate_mesh() {
	PackedVector3Array vertices;
	PackedVector3Array normals;
	PackedColorArray colors;
	PackedVector2Array uvs;
	PackedInt32Array indices;

	std::vector<Color> color_table;
	if (GDC_BlockRegistry *reg = GDC_BlockRegistry::get_singleton()) {
		const int32_t count = reg->get_block_count();
		color_table.resize(count + 1, Color(1.0f, 0.0f, 0.0f, 1.0f));
		for (int32_t i = 1; i <= count; ++i) {
			Ref<GDC_BlockData> data = reg->get_block_by_id(i);
			if (data.is_valid()) {
				color_table[i] = data->get_color();
			}
		}
	}

	for (int y = 0; y < HEIGHT; ++y) {
		for (int z = 0; z < SIZE; ++z) {
			for (int x = 0; x < SIZE; ++x) {
				const int32_t id = get_block(x, y, z);
				if (id <= 0) {
					continue;
				}

				const Color block_color = (id < static_cast<int32_t>(color_table.size()))
						? color_table[id]
						: Color(1.0f, 0.0f, 0.0f, 1.0f);
				const Vector3 offset(x, y, z);

				for (size_t i = 0; i < FACE_LAST + 1; ++i) {
					const Vector3 face_normal = FACE_NORMALS[i];

					const int32_t nx = x + int(face_normal.x);
					const int32_t ny = y + int(face_normal.y);
					const int32_t nz = z + int(face_normal.z);
					if (get_block_including_neighbours(nx, ny, nz) > 0) {
						continue;
					}

					const FaceVertices face_vertices = FACES[i];
					const float brightness = FACE_BRIGHTNESS[i];
					const Color shaded_color = Color(block_color.r * brightness, block_color.g * brightness, block_color.b * brightness);

					const int32_t base = vertices.size();

					for (int j = 0; j < 4; ++j) {
						vertices.append(offset + face_vertices[j]);
						normals.append(face_normal);
						colors.append(shaded_color);
						uvs.append(FACE_UVS[j]);
					}

					indices.append_array({ base, base + 1, base + 2, base, base + 2, base + 3 });
				}
			}
		}
	}

	Array arrays;
	arrays.resize(ArrayMesh::ARRAY_MAX);

	arrays[ArrayMesh::ARRAY_VERTEX] = vertices;
	arrays[ArrayMesh::ARRAY_INDEX] = indices;
	arrays[ArrayMesh::ARRAY_NORMAL] = normals;
	arrays[ArrayMesh::ARRAY_TEX_UV] = uvs;
	arrays[ArrayMesh::ARRAY_COLOR] = colors;

	ArrayMesh *arr_mesh = memnew(ArrayMesh);
	if (vertices.size() > 0) {
		arr_mesh->add_surface_from_arrays(Mesh::PrimitiveType::PRIMITIVE_TRIANGLES, arrays);
	}
	mesh_instance->set_mesh(arr_mesh);

	free_static_bodies();

	if (vertices.size() > 0) {
		add_collision_shape(arr_mesh);
	}
}

int32_t GDC_Chunk::get_block_including_neighbours(const int32_t p_x, const int32_t p_y, const int32_t p_z) const {
	if (p_x >= 0 && p_y >= 0 && p_z >= 0 && p_x < SIZE && p_y < HEIGHT && p_z < SIZE) {
		return get_block(p_x, p_y, p_z);
	}

	if (p_y < 0 || p_y >= HEIGHT) {
		return 0;
	}

	if (p_x < 0) {
		if (neighbours[NEIGHBOUR_NX] != nullptr) {
			return neighbours[NEIGHBOUR_NX]->get_block(p_x + SIZE, p_y, p_z);
		}
		return 0;
	}

	if (p_x >= SIZE) {
		if (neighbours[NEIGHBOUR_PX] != nullptr) {
			return neighbours[NEIGHBOUR_PX]->get_block(p_x - SIZE, p_y, p_z);
		}
		return 0;
	}

	if (p_z < 0) {
		if (neighbours[NEIGHBOUR_NZ] != nullptr) {
			return neighbours[NEIGHBOUR_NZ]->get_block(p_x, p_y, p_z + SIZE);
		}
		return 0;
	}

	if (p_z >= SIZE) {
		if (neighbours[NEIGHBOUR_PZ] != nullptr) {
			return neighbours[NEIGHBOUR_PZ]->get_block(p_x, p_y, p_z - SIZE);
		}
		return 0;
	}

	return 0;
}

GDC_Chunk *GDC_Chunk::get_neighbour(int32_t p_index) const {
	if (p_index >= 0 && p_index < 4) {
		return neighbours[p_index];
	}
	return nullptr;
}

void GDC_Chunk::set_neighbour(int32_t p_index, GDC_Chunk *p_neighbour) {
	if (p_index >= 0 && p_index < 4) {
		neighbours[p_index] = p_neighbour;
	}
}

void GDC_Chunk::free_static_bodies() {
	Array children = get_children();
	for (int i = 0; i < children.size(); ++i) {
		Node *child = Object::cast_to<Node>(children[i]);
		if (!child) {
			continue;
		}

		if (Object::cast_to<StaticBody3D>(child)) {
			child->queue_free();
		}
	}
}

void GDC_Chunk::add_collision_shape(ArrayMesh *p_arr_mesh) {
	StaticBody3D *static_body = memnew(StaticBody3D);
	add_child(static_body);

	CollisionShape3D *collision_shape = memnew(CollisionShape3D);

	Ref<ConcavePolygonShape3D> shape = p_arr_mesh->create_trimesh_shape();
	collision_shape->set_shape(shape);

	static_body->add_child(collision_shape);
}

} // namespace godot
