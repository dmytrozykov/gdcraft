## Builds a texture atlas from individual block face images and tracks UV coordinates.
## Currently a stub — not yet wired into the rendering pipeline.
@tool
class_name AtlasManager
extends Node

## Pixel padding between atlas entries to prevent UV bleeding.
@export var padding: int = 2

var atlas_texture: ImageTexture
## Maps block face identifiers to their UV rectangles within the atlas.
var uv_map: Dictionary = {}
