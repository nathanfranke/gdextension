#include "my_node.hpp"

#include <godot_cpp/core/class_db.hpp>

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void MyNode::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("hello_node"), &MyNode::hello_node);
}

MyNode::MyNode()
{
}

MyNode::~MyNode()
{
}

void MyNode::hello_node()
{
	UtilityFunctions::print("Hello Node!");
}
