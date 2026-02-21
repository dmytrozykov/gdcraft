extends CharacterBody3D

@export_category("Movement")
@export var speed: float = 6.0
@export var sprint_factor: float = 1.5
@export var jump_velocity: float = 5.0
@export var gravity: float = 10.0

@export_category("Camera")
@export var sensitivity: float = 0.003

@export_category("Interaction")
@export var reach: float = 5.0

@export_category("References")
@export var world: World

@onready var _head: Node3D = $Head
@onready var _camera: Camera3D = $Head/Camera3D

var _pitch: float = 0.0

func _ready() -> void:
	Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED)

func _input(event: InputEvent) -> void:
	if event is InputEventMouseMotion and Input.mouse_mode == Input.MOUSE_MODE_CAPTURED:
		_head.rotate_y(-event.relative.x * sensitivity)
		_pitch = clamp(_pitch - event.relative.y * sensitivity, -PI / 2, PI / 2)
		_camera.rotation = Vector3(_pitch, 0, 0)
	if event.is_action_pressed("ui_cancel"):
		if Input.mouse_mode == Input.MOUSE_MODE_VISIBLE:
			Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED)
		else:
			Input.set_mouse_mode(Input.MOUSE_MODE_VISIBLE)
	if Input.mouse_mode == Input.MOUSE_MODE_CAPTURED:
		if event.is_action_pressed("break_block"):
			_raycast(false)
		if event.is_action_pressed("place_block"):
			_raycast(true)

func _overlaps_player(block_pos) -> bool:
	var p = global_position
	return (
		block_pos.x     < p.x + 0.4 and block_pos.x + 1 > p.x - 0.4 and
		block_pos.y     < p.y + 2.0 and block_pos.y + 1 > p.y and
		block_pos.z     < p.z + 0.4 and block_pos.z + 1 > p.z - 0.4
	)

func _raycast(place: bool) -> void:
	var from = _camera.global_position
	var dir = -_camera.global_basis.z
	var hit = world.raycast(from, dir, reach)
	if hit != null:
		if place:
			var p = hit.block_pos + hit.normal
			if not _overlaps_player(p):
				world.set_block_at(Vector3(p.x, p.y, p.z), 1)
		else:
			var p = hit.block_pos
			world.set_block_at(Vector3(p.x, p.y, p.z), 0)

func _physics_process(delta: float) -> void:
	if not is_on_floor():
		velocity.y -= gravity * delta
	else:
		if velocity.y < 0:
			velocity.y = 0

	if Input.is_action_pressed("jump") and is_on_floor():
		velocity.y = jump_velocity

	var horizontal_input = Input.get_vector("move_left", "move_right", "move_forward", "move_backward")
	var direction = (_head.global_transform.basis * Vector3(horizontal_input.x, 0, horizontal_input.y)).normalized()
	var total_speed = speed
	if Input.is_action_pressed("sprint"):
		total_speed *= sprint_factor

	velocity.x = direction.x * total_speed
	velocity.z = direction.z * total_speed

	move_and_slide()
