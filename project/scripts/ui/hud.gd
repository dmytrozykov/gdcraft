## Heads-up display: crosshair and optional debug overlay.
extends CanvasLayer

@export var show_debug_info: bool = true

@onready var fps_label: Label = $DebugInfo/FPSLabel

func _ready() -> void:
	fps_label.visible = show_debug_info

func _process(_delta: float) -> void:
	if show_debug_info:
		fps_label.text = "FPS: %d" % Engine.get_frames_per_second()
