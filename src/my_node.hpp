#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/viewport.hpp>

#include <godot_cpp/core/binder_common.hpp>

using namespace godot;

class MyNode : public Node
{
	GDCLASS(MyNode, Node);

protected:
	static void _bind_methods();

public:
	MyNode();
	~MyNode();

	void hello_node();
};
