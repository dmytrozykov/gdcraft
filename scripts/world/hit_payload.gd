class_name HitPayload

var block_pos: Vector3i
var normal: Vector3i
var block_id: int

func _init(block_pos_: Vector3i, normal_: Vector3i, block_id_: int) -> void:
	self.block_pos = block_pos_
	self.normal = normal_
	self.block_id = block_id_
