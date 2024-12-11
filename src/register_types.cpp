#include "register_types.h"

#include <gdextension_interface.h>
#include <godot_cpp/classes/engine.hpp>

#include "my_node.hpp"
#include "my_singleton.hpp"

static MySingleton *_my_singleton;

void gdextension_initialize(ModuleInitializationLevel p_level)
{
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE)
	{
		ClassDB::register_class<MyNode>();
		ClassDB::register_class<MySingleton>();

		_my_singleton = memnew(MySingleton);
		Engine::get_singleton()->register_singleton("MySingleton", MySingleton::get_singleton());
	}
}

void gdextension_terminate(ModuleInitializationLevel p_level)
{
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE)
	{
		Engine::get_singleton()->unregister_singleton("MySingleton");
		memdelete(_my_singleton);
	}
}

extern "C"
{
	GDExtensionBool GDE_EXPORT gdextension_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization)
	{
		godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

		init_obj.register_initializer(gdextension_initialize);
		init_obj.register_terminator(gdextension_terminate);
		init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

		return init_obj.init();
	}
}
