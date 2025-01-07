extends Node

@onready var label: Label = $Label
@onready var audio_stream_player = $AudioStreamPlayer

func _ready() -> void:
	label.text += "Hello GDScript!\n"
	label.text += $MyNode.hello_node()
	MySingleton.hello_singleton(label)
	audio_stream_player.play()
