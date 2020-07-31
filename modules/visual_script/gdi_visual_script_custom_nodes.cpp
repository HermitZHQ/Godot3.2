#include "gdi_visual_script_custom_nodes.h"

#include "core/engine.h"
#include "core/io/resource_loader.h"
#include "core/os/os.h"
#include "scene/main/node.h"
#include "scene/main/scene_tree.h"
#include "visual_script_nodes.h"

#include "core/os/input.h"
#include "core/bind/core_bind.h"

#include "scene/3d/mesh_instance.h"
#include "scene/3d/area.h"
#include "scene/3d/collision_shape.h"
#include "scene/resources/box_shape.h"
//#include "scene/3d/collision_polygon.h"
//#include "scene/resources/plane_shape.h"


int GDIVisualScriptCustomNode::get_output_sequence_port_count() const {

	static int i = 0;
	switch (custom_mode)
	{
	case GDIVisualScriptCustomNode::ACTIVE:
	case GDIVisualScriptCustomNode::LOOP:
	case GDIVisualScriptCustomNode::TASK_CONTROL:
	case GDIVisualScriptCustomNode::KEYBOARD:
	case GDIVisualScriptCustomNode::MOUSE:
		return 1;
	case GDIVisualScriptCustomNode::AREA_TIGGER:
	case GDIVisualScriptCustomNode::TIMER:
	case GDIVisualScriptCustomNode::COMBINATION:
		return 2;
	case GDIVisualScriptCustomNode::TASK_SPLIT:
		return task_split_num;
	case GDIVisualScriptCustomNode::INIT:
	default:
		return 0;
	}
}

bool GDIVisualScriptCustomNode::has_input_sequence_port() const {

	switch (custom_mode)
	{
	case GDIVisualScriptCustomNode::ACTIVE:
	case GDIVisualScriptCustomNode::LOOP:
		return false;
	case GDIVisualScriptCustomNode::TASK_SPLIT:
	case GDIVisualScriptCustomNode::TASK_CONTROL:
	case GDIVisualScriptCustomNode::KEYBOARD:
	case GDIVisualScriptCustomNode::MOUSE:
	case GDIVisualScriptCustomNode::AREA_TIGGER:
	case GDIVisualScriptCustomNode::TIMER:
	case GDIVisualScriptCustomNode::COMBINATION:
	case GDIVisualScriptCustomNode::INIT:
	case GDIVisualScriptCustomNode::INIT_PARTIAL:
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
	else if (custom_mode == TASK_CONTROL) {
		return 2;
	}
	else if (custom_mode == KEYBOARD) {
		return 1;
	}
	else if (custom_mode == MOUSE) {
		return 0;
	}
	else if (custom_mode == AREA_TIGGER) {
		return 2;
	}
	else if (custom_mode == TIMER) {
		return 3;
	}
	else if (custom_mode == TIMER) {
		return 3;
	}
	else if (custom_mode == COMBINATION) {
		return use_default_args;
	}
	else if (custom_mode == INIT) {
		return 0;
	}
	else if (custom_mode == INIT_PARTIAL) {
		return 1;
	}
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
	else if (custom_mode == TASK_SPLIT) {
		return 1;
	}
	else if (custom_mode == TASK_CONTROL) {
		return 0;
	}
	else if (custom_mode == KEYBOARD) {
		return 2;
	}
	else if (custom_mode == MOUSE) {
		return 3;
	}
	else if (custom_mode == AREA_TIGGER) {
		return 1;
	}
	else if (custom_mode == TIMER) {
		return 1;
	}
	else if (custom_mode == COMBINATION) {
		return 1;
	}
	else if (custom_mode == INIT) {
		return 0;
	}
	else {
		return 0;
	}
}

String GDIVisualScriptCustomNode::get_output_sequence_port_text(int p_port) const {

	switch (custom_mode)
	{
	case GDIVisualScriptCustomNode::ACTIVE:
	case GDIVisualScriptCustomNode::LOOP:
	case GDIVisualScriptCustomNode::TASK_CONTROL:
	case GDIVisualScriptCustomNode::KEYBOARD:
	case GDIVisualScriptCustomNode::MOUSE:
	case GDIVisualScriptCustomNode::INIT:
		return String();
	case GDIVisualScriptCustomNode::AREA_TIGGER:
	case GDIVisualScriptCustomNode::COMBINATION:
	case GDIVisualScriptCustomNode::TIMER: {
		switch (p_port)
		{
		case 0: {
			return L"激活时";
		}
		case 1: {
			return L"未激活时";
		}
		default:
			return String();
		}
		break;
	}
	case GDIVisualScriptCustomNode::TASK_SPLIT: {
		return String(L"子任务-") + itos(p_port);
		break;
	}
	default:
		return String();
	}
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
	else if (custom_mode == TASK_CONTROL) {

		switch (p_idx)
		{
		case 0: {

			ret.name = L"激活";
			ret.type = Variant::BOOL;
			break;
		}
		case 1: {

			ret.name = L"只执行一次";
			ret.type = Variant::BOOL;
			break;
		}
		default:
			break;
		}
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
	else if (custom_mode == AREA_TIGGER) {

		switch (p_idx)
		{
		case 0: {

			ret.name = L"Area节点";
			ret.type = Variant::NODE_PATH;
			break;
		}
		case 1: {

			ret.name = L"触发节点";
			ret.type = Variant::NODE_PATH;
			break;
		}
		default: {

			ret.name = L"未处理类型";
			ret.type = Variant::NIL;
			break;
		}
		}
	}
	else if (custom_mode == TIMER) {

		switch (p_idx)
		{
		case 0: {

			ret.name = L"分";
			ret.type = Variant::INT;
			break;
		}
		case 1: {

			ret.name = L"秒";
			ret.type = Variant::INT;
			break;
		}
		case 2: {

			ret.name = L"毫秒";
			ret.type = Variant::INT;
			break;
		}
		default: {
			
			ret.name = L"未处理类型";
			ret.type = Variant::NIL;
			break;
		}
		}
	}
	else if (custom_mode == COMBINATION) {

		ret.name = L"任务";
		ret.type = Variant::BOOL;
	}
	else if (custom_mode == INIT_PARTIAL) {

		ret.name = L"任务拆分实例";
		ret.type = Variant::OBJECT;
	}
// 	else if (custom_mode == MAT_ALBEDO) {
// 
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
	else if (custom_mode == TASK_SPLIT) {

		ret.name = L"实例";
		ret.type = Variant::OBJECT;
// 		ret.hint = PROPERTY_HINT_OBJECT_ID;
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
			break;
		}
		case 1: {
			ret.name = L"右键";
			ret.type = Variant::BOOL;
			break;
		}
		case 2: {
			ret.name = L"双击";
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
	else if (custom_mode == AREA_TIGGER) {

		switch (p_idx)
		{
		case 0: {
			ret.name = L"已触发";
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
	else if (custom_mode == TIMER || custom_mode == COMBINATION) {

		switch (p_idx)
		{
		case 0: {
			ret.name = L"已触发";
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

	return ret;
}

String GDIVisualScriptCustomNode::get_caption() const {

	if (custom_mode == ACTIVE)
		return L"激活";
	else if (custom_mode == LOOP)
		return L"循环";
	else if (custom_mode == TASK_SPLIT)
		return L"任务拆分";
	else if (custom_mode == TASK_CONTROL)
		return L"任务控制";
	else if (custom_mode == KEYBOARD)
		return L"键盘";
	else if (custom_mode == MOUSE)
		return L"鼠标";
	else if (custom_mode == AREA_TIGGER)
		return L"空间触发器";
	else if (custom_mode == TIMER)
		return L"计时器";
	else if (custom_mode == COMBINATION)
		return L"任务组合";
	else if (custom_mode == INIT)
		return L"初始化";
	else if (custom_mode == INIT_PARTIAL)
		return L"部分初始化";
	else
		return L"未处理类型";
}

String GDIVisualScriptCustomNode::get_text() const {

	if (custom_mode == ACTIVE)
		return L"初始化时调用";
	else if (custom_mode == LOOP)
		return L"循环调用";
	else if (custom_mode == TASK_SPLIT)
		return String(L"拆分原有任务线");
	else if (custom_mode == TASK_CONTROL)
		return L"可单独屏蔽部分流程";
	else if (custom_mode == KEYBOARD)
		return L"在下方填入键值（如：a）";
	else if (custom_mode == MOUSE)
		return L"鼠标按键处理";
	else if (custom_mode == AREA_TIGGER)
		return L"其他节点进入时触发";
	else if (custom_mode == TIMER)
		return L"到设定时间后触发";
	else if (custom_mode == COMBINATION)
		return L"所有任务为真后触发";
	else if (custom_mode == INIT)
		return L"恢复场景到初始状态";
	else if (custom_mode == INIT_PARTIAL)
		return L"恢复场景到子任务分支";
	else
		return L"未处理类型";
}

void GDIVisualScriptCustomNode::set_custom_mode(CustomMode p_mode) {
	
	custom_mode = p_mode;
	if (TASK_CONTROL == p_mode) {
		set_task_id(global_task_id++);
	}
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

void GDIVisualScriptCustomNode::area_trigger_entered_signal_callback(Node *area) {

	int res = area_trigger_entered_area_vec.find(area);
	if (res >= 0) {
		return;
	}

// 	printf("area enter...\n");
	area_trigger_entered_area_vec.push_back(area);
}

void GDIVisualScriptCustomNode::area_trigger_exited_signal_callback(Node *area) {

	int res = area_trigger_entered_area_vec.find(area);
	if (res >= 0) {
// 		printf("area exit...\n");
		area_trigger_entered_area_vec.erase(area);
	}
}

int GDIVisualScriptCustomNode::get_area_trigger_entered_area_num() const {

	return area_trigger_entered_area_vec.size();
}

void GDIVisualScriptCustomNode::set_task_id(unsigned int id) {

	task_id = id;
}

unsigned int GDIVisualScriptCustomNode::get_task_id() const {

	return task_id;
}

void GDIVisualScriptCustomNode::set_task_split_num(unsigned int num) {

	task_split_num = num;
	ports_changed_notify();
}

unsigned int GDIVisualScriptCustomNode::get_task_split_num() const {

	return task_split_num;
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

	// 修改点：增加需要的函数和属性绑定
	ClassDB::bind_method(D_METHOD("set_custom_mode", "custom_mode"), &GDIVisualScriptCustomNode::set_custom_mode);
	ClassDB::bind_method(D_METHOD("get_custom_mode"), &GDIVisualScriptCustomNode::get_custom_mode);

	ClassDB::bind_method(D_METHOD("area_trigger_entered_signal_callback", "area"), &GDIVisualScriptCustomNode::area_trigger_entered_signal_callback);
	ClassDB::bind_method(D_METHOD("area_trigger_exited_signal_callback", "area"), &GDIVisualScriptCustomNode::area_trigger_exited_signal_callback);

	ClassDB::bind_method(D_METHOD("set_task_split_num", "num"), &GDIVisualScriptCustomNode::set_task_split_num);
	ClassDB::bind_method(D_METHOD("get_task_split_num"), &GDIVisualScriptCustomNode::get_task_split_num);

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
	ADD_PROPERTY(PropertyInfo(Variant::INT, "custom_mode", PROPERTY_HINT_NONE, "active,key,mouse", PROPERTY_USAGE_NOEDITOR | PROPERTY_USAGE_STORAGE), "set_custom_mode", "get_custom_mode");
// 	ADD_PROPERTY(PropertyInfo(Variant::INT, "call_mode", PROPERTY_HINT_ENUM, "Self,Node Path,Instance,Basic Type,Singleton"), "set_call_mode", "get_call_mode");
// 	ADD_PROPERTY(PropertyInfo(Variant::STRING, "base_type", PROPERTY_HINT_TYPE_STRING, "Object"), "set_base_type", "get_base_type");
// 	ADD_PROPERTY(PropertyInfo(Variant::STRING, "base_script", PROPERTY_HINT_FILE, script_ext_hint), "set_base_script", "get_base_script");
// 	ADD_PROPERTY(PropertyInfo(Variant::STRING, "singleton"), "set_singleton", "get_singleton");
// 	ADD_PROPERTY(PropertyInfo(Variant::INT, "basic_type", PROPERTY_HINT_ENUM, bt), "set_basic_type", "get_basic_type");
// 	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "node_path", PROPERTY_HINT_NODE_PATH_TO_EDITED_NODE), "set_base_path", "get_base_path");
// 	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "argument_cache", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR | PROPERTY_USAGE_INTERNAL), "_set_argument_cache", "_get_argument_cache");
// 	ADD_PROPERTY(PropertyInfo(Variant::STRING, "function"), "set_function", "get_function"); //when set, if loaded properly, will override argument count.
	ADD_PROPERTY(PropertyInfo(Variant::INT, L"(组合)任务数量"), "set_use_default_args", "get_use_default_args");
	ADD_PROPERTY(PropertyInfo(Variant::INT, L"(任务拆分)数量"), "set_task_split_num", "get_task_split_num");
// 	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "validate"), "set_validate", "get_validate");
// 	ADD_PROPERTY(PropertyInfo(Variant::INT, "rpc_call_mode", PROPERTY_HINT_ENUM, "Disabled,Reliable,Unreliable,ReliableToID,UnreliableToID"), "set_rpc_call_mode", "get_rpc_call_mode"); //when set, if loaded properly, will override argument count.

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

	bool task_ctrl_already_execute_once_flag = false;

	// record singleton to increase the invoke efficiency
	_OS *_os = nullptr;
	OS *os = nullptr;
	Input *input = nullptr;

	GDIVisualScriptNodeInstanceCustom()
		:_os(_OS::get_singleton())
		, os(OS::get_singleton())
		, input(Input::get_singleton())
	{}

	virtual int get_working_memory_size() const { return 1; }
	//virtual bool is_output_port_unsequenced(int p_idx) const { return false; }
	//virtual bool get_output_port_unsequenced(int p_idx,Variant* r_value,Variant* p_working_mem,String &r_error) const { return true; }

	void mouse_handle_func(const Variant **p_inputs, Variant **p_outputs) {

		static Point2 mouse_point;
		auto left_button_pressed_flag = input->is_mouse_button_pressed(BUTTON_LEFT);
		auto right_button_pressed_flag = input->is_mouse_button_pressed(BUTTON_RIGHT);
		*p_outputs[0] = left_button_pressed_flag;
		*p_outputs[1] = right_button_pressed_flag;
		*p_outputs[2] = false;

// 		os->print("test output[%d], [%d], [%d]\n", b1, b2,*p_outputs[2]);

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
			// prevent drag double click
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
	}

	void check_child_mesh_area_func(Node *node, bool &dirty_flag, bool &has_area_flag, Vector3 &min_pos, Vector3 &max_pos) {

		int child_num = node->get_child_count();
		for (int i = 0; i < child_num; ++i) {
			Node *child = node->get_child(i);
			int child_chlid_num = child->get_child_count();
			if (child_chlid_num > 0) {
				check_child_mesh_area_func(child, dirty_flag, has_area_flag, min_pos, max_pos);
			}

			MeshInstance *mesh = Object::cast_to<MeshInstance>(child);
			Area *area = Object::cast_to<Area>(child);
			if (nullptr != mesh) {
				auto aabb = mesh->get_transformed_aabb();
				auto tmp_max_pos = aabb.position + aabb.size;
				auto tmp_min_pos = aabb.position;

				if (!dirty_flag) {
					min_pos = tmp_min_pos;
					max_pos = tmp_max_pos;
					dirty_flag = true;
				}
				else {
					min_pos.x = (tmp_min_pos.x < min_pos.x ? tmp_min_pos.x : min_pos.x);
					min_pos.y = (tmp_min_pos.y < min_pos.y ? tmp_min_pos.y : min_pos.y);
					min_pos.z = (tmp_min_pos.z < min_pos.z ? tmp_min_pos.z : min_pos.z);

					max_pos.x = (tmp_max_pos.x > max_pos.x ? tmp_max_pos.x : max_pos.x);
					max_pos.y = (tmp_max_pos.y > max_pos.y ? tmp_max_pos.y : max_pos.y);
					max_pos.z = (tmp_max_pos.z > max_pos.z ? tmp_max_pos.z : max_pos.z);
				}
			}
			else if (nullptr != area) {
				CollisionShape *cs = nullptr;
				int child_num = area->get_child_count();
				for (int i = 0; i < child_num; ++i) {
					cs = Object::cast_to<CollisionShape>(area->get_child(i));
					if (nullptr != cs) {
						has_area_flag = true;
						break;
					}
				}
			}
		}
	}
	int area_trigger_handle_func(const Variant **p_inputs, Variant **p_outputs, Variant::CallError &r_error, String &r_error_str) {

		Object *object = instance->get_owner_ptr();
		Node *node = Object::cast_to<Node>(object);
		if (nullptr == node) {
			r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
			r_error_str = "[GDI]area trigger, can not convert obj to node";
			return 1;
		}

		Node *root = node->get_tree()->get_current_scene();
		if (nullptr == root) {
			r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
			r_error_str = "[GDI]area trigger, can't find edit root node";
			return 1;
		}

		NodePath path;
		path = *p_inputs[0];
		Area *area = Object::cast_to<Area>(node->get_node(path));
		if (nullptr == area) {
			r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
			r_error_str = "[GDI]area trigger, can't find area";
			return 1;
		}

		path = *p_inputs[1];
		Spatial *target = Object::cast_to<Spatial>(node->get_node(path));
		if (nullptr == target) {
			r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
			r_error_str = "[GDI]area trigger, can't find target(spatial)";
			return 1;
		}

		// calculate the comb aabb manual
		int child_num = target->get_child_count();
		Vector3 min_pos, max_pos;
		bool dirty_flag = false;
		bool has_area_flag = false;

		// check self mesh
		MeshInstance *self_mesh = Object::cast_to<MeshInstance>(target);
		if (nullptr != self_mesh) {
			auto aabb = self_mesh->get_transformed_aabb();
			auto tmp_max_pos = aabb.position + aabb.size;
			auto tmp_min_pos = aabb.position;

			if (!dirty_flag) {
				min_pos = tmp_min_pos;
				max_pos = tmp_max_pos;
				dirty_flag = true;
			}
		}

		// check self area
		Area *self_area = Object::cast_to<Area>(target);
		if (nullptr != self_area) {
			CollisionShape *cs = nullptr;
			int child_num = self_area->get_child_count();
			for (int i = 0; i < child_num; ++i) {
				cs = Object::cast_to<CollisionShape>(self_area->get_child(i));
				if (nullptr != cs) {
					has_area_flag = true;
					break;
				}
			}
		}

		// check child mesh and area situation
		check_child_mesh_area_func(target, dirty_flag, has_area_flag, min_pos, max_pos);

		Vector3 target_pos = target->get_global_transform().origin;
		Vector3 center_pos = (max_pos + min_pos) / 2.0;
		Vector3 dir = center_pos - target_pos;
		float diff_len = dir.length();
		dir.normalize();
		// the 0.1 extents prevents the null area, if no valid mesh under the node
		Vector3 extents = (max_pos - center_pos) + Vector3(0.1, 0.1, 0.1);

		static bool first_flag = true;
		if (first_flag) {
			// add the Area node with collision shape dynamicly
			if (!has_area_flag) {
				Area *new_area = memnew(Area);
				CollisionShape *cs = memnew(CollisionShape);
				BoxShape *bs = memnew(BoxShape);
				bs->set_extents(extents);
	
				new_area->add_child(cs);
				target->add_child(new_area);
				cs->set_shape(bs);
	
				new_area->translate(dir * diff_len);
// 				os->print("[GDI]attach the Area node manual\n");
			}

			auto err = area->connect("area_entered", this->node, "area_trigger_entered_signal_callback");
			if (OK != err) {
				r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
				r_error_str = "[GDI]area trigger, connect signal failed";
			}
			err = area->connect("area_exited", this->node, "area_trigger_exited_signal_callback");
			if (OK != err) {
				r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
				r_error_str = "[GDI]area trigger, connect signal failed";
			}
			first_flag = false;
		}

		bool area_entered_flag = this->node->get_area_trigger_entered_area_num() > 0 ? true : false;
		*p_outputs[0] = area_entered_flag;

		return (area_entered_flag ? 0 : 1);
	}

	int timer_handle_func(const Variant **p_inputs, Variant **p_outputs) {

// 		static int hour = *p_inputs[0];
		static int minute = *p_inputs[0];
		static int sec = *p_inputs[1];
		static int msec = *p_inputs[2];

		static uint64_t start_time = os->get_system_time_msecs();

		bool reach_target_time_flag =
			(os->get_system_time_msecs() - start_time > (msec + sec * 1000 + minute * 1000 * 60)) ? true : false;

		*p_outputs[0] = reach_target_time_flag;
		if (reach_target_time_flag) {
			// reset the start time
			start_time = os->get_system_time_msecs();

			return 0;
		}

		return 1;
	}

	int combination_handle_func(const Variant **p_inputs, Variant **p_outputs) {

		int arg_num = node->get_use_default_args();
		bool all_pass_flag = true;
		for (int i = 0; i < arg_num; ++i) {
			if (!(bool(*p_inputs[i]))) {
				all_pass_flag = false;
				break;
			}
		}

		*p_outputs[0] = all_pass_flag;
		if (all_pass_flag) {
			return 0;
		}
		else {
			return 1;
		}
	}

	void collect_init_info(Node *node, Map<Spatial*, Transform> &objs_init_trans_map) {

		Spatial *spa = Object::cast_to<Spatial>(node);
		if (nullptr != spa) {
			objs_init_trans_map.insert(spa, spa->get_global_transform());
			os->print("[GDI]insert init trans info, node name[%S], addr[%X]\n", String(spa->get_name()), spa);
		}

		int child_num = node->get_child_count();
		for (int i = 0; i < child_num; ++i) {
			Node *child = node->get_child(i);
			int child_child_num = child->get_child_count();
			if (child_child_num > 0) {
				collect_init_info(child, objs_init_trans_map);
			}
			else {
				spa = Object::cast_to<Spatial>(child);
				if (nullptr != spa) {
					objs_init_trans_map.insert(spa, spa->get_global_transform());
					os->print("[GDI]insert init trans info, node name[%S], addr[%X]\n", String(spa->get_name()), spa);
				}
			}
		}
	}

	virtual int step(const Variant **p_inputs, Variant **p_outputs, StartMode p_start_mode, Variant *p_working_mem, Variant::CallError &r_error, String &r_error_str) {

// 		if (p_start_mode == VisualScriptCustomNode::START_MODE_BEGIN_SEQUENCE) {
// 			os->print("Test begin sequence....\n");
// 		}
// 		else {
// 			os->print("other start mode[%d]\n", p_start_mode);
// 		}

		// initialize relevant
		static bool init_objs_info_flag = false;
		static Map<Spatial*, Transform> objs_init_trans_map;
		if (!init_objs_info_flag) {
			Object *object = instance->get_owner_ptr();
			Node *node = Object::cast_to<Node>(object);
			if (nullptr == node) {
				r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
				r_error_str = "[GDI]collect init info failed, can't get node";
				return 0;
			}

			Node *root = node->get_tree()->get_current_scene();
			if (nullptr == root) {
				r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
				r_error_str = "[GDI]collect init info failed, can't get root";
				return 0;
			}

			os->print("start collect init info, addr[%x]\n", this);
			collect_init_info(root, objs_init_trans_map);
			os->print("end collect init info\n");
			init_objs_info_flag = true;
		}

		switch (custom_mode) {

		case GDIVisualScriptCustomNode::ACTIVE: {

			break;
		}
		case GDIVisualScriptCustomNode::LOOP: {

			break;
		}
		case GDIVisualScriptCustomNode::TASK_SPLIT: {

			*p_outputs[0] = node;
			static unsigned int cur_execute_index = 0;

			bool execute_last_flag = false;
			if (cur_execute_index == node->get_task_split_num() - 1) {
				execute_last_flag = true;
			}
			unsigned int tmp_index = cur_execute_index++;
			if (cur_execute_index == node->get_task_split_num()) {
				cur_execute_index = 0;
			}

			return execute_last_flag ? tmp_index : (tmp_index | STEP_FLAG_PUSH_STACK_BIT);
		}
		case GDIVisualScriptCustomNode::TASK_CONTROL: {

			bool active_flag = *p_inputs[0];
			bool only_execute_once_flag = *p_inputs[1];

			if (!active_flag) {
				if (nullptr != p_working_mem) {
					p_working_mem[0] = STEP_EXIT_FUNCTION_BIT;
				}
				return STEP_EXIT_FUNCTION_BIT;
// 				return 0 | STEP_FLAG_PUSH_STACK_BIT;
			}
			else {
				if (only_execute_once_flag && !task_ctrl_already_execute_once_flag) {
					task_ctrl_already_execute_once_flag = true;
					return 0;
				}
				else if (only_execute_once_flag && task_ctrl_already_execute_once_flag) {
					if (nullptr != p_working_mem) {
						p_working_mem[0] = STEP_EXIT_FUNCTION_BIT;
					}
					return STEP_EXIT_FUNCTION_BIT;
				}
				else {
					return 0;
				}
			}
		}
		case GDIVisualScriptCustomNode::KEYBOARD: {

			String key = *p_inputs[0];
			if (key.length() != 1) {
				r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
				r_error_str = "[GDI]keyboard, key string length not 1";
				return 0;
			}

			auto sc = _os->find_scancode_from_string(key);
			auto pressedFlag = input->is_key_pressed(sc);

			*p_outputs[0] = pressedFlag;
			*p_outputs[1] = !pressedFlag;

			return 0;
		}
		case GDIVisualScriptCustomNode::MOUSE: {

			mouse_handle_func(p_inputs, p_outputs);
			break;
		}
		case GDIVisualScriptCustomNode::AREA_TIGGER: {

			int ret = area_trigger_handle_func(p_inputs, p_outputs, r_error, r_error_str);
			return ret;
		}
		case GDIVisualScriptCustomNode::TIMER: {

			int ret = timer_handle_func(p_inputs, p_outputs);
			return ret;
		}
		case GDIVisualScriptCustomNode::COMBINATION: {

			int ret = combination_handle_func(p_inputs, p_outputs);
			return ret;
		}
		case GDIVisualScriptCustomNode::INIT: {

			for (auto e = objs_init_trans_map.front(); e; e = e->next()) {
				e->key()->set_global_transform(e->value());
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
	task_id = 0;
	task_split_num = 2;
}

GDIVisualScriptCustomNode::~GDIVisualScriptCustomNode() {

	if (TASK_CONTROL == custom_mode) {
		--global_task_id;
	}
}

unsigned int GDIVisualScriptCustomNode::global_task_id = 1;
