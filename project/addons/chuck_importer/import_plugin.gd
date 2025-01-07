@tool
extends EditorImportPlugin

func _get_importer_name() -> String:
	return "chodot.chuckfile"

func _get_visible_name() -> String:
	return "Chuck File"

func _get_recognized_extensions() -> PackedStringArray:
	return ["ck"]

func _get_save_extension() -> String:
	return "tres"

func _get_resource_type() -> String:
	return "ChuckFile"

func _get_preset_count():
	return 1

func _get_preset_name(preset_index):
	return "Default"

func _get_import_options(path, preset_index):
	return []

func _get_option_visibility(path, option_name, options):
	return true

func _get_priority() -> float:
	return 1

func _get_import_order() -> int:
	return 0

func _import(source_file, save_path, options, r_platform_variants, r_gen_files):
	var file = FileAccess.open(source_file, FileAccess.READ)
	if file == null:
		return FileAccess.get_open_error()
	
	var contents: String = file.get_as_text()
	var chuck_file := ChuckFile.new()
	chuck_file.contents = contents
	
	return ResourceSaver.save(chuck_file, "%s.%s" % [save_path, _get_save_extension()])
