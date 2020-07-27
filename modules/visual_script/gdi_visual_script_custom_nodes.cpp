#include "gdi_visual_script_custom_nodes.h"

#include "core/engine.h"
#include "core/io/resource_loader.h"
#include "core/os/os.h"
#include "scene/main/node.h"
#include "scene/main/scene_tree.h"
#include "visual_script_nodes.h"

#include "core/os/input.h"
#include "core/bind/core_bind.h"

// 修改点：
#include "scene/3d/mesh_instance.h"


int GDIVisualScriptCustomNode::get_output_sequence_port_count() const {

	return 1;
}

bool GDIVisualScriptCustomNode::has_input_sequence_port() const {

	switch (custom_mode)
	{
	case GDIVisualScriptCustomNode::ACTIVE:
	case GDIVisualScriptCustomNode::LOOP:
		return false;
	case GDIVisualScriptCustomNode::KEYBOARD:
	case GDIVisualScriptCustomNode::MOUSE:
		return true;
	default:
		return false;
	}
}
#ifdef TOOLS_ENABLED

static Node *_find_script_node(Node *p_edited_scene, Node *p_current_node, const Ref<Script> &script) {

	if (p_edited_scene != p_current_node && p_current_node->get_owner() != p_edited_scene)
		return NULL;

	Ref<Script> scr = p_current_node->get_script();

	if (scr.is_valid() && scr == script)
		return p_current_node;

	for (int i = 0; i < p_current_node->get_child_count(); i++) {
		Node *n = _find_script_node(p_edited_scene, p_current_node->get_child(i), script);
		if (n)
			return n;
	}

	return NULL;
}

#endif
Node *GDIVisualScriptCustomNode::_get_base_node() const {

#ifdef TOOLS_ENABLED
	Ref<Script> script = get_visual_script();
	if (!script.is_valid())
		return NULL;

	MainLoop *main_loop = OS::get_singleton()->get_main_loop();
	SceneTree *scene_tree = Object::cast_to<SceneTree>(main_loop);

	if (!scene_tree)
		return NULL;

	Node *edited_scene = scene_tree->get_edited_scene_root();

	if (!edited_scene)
		return NULL;

	Node *script_node = _find_script_node(edited_scene, edited_scene, script);

	if (!script_node)
		return NULL;

	if (!script_node->has_node(base_path))
		return NULL;

	Node *path_to = script_node->get_node(base_path);

	return path_to;
#else

	return NULL;
#endif
}

StringName GDIVisualScriptCustomNode::_get_base_type() const {

	return base_type;
}

int GDIVisualScriptCustomNode::get_input_value_port_count() const {

	if (custom_mode == ACTIVE) {
		return 0;
	}
	else if (custom_mode == LOOP) {
		return 0;
	}
	else if (custom_mode == KEYBOARD) {
		return 1;
	}
	else if (custom_mode == MOUSE) {
		return 0;
	}
// 	else if (custom_mode == MAT_ALBEDO) {
// 		return 3;
// 	}
	else {
		return 0;
	}
}
int GDIVisualScriptCustomNode::get_output_value_port_count() const {

	if (custom_mode == ACTIVE) {
		return 0;
	}
	else if (custom_mode == LOOP) {
		return 0;
	}
	else if (custom_mode == KEYBOARD) {
		return 2;
	}
	else if (custom_mode == MOUSE) {
		return 3;
	}
// 	else if (custom_mode == MAT_ALBEDO) {
// 		return 0;
// 	}
	else {
		return 0;
	}
}

String GDIVisualScriptCustomNode::get_output_sequence_port_text(int p_port) const {

	return String();
}

PropertyInfo GDIVisualScriptCustomNode::get_input_value_port_info(int p_idx) const {

	PropertyInfo ret;
	if (custom_mode == ACTIVE) {
		ret.name = L"激活";
		ret.type = Variant::NIL;
	}
	else if (custom_mode == LOOP) {
		ret.name = L"循环";
		ret.type = Variant::NIL;
	}
	else if (custom_mode == KEYBOARD) {
		ret.name = L"键值";
		ret.type = Variant::STRING;
		ret.hint = PROPERTY_HINT_TYPE_STRING_CHAR;
	}
	else if (custom_mode == MOUSE) {
		ret.name = L"鼠标";
		ret.type = Variant::NIL;
	}
// 	else if (custom_mode == MAT_ALBEDO) {
// 		switch (p_idx)
// 		{
// 		case 0: {
// 			ret.name = L"MeshInst节点路径";
// 			ret.type = Variant::NODE_PATH;
// 			break;
// 		}
// 		case 1: {
// 			ret.name = L"颜色";
// 			ret.type = Variant::COLOR;
// 			break;
// 		}
// 		case 2: {
// 			ret.name = L"surface索引";
// 			ret.type = Variant::INT;
// 			break;
// 		}
// 		default:
// 			break;
// 		}
// 	}
	else {
		ret.name = L"未处理类型";
		ret.type = Variant::NIL;
	}

	return ret;
}

PropertyInfo GDIVisualScriptCustomNode::get_output_value_port_info(int p_idx) const {

	PropertyInfo ret;
	if (custom_mode == ACTIVE) {
		ret.name = L"激活";
		ret.type = Variant::BOOL;
	}
	else if (custom_mode == LOOP) {
		ret.name = L"循环";
		ret.type = Variant::BOOL;
	}
	else if (custom_mode == KEYBOARD) {
		switch (p_idx)
		{
		case 0: {
			ret.name = L"按下";
			ret.type = Variant::BOOL;
			break;
		}
		case 1: {
			ret.name = L"释放";
			ret.type = Variant::BOOL;
			break;
		}
		default: {
			ret.name = L"未处理类型";
			ret.type = Variant::BOOL;
			break;
		}
		}
	}
	else if (custom_mode == MOUSE) {
		switch (p_idx)
		{
		case 0: {
			ret.name = L"左键";
			ret.type = Variant::BOOL;
// 			OS::get_singleton()->print("test1");
			break;
		}
		case 1: {
			ret.name = L"右键";
			ret.type = Variant::BOOL;
// 			OS::get_singleton()->print("test2");
			break;
		}
		case 2: {
			ret.name = L"双击";
			ret.type = Variant::BOOL;
// 			OS::get_singleton()->print("test3");
			break;
		}
		default: {
			ret.name = L"未处理类型";
			ret.type = Variant::BOOL;
			break;
		}
		}
	}

	return ret;
}

String GDIVisualScriptCustomNode::get_caption() const {
	if (custom_mode == ACTIVE)
		return L"激活";
	else if (custom_mode == LOOP)
		return L"循环";
	else if (custom_mode == KEYBOARD)
		return L"键盘";
	else if (custom_mode == MOUSE)
		return L"鼠标";
// 	else if (custom_mode == MAT_ALBEDO)
// 		return L"颜色";
	else
		return L"未处理类型";
}

String GDIVisualScriptCustomNode::get_text() const {

	if (custom_mode == ACTIVE)
		return L"初始化时调用";
	else if (custom_mode == LOOP)
		return L"循环调用";
	else if (custom_mode == KEYBOARD)
		return L"在下方填入键值（如：a）";
	else if (custom_mode == MOUSE)
		return L"鼠标按键处理";
// 	else if (custom_mode == MAT_ALBEDO)
// 		return L"改变材质颜色";
	else
		return L"未处理类型";
}

void GDIVisualScriptCustomNode::set_custom_mode(CustomMode p_mode) {
	
	custom_mode = p_mode;
// 	OS::get_singleton()->print("set custom mode: [%d], this[%x]\n", custom_mode, this);
}

GDIVisualScriptCustomNode::CustomMode GDIVisualScriptCustomNode::get_custom_mode() const {

// 	OS::get_singleton()->print("get custom mode: [%d], this[%x]\n", custom_mode, this);
	return custom_mode;
}

void GDIVisualScriptCustomNode::set_rpc_call_mode(RPCCallMode p_mode) {
	if (rpc_call_mode == p_mode)
		return;
	rpc_call_mode = p_mode;
	ports_changed_notify();
	_change_notify();
}

GDIVisualScriptCustomNode::RPCCallMode GDIVisualScriptCustomNode::get_rpc_call_mode() const {
	return rpc_call_mode;
}

void GDIVisualScriptCustomNode::set_call_mode(CallMode p_mode) {
	if (call_mode == p_mode)
		return;

	call_mode = p_mode;
	_change_notify();
	ports_changed_notify();
}

GDIVisualScriptCustomNode::CallMode GDIVisualScriptCustomNode::get_call_mode() const {
	return call_mode;
}

void GDIVisualScriptCustomNode::set_basic_type(Variant::Type p_type) {

	if (basic_type == p_type)
		return;
	basic_type = p_type;

	_change_notify();
	ports_changed_notify();
}

Variant::Type GDIVisualScriptCustomNode::get_basic_type() const {

	return basic_type;
}

void GDIVisualScriptCustomNode::set_base_type(const StringName &p_type) {

	if (base_type == p_type)
		return;

	base_type = p_type;
	_change_notify();
	ports_changed_notify();
}

StringName GDIVisualScriptCustomNode::get_base_type() const {

	return base_type;
}

void GDIVisualScriptCustomNode::set_base_script(const String &p_path) {

	if (base_script == p_path)
		return;

	base_script = p_path;
	_change_notify();
	ports_changed_notify();
}

String GDIVisualScriptCustomNode::get_base_script() const {

	return base_script;
}

void GDIVisualScriptCustomNode::set_singleton(const StringName &p_type) {

	if (singleton == p_type)
		return;

	singleton = p_type;
	Object *obj = Engine::get_singleton()->get_singleton_object(singleton);
	if (obj) {
		base_type = obj->get_class();
	}

	_change_notify();
	ports_changed_notify();
}

StringName GDIVisualScriptCustomNode::get_singleton() const {

	return singleton;
}

void GDIVisualScriptCustomNode::_update_method_cache() {
	StringName type;
	Ref<Script> script;

// 	if (call_mode == CALL_MODE_NODE_PATH) {
// 
// 		Node *node = _get_base_node();
// 		if (node) {
// 			type = node->get_class();
// 			base_type = type; //cache, too
// 			script = node->get_script();
// 		}
// 	} else if (call_mode == CALL_MODE_SELF) {
// 
// 		if (get_visual_script().is_valid()) {
// 			type = get_visual_script()->get_instance_base_type();
// 			base_type = type; //cache, too
// 			script = get_visual_script();
// 		}
// 
// 	} else if (call_mode == CALL_MODE_SINGLETON) {
// 
// 		Object *obj = Engine::get_singleton()->get_singleton_object(singleton);
// 		if (obj) {
// 			type = obj->get_class();
// 			script = obj->get_script();
// 		}
// 
// 	} else if (call_mode == CALL_MODE_INSTANCE) {
// 
// 		type = base_type;
// 		if (base_script != String()) {
// 
// 			if (!ResourceCache::has(base_script) && ScriptServer::edit_request_func) {
// 
// 				ScriptServer::edit_request_func(base_script); //make sure it's loaded
// 			}
// 
// 			if (ResourceCache::has(base_script)) {
// 
// 				script = Ref<Resource>(ResourceCache::get(base_script));
// 			} else {
// 				return;
// 			}
// 		}
// 	}
// 
// 	MethodBind *mb = ClassDB::get_method(type, function);
// 	if (mb) {
// 		use_default_args = mb->get_default_argument_count();
// 		method_cache = MethodInfo();
// 		for (int i = 0; i < mb->get_argument_count(); i++) {
// #ifdef DEBUG_METHODS_ENABLED
// 			method_cache.arguments.push_back(mb->get_argument_info(i));
// #else
// 			method_cache.arguments.push_back(PropertyInfo());
// #endif
// 		}
// 
// 		if (mb->is_const()) {
// 			method_cache.flags |= METHOD_FLAG_CONST;
// 		}
// 
// #ifdef DEBUG_METHODS_ENABLED
// 
// 		method_cache.return_val = mb->get_return_info();
// #endif
// 
// 		if (mb->is_vararg()) {
// 			//for vararg just give it 10 arguments (should be enough for most use cases)
// 			for (int i = 0; i < 10; i++) {
// 				method_cache.arguments.push_back(PropertyInfo(Variant::NIL, "arg" + itos(i)));
// 				use_default_args++;
// 			}
// 		}
// 	} else if (script.is_valid() && script->has_method(function)) {
// 
// 		method_cache = script->get_method_info(function);
// 		use_default_args = method_cache.default_arguments.size();
// 	}
}

void GDIVisualScriptCustomNode::set_function(const StringName &p_type) {

	if (function == p_type)
		return;

	function = p_type;

// 	if (call_mode == CALL_MODE_BASIC_TYPE) {
// 		use_default_args = Variant::get_method_default_arguments(basic_type, function).size();
// 	} else {
// 		//update all caches
// 
// 		_update_method_cache();
// 	}
// 
// 	_change_notify();
// 	ports_changed_notify();
}
StringName GDIVisualScriptCustomNode::get_function() const {

	return function;
}

void GDIVisualScriptCustomNode::set_base_path(const NodePath &p_type) {

	if (base_path == p_type)
		return;

	base_path = p_type;
	_change_notify();
	ports_changed_notify();
}

NodePath GDIVisualScriptCustomNode::get_base_path() const {

	return base_path;
}

void GDIVisualScriptCustomNode::set_use_default_args(int p_amount) {

	if (use_default_args == p_amount)
		return;

	use_default_args = p_amount;
	ports_changed_notify();
}

int GDIVisualScriptCustomNode::get_use_default_args() const {

	return use_default_args;
}

void GDIVisualScriptCustomNode::set_validate(bool p_amount) {

	validate = p_amount;
}

bool GDIVisualScriptCustomNode::get_validate() const {

	return validate;
}

void GDIVisualScriptCustomNode::_set_argument_cache(const Dictionary &p_cache) {
	//so everything works in case all else fails
	method_cache = MethodInfo::from_dict(p_cache);
}

Dictionary GDIVisualScriptCustomNode::_get_argument_cache() const {

	return method_cache;
}

void GDIVisualScriptCustomNode::_validate_property(PropertyInfo &property) const {
		
}

void GDIVisualScriptCustomNode::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_custom_mode", "custom_mode"), &GDIVisualScriptCustomNode::set_custom_mode);
	ClassDB::bind_method(D_METHOD("get_custom_mode"), &GDIVisualScriptCustomNode::get_custom_mode);

	ClassDB::bind_method(D_METHOD("set_call_mode", "mode"), &GDIVisualScriptCustomNode::set_call_mode);
	ClassDB::bind_method(D_METHOD("get_call_mode"), &GDIVisualScriptCustomNode::get_call_mode);

	ClassDB::bind_method(D_METHOD("set_rpc_call_mode", "mode"), &GDIVisualScriptCustomNode::set_rpc_call_mode);
	ClassDB::bind_method(D_METHOD("get_rpc_call_mode"), &GDIVisualScriptCustomNode::get_rpc_call_mode);

	ClassDB::bind_method(D_METHOD("set_base_type", "base_type"), &GDIVisualScriptCustomNode::set_base_type);
	ClassDB::bind_method(D_METHOD("get_base_type"), &GDIVisualScriptCustomNode::get_base_type);

	ClassDB::bind_method(D_METHOD("set_base_script", "base_script"), &GDIVisualScriptCustomNode::set_base_script);
	ClassDB::bind_method(D_METHOD("get_base_script"), &GDIVisualScriptCustomNode::get_base_script);

	ClassDB::bind_method(D_METHOD("set_basic_type", "basic_type"), &GDIVisualScriptCustomNode::set_basic_type);
	ClassDB::bind_method(D_METHOD("get_basic_type"), &GDIVisualScriptCustomNode::get_basic_type);

	ClassDB::bind_method(D_METHOD("set_singleton", "singleton"), &GDIVisualScriptCustomNode::set_singleton);
	ClassDB::bind_method(D_METHOD("get_singleton"), &GDIVisualScriptCustomNode::get_singleton);

	ClassDB::bind_method(D_METHOD("set_function", "function"), &GDIVisualScriptCustomNode::set_function);
	ClassDB::bind_method(D_METHOD("get_function"), &GDIVisualScriptCustomNode::get_function);

	ClassDB::bind_method(D_METHOD("set_base_path", "base_path"), &GDIVisualScriptCustomNode::set_base_path);
	ClassDB::bind_method(D_METHOD("get_base_path"), &GDIVisualScriptCustomNode::get_base_path);

	ClassDB::bind_method(D_METHOD("set_use_default_args", "amount"), &GDIVisualScriptCustomNode::set_use_default_args);
	ClassDB::bind_method(D_METHOD("get_use_default_args"), &GDIVisualScriptCustomNode::get_use_default_args);

	ClassDB::bind_method(D_METHOD("_set_argument_cache", "argument_cache"), &GDIVisualScriptCustomNode::_set_argument_cache);
	ClassDB::bind_method(D_METHOD("_get_argument_cache"), &GDIVisualScriptCustomNode::_get_argument_cache);

	ClassDB::bind_method(D_METHOD("set_validate", "enable"), &GDIVisualScriptCustomNode::set_validate);
	ClassDB::bind_method(D_METHOD("get_validate"), &GDIVisualScriptCustomNode::get_validate);

	String bt;
	for (int i = 0; i < Variant::VARIANT_MAX; i++) {
		if (i > 0)
			bt += ",";

		bt += Variant::get_type_name(Variant::Type(i));
	}

	List<String> script_extensions;
	for (int i = 0; i < ScriptServer::get_language_count(); i++) {
		ScriptServer::get_language(i)->get_recognized_extensions(&script_extensions);
	}

	String script_ext_hint;
	for (List<String>::Element *E = script_extensions.front(); E; E = E->next()) {
		if (script_ext_hint != String())
			script_ext_hint += ",";
		script_ext_hint += "*." + E->get();
	}

	// 修改点：必须加入要保存的属性信息，否则这些属性在不同实例（比如从编辑器启动到真实场景后）中无法传导
	ADD_PROPERTY(PropertyInfo(Variant::INT, "custom_mode", PROPERTY_HINT_ENUM, "active,key,mouse"), "set_custom_mode", "get_custom_mode");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "call_mode", PROPERTY_HINT_ENUM, "Self,Node Path,Instance,Basic Type,Singleton"), "set_call_mode", "get_call_mode");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "base_type", PROPERTY_HINT_TYPE_STRING, "Object"), "set_base_type", "get_base_type");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "base_script", PROPERTY_HINT_FILE, script_ext_hint), "set_base_script", "get_base_script");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "singleton"), "set_singleton", "get_singleton");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "basic_type", PROPERTY_HINT_ENUM, bt), "set_basic_type", "get_basic_type");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "node_path", PROPERTY_HINT_NODE_PATH_TO_EDITED_NODE), "set_base_path", "get_base_path");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "argument_cache", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR | PROPERTY_USAGE_INTERNAL), "_set_argument_cache", "_get_argument_cache");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "function"), "set_function", "get_function"); //when set, if loaded properly, will override argument count.
	ADD_PROPERTY(PropertyInfo(Variant::INT, "use_default_args"), "set_use_default_args", "get_use_default_args");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "validate"), "set_validate", "get_validate");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "rpc_call_mode", PROPERTY_HINT_ENUM, "Disabled,Reliable,Unreliable,ReliableToID,UnreliableToID"), "set_rpc_call_mode", "get_rpc_call_mode"); //when set, if loaded properly, will override argument count.

	BIND_ENUM_CONSTANT(CALL_MODE_SELF);
	BIND_ENUM_CONSTANT(CALL_MODE_NODE_PATH);
	BIND_ENUM_CONSTANT(CALL_MODE_INSTANCE);
	BIND_ENUM_CONSTANT(CALL_MODE_BASIC_TYPE);
	BIND_ENUM_CONSTANT(CALL_MODE_SINGLETON);

	BIND_ENUM_CONSTANT(RPC_DISABLED);
	BIND_ENUM_CONSTANT(RPC_RELIABLE);
	BIND_ENUM_CONSTANT(RPC_UNRELIABLE);
	BIND_ENUM_CONSTANT(RPC_RELIABLE_TO_ID);
	BIND_ENUM_CONSTANT(RPC_UNRELIABLE_TO_ID);
}

class GDIVisualScriptNodeInstanceCustom : public VisualScriptNodeInstance {
public:
	GDIVisualScriptCustomNode::CustomMode custom_mode;
	NodePath node_path;
	int input_args;
	bool validate;
	int returns;
	StringName function;
	StringName singleton;

	GDIVisualScriptCustomNode *node;
	VisualScriptInstance *instance;

	//virtual int get_working_memory_size() const { return 0; }
	//virtual bool is_output_port_unsequenced(int p_idx) const { return false; }
	//virtual bool get_output_port_unsequenced(int p_idx,Variant* r_value,Variant* p_working_mem,String &r_error) const { return true; }

	virtual int step(const Variant **p_inputs, Variant **p_outputs, StartMode p_start_mode, Variant *p_working_mem, Variant::CallError &r_error, String &r_error_str) {

		static _OS *_os = nullptr;
		static OS *os = nullptr;
		static Input *input = nullptr;
		if (nullptr == _os) {
			_os = _OS::get_singleton();
		}
		if (nullptr == os) {
			os = OS::get_singleton();
		}
		if (nullptr == input) {
			input = Input::get_singleton();
		}
		if (p_start_mode == VisualScriptCustomNode::START_MODE_BEGIN_SEQUENCE) {
// 			os->print("Test begin sequence....\n");
		}
		static Point2 mouse_point;

		switch (custom_mode) {
		case GDIVisualScriptCustomNode::ACTIVE: {

			break;
		}
		case GDIVisualScriptCustomNode::LOOP: {

// 			Object *object = instance->get_owner_ptr();
// 			*p_outputs[0] = object->call("_process", p_inputs, input_args, r_error);
			break;
		}
		case GDIVisualScriptCustomNode::KEYBOARD: {
			String key = *p_inputs[0];
			if (key.length() != 1) {
				break;
			}

			auto sc = _os->find_scancode_from_string(key);
			auto pressedFlag = input->is_key_pressed(sc);

			*p_outputs[0] = pressedFlag;
			*p_outputs[1] = !pressedFlag;

			break;
		}
		case GDIVisualScriptCustomNode::MOUSE: {
			auto left_button_pressed_flag = input->is_mouse_button_pressed(BUTTON_LEFT);
			auto right_button_pressed_flag = input->is_mouse_button_pressed(BUTTON_RIGHT);
			*p_outputs[0] = left_button_pressed_flag;
			*p_outputs[1] = right_button_pressed_flag;
			*p_outputs[2] = false;

// 			os->print("test output[%d], [%d], [%d]\n", b1, b2,*p_outputs[2]);

			static uint64_t time = 0;
			static bool first_click_pressed_flag = false;
			static bool first_click_released_flag = false;
			static bool second_click_pressed_flag = false;
			static const unsigned int double_click_interval = 500;

			// reset, if time alreay over
			if (os->get_system_time_msecs() - time >= double_click_interval) {
				time = 0;
				second_click_pressed_flag = false;
				first_click_pressed_flag = false;
				first_click_released_flag = false;
			}

			if (!first_click_pressed_flag && left_button_pressed_flag) {
				first_click_pressed_flag = true;
				time = os->get_system_time_msecs();
				// 防止拖动操作后点击误判为双击
				mouse_point = os->get_mouse_position();
			}
			else if (first_click_pressed_flag && !first_click_released_flag && !left_button_pressed_flag) {
				first_click_released_flag = true;
			}
			else if (first_click_pressed_flag && first_click_released_flag && left_button_pressed_flag) {
				second_click_pressed_flag = true;
			}
			else if (second_click_pressed_flag && !left_button_pressed_flag) {

				auto diff_time = os->get_system_time_msecs() - time;
// 				os->print("enter second click, diff time[%d]\n", diff_time);
				if (diff_time < double_click_interval && mouse_point == os->get_mouse_position()) {
					*p_outputs[2] = true;
				}

				time = 0;
				second_click_pressed_flag = false;
				first_click_pressed_flag = false;
				first_click_released_flag = false;
			}

			break;
		}
		// 先保留下，很有可能后面又会加入这种简化版的节点
// 		case GDIVisualScriptCustomNode::MAT_ALBEDO: {
// 			Object *object = instance->get_owner_ptr();
// 			Node *node = Object::cast_to<Node>(object);
// 			if (nullptr == node) {
// // 				os->print("[GDI]Material albedo node, can not convert obj to node\n");
// 				r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
// 				r_error_str = "[GDI]Material albedo node, can not convert obj to node";
// 				return 0;
// 			}
// 
// 			NodePath path = *p_inputs[0];
// 			Node *target_node = node->get_node(path);
// 			if (nullptr == target_node) {
// // 				os->print("[GDI]Material albedo node, target node is null\n");
// 				r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
// 				r_error_str = "[GDI]Material albedo node, target node is null";
// 				return 0;
// 			}
// 
// 			MeshInstance *mesh = Object::cast_to<MeshInstance>(target_node);
// 			if (nullptr == mesh) {
// // 				os->print("[GDI]Material albedo node, can not convert to mesh inst\n");
// 				r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
// 				r_error_str = "[GDI]Material albedo node, can not convert to mesh inst";
// 				return 0;
// 			}
// 			if (mesh->get_surface_material_count() == 0) {
// // 				os->print("[GDI]Material albedo node, surface is null\n");
// 				r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
// 				r_error_str = "[GDI]Material albedo node, surface is null";
// 				return 0;
// 			}
// 
// 			unsigned int surface_index = *p_inputs[2];
// 			if (surface_index > mesh->get_surface_material_count() - 1) {
// // 				os->print("[GDI]Material albedo node, invalid surface index\n");
// 				r_error.error = Variant::CallError::CALL_ERROR_INVALID_ARGUMENT;
// 				r_error_str = "[GDI]Material albedo node, invalid surface index";
// 				return 0;
// 			}
// 
// 			auto mat = Object::cast_to<SpatialMaterial>(*(mesh->get_surface_material(surface_index)));
// 			if (nullptr == mat) {
// // 				os->print("[GDI]Material albedo node, can not find material with index[%d]\n", surface_index);
// 				r_error.error = Variant::CallError::CALL_ERROR_INVALID_ARGUMENT;
// 				r_error_str = "[GDI]Material albedo node, can not find material with index";
// 				return 0;
// 			}
// 
// 			mat->set_albedo(*p_inputs[1]);
// 			mesh->set_surface_material(surface_index, mat);
// 
// 			break;
// 		}
		}

		if (!validate) {

			//ignore call errors if validation is disabled
			r_error.error = Variant::CallError::CALL_OK;
			r_error_str = String();
		}

		return 0;
	}
};

VisualScriptNodeInstance *GDIVisualScriptCustomNode::instance(VisualScriptInstance *p_instance) {

	GDIVisualScriptNodeInstanceCustom *instance = memnew(GDIVisualScriptNodeInstanceCustom);
	instance->node = this;
	instance->instance = p_instance;
	instance->singleton = singleton;
	instance->function = function;
	instance->custom_mode = get_custom_mode();
// 	OS::get_singleton()->print("custom mode:[%d], this[%x]\n", instance->custom_mode, this);
	instance->returns = get_output_value_port_count();
	instance->node_path = base_path;
	instance->input_args = get_input_value_port_count();
	instance->validate = validate;
	return instance;
}

GDIVisualScriptCustomNode::TypeGuess GDIVisualScriptCustomNode::guess_output_type(TypeGuess *p_inputs, int p_output) const {

// 	if (p_output == 0 && call_mode == CALL_MODE_INSTANCE) {
// 		return p_inputs[0];
// 	}

	return VisualScriptNode::guess_output_type(p_inputs, p_output);
}

GDIVisualScriptCustomNode::GDIVisualScriptCustomNode() {

	custom_mode = ACTIVE;
	validate = true;
	basic_type = Variant::NIL;
	use_default_args = 0;
	base_type = "Object";
}
