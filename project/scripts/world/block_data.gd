## Data resource describing a single block type (name, color, assigned ID).
## Instances are created as [code].tres[/code] files and loaded via [BlockSet].
class_name BlockData
extends Resource

@export var name: String
@export var color: Color

## Numeric ID assigned at runtime by [BlockRegistry]. 0 is reserved for air.
var id: int
