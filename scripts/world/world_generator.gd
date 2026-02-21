## Procedurally populates a [World] with [Chunk]s on [method _ready].
## Replace or extend [method generate] to implement custom terrain algorithms.
class_name WorldGenerator
extends Node


@export var world: World


func _ready() -> void:
	generate()


## Fills a 4×4 grid of chunks with a simple flat terrain.
func generate() -> void:
	if world == null:
		push_warning("WorldGenerator: world node is not set.")
		return

	for z in range(4):
		for x in range(4):
			var chunk := Chunk.new()
			chunk.fill_range(Vector3i.ZERO,     Vector3i(16, 2, 16), BlockRegistry.get_block_by_name("stone").id)
			chunk.fill_range(Vector3i(0, 2, 0), Vector3i(16, 4, 16), BlockRegistry.get_block_by_name("dirt").id)
			chunk.fill_range(Vector3i(0, 4, 0), Vector3i(16, 5, 16), BlockRegistry.get_block_by_name("grass").id)
			chunk.set_block(8, 6, 8, BlockRegistry.get_block_by_name("stone").id)
			world.register_chunk(chunk, Vector2i(x, z))
			chunk.prepare()
			chunk.generate_mesh()
