#include "my_node.hpp"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <chuck.h>
#include <iostream>
#include <streambuf>
#include <string>

using namespace godot;

// Custom stream buffer to capture stderr
class StderrRedirector : public std::streambuf {
public:
    StderrRedirector(std::streambuf *buf) : original_buf(buf) {}
    std::streambuf *original_buf;
    std::string buffer;

protected:
    virtual int overflow(int c) override {
        if (c == '\n') {
            UtilityFunctions::print(buffer.c_str());
            buffer.clear();
        } else {
            buffer += static_cast<char>(c);
        }
        return c;
    }
};

StderrRedirector *stderr_redirector = nullptr;
std::streambuf *original_stderr_buf = nullptr;


void MyNode::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("get_audio_stream_player"), &MyNode::get_audio_stream_player);
	ClassDB::bind_method(D_METHOD("set_audio_stream_player", "p_audio_stream_player"), &MyNode::set_audio_stream_player);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "AudioStreamPlayer", PROPERTY_HINT_NODE_TYPE, "AudioStreamPlayer"), "set_audio_stream_player", "get_audio_stream_player");


	ClassDB::bind_method(D_METHOD("hello_node"), &MyNode::hello_node);
};

MyNode::MyNode()
{
    // Redirect stderr
    original_stderr_buf = std::cerr.rdbuf();
    stderr_redirector = new StderrRedirector(original_stderr_buf);
    std::cerr.rdbuf(stderr_redirector);

	UtilityFunctions::print("Initiating Chuck...");

	// instantiate a ChucK instance
	// Uncommenting this next line is what seems to break the SharedLib
    the_chuck = new ChucK();

        // set some parameters: sample rate
    the_chuck->setParam( CHUCK_PARAM_SAMPLE_RATE, 44100 );
    // number of input channels
    the_chuck->setParam( CHUCK_PARAM_INPUT_CHANNELS, 2 );
    // number of output channels
    the_chuck->setParam( CHUCK_PARAM_OUTPUT_CHANNELS, 2 );
    // whether to halt the VM when there is no more shred running
    the_chuck->setParam( CHUCK_PARAM_VM_HALT, TRUE );
    // set hint so internally can advise things like async data writes etc.
    the_chuck->setParam( CHUCK_PARAM_IS_REALTIME_AUDIO_HINT, FALSE );
    // turn on logging to see what ChucK is up to; higher == more info
    // chuck->setLogLevel( 3 );

    // allocate audio buffers, also after parameters are set
    alloc_global_buffers( 1024 );

    // initialize ChucK, after the parameters are set
    the_chuck->init();

	UtilityFunctions::print("Chuck Initiated.");
}

MyNode::~MyNode()
{
    // deallocate
    cleanup_global_buffers();
    
    // clean up ChucK
    CK_SAFE_DELETE( the_chuck );

    // Restore original stderr
    std::cerr.rdbuf(original_stderr_buf);
    CK_SAFE_DELETE( stderr_redirector );
}

void MyNode::_ready()
{
    // start ChucK VM and synthesis engine
    the_chuck->start();

    // compile some ChucK code from string (this can be called on-the-fly and repeatedly)
    if( !the_chuck->compileCode( "repeat(10) { <<< \"hello ChucK! random integer:\", Math.random2(1,100) >>>; }", "", 1 ) )
    {
        // got error, baillng out...
        exit( 1 );
    }
    // keep going as long as a shred is still running
    // (as long as CHUCK_PARAM_VM_HALT is set to TRUE)
    while( the_chuck->vm_running() )
    {
        // run ChucK for the next `bufferSize`
        // pre-condition: `input` constains any audio input
        // post-condition: `output` constains any synthesized audio
        the_chuck->run( g_inputBuffer, g_outputBuffer, g_bufferSize );
    }
}

void MyNode::_process(double delta)
{

    // Lines below are from godot-pure-data repo, and should give a hint how to get this to work with AudioStreamPlayer
    // if(is_playing()) 
	// {
	// 	Ref<AudioStreamGeneratorPlayback> p = get_stream_playback();
	// 	if(p.is_valid()) {	
	// 		int nframes = std::min(p->get_frames_available(), 2048);

	// 		int ticks = nframes / libpd_blocksize();


	// 		if(::libpd_process_float(ticks, inbuf_, outbuf_) != 0)
	// 		{
	// 			return;
	// 		}

	// 		for(int i = 0; i < nframes; i++)
	// 		{
	// 			auto v = Vector2(outbuf_[i*2], outbuf_[(i*2)+1]);
	// 			v = v.clamp(Vector2(-1, -1), Vector2(1, 1));

	// 			p->push_frame(v);
	// 		}
	// 	}
	// }
}

godot::String MyNode::hello_node()
{
	UtilityFunctions::print("hello_node() function called.");
	return "Hello GDExtension Node\n";
}


//-----------------------------------------------------------------------------
// initialize global audio buffers
//-----------------------------------------------------------------------------
void MyNode::alloc_global_buffers( t_CKINT bufferSize )
{
    // good practice to clean, in case this function is called more than once
    cleanup_global_buffers();

    // allocate audio buffers as expected by ChucK's run() function below
    g_inputBuffer = new SAMPLE[bufferSize * the_chuck->getParamInt(CHUCK_PARAM_INPUT_CHANNELS)];
    g_outputBuffer = new SAMPLE[bufferSize * the_chuck->getParamInt(CHUCK_PARAM_OUTPUT_CHANNELS)];
    // zero out
    memset( g_inputBuffer, 0, bufferSize * the_chuck->getParamInt(CHUCK_PARAM_INPUT_CHANNELS) );
    memset( g_outputBuffer, 0, bufferSize * the_chuck->getParamInt(CHUCK_PARAM_OUTPUT_CHANNELS) );
    // set the buffer size
    g_bufferSize = bufferSize;
}


//-----------------------------------------------------------------------------
// cleanup global audio buffers
//-----------------------------------------------------------------------------
void MyNode::cleanup_global_buffers()
{
    // reclaim memory
    CK_SAFE_DELETE_ARRAY( g_inputBuffer );
    CK_SAFE_DELETE_ARRAY( g_outputBuffer );
    // reset
    g_bufferSize = 0;
}

// void MyNode::set_audio_stream_player(AudioStreamPlayer* p_audio_stream_player)
// {
//     audio_stream_player = p_audio_stream_player;
// }

// AudioStreamPlayer* MyNode::get_audio_stream_player()
// {
//     return audio_stream_player;
// }
