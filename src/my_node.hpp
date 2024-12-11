#pragma once

#include <godot_cpp/classes/node.hpp>

using namespace godot;

class MyNode : public Node
{
	GDCLASS(MyNode, Node);

protected:
	static void _bind_methods();

public:
	MyNode();
	~MyNode();

	void _ready() override;
	void _process(double delta) override;

	godot::String hello_node();
};
