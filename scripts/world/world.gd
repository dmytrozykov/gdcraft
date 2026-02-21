## Manages all loaded [Chunk]s, routes world-space queries to the correct chunk,
## and handles neighbour mesh invalidation after block edits.
class_name World
extends Node3D


@export_category("Chunk Parameters")
@export var chunk_size: Vector3i = Vector3i(16, 128, 16)

var _chunks: Dictionary[Vector2i, Chunk] = {}


## Adds [param chunk] to the scene, positions it at [param chunk_coord], and wires
## up bidirectional neighbour references so mesh generation can cull cross-chunk faces.
func register_chunk(chunk: Chunk, chunk_coord: Vector2i) -> void:
	_chunks[chunk_coord] = chunk
	add_child(chunk)
	chunk.position = Vector3(chunk_coord.x * chunk_size.x, 0, chunk_coord.y * chunk_size.z)

	var px := get_chunk(chunk_coord + Vector2i(1, 0))
	var nx := get_chunk(chunk_coord + Vector2i(-1, 0))
	var pz := get_chunk(chunk_coord + Vector2i(0, 1))
	var nz := get_chunk(chunk_coord + Vector2i(0, -1))
	chunk.neighbours[Chunk.NEIGHBOUR_PX] = px
	chunk.neighbours[Chunk.NEIGHBOUR_NX] = nx
	chunk.neighbours[Chunk.NEIGHBOUR_PZ] = pz
	chunk.neighbours[Chunk.NEIGHBOUR_NZ] = nz

	if px: px.neighbours[Chunk.NEIGHBOUR_NX] = chunk
	if nx: nx.neighbours[Chunk.NEIGHBOUR_PX] = chunk
	if pz: pz.neighbours[Chunk.NEIGHBOUR_NZ] = chunk
	if nz: nz.neighbours[Chunk.NEIGHBOUR_PZ] = chunk


## Returns the [Chunk] at grid coordinate [param chunk_coord], or [code]null[/code] if not loaded.
func get_chunk(chunk_coord: Vector2i) -> Chunk:
	return _chunks.get(chunk_coord, null)


## Returns the [Chunk] that contains [param world_pos], or [code]null[/code] if not loaded.
func get_chunk_at(world_pos: Vector3) -> Chunk:
	return get_chunk(world_pos_to_chunk_coord(world_pos))


## Returns the block ID at [param world_pos], or [code]-1[/code] if the chunk is not loaded.
func get_block_at(world_pos: Vector3) -> int:
	var chunk := get_chunk_at(world_pos)
	if chunk == null:
		return -1
	var local := world_to_local(world_pos)
	return chunk.get_block(local.x, local.y, local.z)


## Sets the block at [param world_pos] to [param id] and regenerates affected chunk meshes.
## Also regenerates the mesh of any neighbour chunk whose border is adjacent to the edit.
func set_block_at(world_pos: Vector3, id: int) -> void:
	var chunk := get_chunk_at(world_pos)
	if chunk == null:
		return
	var local := world_to_local(world_pos)
	chunk.set_block(local.x, local.y, local.z, id)
	chunk.generate_mesh()

	var chunk_coord := world_pos_to_chunk_coord(world_pos)
	if local.x == 0:
		var neighbour := get_chunk(chunk_coord + Vector2i(-1, 0))
		if neighbour: neighbour.generate_mesh()
	if local.x == chunk_size.x - 1:
		var neighbour := get_chunk(chunk_coord + Vector2i(1, 0))
		if neighbour: neighbour.generate_mesh()
	if local.z == 0:
		var neighbour := get_chunk(chunk_coord + Vector2i(0, -1))
		if neighbour: neighbour.generate_mesh()
	if local.z == chunk_size.z - 1:
		var neighbour := get_chunk(chunk_coord + Vector2i(0, 1))
		if neighbour: neighbour.generate_mesh()


## Converts a world-space position to a chunk grid coordinate.
func world_pos_to_chunk_coord(world_pos: Vector3) -> Vector2i:
	return Vector2i(floori(world_pos.x / chunk_size.x), floori(world_pos.z / chunk_size.z))


## Converts a world-space position to a local block coordinate within its chunk.
func world_to_local(world_pos: Vector3) -> Vector3i:
	var local_x := floori(world_pos.x) % chunk_size.x
	var local_y := floori(world_pos.y)
	var local_z := floori(world_pos.z) % chunk_size.z
	if local_x < 0: local_x += chunk_size.x
	if local_z < 0: local_z += chunk_size.z
	return Vector3i(local_x, local_y, local_z)


## Casts a voxel ray from [param from] in direction [param dir] up to [param max_dist] units.
## Returns a [HitPayload] on the first solid block hit, or [code]null[/code] if nothing is hit.
func raycast(from: Vector3, dir: Vector3, max_dist: float) -> HitPayload:
	var dx := dir.x
	var dy := dir.y
	var dz := dir.z

	var ix := floori(from.x)
	var iy := floori(from.y)
	var iz := floori(from.z)

	var step_x := 1 if dx >= 0.0 else -1
	var step_y := 1 if dy >= 0.0 else -1
	var step_z := 1 if dz >= 0.0 else -1

	const INF := 1e30

	var tx_delta: float = INF if dx == 0.0 else abs(1.0 / dx)
	var ty_delta: float = INF if dy == 0.0 else abs(1.0 / dy)
	var tz_delta: float = INF if dz == 0.0 else abs(1.0 / dz)

	var tx_max: float = tx_delta * (1.0 - fmod(from.x - ix, 1.0)) if step_x > 0 else tx_delta * fmod(from.x - ix, 1.0)
	var ty_max: float = ty_delta * (1.0 - fmod(from.y - iy, 1.0)) if step_y > 0 else ty_delta * fmod(from.y - iy, 1.0)
	var tz_max: float = tz_delta * (1.0 - fmod(from.z - iz, 1.0)) if step_z > 0 else tz_delta * fmod(from.z - iz, 1.0)

	var stepped_index := -1
	var t := 0.0
	while t <= max_dist:
		var block := get_block_at(Vector3(ix, iy, iz))
		if block > 0 and stepped_index != -1:
			var norm := Vector3i.ZERO
			if stepped_index == 0: norm.x = -step_x
			elif stepped_index == 1: norm.y = -step_y
			elif stepped_index == 2: norm.z = -step_z
			return HitPayload.new(Vector3i(ix, iy, iz), norm, block)
		if tx_max < ty_max:
			if tx_max < tz_max:
				ix += step_x; t = tx_max; tx_max += tx_delta; stepped_index = 0
			else:
				iz += step_z; t = tz_max; tz_max += tz_delta; stepped_index = 2
		else:
			if ty_max < tz_max:
				iy += step_y; t = ty_max; ty_max += ty_delta; stepped_index = 1
			else:
				iz += step_z; t = tz_max; tz_max += tz_delta; stepped_index = 2
	return null
