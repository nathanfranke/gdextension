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
	ClassDB::bind_method(D_METHOD("hello_node"), &MyNode::hello_node);
}

MyNode::MyNode()
{
	UtilityFunctions::print("Initiating Chuck...");

	// instantiate a ChucK instance
	// Uncommenting this next line is what seems to break the SharedLib
    // the_chuck = new ChucK();

	UtilityFunctions::print("Chuck Initiated.");

	// Redirect stderr
    original_stderr_buf = std::cerr.rdbuf();
    stderr_redirector = new StderrRedirector(original_stderr_buf);
    std::cerr.rdbuf(stderr_redirector);
}

MyNode::~MyNode()
{
    // Restore original stderr
    std::cerr.rdbuf(original_stderr_buf);
    CK_SAFE_DELETE( stderr_redirector );

	CK_SAFE_DELETE( the_chuck );
}

// Override built-in methods with your own logic. Make sure to declare them in the header as well!

void MyNode::_ready()
{

}

void MyNode::_process(double delta)
{
}

godot::String MyNode::hello_node()
{
	UtilityFunctions::print("hello_node() function called.");
	return "Hello GDExtension Node\n";
}