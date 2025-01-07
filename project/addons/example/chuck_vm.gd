class_name ChuckVM extends MyNode

@export var chuck_file: ChuckFile

func _ready() -> void:
	var chuck_path: String = ProjectSettings.globalize_path(chuck_file.resource_path)
	add_shred(chuck_path)
	await get_tree().create_timer(2).timeout
	var ids = get_shred_ids()
	remove_all_shreds()
