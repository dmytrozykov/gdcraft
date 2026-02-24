#include "chunk.h"

#include <algorithm>

#include <godot_cpp/core/class_db.hpp>

#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/concave_polygon_shape3d.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/static_body3d.hpp>

#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/packed_color_array.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

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

const std::array<FaceVertices, FACE_LAST + 1> FACES = {{
    // Front
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
    { Vector3(0, 0, 0), Vector3(0, 0, 1), Vector3(1, 0, 1), Vector3(1, 0, 0) }
}};

const std::array<Vector3, FACE_LAST + 1> FACE_NORMALS = {
    Vector3(0, 0, 1), Vector3(0, 0, -1), Vector3(-1, 0, 0),  Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, -1, 0),
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
    std::fill(p_neighbours.begin(), p_neighbours.end(), nullptr);

    p_mesh_instance = memnew(MeshInstance3D);
    p_mesh_instance->set_gi_mode(GeometryInstance3D::GI_MODE_DYNAMIC);
    
    StandardMaterial3D *p_mat = memnew(StandardMaterial3D);
    p_mat->set("vertex_color_use_as_albedo", true);
    p_mesh_instance->set_material_override(p_mat);

    add_child(p_mesh_instance);
}

int32_t GDC_Chunk::get_block(const int32_t x, const int32_t y, const int32_t z) const {
	if (x >= 0 && y >= 0 && z >= 0 && x < SIZE && y < HEIGHT && z < SIZE) {
		return blocks[(y * SIZE * SIZE) + (z * SIZE) + x];
    }
	return -1;
}

void GDC_Chunk::set_block(const int32_t x, const int32_t y, const int32_t z, uint32_t id) {
    if (x >= 0 && y >= 0 && z >= 0 && x < SIZE && y < HEIGHT && z < SIZE) {
		blocks[(y * SIZE * SIZE) + (z * SIZE) + x] = id;
    }
}

void GDC_Chunk::fill(uint32_t id) {
    std::fill(blocks.begin(), blocks.end(), id);
}

void GDC_Chunk::fill_range(Vector3i from, Vector3i to, uint32_t id) {
    for (int y = from.y; y < to.y; ++y) {
        for (int z = from.z; z < to.z; ++z) {
            for (int x = from.x; x < to.x; ++x) {
                set_block(x, y, z, id);
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

    for (int y = 0; y < HEIGHT; ++y) {
        for (int z = 0; z < SIZE; ++z) {
            for (int x = 0; x < SIZE; ++x) {
                const int32_t id = get_block(x, y, z);
                if (id <= 0) { continue; }

                // TODO: Get from registry instead
                const Color block_color(1.0f, 0.0f, 0.0f, 1.0f);
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
                    
                    indices.append_array({base, base + 1, base + 2, base, base + 2, base + 3});
                }
            }
        }
    }

    Array arrays;
    arrays.resize(ArrayMesh::ARRAY_MAX);

    arrays[ArrayMesh::ARRAY_VERTEX] = vertices;
    arrays[ArrayMesh::ARRAY_INDEX]  = indices;
    arrays[ArrayMesh::ARRAY_NORMAL] = normals;
    arrays[ArrayMesh::ARRAY_TEX_UV] = uvs;
    arrays[ArrayMesh::ARRAY_COLOR]  = colors;

    ArrayMesh *p_arr_mesh = memnew(ArrayMesh);
    if (vertices.size() > 0) {
        p_arr_mesh->add_surface_from_arrays(Mesh::PrimitiveType::PRIMITIVE_TRIANGLES, arrays);
    }
    p_mesh_instance->set_mesh(p_arr_mesh);

    free_static_bodies();

    if (vertices.size() > 0) {
        add_collision_shape(p_arr_mesh);
    }
}

uint32_t GDC_Chunk::get_block_including_neighbours(const int32_t x, const int32_t y, const int32_t z) const {
    if (x >= 0 && y >= 0 && z >= 0 && x < SIZE && y < HEIGHT && z < SIZE) {
		return get_block(x, y, z);
    }

    if (y < 0 || y >= HEIGHT) { return 0; }

    if (x < 0) {
        if (p_neighbours[NEIGHBOUR_NX] != nullptr) {
            return p_neighbours[NEIGHBOUR_NX]->get_block(x + SIZE, y, z);
        }
        return 0;
    }

    if (x >= SIZE) {
        if (p_neighbours[NEIGHBOUR_PX] != nullptr) {
            return p_neighbours[NEIGHBOUR_PX]->get_block(x - SIZE, y, z);
        }
        return 0;
    }

    if (z < 0) {
        if (p_neighbours[NEIGHBOUR_NZ] != nullptr) {
            return p_neighbours[NEIGHBOUR_NZ]->get_block(x, y, z + SIZE);
        }
        return 0;
    }

    if (z >= SIZE) {
        if (p_neighbours[NEIGHBOUR_PZ] != nullptr) {
            return p_neighbours[NEIGHBOUR_PZ]->get_block(x, y, z - SIZE);
        }
        return 0;
    }

	return 0;
}

GDC_Chunk *GDC_Chunk::get_neighbour(int32_t index) const {
    if (index >= 0 && index < 4) {
        return p_neighbours[index];
    }
    return nullptr;
}

void GDC_Chunk::set_neighbour(int32_t index, GDC_Chunk *neighbour) {
    if (index >= 0 && index < 4) {
        p_neighbours[index] = neighbour;
    }
}

void GDC_Chunk::free_static_bodies() {
    Array children = get_children();
    for (int i = 0; i < children.size(); ++i) {
        Node *p_child = Object::cast_to<Node>(children[i]);
        if (!p_child) { continue; }

        if (Object::cast_to<StaticBody3D>(p_child)) {
            p_child->queue_free();
        }
    }
}

void GDC_Chunk::add_collision_shape(ArrayMesh *p_arr_mesh) {
    StaticBody3D *p_static_body = memnew(StaticBody3D);
    add_child(p_static_body);

    CollisionShape3D *p_collision_shape = memnew(CollisionShape3D);

    Ref<ConcavePolygonShape3D> shape = p_arr_mesh->create_trimesh_shape();
    p_collision_shape->set_shape(shape);

    p_static_body->add_child(p_collision_shape);
}
