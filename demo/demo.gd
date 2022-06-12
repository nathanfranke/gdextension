extends Node

func _ready() -> void:
	print("Hello GDScript!")
	# TODO: Static https://github.com/godotengine/godot/issues/61963
	Example.new().hello_extension()
