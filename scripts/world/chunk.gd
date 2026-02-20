class_name Chunk
extends Node3D

const SIZE = Vector3i(16, 128, 16)

var blocks: Array[int]
var mesh_instance: MeshInstance3D
var neighbours: Array[Chunk]

const NEIGHBOUR_PX = 0
const NEIGHBOUR_NX = 1
const NEIGHBOUR_PZ = 2
const NEIGHBOUR_NZ = 3

const FACE_BRIGHTNESS = {
	Vector3(0, 1, 0):  1.0,
	Vector3(0, -1, 0): 0.6,
	Vector3(0, 0, 1):  0.85,
	Vector3(0, 0, -1): 0.75,
	Vector3(1, 0, 0):  0.9,
	Vector3(-1, 0, 0): 0.8,
}

func _init() -> void:
	blocks.resize(SIZE.x * SIZE.y * SIZE.z)
	blocks.fill(0)
	neighbours.resize(6)
	neighbours.fill(null)
	mesh_instance = MeshInstance3D.new()
	mesh_instance.gi_mode = GeometryInstance3D.GI_MODE_DYNAMIC
	
func prepare() -> void:
	add_child(mesh_instance)
	var mat = StandardMaterial3D.new()
	mat.vertex_color_use_as_albedo = true
	mesh_instance.material_override = mat

func get_block(x: int, y: int, z: int) -> int:
	if x >= 0 && y >= 0 && z >= 0 && x < SIZE.x && y < SIZE.y && z < SIZE.z:
		return blocks[(z * SIZE.x * SIZE.y) + (y * SIZE.x) + x]
	return -1

func _get_block_including_neighbours(x: int, y: int, z: int) -> int:
	if x >= 0 && y >= 0 && z >= 0 && x < SIZE.x && y < SIZE.y && z < SIZE.z:
		return get_block(x, y, z)

	if y < 0 || y >= SIZE.y:
		return 0

	if x < 0:
		if neighbours[NEIGHBOUR_NX] != null:
			return neighbours[NEIGHBOUR_NX].get_block(x + SIZE.x, y, z)
		return 0
	if x >= SIZE.x:
		if neighbours[NEIGHBOUR_PX] != null:
			return neighbours[NEIGHBOUR_PX].get_block(x - SIZE.x, y, z)
		return 0

	if z < 0:
		if neighbours[NEIGHBOUR_NZ] != null:
			return neighbours[NEIGHBOUR_NZ].get_block(x, y, z + SIZE.z)
		return 0
	if z >= SIZE.z:
		if neighbours[NEIGHBOUR_PZ] != null:
			return neighbours[NEIGHBOUR_PZ].get_block(x, y, z - SIZE.z)
		return 0
	return 0

func set_block(x: int, y: int, z: int, id: int) -> void:
	if x >= 0 && y >= 0 && z >= 0 && x < SIZE.x && y < SIZE.y && z < SIZE.z:
		blocks[(z * SIZE.x * SIZE.y) + (y * SIZE.x) + x] = id

func fill(id: int) -> void:
	blocks.fill(id)

func fill_range(from: Vector3i, to: Vector3i, id: int) -> void:
	for y in range(from.y, to.y):
		for z in range(from.z, to.z):
			for x in range(from.x, to.x):
				set_block(x, y, z, id)

func generate_mesh() -> void:
	var vertices = PackedVector3Array()
	var indices = PackedInt32Array()
	var normals = PackedVector3Array()
	var uvs = PackedVector2Array()
	var colors = PackedColorArray()

	var faces = [
		[Vector3( 0,  0,  1), Vector3(0, 0, 1), Vector3(0, 1, 1), Vector3(1, 1, 1), Vector3(1, 0, 1)],
		[Vector3( 0,  0, -1), Vector3(1, 0, 0), Vector3(1, 1, 0), Vector3(0, 1, 0), Vector3(0, 0, 0)],
		[Vector3(-1,  0,  0), Vector3(0, 0, 0), Vector3(0, 1, 0), Vector3(0, 1, 1), Vector3(0, 0, 1)],
		[Vector3( 1,  0,  0), Vector3(1, 0, 1), Vector3(1, 1, 1), Vector3(1, 1, 0), Vector3(1, 0, 0)],
		[Vector3( 0,  1,  0), Vector3(0, 1, 1), Vector3(0, 1, 0), Vector3(1, 1, 0), Vector3(1, 1, 1)],
		[Vector3( 0, -1,  0), Vector3(0, 0, 0), Vector3(0, 0, 1), Vector3(1, 0, 1), Vector3(1, 0, 0)],
	]

	var face_uvs = PackedVector2Array([Vector2(0,0), Vector2(1,0), Vector2(1,1), Vector2(0,1)])

	for y in SIZE.y:
		for z in SIZE.z:
			for x in SIZE.x:
				var id = get_block(x, y, z)
				# Don't render air
				if id <= 0:
					continue

				var block_color = BlockRegistry.get_block_by_id(id).color
				var offset = Vector3(x, y, z)

				for face in faces:
					var normal: Vector3 = face[0]
					
					var nx = x + int(normal.x)
					var ny = y + int(normal.y)
					var nz = z + int(normal.z)
					if _get_block_including_neighbours(nx, ny, nz) > 0:
						continue
						
					var brightness = FACE_BRIGHTNESS[normal]
					var shaded_color = Color(block_color.r * brightness, block_color.g * brightness, block_color.b * brightness)
					var base = vertices.size()
					for i in 4:
						vertices.append(offset + face[i + 1])
						normals.append(normal)
						colors.append(shaded_color)
					uvs.append_array(face_uvs)
					indices.append_array([base, base+1, base+2, base, base+2, base+3])

	var arrays = []
	arrays.resize(Mesh.ARRAY_MAX)
	arrays[Mesh.ARRAY_VERTEX] = vertices
	arrays[Mesh.ARRAY_INDEX]  = indices
	arrays[Mesh.ARRAY_NORMAL] = normals
	arrays[Mesh.ARRAY_TEX_UV] = uvs
	arrays[Mesh.ARRAY_COLOR]  = colors

	var arr_mesh = ArrayMesh.new()
	if vertices.size() > 0:
		arr_mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, arrays)
	mesh_instance.mesh = arr_mesh
	
	for child in get_children():
		if child is StaticBody3D:
			child.queue_free()
	
	if vertices.size() > 0:
		var static_body = StaticBody3D.new()
		add_child(static_body)
		var collision = CollisionShape3D.new()
		collision.shape = arr_mesh.create_trimesh_shape()
		static_body.add_child(collision)
