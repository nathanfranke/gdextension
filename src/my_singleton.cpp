#include "my_singleton.hpp"

#include <godot_cpp/core/class_db.hpp>

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void MySingleton::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("hello_singleton"), &MySingleton::hello_singleton);
}

MySingleton::MySingleton()
{
}

MySingleton::~MySingleton()
{
}

void MySingleton::hello_singleton()
{
	UtilityFunctions::print("Hello Singleton!");
}
