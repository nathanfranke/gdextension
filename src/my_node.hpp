#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/audio_stream_player.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <chuck.h>

using namespace godot;

class MyNode : public Node
{
	GDCLASS(MyNode, Node);

private:
	// our ChucK instance
	ChucK * the_chuck = NULL;
	// audio input buffer
	SAMPLE * g_inputBuffer = NULL;
	// audio output buffer
	SAMPLE * g_outputBuffer = NULL;
	// our audio buffer size
	t_CKINT g_bufferSize = 0;


	// Reference to godot audio stream player
	AudioStreamPlayer* audio_stream_player = nullptr;

	// shred id
    t_CKUINT shredID = 0;
    // our shred ID stack
    vector<t_CKUINT> shredIDs;

	// allocate global audio buffers
	void alloc_global_buffers( t_CKINT bufferSize );
	// de-allocate global audio buffers
	void cleanup_global_buffers();

protected:
	static void _bind_methods();

public:
	MyNode();
	~MyNode();

	void _ready() override;
	void _process(double delta) override;

	godot::String hello_node();

	void add_shred(godot::String content);
	void remove_last_shred();
	void remove_shred(int _shredID);
	void remove_all_shreds();
	godot::PackedInt32Array get_shred_ids();

	// Setters and Getters
	void set_audio_stream_player(AudioStreamPlayer* p_audio_stream_player) { audio_stream_player = p_audio_stream_player; };
	AudioStreamPlayer* get_audio_stream_player() const { return audio_stream_player; };
};
