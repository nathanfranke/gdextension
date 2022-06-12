#include "example.hpp"

#include <godot_cpp/core/class_db.hpp>

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void Example::_bind_methods()
{
	// TODO: Static https://github.com/godotengine/godot/issues/61963
	ClassDB::bind_method(D_METHOD("hello_extension"), &Example::hello_extension);
}

Example::Example()
{
}

Example::~Example()
{
}

void Example::hello_extension()
{
	UtilityFunctions::print("Hello GDExtension!");
}
