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

std::unordered_map<std::string, MyNode*> MyNode::instance_map;

// Custom stream buffer to capture stderr and stdout
class StreamRedirector : public std::streambuf {
public:
    StreamRedirector(std::streambuf *buf) : original_buf(buf) {}
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

StreamRedirector *stderr_redirector = nullptr;
StreamRedirector *stdout_redirector = nullptr;
std::streambuf *original_stderr_buf = nullptr;
std::streambuf *original_stdout_buf = nullptr;


void MyNode::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("get_audio_stream_player"), &MyNode::get_audio_stream_player);
	ClassDB::bind_method(D_METHOD("set_audio_stream_player", "p_audio_stream_player"), &MyNode::set_audio_stream_player);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "AudioStreamPlayer", PROPERTY_HINT_NODE_TYPE, "AudioStreamPlayer"), "set_audio_stream_player", "get_audio_stream_player");

	ClassDB::bind_method(D_METHOD("hello_node"), &MyNode::hello_node);

    ClassDB::bind_method(D_METHOD("get_shred_ids"), &MyNode::get_shred_ids);
    ClassDB::bind_method(D_METHOD("run_code", "code"), &MyNode::run_code);
    ClassDB::bind_method(D_METHOD("add_shred", "filename"), &MyNode::add_shred);
    ClassDB::bind_method(D_METHOD("remove_last_shred"), &MyNode::remove_last_shred);
    ClassDB::bind_method(D_METHOD("remove_shred", "shredID"), &MyNode::remove_shred);
    ClassDB::bind_method(D_METHOD("remove_all_shreds"), &MyNode::remove_all_shreds);

    ClassDB::bind_method(D_METHOD("broadcast_global_event", "name"), &MyNode::broadcast_global_event);
    ClassDB::bind_method(D_METHOD("set_global_float", "name", "value"), &MyNode::set_global_float);
    ClassDB::bind_method(D_METHOD("set_global_int", "name", "value"), &MyNode::set_global_int);

    ADD_SIGNAL(MethodInfo("chuck_event", PropertyInfo(Variant::STRING, "event")));
};

MyNode::MyNode()
{
    // Redirect stderr
    std::cerr.rdbuf(stderr_redirector);
    original_stdout_buf = std::cout.rdbuf();
    stdout_redirector = new StreamRedirector(original_stdout_buf);
    std::cout.rdbuf(stdout_redirector);
    stderr_redirector = new StreamRedirector(original_stderr_buf);
    std::cerr.rdbuf(stderr_redirector);

	UtilityFunctions::print("Initiating Chuck...");

	// instantiate a ChucK instance
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

    // start ChucK VM and synthesis engine
    the_chuck->start();

	UtilityFunctions::print("Chuck Initiated.");
}

MyNode::~MyNode()
{
    // deallocate
    cleanup_global_buffers();

    // Clean up the instance_map
    for (auto it = instance_map.begin(); it != instance_map.end(); ) {
        if (it->second == this) {
            it = instance_map.erase(it);
        } else {
            ++it;
        }
    }
    
    // clean up ChucK
    CK_SAFE_DELETE( the_chuck );

    // Restore original stderr
    std::cerr.rdbuf(original_stderr_buf);
    CK_SAFE_DELETE( stderr_redirector );
}

void MyNode::_ready()
{
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

//-----------------------------------------------------------------------------
// Shred Manipulation
//-----------------------------------------------------------------------------

godot::PackedInt32Array MyNode::get_shred_ids()
{
    PackedInt32Array packed_shred_ids;

    cerr << "shredIDs: ";
    for( t_CKUINT i = 0; i < shredIDs.size(); i++ )
    {
        cerr << shredIDs[i] << " ";
        packed_shred_ids.append(shredIDs[i]);
    }
    cerr << endl;

    return packed_shred_ids;
}

// TODO: Printout from <<< >>> is not captured by stderr redirector
void MyNode::run_code(godot::String code)
{
    string content = code.utf8().get_data();
    if (!the_chuck->compileCode( content, "", 1, FALSE)) {
        // got error, baillng out...
        cerr << "cannot not compile code" << endl;
        exit( 1 );
    };
    shredID = the_chuck->vm()->last_id();
    cerr << "adding shred of compiled code with ID: " << shredID  << endl;
    shredIDs.push_back( shredID );

    // Register global events
    if (the_chuck && the_chuck->vm_running()) {
        register_global_events();
    } else {
        UtilityFunctions::print("ChucK VM is not running, cannot register global events.");
    }
}

void MyNode::add_shred(godot::String filename)
{
    string file = filename.utf8().get_data();
    // compile file; FALSE means deferred spork -- thread-safe since
    // we are on a different thread as the audio thread that is calling
    // the_chuck->run(...)
    if(!the_chuck->compileFile( file, "", 1, FALSE )) {
        // got error, baillng out...
        cerr << "cannot not compile file" << endl;
        exit( 1 );
    };
    // get the id of the previously sporked shred
    shredID = the_chuck->vm()->last_id();
    // print out the last VM shred id (should be the shred we just compiled)
    cerr << "adding shred '" << file << "' with ID: " << shredID  << endl;
    // push on stack
    shredIDs.push_back( shredID );

    // Register global events
    if (the_chuck && the_chuck->vm_running()) {
        register_global_events();
    } else {
        UtilityFunctions::print("ChucK VM is not running, cannot register global events.");
    }
}

void MyNode::remove_last_shred()
{
    // if no shreds to remove
    if( !shredIDs.size() )
    {
        cerr << "no shred to remove!" << endl;
        return;
    }
    // create a message; VM will delete
    Chuck_Msg * msg = new Chuck_Msg;
    // message type
    msg->type = CK_MSG_REMOVE;
    // signify no value (will remove last)
    msg->param = shredIDs.back();
    // queue on VM
    the_chuck->vm()->queue_msg( msg );
    // print out the last VM shred id
    cerr << "removing shred with ID: " << shredIDs.back() << endl;
    // pop
    shredIDs.pop_back();
}

void MyNode::remove_shred(int _shredID)
{
    // create a message; VM will delete
    Chuck_Msg * msg = new Chuck_Msg;
    // message type
    msg->type = CK_MSG_REMOVE;
    // which shredID to remove
    msg->param = _shredID;
    // queue on VM
    the_chuck->vm()->queue_msg( msg );
    // print out the last VM shred id
    cerr << "removing shred with ID: " << _shredID << endl;
}

void MyNode::remove_all_shreds()
{
    // create a message; VM will delete
    Chuck_Msg * msg = new Chuck_Msg;
    // message type
    msg->type = CK_MSG_REMOVEALL;
    // queue on VM
    the_chuck->vm()->queue_msg( msg );
    // clear shredIDs
    shredIDs.clear();
    // reset shredID
    shredID = 0;


    cerr << "removing all shreds" << endl;
}

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------

void MyNode::register_global_events()
{
    the_chuck->globals()->getAllGlobalVariables(all_globals_cb_static, this);
}

void MyNode::all_globals_cb_static(const vector<Chuck_Globals_TypeValue> &list, void *data)
{
    MyNode *self = static_cast<MyNode *>(data);
    self->all_globals_cb(list);
}

void MyNode::all_globals_cb( const vector<Chuck_Globals_TypeValue> & list )
{
    for( t_CKUINT i = 0; i < list.size(); i++ )
    {
        // print
        cerr << "    global variable: " << list[i].type << " " << list[i].name << endl;
        
        // check if event is already registered
        if (list[i].type == "Event" && std::find(registered_events.begin(), registered_events.end(), list[i].name) == registered_events.end())
        {
            // Register static callback with a map
            instance_map[list[i].name] = this;
            the_chuck->globals()->listenForGlobalEvent(
                list[i].name.c_str(),
                event_listener_cb_static,
                TRUE
            );
            registered_events.push_back(list[i].name);
        }
    }
}

void MyNode::event_listener_cb_static(const char* name)
{
    // Look up the instance and call the non-static method
    if (instance_map.find(name) != instance_map.end()) {
        instance_map[name]->event_listener_cb(name);
    }
}

void MyNode::event_listener_cb(const char* name)
{
    cerr << "global event: " << name << endl;
    emit_signal("chuck_event", String(name));
}

void MyNode::broadcast_global_event(String name)
{
    the_chuck->globals()->broadcastGlobalEvent( name.utf8().get_data() );
}

void MyNode::set_global_float(String name, double value)
{
    the_chuck->globals()->setGlobalFloat( name.utf8().get_data(), value );
    cerr << "setting global float: " << name.utf8().get_data() << " to " << value << endl;
}

void MyNode::set_global_int(String name, int value)
{
    the_chuck->globals()->setGlobalInt( name.utf8().get_data(), value );
    cerr << "setting global int: " << name.utf8().get_data() << " to " << value << endl;
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
