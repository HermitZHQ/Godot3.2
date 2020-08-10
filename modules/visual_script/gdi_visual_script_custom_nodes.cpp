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
#include "scene/main/viewport.h"
#include "scene/3d/camera.h"
#include "core/node_path.h"


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
			ret.name = L"选中物体";
			ret.type = Variant::OBJECT;
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

void GDIVisualScriptCustomNode::set_mouse_pick_area_path(const NodePath &path) {
	mouse_pick_area_path = path;
}

NodePath GDIVisualScriptCustomNode::get_mouse_pick_area_path() const {
	return mouse_pick_area_path;
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
			// 			if (String(obj->key()->get_name()) == String("Spatial")) {
			// 				auto t = obj->key()->get_global_transform();
			// 				printf("--rest1 transform:\n x1:%f, y1:%f, z1:%f\n x2:%f, y2:%f, z2:%f\n x3:%f, y3:%f, z3:%f\n", t.basis[0].x, t.basis[0].y, t.basis[0].z, t.basis[1].x, t.basis[1].y, t.basis[1].z, t.basis[2].x, t.basis[2].y, t.basis[2].z);
			// 			}

			auto restInfo = objs_state[i];
			Spatial *spa = Object::cast_to<Spatial>(restInfo.node);
			if (nullptr != spa) {
				// must get transform once here, or the update will incorrect
				spa->get_transform();

				spa->set_global_transform(restInfo.trans);
				// 				spa->force_update_transform();
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

	ClassDB::bind_method(D_METHOD("set_mouse_pick_area_path", "path"), &GDIVisualScriptCustomNode::set_mouse_pick_area_path);
	ClassDB::bind_method(D_METHOD("get_mouse_pick_area_path"), &GDIVisualScriptCustomNode::get_mouse_pick_area_path);

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
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "(鼠标)拣选Area", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Area"), "set_mouse_pick_area_path", "get_mouse_pick_area_path");
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

	// area trigger relevant----
	bool first_create_manual_area_flag = false;

	// record singleton to increase the invoke efficiency----
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

			os->print("--------start collect sub task init info, task index[%d]\n", task_split_cur_execute_index);
			Vector<GDIVisualScriptCustomNode::RestoreInfo> spatial_trans_vec;
			collect_init_info(root, spatial_trans_vec);
			sub_task_objs_state_vec.push_back(spatial_trans_vec);

			// store the info to node
			GDIVisualScriptCustomNode *custom_node = Object::cast_to<GDIVisualScriptCustomNode>(this->node);
			if (nullptr != custom_node) {
				// 				os->print("set sub task index and objs state, index[%d] addr[%x]\n", task_split_cur_execute_index, custom_node);
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

	int mouse_handle_func(const Variant **p_inputs, Variant **p_outputs, Variant* p_working_mem, StartMode p_start_mode, Variant::CallError &r_error, String &r_error_str) {

		if (nullptr == p_working_mem) {
			r_error.error = Variant::CallError::CALL_ERROR_INVALID_METHOD;
			r_error_str = "[GDI]mouse, working mem error";
			return 0;
		}

		static Point2 mouse_point_left;
		static Point2 mouse_point_right;
		static Point2 mouse_point_mid;
		auto left_button_pressed_flag = input->is_mouse_button_pressed(BUTTON_LEFT);
		auto right_button_pressed_flag = input->is_mouse_button_pressed(BUTTON_RIGHT);
// 		auto mid_button_pressed_flag = input->is_mouse_button_pressed(BUTTON_MIDDLE); // preserve

		if (input->gdi_get_mouse_button_mask() != 0) {
			input->gdi_reset_mouse_button_mask();
			return 4;
		}		

		*p_outputs[0] = left_button_pressed_flag;
		*p_outputs[1] = right_button_pressed_flag;
		*p_outputs[2] = os->get_mouse_position();
		*p_outputs[3] = (Object*)(nullptr);

		// check area path, if null, we will create it manual
		Node *target = Object::cast_to<Node>((Object*)*p_inputs[0]);
		auto area_path = this->node->get_mouse_pick_area_path();
		if (area_path == NodePath() && nullptr != target) {
			printf("area path is null.....\n");
			this->node->set_mouse_pick_area_path(String("just/a/test"));
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

			if (!first_create_manual_area_flag) {
				// add the Area node with collision shape dynamicly
				if (!has_area_flag) {
					// check child mesh and area situation
					check_child_mesh_area_func(target, dirty_flag, has_area_flag, min_pos, max_pos);

// 					Vector3 target_pos = target->get_global_transform().origin;
// 					Vector3 center_pos = dirty_flag ? ((max_pos + min_pos) / 2.0) : target_pos;
// 					Vector3 dir = center_pos - target_pos;
// 					float diff_len = dir.length();
// 					dir.normalize();
// 					// the 0.1 extents prevents the null area, if no valid mesh under the node
// 					Vector3 extents = dirty_flag ? ((max_pos - center_pos) + Vector3(0.1, 0.1, 0.1)) : Vector3(0.1, 0.1, 0.1);
// 
// 					Area *new_area = memnew(Area);
// 					CollisionShape *cs = memnew(CollisionShape);
// 					BoxShape *bs = memnew(BoxShape);
// 					bs->set_extents(extents);
// 
// 					new_area->add_child(cs);
// 					target->add_child(new_area);
// 					cs->set_shape(bs);

// 					if (dirty_flag) {
// 						new_area->translate(dir * diff_len);
// 					}
					//os->print("[GDI]attach the Area node manual, dirty flag[%d], extents[%f][%f][%f]\n", dirty_flag, extents.x, extents.y, extents.z);
				}
			}
		}

		// check ray intersect
		Object *object = instance->get_owner_ptr();
		Node *node = Object::cast_to<Node>(object);
		Viewport *root = (nullptr != node) ? node->get_tree()->get_root() : nullptr;
		if (nullptr != root) {
			 auto world = root->get_world();
			 PhysicsDirectSpaceState *state = (nullptr != *world) ? world->get_direct_space_state() : nullptr;
			 Camera *cam = root->get_camera();
			 if (nullptr != state && nullptr != cam) {
				 auto p1 = root->get_mouse_position();
				 auto p2 = os->get_mouse_position();
				 auto from = cam->project_ray_origin(os->get_mouse_position());
				 auto to = from + cam->project_ray_normal(os->get_mouse_position()) * 10000.0;

				 PhysicsDirectSpaceState::RayResult res;
				 bool bRes = state->intersect_ray(from, to, res, Set<RID>(), 0xFFFFFFFF, true, true, true);
				 if (bRes) {
					 //CollisionObject *co = Object::cast_to<CollisionObject>(res.collider);
					 //printf("intersect with object[%S]\n", String(co->get_name()));
					 *p_outputs[3] = res.collider;
				 }
				 //printf("test intersect ray enter..............res[%d], shape[%d]\n from[%f][%f][%f]---to[%f][%f][%f]", bRes, res.shape, from[0], from[1], from[2], to.x, to.y, to.z);
			 }
		}

		static uint64_t time = 0;
		static bool first_left_pressed_flag = false;
		static bool first_left_released_flag = false;
		static bool first_left_click_flag = false;
		static bool second_left_pressed_flag = false;
		static bool left_drag_flag = false;

		static bool first_right_pressed_flag = false;
		static bool first_right_released_flag = false;

		static bool first_mid_pressed_flag = false;
		static bool first_mid_released_flag = false;

		static const unsigned int double_click_interval = 500;

		bool double_click_flag = false;
		left_drag_flag = false;

		//----mid button relevant(preserve)
// 		if (!first_mid_pressed_flag && mid_button_pressed_flag) {
// 			first_mid_pressed_flag = true;
// 			mouse_point_mid = os->get_mouse_position();
// 		}
// 		else if (first_mid_pressed_flag && !first_mid_released_flag && !mid_button_pressed_flag) {
// 			first_mid_released_flag = true;
// 		}

		//----right button relevant
		if (!first_right_pressed_flag && right_button_pressed_flag) {
			first_right_pressed_flag = true;
			mouse_point_right = os->get_mouse_position();
		}
		else if (first_right_pressed_flag && !first_right_released_flag && !right_button_pressed_flag) {
			first_right_released_flag = true;
		}

		//----left button relevant
		// check drag first
		if (first_left_pressed_flag && left_button_pressed_flag && mouse_point_left != os->get_mouse_position()) {
			left_drag_flag = true;
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

		// reset, if time alreay over
		if (os->get_system_time_msecs() - time >= double_click_interval) {
			time = 0;
			second_left_pressed_flag = false;
			first_left_click_flag = false;
			if (!left_drag_flag) {
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
		else if (first_left_released_flag && !first_left_click_flag && mouse_point_left == os->get_mouse_position()) {
			//first_left_pressed_flag = false;
			//first_left_released_flag = false;
			first_left_click_flag = true;
			return 0;
		}
		// right click
		else if (first_right_pressed_flag && first_right_released_flag) {
			first_right_pressed_flag = false;
			first_right_released_flag = false;
			if (mouse_point_right == os->get_mouse_position()) {
				return 3;
			}
		}
		// mid click(preserve)
// 		else if (first_mid_pressed_flag && first_mid_released_flag) {
// 			first_mid_pressed_flag = false;
// 			first_mid_released_flag = false;
// 			if (mouse_point_mid == os->get_mouse_position()) {
// 				return 4;
// 			}
// 		}

		p_working_mem[0] = STEP_EXIT_FUNCTION_BIT;
		return STEP_EXIT_FUNCTION_BIT;
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

		if (!first_create_manual_area_flag) {
			// add the Area node with collision shape dynamicly
			if (!has_area_flag) {
				// check child mesh and area situation
				check_child_mesh_area_func(target, dirty_flag, has_area_flag, min_pos, max_pos);

				Vector3 target_pos = target->get_global_transform().origin;
				Vector3 center_pos = dirty_flag ? ((max_pos + min_pos) / 2.0) : target_pos;
				Vector3 dir = center_pos - target_pos;
				float diff_len = dir.length();
				dir.normalize();
				// the 0.1 extents prevents the null area, if no valid mesh under the node
				Vector3 extents = dirty_flag ? ((max_pos - center_pos) + Vector3(0.1, 0.1, 0.1)) : Vector3(0.1, 0.1, 0.1);

				Area *new_area = memnew(Area);
				CollisionShape *cs = memnew(CollisionShape);
				BoxShape *bs = memnew(BoxShape);
				bs->set_extents(extents);

				new_area->add_child(cs);
				target->add_child(new_area);
				cs->set_shape(bs);

				if (dirty_flag) {
					new_area->translate(dir * diff_len);
				}
				//os->print("[GDI]attach the Area node manual, dirty flag[%d], extents[%f][%f][%f]\n", dirty_flag, extents.x, extents.y, extents.z);
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

		// 		bool area_entered_flag = this->node->get_area_trigger_entered_area_num() > 0 ? true : false;
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
		case GDIVisualScriptCustomNode::MOUSE: {

			int ret = mouse_handle_func(p_inputs, p_outputs, p_working_mem, p_start_mode, r_error, r_error_str);
			return ret;
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

			Object *object = instance->get_owner_ptr();
			Node *node = Object::cast_to<Node>(object);
			if (nullptr != node) {
				node->get_tree()->reload_current_scene();
// 				os->print("reload cur scene.......\n");
			}

// 			auto size = objs_init_trans_vec.size();
// 			for (int i = 0; i < size; ++i) {
// 				auto restInfo = objs_init_trans_vec[i];
// 				Spatial *spa = Object::cast_to<Spatial>(restInfo.node);
// 				if (nullptr != spa) {
// 					spa->set_global_transform(restInfo.trans);
// 				}
// 			}
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
