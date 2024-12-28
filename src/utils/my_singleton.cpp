#include "my_singleton.hpp"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

MySingleton *MySingleton::singleton = nullptr;

void MySingleton::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("hello_singleton", "label"), &MySingleton::hello_singleton);
}

MySingleton *MySingleton::get_singleton()
{
	return singleton;
}

MySingleton::MySingleton()
{
	ERR_FAIL_COND(singleton != nullptr);
	singleton = this;
}

MySingleton::~MySingleton()
{
	ERR_FAIL_COND(singleton != this);
	singleton = nullptr;
}

void MySingleton::hello_singleton(godot::Label *label)
{
	label->set_text(label->get_text() + "Hello GDExtension Singleton!\n");
}
