#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/viewport.hpp>

#include <godot_cpp/core/binder_common.hpp>

using namespace godot;

class Example : public Node
{
	GDCLASS(Example, Node);

protected:
	static void _bind_methods();

public:
	Example();
	~Example();

	void hello_extension();
};
