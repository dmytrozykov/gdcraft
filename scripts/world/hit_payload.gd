## Result of a voxel raycast — the hit block's position, face normal, and block ID.
class_name HitPayload

## Grid position of the block that was hit.
var block_pos: Vector3i
## Outward face normal of the hit face (one of the six cardinal directions).
var normal: Vector3i
## Block ID of the hit block.
var block_id: int

func _init(block_pos_: Vector3i, normal_: Vector3i, block_id_: int) -> void:
	self.block_pos = block_pos_
	self.normal = normal_
	self.block_id = block_id_
