## Autoload singleton that owns all [BlockData] instances for the active [BlockSet].
## Access via the global [code]BlockRegistry[/code] name.
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
		var block := block_set.blocks[i]
		block.id = i + 1
		_blocks[i] = block
		_blocks_by_name[block.name.to_lower()] = block

## Returns the [BlockData] for the given numeric [param id]. ID 0 is air and has no entry.
func get_block_by_id(id: int) -> BlockData:
	return _blocks[id - 1]

## Returns the [BlockData] registered under [param block_name] (case-insensitive).
func get_block_by_name(block_name: String) -> BlockData:
	return _blocks_by_name[block_name]

## Returns the total number of registered block types plus one (for air at ID 0).
func get_block_set_size() -> int:
	return _blocks.size() + 1
