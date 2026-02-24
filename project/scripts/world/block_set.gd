## Resource containing the ordered list of [BlockData] entries that make up a block palette.
## Load an instance from a [code].tres[/code] file and assign it to [BlockRegistry].
class_name BlockSet
extends Resource

@export var blocks: Array[BlockData]
