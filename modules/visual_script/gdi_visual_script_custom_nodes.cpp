#include "gdi_visual_script_custom_nodes.h"

#include <string>
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
#include "scene/main/viewport.h"
#include "scene/3d/camera.h"
#include "scene/resources/material.h"
#include "core/node_path.h"
#include "modules/enet/networked_multiplayer_enet.h"
#include "core/io/tcp_server.h"
#include "core/io/stream_peer_tcp.h"

class GDIVisualScriptNodeInstanceCustomMultiPlayer;

int GDIVisualScriptCustomNode::get_output_sequence_port_count() const {

	static int i = 0;
	switch (custom_mode)
	{
	case GDIVisualScriptCustomNode::ACTIVE:
	case GDIVisualScriptCustomNode::LOOP:
	case GDIVisualScriptCustomNode::TASK_CONTROL:
		return 1;
	case GDIVisualScriptCustomNode::AREA_TIGGER:
	case GDIVisualScriptCustomNode::TIMER:
	case GDIVisualScriptCustomNode::COMBINATION:
		return 2;
	case GDIVisualScriptCustomNode::KEYBOARD:
		return 3;
	case GDIVisualScriptCustomNode::MOUSE:
		return 5;
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
		return 1;
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
		return 2;
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
		return 1;
	}
	else if (custom_mode == MOUSE) {
		return 4;
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
	case GDIVisualScriptCustomNode::TASK_SPLIT: {
		return String(L"子任务-") + itos(p_port);
		break;
	}
	case GDIVisualScriptCustomNode::TASK_CONTROL: {
		String();
		break;
	}
	case GDIVisualScriptCustomNode::KEYBOARD: {
		switch (p_port)
		{
		case 0: {
			return L"按下时";
		}
		case 1: {
			return L"按下保持";
		}
		case 2: {
			return L"释放时";
		}
		default:
			return String();
		}
		break;
	}
	case GDIVisualScriptCustomNode::MOUSE: {
		switch (p_port)
		{
		case 0: {
			return L"左键单击";
		}
		case 1: {
			return L"左键双击";
		}
		case 2: {
			return L"左键拖动";
		}
		case 3: {
			return L"右键单击";
		}
		case 4: {
			return L"中键滚轮";
		}
		default:
			return String();
		}
		break;
	}
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
	default:
		return String();
	}

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

		ret.name = L"拣选节点";
		ret.type = Variant::OBJECT;
	}
	else if (custom_mode == AREA_TIGGER) {

		switch (p_idx)
		{
		case 0: {
			ret.name = L"Area节点";
			ret.type = Variant::NODE_PATH;
			ret.hint = PROPERTY_HINT_NODE_PATH_VALID_TYPES;
			ret.hint_string = String("Area");
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

		switch (p_idx)
		{
		case 0: {

			ret.name = L"任务拆分实例";
			ret.type = Variant::OBJECT;
			break;
		}
		case 1: {

			ret.name = L"分支任务索引";
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
			ret.name = L"左键按下";
			ret.type = Variant::BOOL;
			break;
		}
		case 1: {
			ret.name = L"右键按下";
			ret.type = Variant::BOOL;
			break;
		}
		case 2: {
			ret.name = L"鼠标坐标";
			ret.type = Variant::VECTOR2;
			break;
		}
		case 3: {
			ret.name = L"拣选标识";
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
	//OS::get_singleton()->print("set custom mode: [%d], this[%x]\n", custom_mode, this);
}

GDIVisualScriptCustomNode::CustomMode GDIVisualScriptCustomNode::get_custom_mode() const {

	//OS::get_singleton()->print("get custom mode: [%d], this[%x]\n", custom_mode, this);
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

	//printf("area enter...\n");
	area_trigger_entered_area_vec.push_back(area);
}

void GDIVisualScriptCustomNode::area_trigger_exited_signal_callback(Node *area) {

	int res = area_trigger_entered_area_vec.find(area);
	if (res >= 0) {
		//printf("area exit...\n");
		area_trigger_entered_area_vec.erase(area);
	}
}

int GDIVisualScriptCustomNode::get_area_trigger_entered_area_num() const {

	return area_trigger_entered_area_vec.size();
}

bool GDIVisualScriptCustomNode::check_node_in_entered_areas(Node *node) {

	Node *parent = nullptr;
	auto size = area_trigger_entered_area_vec.size();
	for (int i = 0; i < size; ++i) {
		parent = area_trigger_entered_area_vec[i]->get_parent();
		if (area_trigger_entered_area_vec[i] == node ||
			parent == node) {
			return true;
		}
	}

	return false;
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

void GDIVisualScriptCustomNode::add_sub_task_index_and_objs_state(Vector<GDIVisualScriptCustomNode::RestoreInfo> &state_vec) {

	sub_tasks_objs_state_vec.push_back(state_vec);
}

void GDIVisualScriptCustomNode::set_sub_task_cur_index(unsigned int index) {

	sub_task_cur_index = index;
}

unsigned int GDIVisualScriptCustomNode::get_sub_task_cur_index() const {

	return sub_task_cur_index;
}

void GDIVisualScriptCustomNode::restore_sub_task_state(unsigned int index) {

	if (sub_tasks_objs_state_vec.size() - 1 >= index) {
		// 		printf("restore sub task index[%d], addr[%x]\n", index, this);
		auto objs_state = sub_tasks_objs_state_vec[index];
		auto size = objs_state.size();
		for (int i = 0; i < size; ++i) {
			/*if (String(obj->key()->get_name()) == String("Spatial")) {
				auto t = obj->key()->get_global_transform();
				printf("--rest1 transform:\n x1:%f, y1:%f, z1:%f\n x2:%f, y2:%f, z2:%f\n x3:%f, y3:%f, z3:%f\n", t.basis[0].x, t.basis[0].y, t.basis[0].z, t.basis[1].x, t.basis[1].y, t.basis[1].z, t.basis[2].x, t.basis[2].y, t.basis[2].z);
			}*/

			auto restInfo = objs_state[i];
			Spatial *spa = Object::cast_to<Spatial>(restInfo.node);
			if (nullptr != spa) {
				// must get transform once here, or the update will incorrect
				spa->get_transform();

				spa->set_global_transform(restInfo.trans);
				/*spa->force_update_transform();*/
			}
		}
	}
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

class GDIVisualScriptNodeInstanceCustom : public VisualScriptNodeInstance, public GDICustomNodeBase {
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

	// keyboard relevant----
	bool keyboard_already_pressed_flag = false;

	// timer relevant----
	uint64_t start_time = 0;
	bool reach_target_time_flag = false;

	// multi task split relevant----
	unsigned int task_split_cur_execute_index = 0;
	Vector<Vector<GDIVisualScriptCustomNode::RestoreInfo>> sub_task_objs_state_vec;

	// task flow control relevant----
	bool task_ctrl_already_execute_once_flag = false;

	// area trigger(and mouse) relevant----
	bool first_create_manual_area_flag = false;
	Area *manual_created_area = nullptr;
	CollisionObject *selected_area = nullptr;

	virtual int get_working_memory_size() const { return 1; }
	//virtual bool is_output_port_unsequenced(int p_idx) const { return false; }
	//virtual bool get_output_port_unsequenced(int p_idx,Variant* r_value,Variant* p_working_mem,String &r_error) const { return true; }

	int multi_task_split_func(const Variant **p_inputs, Variant **p_outputs, Variant::CallError &r_error, String &r_error_str) {

		// record the objs state
		if (sub_task_objs_state_vec.size() < node->get_task_split_num()) {
			Object *object = instance->get_owner_ptr();
			Node *node = Object::cast_to<Node>(object);
			if (nullptr == node) {
				r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
				r_error_str = "[GDI]multi task, can't get node";
				return 0;
			}

			Node *root = node->get_tree()->get_current_scene();
			if (nullptr == root) {
				r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
				r_error_str = "[GDI]multi task, can't get root";
				return 0;
			}

			//os->print("--------start collect sub task init info, task index[%d]\n", task_split_cur_execute_index);
			Vector<GDIVisualScriptCustomNode::RestoreInfo> spatial_trans_vec;
			collect_init_info(root, spatial_trans_vec);
			sub_task_objs_state_vec.push_back(spatial_trans_vec);

			// store the info to node
			GDIVisualScriptCustomNode *custom_node = Object::cast_to<GDIVisualScriptCustomNode>(this->node);
			if (nullptr != custom_node) {
				//os->print("set sub task index and objs state, index[%d] addr[%x]\n", task_split_cur_execute_index, custom_node);
				custom_node->add_sub_task_index_and_objs_state(spatial_trans_vec);
			}
		}

		*p_outputs[0] = node;

		bool execute_last_flag = false;
		if (task_split_cur_execute_index == node->get_task_split_num() - 1) {
			execute_last_flag = true;
		}
		unsigned int tmp_index = task_split_cur_execute_index++;
		if (task_split_cur_execute_index == node->get_task_split_num()) {
			task_split_cur_execute_index = 0;
		}

		return execute_last_flag ? tmp_index : (tmp_index | STEP_FLAG_PUSH_STACK_BIT);
	}

	int task_flow_control_func(const Variant **p_inputs, Variant **p_outputs, Variant* p_working_mem) {

		bool active_flag = *p_inputs[0];
		bool only_execute_once_flag = *p_inputs[1];

		if (!active_flag) {
			if (nullptr != p_working_mem) {
				p_working_mem[0] = STEP_EXIT_FUNCTION_BIT;
			}
			return STEP_EXIT_FUNCTION_BIT;
			//return 0 | STEP_FLAG_PUSH_STACK_BIT;
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

	int keyboard_handle_func(const Variant **p_inputs, Variant **p_outputs, Variant* p_working_mem, Variant::CallError &r_error, String &r_error_str) {

		String key = *p_inputs[0];
		if (nullptr == p_working_mem) {
			r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
			r_error_str = "[GDI]keyboard, working mem error";
			return 0;
		}

		auto sc = _os->find_scancode_from_string(key);
		auto pressedFlag = input->is_key_pressed(sc);
		*p_outputs[0] = pressedFlag;

		if (!keyboard_already_pressed_flag && pressedFlag) {
			keyboard_already_pressed_flag = true;
			return 0;
		}
		else if (keyboard_already_pressed_flag && pressedFlag) {
			return 1;
		}
		else if (keyboard_already_pressed_flag && !pressedFlag) {
			keyboard_already_pressed_flag = false;
			return 2;
		}

		p_working_mem[0] = STEP_EXIT_FUNCTION_BIT;
		return STEP_EXIT_FUNCTION_BIT;
	}
	
	void manual_generate_area(Spatial *target, const Vector3 min_pos, const Vector3 max_pos, bool dirty_flag) {

		Vector3 target_pos = target->get_global_transform().origin;
		Vector3 center_pos = dirty_flag ? ((max_pos + min_pos) / 2.0) : target_pos;
		Vector3 dir = center_pos - target_pos;
		float diff_len = dir.length();
		dir.normalize();
		// the 0.1 extents prevents the null area, if no valid mesh under the node
		Vector3 extents = dirty_flag ? ((max_pos - center_pos) + Vector3(0.1, 0.1, 0.1)) : Vector3(0.1, 0.1, 0.1);

		manual_created_area = memnew(Area);
		manual_created_area->set_name("gdi_visual_node_manual_area");

		CollisionShape *cs = memnew(CollisionShape);
		BoxShape *bs = memnew(BoxShape);
		bs->set_extents(extents);

		manual_created_area->add_child(cs);
		target->add_child(manual_created_area);
		cs->set_shape(bs);

		if (dirty_flag) {
			manual_created_area->translate(dir * diff_len);
		}
		//os->print("[GDI]attach the Area node manual, dirty flag[%d], extents[%f][%f][%f]\n", dirty_flag, extents.x, extents.y, extents.z);
	}
	void check_self_mesh_area_aabb(Node *target, Vector3 &min_pos, Vector3 &max_pos, bool &dirty_flag, bool &has_area_flag) {

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

		check_self_mesh_area_aabb(target, min_pos, max_pos, dirty_flag, has_area_flag);

		if (!first_create_manual_area_flag) {
			// add the Area node with collision shape dynamicly
			if (!has_area_flag) {
				// check child mesh and area situation
				check_child_mesh_area_func(target, dirty_flag, has_area_flag, min_pos, max_pos);
				manual_generate_area(target, min_pos, max_pos, dirty_flag);
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

			first_create_manual_area_flag = true;
		}

		//bool area_entered_flag = this->node->get_area_trigger_entered_area_num() > 0 ? true : false;
		bool area_entered_flag = this->node->check_node_in_entered_areas(target);
		*p_outputs[0] = area_entered_flag;

		return (area_entered_flag ? 0 : 1);
	}

	int timer_handle_func(const Variant **p_inputs, Variant **p_outputs) {

		int minute = *p_inputs[0];
		int sec = *p_inputs[1];
		int msec = *p_inputs[2];

		if (0 == start_time) {
			start_time = os->get_system_time_msecs();
		}

		reach_target_time_flag =
			(os->get_system_time_msecs() - start_time >= (msec + sec * 1000 + minute * 1000 * 60)) ? true : false;

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

	void collect_init_info(Node *node, Vector<GDIVisualScriptCustomNode::RestoreInfo> &objs_init_trans_vec) {

		Spatial *spa = Object::cast_to<Spatial>(node);
		if (nullptr != spa) {
			objs_init_trans_vec.push_back(GDIVisualScriptCustomNode::RestoreInfo(spa, spa->get_global_transform()));
			//os->print("[GDI]insert init trans info, node name[%S], addr[%X]\n", String(spa->get_name()), spa);

			//if (String(spa->get_name()) == String("Spatial")) {
			//	auto t = spa->get_global_transform();
			//	os->print("transform:\n x1:%f, y1:%f, z1:%f\n x2:%f, y2:%f, z2:%f\n x3:%f, y3:%f, z3:%f\n", t.basis[0].x, t.basis[0].y, t.basis[0].z, t.basis[1].x, t.basis[1].y, t.basis[1].z, t.basis[2].x, t.basis[2].y, t.basis[2].z);
			//}
		}

		int child_num = node->get_child_count();
		for (int i = 0; i < child_num; ++i) {
			Node *child = node->get_child(i);
			int child_child_num = child->get_child_count();
			if (child_child_num > 0) {
				collect_init_info(child, objs_init_trans_vec);
			}
			else {
				spa = Object::cast_to<Spatial>(child);
				if (nullptr != spa) {
					objs_init_trans_vec.push_back(GDIVisualScriptCustomNode::RestoreInfo(spa, spa->get_global_transform()));
					//os->print("[GDI]insert init trans info, node name[%S], addr[%X]\n", String(spa->get_name()), spa);

					//if (String(spa->get_name()) == String("Spatial")) {
					//	auto t = spa->get_global_transform();
					//	os->print("transform:\n x1:%f, y1:%f, z1:%f\n x2:%f, y2:%f, z2:%f\n x3:%f, y3:%f, z3:%f\n", t.basis[0].x, t.basis[0].y, t.basis[0].z, t.basis[1].x, t.basis[1].y, t.basis[1].z, t.basis[2].x, t.basis[2].y, t.basis[2].z);
					//}
				}
			}
		}
	}

	virtual int step(const Variant **p_inputs, Variant **p_outputs, StartMode p_start_mode, Variant *p_working_mem, Variant::CallError &r_error, String &r_error_str) {

		// initialize relevant objs only once
		static bool init_objs_info_flag = false;
		static Vector<GDIVisualScriptCustomNode::RestoreInfo> objs_init_trans_vec;
		if (!init_objs_info_flag || objs_init_trans_vec.size() == 0) {
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
			collect_init_info(root, objs_init_trans_vec);
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

			int ret = multi_task_split_func(p_inputs, p_outputs, r_error, r_error_str);
			return ret;
		}
		case GDIVisualScriptCustomNode::TASK_CONTROL: {

			int ret = task_flow_control_func(p_inputs, p_outputs, p_working_mem);
			return ret;
		}
		case GDIVisualScriptCustomNode::KEYBOARD: {

			int ret = keyboard_handle_func(p_inputs, p_outputs, p_working_mem, r_error, r_error_str);
			return ret;
		}
		//case GDIVisualScriptCustomNode::MOUSE: {

		//	int ret = mouse_handle_func(p_inputs, p_outputs, p_working_mem, p_start_mode, r_error, r_error_str);
		//	return ret;
		//}
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

			Object *object = instance->get_owner_ptr();
			Node *node = Object::cast_to<Node>(object);
			if (nullptr != node) {
				node->get_tree()->reload_current_scene();
 				//os->print("reload cur scene.......\n");
			}

 			//auto size = objs_init_trans_vec.size();
 			//for (int i = 0; i < size; ++i) {
 			//	auto restInfo = objs_init_trans_vec[i];
 			//	Spatial *spa = Object::cast_to<Spatial>(restInfo.node);
 			//	if (nullptr != spa) {
 			//		spa->set_global_transform(restInfo.trans);
 			//	}
 			//}
			break;
		}
		case GDIVisualScriptCustomNode::INIT_PARTIAL: {

			GDIVisualScriptCustomNode *customNode = Object::cast_to<GDIVisualScriptCustomNode>(*p_inputs[0]);
			if (nullptr == customNode) {
				r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
				r_error_str = "[GDI]init partial failed, can't convert node to custom node";
				return 0;
			}

			unsigned int index = *p_inputs[1];
			customNode->restore_sub_task_state(index);

			break;
		}
		// 先保留下，很有可能后面又会加入这种简化版的节点
 		//case GDIVisualScriptCustomNode::MAT_ALBEDO: {
 		//	Object *object = instance->get_owner_ptr();
 		//	Node *node = Object::cast_to<Node>(object);
 		//	if (nullptr == node) {
  	//			os->print("[GDI]Material albedo node, can not convert obj to node\n");
 		//		r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
 		//		r_error_str = "[GDI]Material albedo node, can not convert obj to node";
 		//		return 0;
 		//	}
 
 		//	NodePath path = *p_inputs[0];
 		//	Node *target_node = node->get_node(path);
 		//	if (nullptr == target_node) {
  	//			os->print("[GDI]Material albedo node, target node is null\n");
 		//		r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
 		//		r_error_str = "[GDI]Material albedo node, target node is null";
 		//		return 0;
 		//	}
 
 		//	MeshInstance *mesh = Object::cast_to<MeshInstance>(target_node);
 		//	if (nullptr == mesh) {
  	//			os->print("[GDI]Material albedo node, can not convert to mesh inst\n");
 		//		r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
 		//		r_error_str = "[GDI]Material albedo node, can not convert to mesh inst";
 		//		return 0;
 		//	}
 		//	if (mesh->get_surface_material_count() == 0) {
  	//			os->print("[GDI]Material albedo node, surface is null\n");
 		//		r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
 		//		r_error_str = "[GDI]Material albedo node, surface is null";
 		//		return 0;
 		//	}
 
 		//	unsigned int surface_index = *p_inputs[2];
 		//	if (surface_index > mesh->get_surface_material_count() - 1) {
  	//			os->print("[GDI]Material albedo node, invalid surface index\n");
 		//		r_error.error = Variant::CallError::CALL_ERROR_INVALID_ARGUMENT;
 		//		r_error_str = "[GDI]Material albedo node, invalid surface index";
 		//		return 0;
 		//	}
 
 		//	auto mat = Object::cast_to<SpatialMaterial>(*(mesh->get_surface_material(surface_index)));
 		//	if (nullptr == mat) {
  	//			os->print("[GDI]Material albedo node, can not find material with index[%d]\n", surface_index);
 		//		r_error.error = Variant::CallError::CALL_ERROR_INVALID_ARGUMENT;
 		//		r_error_str = "[GDI]Material albedo node, can not find material with index";
 		//		return 0;
 		//	}
 
 		//	mat->set_albedo(*p_inputs[1]);
 		//	mesh->set_surface_material(surface_index, mat);
 
 		//	break;
 		//}
		}

		if (!validate) {

			//ignore call errors if validation is disabled
			r_error.error = Variant::CallError::CALL_OK;
			r_error_str = String();
		}

		return 0;
	}
};

VisualScriptNodeInstance * GDIVisualScriptCustomNode::instance(VisualScriptInstance *p_instance) {

	GDIVisualScriptNodeInstanceCustom *instance = memnew(GDIVisualScriptNodeInstanceCustom);
	instance->node = this;
	instance->instance = p_instance;
	instance->singleton = singleton;
	instance->function = function;
	instance->custom_mode = get_custom_mode();
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
	sub_task_cur_index = 0;
}

GDIVisualScriptCustomNode::~GDIVisualScriptCustomNode() {

	if (TASK_CONTROL == custom_mode) {
		--global_task_id;
	}
}

unsigned int GDIVisualScriptCustomNode::global_task_id = 1;

// ----------------------------------------------Mouse relevant
class GDIVisualScriptNodeInstanceCustomMouse :
	public VisualScriptNodeInstance, public GDICustomNodeBase, public Object {
public:
	int input_args;
	bool validate;
	int returns;

	GDIVisualScriptCustomNodeMouse *node;
	VisualScriptInstance *instance;

	// ----mouse key states relevant
	uint64_t time = 0;

	Point2 mouse_point_left;
	Point2 mouse_point_right;
	Point2 mouse_point_mid;

	bool drag_flag = false;
	// the reason why I set the flags separately, is to prevent the planner change her mind
	// she may wants to support the keys at same time
	bool first_left_pressed_flag = false;
	bool first_left_released_flag = false;
	bool first_left_click_flag = false;
	bool second_left_pressed_flag = false;

	bool first_right_pressed_flag = false;
	bool first_right_released_flag = false;
	bool first_right_click_flag = false;
	bool second_right_pressed_flag = false;

	bool first_mid_pressed_flag = false;
	bool first_mid_released_flag = false;
	bool first_mid_click_flag = false;
	bool second_mid_pressed_flag = false;

	bool mouse_key_down_flag = false;// use this to do the single click check separately(not mix with double click)

	const unsigned int double_click_interval = 500;

	// ----ray intersect
	Spatial *target = nullptr;
	Viewport *view_port = nullptr;
	Camera *cam = nullptr;
	PhysicsDirectSpaceState *state = nullptr;
	Area *manual_created_area = nullptr;
	CollisionObject *selected_area = nullptr;
	bool first_create_manual_area_flag = false;

	// -----miscellaneous
	String key_name;
	GDIVisualScriptCustomNodeMouse::MouseKey key_type;

	void manual_generate_area(Spatial *target, const Vector3 min_pos, const Vector3 max_pos, bool dirty_flag) {

		Vector3 target_pos = target->get_global_transform().origin;
		Vector3 center_pos = dirty_flag ? ((max_pos + min_pos) / 2.0) : target_pos;
		Vector3 dir = center_pos - target_pos;
		float diff_len = dir.length();
		dir.normalize();
		// the 0.1 extents prevents the null area, if no valid mesh under the node
		Vector3 extents = dirty_flag ? ((max_pos - center_pos) + Vector3(0.1, 0.1, 0.1)) : Vector3(0.1, 0.1, 0.1);

		manual_created_area = memnew(Area);
		manual_created_area->set_name("gdi_visual_node_manual_area");

		CollisionShape *cs = memnew(CollisionShape);
		BoxShape *bs = memnew(BoxShape);
		bs->set_extents(extents);

		manual_created_area->add_child(cs);
		target->add_child(manual_created_area);
		cs->set_shape(bs);

		if (dirty_flag) {
			manual_created_area->translate(dir * diff_len);
		}
		//os->print("[GDI]attach the Area node manual, dirty flag[%d], extents[%f][%f][%f]\n", dirty_flag, extents.x, extents.y, extents.z);
	}
	void check_self_mesh_area_aabb(Node *target, Vector3 &min_pos, Vector3 &max_pos, bool &dirty_flag, bool &has_area_flag) {

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
	void check_mouse_ray_intersect(Variant **p_outputs) {

		if (nullptr == view_port) {
			Object *object = instance->get_owner_ptr();
			Node *node = Object::cast_to<Node>(object);
			view_port = (nullptr != node) ? node->get_tree()->get_root() : nullptr;

			if (nullptr != view_port) {
				auto world = view_port->get_world();
				state = (nullptr != *world) ? world->get_direct_space_state() : nullptr;
				cam = view_port->get_camera();
			}
		}

		if (nullptr != view_port && nullptr != state && nullptr != cam) {
			auto from = cam->project_ray_origin(os->get_mouse_position());
			auto to = from + cam->project_ray_normal(os->get_mouse_position()) * 10000.0;

			PhysicsDirectSpaceState::RayResult res;
			bool bRes = state->intersect_ray(from, to, res, Set<RID>(), 0xFFFFFFFF, true, true, true);
			if (bRes) {
				CollisionObject *co = Object::cast_to<CollisionObject>(res.collider);
				//printf("intersect with object[%S]\n", String(co->get_name()));

				if (nullptr != selected_area &&
					co->get_name() == selected_area->get_name() &&
					co->get_global_transform() == selected_area->get_global_transform()) {
					//*p_outputs[3] = co;
					*p_outputs[3] = true;
					//printf("ray match succeed1...\n");
				}
				else if (nullptr != manual_created_area &&
					co->get_name() == manual_created_area->get_name() &&
					co->get_global_transform() == manual_created_area->get_global_transform()) {
					//*p_outputs[3] = co;
					*p_outputs[3] = true;
					//printf("ray match succeed2...\n");
				}
				else {
					//*p_outputs[3] = (Object*)nullptr;
					*p_outputs[3] = false;
				}
			}
			//printf("test intersect ray enter..............res[%d], shape[%d]\n from[%f][%f][%f]---to[%f][%f][%f]", bRes, res.shape, from[0], from[1], from[2], to.x, to.y, to.z);
		}
	}
	int mouse_handle_func(const Variant **p_inputs, Variant **p_outputs, Variant* p_working_mem, StartMode p_start_mode, Variant::CallError &r_error, String &r_error_str) {

		if (nullptr == p_working_mem) {
			r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
			r_error_str = "[GDI]mouse, working mem error";
			return 0;
		}

		// check input mouse key name
		if (key_name == String()) {
			key_name = *p_inputs[0];
			//printf("test key name[%S]\n", key_name);

			if (key_name == std::to_string(GDIVisualScriptCustomNodeMouse::LEFT).c_str()) {
				key_type = GDIVisualScriptCustomNodeMouse::LEFT;
			}
			else if (key_name == std::to_string(GDIVisualScriptCustomNodeMouse::RIGHT).c_str()) {
				key_type = GDIVisualScriptCustomNodeMouse::RIGHT;
			}
			else if (key_name == std::to_string(GDIVisualScriptCustomNodeMouse::MID).c_str()) {
				key_type = GDIVisualScriptCustomNodeMouse::MID;
			}
			else {
				r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
				r_error_str = "[GDI]mouse, key name error";
				return 0;
			}
		}

		// get all keys flag
		auto left_button_pressed_flag = input->is_mouse_button_pressed(BUTTON_LEFT);
		auto right_button_pressed_flag = input->is_mouse_button_pressed(BUTTON_RIGHT);
		auto mid_button_pressed_flag = input->is_mouse_button_pressed(BUTTON_MIDDLE);
		auto mouse_btn_mask = input->gdi_get_mouse_button_mask();
		int wheel_value = 0;
		if ((mouse_btn_mask & (1 << (BUTTON_WHEEL_UP - 1))) != 0) {
			wheel_value = 1;
		}
		else if ((mouse_btn_mask & (1 << (BUTTON_WHEEL_DOWN - 1))) != 0) {
			wheel_value = -1;
		}

		// output the first param(pressed)
		if (key_type == GDIVisualScriptCustomNodeMouse::LEFT) {
			*p_outputs[0] = left_button_pressed_flag;
		}
		else if (key_type == GDIVisualScriptCustomNodeMouse::RIGHT) {
			*p_outputs[0] = right_button_pressed_flag;
		}
		else if (key_type == GDIVisualScriptCustomNodeMouse::MID) {
			*p_outputs[0] = mid_button_pressed_flag;
		}

		*p_outputs[1] = wheel_value;
		*p_outputs[2] = os->get_mouse_position();
		*p_outputs[3] = false;

		// check area path, if null, we will create it manual
		if (nullptr == target) {
			target = Object::cast_to<Spatial>((Object*)*p_inputs[1]);
		}
		auto area_path = this->node->get_mouse_pick_area_path();
		Object *object = instance->get_owner_ptr();
		Node *node = Object::cast_to<Node>(object);
		if (area_path != NodePath() && nullptr != node) {
			selected_area = Object::cast_to<CollisionObject>(node->get_node(area_path));
		}

		if (area_path == NodePath() && nullptr != target) {
			//printf("area path is null.....\n");

			// calculate the comb aabb manual
			int child_num = target->get_child_count();
			Vector3 min_pos, max_pos;
			bool dirty_flag = false;
			bool has_area_flag = false;

			check_self_mesh_area_aabb(target, min_pos, max_pos, dirty_flag, has_area_flag);

			if (!first_create_manual_area_flag) {
				// add the Area node with collision shape dynamicly
				if (!has_area_flag) {
					// check child mesh and area situation, 
// 					check_child_mesh_area_func(target, dirty_flag, has_area_flag, min_pos, max_pos);
					manual_generate_area(target, min_pos, max_pos, dirty_flag);
					first_create_manual_area_flag = true;
				}
			}
		}

		// check ray intersect
		check_mouse_ray_intersect(p_outputs);

		// ----mouse wheel
		if (mouse_btn_mask != 0) {
			input->gdi_reset_mouse_button_mask();
			if ((mouse_btn_mask & (1 << (BUTTON_WHEEL_UP - 1))) != 0 ||
				(mouse_btn_mask & (1 << (BUTTON_WHEEL_DOWN - 1))) != 0) {
				return 3;
			}
		}

		bool double_click_flag = false;
		drag_flag = false;

		if (key_type == GDIVisualScriptCustomNodeMouse::LEFT) {

			// ----left button relevant
			// check drag first
			if (first_left_pressed_flag && left_button_pressed_flag && mouse_point_left != os->get_mouse_position()) {
				drag_flag = true;
			}
			else {
				if (!first_left_pressed_flag && left_button_pressed_flag) {
					first_left_pressed_flag = true;
					time = os->get_system_time_msecs();
					// prevent drag double click
					mouse_point_left = os->get_mouse_position();
				}
				else if (first_left_pressed_flag && !first_left_released_flag && !left_button_pressed_flag) {
					first_left_released_flag = true;
				}
				else if (first_left_released_flag && left_button_pressed_flag) {
					second_left_pressed_flag = true;
				}
				else if (second_left_pressed_flag && !left_button_pressed_flag) {
					auto diff_time = os->get_system_time_msecs() - time;
					// 			os->print("enter second click, diff time[%d]\n", diff_time);
					if (diff_time < double_click_interval && mouse_point_left == os->get_mouse_position()) {
						double_click_flag = true;
					}

					time = 0;
					second_left_pressed_flag = false;
					first_left_pressed_flag = false;
					first_left_released_flag = false;
				}
			}

			// double click
			if (double_click_flag) {
				return 1;
			}
			// drag
			else if (first_left_pressed_flag && !first_left_released_flag && mouse_point_left != os->get_mouse_position()) {
				return 2;
			}
			// left click
// 			else if (first_left_released_flag && !first_left_click_flag && mouse_point_left == os->get_mouse_position()) {
// 				first_left_click_flag = true;
// 				return 0;
// 			}

			if (!mouse_key_down_flag && left_button_pressed_flag) {
				mouse_key_down_flag = true;
			}
			else if (!left_button_pressed_flag && mouse_key_down_flag) {
				mouse_key_down_flag = false;
				return 0;
			}
		}
		else if (key_type == GDIVisualScriptCustomNodeMouse::RIGHT) {

			// ----right button relevant
			// check drag first
			if (first_right_pressed_flag && right_button_pressed_flag && mouse_point_right != os->get_mouse_position()) {
				drag_flag = true;
			}
			else {
				if (!first_right_pressed_flag && right_button_pressed_flag) {
					first_right_pressed_flag = true;
					time = os->get_system_time_msecs();
					// prevent drag double click
					mouse_point_right = os->get_mouse_position();
				}
				else if (first_right_pressed_flag && !first_right_released_flag && !right_button_pressed_flag) {
					first_right_released_flag = true;
				}
				else if (first_right_released_flag && right_button_pressed_flag) {
					second_right_pressed_flag = true;
				}
				else if (second_right_pressed_flag && !right_button_pressed_flag) {
					auto diff_time = os->get_system_time_msecs() - time;
					// 			os->print("enter second click, diff time[%d]\n", diff_time);
					if (diff_time < double_click_interval && mouse_point_right == os->get_mouse_position()) {
						double_click_flag = true;
					}

					time = 0;
					second_right_pressed_flag = false;
					first_right_pressed_flag = false;
					first_right_released_flag = false;
				}
			}

			// double click
			if (double_click_flag) {
				return 1;
			}
			// drag
			else if (first_right_pressed_flag && !first_right_released_flag && mouse_point_right != os->get_mouse_position()) {
				return 2;
			}
			// right click
// 			else if (first_right_released_flag && !first_right_click_flag && mouse_point_right == os->get_mouse_position()) {
// 				first_right_click_flag = true;
// 				return 0;
// 			}

			if (!mouse_key_down_flag && right_button_pressed_flag) {
				mouse_key_down_flag = true;
			}
			else if (!right_button_pressed_flag && mouse_key_down_flag) {
				mouse_key_down_flag = false;
				return 0;
			}
		}
		else if (key_type == GDIVisualScriptCustomNodeMouse::MID) {

			// ----mid button relevant(preserve)
			// check drag first
			if (first_mid_pressed_flag && mid_button_pressed_flag && mouse_point_mid != os->get_mouse_position()) {
				drag_flag = true;
			}
			else {
				if (!first_mid_pressed_flag && mid_button_pressed_flag) {
					first_mid_pressed_flag = true;
					time = os->get_system_time_msecs();
					// prevent drag double click
					mouse_point_mid = os->get_mouse_position();
				}
				else if (first_mid_pressed_flag && !first_mid_released_flag && !mid_button_pressed_flag) {
					first_mid_released_flag = true;
				}
				else if (first_mid_released_flag && mid_button_pressed_flag) {
					second_mid_pressed_flag = true;
				}
				else if (second_mid_pressed_flag && !mid_button_pressed_flag) {
					auto diff_time = os->get_system_time_msecs() - time;
					// 			os->print("enter second click, diff time[%d]\n", diff_time);
					if (diff_time < double_click_interval && mouse_point_mid == os->get_mouse_position()) {
						double_click_flag = true;
					}

					time = 0;
					second_mid_pressed_flag = false;
					first_mid_pressed_flag = false;
					first_mid_released_flag = false;
				}
			}

			// double click
			if (double_click_flag) {
				return 1;
			}
			// drag
			else if (first_mid_pressed_flag && !first_mid_released_flag && mouse_point_mid != os->get_mouse_position()) {
				return 2;
			}
			// mid click
// 			else if (first_mid_released_flag && !first_mid_click_flag && mouse_point_mid == os->get_mouse_position()) {
// 				first_mid_click_flag = true;
// 				return 0;
// 			}

			if (!mouse_key_down_flag && mid_button_pressed_flag) {
				mouse_key_down_flag = true;
			}
			else if (!mid_button_pressed_flag && mouse_key_down_flag) {
				mouse_key_down_flag = false;
				return 0;
			}
		}

		// ----reset, if time over
		if (os->get_system_time_msecs() - time >= double_click_interval) {
			time = 0;
			second_left_pressed_flag = false;
			first_left_click_flag = false;

			second_right_pressed_flag = false;
			first_right_click_flag = false;

			second_mid_pressed_flag = false;
			first_mid_click_flag = false;

			if (!drag_flag) {
				first_left_pressed_flag = false;
				first_left_released_flag = false;

				first_right_pressed_flag = false;
				first_right_released_flag = false;

				first_mid_pressed_flag = false;
				first_mid_released_flag = false;
			}
		}

		p_working_mem[0] = STEP_EXIT_FUNCTION_BIT;
		return STEP_EXIT_FUNCTION_BIT;
	}

	virtual int step(const Variant **p_inputs, Variant **p_outputs, StartMode p_start_mode, Variant *p_working_mem, Variant::CallError &r_error, String &r_error_str) override	{

		int ret = mouse_handle_func(p_inputs, p_outputs, p_working_mem, p_start_mode, r_error, r_error_str);
		return ret;
	}

	virtual int get_working_memory_size() const override {
		return 1;
	}

};

void GDIVisualScriptCustomNodeMouse::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_mouse_pick_area_path", "path"), &GDIVisualScriptCustomNodeMouse::set_mouse_pick_area_path);
	ClassDB::bind_method(D_METHOD("get_mouse_pick_area_path"), &GDIVisualScriptCustomNodeMouse::get_mouse_pick_area_path);

	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, L"(鼠标)拣选碰撞区", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Area,KinematicBody,PhysicalBone,RigidBody,VehicleBody,StaticBody"), "set_mouse_pick_area_path", "get_mouse_pick_area_path");
}

String GDIVisualScriptCustomNodeMouse::get_mouse_key_string(MouseKey key) const {

	return mouse_key_list[key];
}

int GDIVisualScriptCustomNodeMouse::get_output_sequence_port_count() const {

	return 4;
}

bool GDIVisualScriptCustomNodeMouse::has_input_sequence_port() const {

	return true;
}

String GDIVisualScriptCustomNodeMouse::get_output_sequence_port_text(int p_port) const {

	switch (p_port)
	{
	case 0: {
		return L"单击";
	}
	case 1: {
		return L"双击";
	}
	case 2: {
		return L"拖动";
	}
	case 3: {
		return L"滚轮";
	}
	default:
		return L"未处理索引";
	}
}

int GDIVisualScriptCustomNodeMouse::get_input_value_port_count() const {

	return 2;
}

int GDIVisualScriptCustomNodeMouse::get_output_value_port_count() const {

	return 4;
}

PropertyInfo GDIVisualScriptCustomNodeMouse::get_input_value_port_info(int p_idx) const {

	PropertyInfo pi;

	switch (p_idx)
	{
	case 0: {
		pi.name = L"键位选择";
		pi.type = Variant::INT;
		pi.hint = PROPERTY_HINT_ENUM;
		pi.hint_string = mouse_key_list[LEFT] + "," + mouse_key_list[RIGHT] + "," + mouse_key_list[MID];
		break;
	}
	case 1: {
		pi.name = L"拣选节点";
		pi.type = Variant::OBJECT;
		break;
	}
	default:
		break;
	}

	return pi;
}

PropertyInfo GDIVisualScriptCustomNodeMouse::get_output_value_port_info(int p_idx) const {

	PropertyInfo pi;

	switch (p_idx)
	{
	case 0: {
		pi.name = L"按下";
		pi.type = Variant::BOOL;
		break;
	}
	case 1: {
		pi.name = L"滚轮值";
		pi.type = Variant::INT;
		break;
	}
	case 2: {
		pi.name = L"鼠标坐标";
		pi.type = Variant::VECTOR2;
		break;
	}
	case 3: {
		pi.name = L"拣选标识";
		pi.type = Variant::BOOL;
		break;
	}
	default: {
		pi.name = L"未处理类型";
		pi.type = Variant::NIL;
		break;
	}
	}

	return pi;
}

String GDIVisualScriptCustomNodeMouse::get_caption() const {

	return L"鼠标";
}

String GDIVisualScriptCustomNodeMouse::get_category() const {

	return L"custom";
}

String GDIVisualScriptCustomNodeMouse::get_text() const {

	return L"鼠标按键处理";
}

void GDIVisualScriptCustomNodeMouse::set_mouse_pick_area_path(const NodePath &path) {

	mouse_pick_area_path = path;
}

NodePath GDIVisualScriptCustomNodeMouse::get_mouse_pick_area_path() const {

	return mouse_pick_area_path;
}

VisualScriptNodeInstance * GDIVisualScriptCustomNodeMouse::instance(VisualScriptInstance *p_instance) {

	GDIVisualScriptNodeInstanceCustomMouse *instance = memnew(GDIVisualScriptNodeInstanceCustomMouse);
	instance->node = this;
	instance->instance = p_instance;
	instance->returns = get_output_value_port_count();
	instance->input_args = get_input_value_port_count();
	return instance;
}

GDIVisualScriptCustomNodeMouse::GDIVisualScriptCustomNodeMouse() {

}

GDIVisualScriptCustomNodeMouse::~GDIVisualScriptCustomNodeMouse() {

}

// ----------------------------------Multi Player(sync)
void GDIVisualScriptNodeInstanceCustomMultiPlayer::_bind_methods() {

	ClassDB::bind_method(D_METHOD("peer_connected", "id"), &GDIVisualScriptNodeInstanceCustomMultiPlayer::peer_connected);
	ClassDB::bind_method(D_METHOD("peer_disconnected", "id"), &GDIVisualScriptNodeInstanceCustomMultiPlayer::peer_disconnected);

	ClassDB::bind_method(D_METHOD("client_connected_to_server"), &GDIVisualScriptNodeInstanceCustomMultiPlayer::client_connected_to_server);
	ClassDB::bind_method(D_METHOD("client_connection_failed"), &GDIVisualScriptNodeInstanceCustomMultiPlayer::client_connection_failed);
	ClassDB::bind_method(D_METHOD("client_server_disconnected"), &GDIVisualScriptNodeInstanceCustomMultiPlayer::client_server_disconnected);

	ClassDB::bind_method(D_METHOD("rpc_call_test_func"), &GDIVisualScriptNodeInstanceCustomMultiPlayer::rpc_call_test_func);
// 	ClassDB::add_virtual_method("GDIVisualScriptNodeInstanceCustomMultiPlayer", MethodInfo(Variant::NIL, "rpc_call_test_func"));
// 	ClassDB::add_virtual_method("Node", MethodInfo(Variant::NIL, "rpc_call_test_func"));
// 	ClassDB::add_virtual_method("VisualScriptInstance", MethodInfo(Variant::NIL, "rpc_call_test_func"));
}

GDIVisualScriptNodeInstanceCustomMultiPlayer::GDIVisualScriptNodeInstanceCustomMultiPlayer() {

	multi_player_enet.instance();
	server.instance();
	client.instance();
}


GDIVisualScriptNodeInstanceCustomMultiPlayer& GDIVisualScriptNodeInstanceCustomMultiPlayer::get_singleton() {

	static GDIVisualScriptNodeInstanceCustomMultiPlayer inst;
	return inst;
}
GDIVisualScriptNodeInstanceCustomMultiPlayer::~GDIVisualScriptNodeInstanceCustomMultiPlayer() {

}

void GDIVisualScriptNodeInstanceCustomMultiPlayer::peer_connected(int id) {

	os->print("peer connected[%d]..\n", id);
	// 	connection_status = CONNECTED;
}

void GDIVisualScriptNodeInstanceCustomMultiPlayer::peer_disconnected(int id) {

	os->print("peer disconnected[%d]..\n", id);
}

void GDIVisualScriptNodeInstanceCustomMultiPlayer::client_connected_to_server() {

	os->print("client, connected to server\n");
	Node *node = Object::cast_to<Node>(instance->get_owner_ptr());
	if (nullptr == node) {
		return;
	}

// 	node->rpc("rpc_call_test_func");
}

void GDIVisualScriptNodeInstanceCustomMultiPlayer::client_connection_failed() {

	os->print("client, connection failed\n");
}

void GDIVisualScriptNodeInstanceCustomMultiPlayer::client_server_disconnected() {

	os->print("client, server disconnected\n");
}

void GDIVisualScriptNodeInstanceCustomMultiPlayer::rpc_call_test_func() {

	os->print("rpc test call func\n");
}

int GDIVisualScriptNodeInstanceCustomMultiPlayer::server_create_func(const Variant **p_inputs, Variant **p_outputs, StartMode p_start_mode, Variant *p_working_mem, Variant::CallError &r_error, String &r_error_str) {

	int port = *p_inputs[2];
	int connect_num = *p_inputs[3];
	static const unsigned int max_connect = 32;
	if (connect_num > max_connect) {
		r_error.error = Variant::CallError::CALL_ERROR_INVALID_ARGUMENT;
		r_error_str = "[GDI]max connect num is 32";
		return 0;
	}

	// method1: use multi player(gds)
// 	Error err = multi_player_enet->create_server(port, connect_num);
// 	if (Error::OK != err) {
// 		create_succeed_flag = false;
// 
// 		r_error.error = Variant::CallError::CALL_ERROR_INVALID_ARGUMENT;
// 		r_error_str = "[GDI]create server failed";
// 		if (0 == port || port > 0xFF) {
// 			r_error_str = "[GDI]invalid port num";
// 		}
// 
// 		return 0;
// 	}
// 
// 	create_succeed_flag = true;
// 	os->print("create server succeed\n");

	// method2: use low-level tcp
	Error err = server->listen(port);
	if (Error::OK == err) {
		create_succeed_flag = true;
		os->print("create server succeed\n");
	}

	return 0;
}

int GDIVisualScriptNodeInstanceCustomMultiPlayer::client_create_func(const Variant **p_inputs, Variant **p_outputs, StartMode p_start_mode, Variant *p_working_mem, Variant::CallError &r_error, String &r_error_str) {

	String ip = *p_inputs[1];
	int port = *p_inputs[2];
	int pw = *p_inputs[4];

	// method1: use multi player
// 	Error err = multi_player_enet->create_client(ip, port);
// 	if (Error::OK != err) {
// 		create_succeed_flag = false;
// 
// 		r_error.error = Variant::CallError::CALL_ERROR_INVALID_ARGUMENT;
// 		r_error_str = "[GDI]create client failed";
// 		if (0 == port || port > 0xFF) {
// 			r_error_str = "[GDI]invalid port num";
// 		}
// 		if (String() == ip) {
// 			r_error_str = "[GDI]invalid ip str";
// 		}
// 
// 		return 0;
// 	}

	// method2: use tcp peer
	Error err = client->connect_to_host(ip, port);
	if (err != Error::OK) {
		r_error.error = Variant::CallError::CALL_ERROR_INVALID_ARGUMENT;
		r_error_str = "[GDI]create client failed";

		if (0 == port || port > 0xFF) {
			r_error_str = "[GDI]invalid port num";
		}
		if (String() == ip) {
			r_error_str = "[GDI]invalid ip str";
		}

		return 0;
	}

	Array arr;
	arr.push_back((int)C2S_CLIENT_VERIFY_PW);
	arr.push_back(pw);
	client->put_var(arr);
	create_succeed_flag = true;
	os->print("create client succeed\n");

	return 0;
}

void GDIVisualScriptNodeInstanceCustomMultiPlayer::generate_all_nodes_sync_data_info(Node *node) {

	SyncDataInfo sdi(node);
	stored_sync_data_info_map.insert(node, sdi);

	auto child_count = node->get_child_count();
	if (child_count > 0) {
		for (int i = 0; i < child_count; ++i) {
			Node *child = node->get_child(i);
			generate_all_nodes_sync_data_info(child);
		}
	}
}

void GDIVisualScriptNodeInstanceCustomMultiPlayer::update_all_nodes_sync_data_info(Node *node) {

	SyncDataInfo sdi(node);
	auto e = stored_sync_data_info_map.find(node);
	if (nullptr != e && !(e->value() == sdi)) {
		//update_stored_nodes_info(e->value(), sdi);
		e->value() = sdi;
		changed_data_info_vec.push_back(sdi);
		//os->print("sync data changed\n");
	}

	auto child_count = node->get_child_count();
	if (child_count > 0) {
		for (int i = 0; i < child_count; ++i) {
			Node *child = node->get_child(i);
			update_all_nodes_sync_data_info(child);
		}
	}
}

void GDIVisualScriptNodeInstanceCustomMultiPlayer::update_stored_nodes_info(SyncDataInfo &stored, SyncDataInfo &changed) {

	// due to the mechanism of material, we should use the old albedo instead of the new one
	Vector<Color> albedo_vec = stored.albedo_vec;
	stored = changed;
	changed.albedo_vec = albedo_vec;
}

Node* GDIVisualScriptNodeInstanceCustomMultiPlayer::find_node_with_id_and_name(uint64_t id, const String &name) {

	for (auto e = stored_sync_data_info_map.front(); e; e = e->next()) {
		if (e->value().instance_id == id && e->value().name == name) {
			return e->key();
		}
	}

	return nullptr;
}

void GDIVisualScriptNodeInstanceCustomMultiPlayer::sync_data_with_node(Node *node, SyncDataInfo sdi) {

	// update the transform, visible
	Spatial *spatial = Object::cast_to<Spatial>(node);
	if (nullptr != spatial) {
		spatial->set_global_transform(sdi.transform);
		spatial->set_visible(sdi.visible);
	}

	// update the mat(albedo, tex...)
	MeshInstance *mi = Object::cast_to<MeshInstance>(node);
	if (nullptr != mi) {
		auto mat_num = mi->get_surface_material_count();
		if (mat_num != sdi.albedo_vec.size()) {
			os->print("[GDI]this case not handled, should report\n");
			return;
		}

		for (int i = 0; i < mat_num; ++i) {
			SpatialMaterial *sm = Object::cast_to<SpatialMaterial>(*(mi->get_surface_material(i)));
			if (nullptr == sm) {
				continue;
			}

			// albedo tex
			//os->print("begin to set albedo tex........\n");
			auto e = sdi.albedo_tex_map.find(i);
			auto tex = sm->get_texture(SpatialMaterial::TEXTURE_ALBEDO);
			if (nullptr != *tex && nullptr != e && e->value() != tex->get_path()) {
				tex->set_path(e->value(), true);
				tex->reload_from_file();
				os->print("sync albedo tex1..........\n");
			}
			else if (nullptr == *tex && nullptr != e) {
				Ref<StreamTexture> new_tex;
				new_tex.instance();
				new_tex->load(e->value());
				sm->set_texture(SpatialMaterial::TEXTURE_ALBEDO, new_tex);
// 				sm->set_albedo(Color(1, 1, 1));
				os->print("sync albedo tex2..........\n");
			}
			else if (nullptr == e && nullptr != *tex) {
				sm->set_texture(SpatialMaterial::TEXTURE_ALBEDO, nullptr);
				os->print("sync albedo tex3..........\n");
			}

			// albedo
			sm->set_albedo(sdi.albedo_vec[i]);
		}
	}

	auto e = stored_sync_data_info_map.find(node);
	if (nullptr != e) {
		e->value() = SyncDataInfo(node);
	}
}

int GDIVisualScriptNodeInstanceCustomMultiPlayer::step(const Variant **p_inputs, Variant **p_outputs, StartMode p_start_mode, Variant *p_working_mem, Variant::CallError &r_error, String &r_error_str) {

	bool is_server_flag = (bool)*p_inputs[0];
	static Node *node = Object::cast_to<Node>(instance->get_owner_ptr());
	if (nullptr == node) {
		r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
		r_error_str = "[GDI]can't get instance node";
		return 0;
	}

	static Node *root = node->get_tree()->get_current_scene();
	if (nullptr == root) {
		r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
		r_error_str = "[GDI]can't get root node";
		return 0;
	}

	if (!already_create_flag) {
		// init all nodes data info at the create time
		generate_all_nodes_sync_data_info(root);

		// for test, output all nodes info
		//for (auto e = stored_sync_data_info_map.front(); e; e = e->next()) {
		//	os->print("[Node info]name[%S], id[%d]\n", e->value().name, e->value().instance_id);
		//}

		// Server----
		if (is_server_flag) {
			server_create_func(p_inputs, p_outputs, p_start_mode, p_working_mem, r_error, r_error_str);
		}
		// Client----
		else {
			client_create_func(p_inputs, p_outputs, p_start_mode, p_working_mem, r_error, r_error_str);
		}

		already_create_flag = true;
	}

	// Server, accept new client
	if (is_server_flag) {
		if (server->is_listening()) {
			auto new_client = server->take_connection();

			if (nullptr != *new_client) {
				server_clients_map.insert(new_client->get_instance_id(), new_client);
				os->print("new client connected\n");
			}
		}
	}

	changed_data_info_vec.clear();
	update_all_nodes_sync_data_info(root);

	int changed_data_num = changed_data_info_vec.size();
	if (changed_data_num > 0) {
		// if it is server, just broadcast the msg, if not, send to server first, let it do the broadcast
		Array arr;
		arr.push_back(is_server_flag ? (int)S2C_DATA_SYNC : (int)C2S_CLIENT_DATA_CHANGE);
		arr.push_back(changed_data_num);
		os->print("--------push back changed data start, server-flag[%d]\n", is_server_flag);
		for (int i = 0; i < changed_data_num; ++i) {
			auto &data = changed_data_info_vec[i];
			arr.push_back(data.instance_id);
			arr.push_back(data.name);
			arr.push_back(data.transform);
			arr.push_back(data.visible);
			// mat num
			auto mat_num = data.surf_mat_vec.size();
			arr.push_back(mat_num);
			for (int j = 0; j < mat_num; ++j) {
				arr.push_back(data.albedo_vec[j]);
				//os->print("changed color:[%f][%f][%f]\n", data.albedo_vec[j].r, data.albedo_vec[j].g, data.albedo_vec[j].b);
			}
			// albedo tex
			auto albedo_tex_num = data.albedo_tex_map.size();
			arr.push_back(albedo_tex_num);
			for (auto e = data.albedo_tex_map.front(); e; e = e->next()) {
				arr.push_back(e->key());
				arr.push_back(e->value());
			}
			os->print("push back changed data, tex num[%d], mat num[%d], node[%S]\n", albedo_tex_num, mat_num, data.name);
		}

		if (is_server_flag) {
			for (auto e = server_clients_map.front(); e; e = e->next()) {
				auto &socket = e->value();
				socket->put_var(arr);
			}
		} 
		else {
			client->put_var(arr);
// 			os->print("client send sync data...\n");
		}
	}

	// Client, sync data
	if (!is_server_flag && client->is_connected_to_host() && client->get_available_bytes()) {
		Variant var = client->get_var();
		Array arr = var;
		int protocol = arr.pop_front();

		switch (protocol) {
		case S2C_SERVER_VERIFY_PW_FAILED: {
			client->disconnect_from_host();
			r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
			r_error_str = "[GDI]multiplayer password wrong";
			return 0;
		}
		case S2C_DATA_SYNC: {
			int data_num = arr.pop_front();
			os->print("--------client sync data, protocol[%d], data_num[%d]\n", protocol, data_num);

			for (int i = 0; i < data_num; ++i) {
				uint64_t instance_id = arr.pop_front();
				String name = arr.pop_front();
				Transform trans = arr.pop_front();
				bool visible = arr.pop_front();
				os->print("sync data id[%d], name[%S]\n", instance_id, name);
				// albedo
				Vector<Color> albedo_vec;
				int mat_num = arr.pop_front();
				for (int j = 0; j < mat_num; ++j) {
					albedo_vec.push_back(arr.pop_front());
					//os->print("sync color:[%f][%f][%f]\n", colorVec[j].r, colorVec[j].g, colorVec[j].b);
				}
				// albedo tex
				Map<int, String> albedo_tex_map;
				int albedo_tex_num = arr.pop_front();
				for (int j = 0; j < albedo_tex_num; ++j) {
					int idx = arr.pop_front();
					String tex_path = arr.pop_front();
					albedo_tex_map.insert(idx, tex_path);
				}

				// use id and name to find the matched node
				Node *node = find_node_with_id_and_name(instance_id, name);
				if (nullptr == node) {
					os->print("server sync data failed, not found matched node, id[%d], name[%S]\n", instance_id, name);
					continue;
				}

				os->print("albedo tex num[%d], mat num[%d], node[%S]\n", albedo_tex_num, mat_num, String(node->get_name()));
				sync_data_with_node(node, SyncDataInfo(trans, visible, albedo_vec, albedo_tex_map));
			}
			break;
		}
		default:
			break;
		}
	}

	// Server, broadcast sync data
	if (is_server_flag) {
		delete_clients_vec.clear();
		for (auto e = server_clients_map.front(); e; e = e->next()) {
			auto &client = e->value();
			if (!client->is_connected_to_host() || !client->get_available_bytes()) {
				continue;
			}

			Variant var = client->get_var();
			Variant varTmp = var.duplicate();
			Array arr = var;
			int protocol = arr.pop_front();
	
			switch (protocol) {
			case C2S_CLIENT_VERIFY_PW: {
				int pw = *p_inputs[4];
				int client_pw = arr.pop_front();

				if (client_pw != pw) {
					delete_clients_vec.push_back(e->key());

					Array arr;
					arr.push_back((int)S2C_SERVER_VERIFY_PW_FAILED);
					client->put_var(arr);
				}
				break;
			}
			case C2S_CLIENT_DATA_CHANGE: {
// 				os->print("server recv client data change, id[%d]\n", e->key());

				// sync server
				{
					int data_num = arr.pop_front();
					//os->print("get changed data, protocol[%d], data_num[%d]\n", protocol, data_num);
	
					for (int i = 0; i < data_num; ++i) {
						uint64_t instance_id = arr.pop_front();
						String name = arr.pop_front();
						Transform trans = arr.pop_front();
						bool visible = arr.pop_front();
						// albedo
						Vector<Color> albedo_vec;
						int mat_num = arr.pop_front();
						for (int j = 0; j < mat_num; ++j) {
							albedo_vec.push_back(arr.pop_front());
							//os->print("sync color:[%f][%f][%f]\n", colorVec[j].r, colorVec[j].g, colorVec[j].b);
						}
						// albedo tex
						Map<int, String> albedo_tex_map;
						int albedo_tex_num = arr.pop_front();
						for (int j = 0; j < albedo_tex_num; ++j) {
							int idx = arr.pop_front();
							String tex_path = arr.pop_front();
							albedo_tex_map.insert(idx, tex_path);
						}

						// use id and name to find the matched node
						Node *node = find_node_with_id_and_name(instance_id, name);
						if (nullptr == node) {
							os->print("server sync data failed, not found matched node, id[%d], name[%S]\n", instance_id, name);
							continue;
						}

						sync_data_with_node(node, SyncDataInfo(trans, visible, albedo_vec, albedo_tex_map));
					}
				}

				// sync other clients
				// chanage protocol to broadcast
				varTmp.set(0, (int)S2C_DATA_SYNC);
				for (auto e2 = server_clients_map.front(); e2; e2 = e2->next()) {
					if (e->key() == e2->key()) {
						continue;
					}

					auto &socket = e2->value();
					socket->put_var(varTmp);
				}

				break;
			}
			default: {
				break;
			}
			}
		}

		for (int i = 0; i < delete_clients_vec.size(); ++i) {
			os->print("server delete client[%d]\n", delete_clients_vec[i]);
			server_clients_map.erase(delete_clients_vec[i]);
		}
	}

	*p_outputs[0] = create_succeed_flag;
	return 0;
}

int GDIVisualScriptNodeInstanceCustomMultiPlayer::get_working_memory_size() const {
	return 1;
}

void GDIVisualScriptCustomNodeMultiPlayer::_bind_methods() {

// 	ClassDB::add_virtual_method("GDIVisualScriptCustomNodeMultiPlayer", MethodInfo(Variant::NIL, "rpc_call_test_func"));
// 	ClassDB::bind_method(D_METHOD("rpc_call_test_func"), &GDIVisualScriptCustomNodeMultiPlayer::rpc_call_test_func);
}

int GDIVisualScriptCustomNodeMultiPlayer::get_output_sequence_port_count() const {
	return 1;
}

bool GDIVisualScriptCustomNodeMultiPlayer::has_input_sequence_port() const {
	return true;
}

String GDIVisualScriptCustomNodeMultiPlayer::get_output_sequence_port_text(int p_port) const {
	return String();
}

int GDIVisualScriptCustomNodeMultiPlayer::get_input_value_port_count() const {
	return 5;
}

int GDIVisualScriptCustomNodeMultiPlayer::get_output_value_port_count() const {
	return 1;
}

PropertyInfo GDIVisualScriptCustomNodeMultiPlayer::get_input_value_port_info(int p_idx) const {

	PropertyInfo pi;
	switch (p_idx)
	{
	case 0: {
		pi.name = L"本机为服务器";
		pi.type = Variant::BOOL;
		break;
	}
	case 1: {
		pi.name = L"服务器IP地址";
		pi.type = Variant::STRING;
		break;
	}
	case 2: {
		pi.name = L"端口";
		pi.type = Variant::INT;
		break;
	}
	case 3: {
		pi.name = L"最大连接数";
		pi.type = Variant::INT;
		break;
	}
	case 4: {
		pi.name = L"密码";
		pi.type = Variant::INT;
		break;
	}
	default:
		break;
	}

	return pi;
}

PropertyInfo GDIVisualScriptCustomNodeMultiPlayer::get_output_value_port_info(int p_idx) const {

	PropertyInfo pi;
	pi.name = L"成功";
	pi.type = Variant::BOOL;
	return pi;
}

String GDIVisualScriptCustomNodeMultiPlayer::get_caption() const {

	return L"多人协同";
}

String GDIVisualScriptCustomNodeMultiPlayer::get_category() const {

	return L"custom";
}

String GDIVisualScriptCustomNodeMultiPlayer::get_text() const {

	return L"创建服务器或客户端";
}

VisualScriptNodeInstance * GDIVisualScriptCustomNodeMultiPlayer::instance(VisualScriptInstance *p_instance) {

// 	VisualScript *vs = p_instance->get_script_ptr();
// 	if (nullptr != vs) {
// 		vs->add_function("rpc_call_test_func");
// 		os->print("add rpc_call_test_func\n");
// 	}

// 	os->print("create multi player instance.....\n");
	// keep multi player singleton(also limit the create num in visual_script_editor)
	GDIVisualScriptNodeInstanceCustomMultiPlayer *instance = &GDIVisualScriptNodeInstanceCustomMultiPlayer::get_singleton();
	instance->node = this;
	instance->instance = p_instance;
	instance->returns = get_output_value_port_count();
	instance->input_args = get_input_value_port_count();
	return instance;
}

GDIVisualScriptCustomNodeMultiPlayer::ConnectionStatus GDIVisualScriptCustomNodeMultiPlayer::get_connection_status() const {

	return connection_status;
}


void GDIVisualScriptCustomNodeMultiPlayer::rpc_call_test_func() {

	//os->print("test rpc func...\n");
}

GDIVisualScriptCustomNodeMultiPlayer::GDIVisualScriptCustomNodeMultiPlayer() {

}

GDIVisualScriptCustomNodeMultiPlayer::~GDIVisualScriptCustomNodeMultiPlayer() {

}

GDICustomNodeBase::GDICustomNodeBase()
	:_os(_OS::get_singleton())
	, os(OS::get_singleton())
	, input(Input::get_singleton()) {

}

GDICustomNodeBase::~GDICustomNodeBase() {

}

GDIVisualScriptNodeInstanceCustomMultiPlayer::SyncDataInfo::SyncDataInfo(Node *node)
	:SyncDataInfo()
{
	if (nullptr == node) {
		return;
	}

	instance_id = node->get_instance_id();
	name = node->get_name();

	Spatial *spatial = Object::cast_to<Spatial>(node);
	if (nullptr != spatial) {
		transform = spatial->get_global_transform();
		visible = spatial->is_visible();
	}

	// Material relevant(albedo, texture...)
	MeshInstance *mi = Object::cast_to<MeshInstance>(node);
	if (nullptr != mi) {
		surf_mat_vec.clear();
		auto surf_num = mi->get_surface_material_count();
		for (int i = 0; i < surf_num; ++i) {
			auto mat = mi->get_surface_material(i);
			surf_mat_vec.push_back(mat);

			Ref<SpatialMaterial> sm = Object::cast_to<SpatialMaterial>(*mat);
			if (nullptr != *sm) {
				// albedo
				albedo_vec.push_back(sm->get_albedo());

				// albedo tex
				auto tex = sm->get_texture(SpatialMaterial::TextureParam::TEXTURE_ALBEDO);
				if (nullptr != *tex) {
					albedo_tex_map.insert(i, tex->get_path());
				}
			}
		}
	}
}

GDIVisualScriptNodeInstanceCustomMultiPlayer::SyncDataInfo::SyncDataInfo(const Transform &trans, bool v, const Vector<Color> &albedo_vec, const Map<int, String> &albedo_tex_map)
	:transform(trans), visible(v)
{
	this->albedo_vec = albedo_vec;
	this->albedo_tex_map = albedo_tex_map;
}

GDIVisualScriptNodeInstanceCustomMultiPlayer::SyncDataInfo::SyncDataInfo()
	:instance_id(0), visible(true)
{
	surf_mat_vec.clear();
}

bool GDIVisualScriptNodeInstanceCustomMultiPlayer::SyncDataInfo::operator==(const SyncDataInfo &other) {

	// we should keep left param be the stored one(left == right)
	// check mat----
	bool same_mat_flag = true;
	auto left_mat_num = surf_mat_vec.size();
	auto right_mat_num = other.surf_mat_vec.size();
	// check mat num
	if (left_mat_num != right_mat_num) {
		same_mat_flag = false;
	}
	else {
		// check mat albedo, tex
		for (int i = 0; i < left_mat_num; ++i) {
			// albedo--
			if (nullptr == *surf_mat_vec[i] || nullptr == *other.surf_mat_vec[i]) {
				//OS::get_singleton()->print("null mat, left[%x], right[%x]\n", *surf_mat_vec[i], *other.surf_mat_vec[i]);
				continue;
			}

			auto left_albedo = albedo_vec[i];
			auto right_albedo = other.surf_mat_vec[i]->get_albedo();

			if (left_albedo != right_albedo) {
				//OS::get_singleton()->print("albedo changed............\n");
				same_mat_flag = false;
				break;
			}

			// albedo tex--
			auto e = albedo_tex_map.find(i);
			auto right_tex = other.surf_mat_vec[i]->get_texture(SpatialMaterial::TEXTURE_ALBEDO);
			if (nullptr != *right_tex) {
				String right_path = right_tex->get_path();
				//OS::get_singleton()->print("left path[%S], right paht[%S]\n", left_path, right_path);
				String left_path = nullptr == e ? "" : e->value();

				if (left_path != right_path) {
					OS::get_singleton()->print("albedo tex changed1............\n");
					same_mat_flag = false;
					break;
				}
			}
			else {
				if (nullptr != e) {
					OS::get_singleton()->print("albedo tex changed2............\n");
					same_mat_flag = false;
					break;
				}
			}
		}

		// todo(maybe some other property...)
	}

	if (transform == other.transform &&
		visible == other.visible &&
		same_mat_flag) {
		return true;
	}
	else {
		return false;
	}
}
