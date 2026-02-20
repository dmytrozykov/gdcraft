extends CanvasLayer

@onready var fps_label: Label = $DebugInfo/FPSLabel

@export var show_debug_info: bool = true

func _ready() -> void:
	if show_debug_info:
		fps_label.visible = true

func _process(_delta: float) -> void:
	if show_debug_info:
		fps_label.text = "FPS: %d" % Engine.get_frames_per_second()
