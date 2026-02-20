@tool
extends Node

@export var block_set: BlockSet

var _blocks: Array[BlockData] = []
var _blocks_by_name: Dictionary[String, BlockData] = {}

func _enter_tree() -> void:
	_reload()

func _reload() -> void:
	_blocks.resize(block_set.blocks.size())
	
	for i in range(block_set.blocks.size()):
		var block = block_set.blocks[i]
		block.id = i + 1
		
		_blocks[i] = block
		_blocks_by_name[block.name.to_lower()] = block

func get_block_by_id(id: int) -> BlockData:
	# 0 reserved for air
	return _blocks[id - 1]

func get_block_by_name(block_name: String) -> BlockData:
	return _blocks_by_name[block_name]

func get_block_set_size() -> int:
	return _blocks.size() + 1
