#include "my_node.hpp"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/audio_stream_generator_playback.hpp>
#include <chuck.h>
#include <chuck_vm.h>
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
    ClassDB::bind_method(D_METHOD("add_shred", "filename"), &MyNode::add_shred);
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
    the_chuck->setParam( CHUCK_PARAM_VM_HALT, FALSE );
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

    if (!the_chuck->vm_running()) {
        UtilityFunctions::print("ChucK VM not running.");
        return;
    }
}

void MyNode::_process(double delta)
{
    // check if we have an audio stream player
    if (audio_stream_player == nullptr)
    {
        return;
    }

    if (audio_stream_player->is_playing())
    {
        // Get the stream playback
        Ref<AudioStreamGeneratorPlayback> p = audio_stream_player->get_stream_playback();
        if (p.is_valid())
        {
            // Get the number of frames available
            int nframes = std::min(p->get_frames_available(), 2048);

            // Check if the ChucK VM is running
            // if (!the_chuck->vm_running()) {
            //     UtilityFunctions::print("ChucK VM not running.");
            //     return;
            // }

            // Fill buffers with audio data
            the_chuck->run(g_inputBuffer, g_outputBuffer, nframes);
            
            PackedVector2Array out_buffer_frames;
            out_buffer_frames.resize(nframes);
            for (int i = 0; i < nframes; i++) {
                out_buffer_frames[i] = Vector2(g_outputBuffer[i * 2], g_outputBuffer[i * 2 + 1]);
            }

            // Push the audio data to the stream
            p->push_buffer(out_buffer_frames);

        }
    }
}

godot::String MyNode::hello_node()
{
	return "Hello GDExtension Node\n";
}

void MyNode::add_shred(godot::String filename)
{
    string file = filename.utf8().get_data();
    // compile file; FALSE means deferred spork -- thread-safe since
    // we are on a different thread as the audio thread that is calling
    // the_chuck->run(...)
    the_chuck->compileFile( file, "", 1, FALSE );
    // get the id of the previously sporked shred
    shredID = the_chuck->vm()->last_id();
    // print out the last VM shred id (should be the shred we just compiled)
    cerr << "adding shred '" << file << "' with ID: " << shredID  << endl;
    // push on stack
    shredIDs.push_back( shredID );
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
