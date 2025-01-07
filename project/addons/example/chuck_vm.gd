class_name ChuckVM extends MyNode

@export var chuck_file: ChuckFile

func _ready() -> void:
	var chuck_path: String = ProjectSettings.globalize_path(chuck_file.resource_path)
	add_shred(chuck_path)
