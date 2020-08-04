#ifndef GDI_VISUAL_SCRIPT_CUSTOM_NODES_H
#define GDI_VISUAL_SCRIPT_CUSTOM_NODES_H

#include "visual_script.h"

class Spatial;
class Transfrom;
class GDIVisualScriptCustomNode : public VisualScriptNode {

	GDCLASS(GDIVisualScriptCustomNode, VisualScriptNode);

public:
	enum CallMode {
		CALL_MODE_SELF,
		CALL_MODE_NODE_PATH,
		CALL_MODE_INSTANCE,
		CALL_MODE_BASIC_TYPE,
		CALL_MODE_SINGLETON,
	};

	enum RPCCallMode {
		RPC_DISABLED,
		RPC_RELIABLE,
		RPC_UNRELIABLE,
		RPC_RELIABLE_TO_ID,
		RPC_UNRELIABLE_TO_ID
	};

	enum CustomMode
	{
		ACTIVE,
		LOOP,
		TASK_SPLIT,
		TASK_CONTROL,
		KEYBOARD,
		MOUSE,
		AREA_TIGGER,
		TIMER,
		COMBINATION,
		INIT,
		INIT_PARTIAL,

		MAX_COUNT
	};

	struct RestoreInfo
	{
		Node *node;
		Transform trans;

		RestoreInfo()
			:node(nullptr)
		{}

		RestoreInfo(Node *n, const Transform &t)
			:node(n), trans(t)
		{}
	};

private:
	CustomMode custom_mode;
	CallMode call_mode;
	RPCCallMode rpc_call_mode;
	StringName base_type;
	String base_script;
	Variant::Type basic_type;
	NodePath base_path;
	StringName function;
	int use_default_args;
	StringName singleton;
	bool validate;

	Node *_get_base_node() const;
	StringName _get_base_type() const;

	MethodInfo method_cache;
	void _update_method_cache();

	void _set_argument_cache(const Dictionary &p_cache);
	Dictionary _get_argument_cache() const;

protected:
	virtual void _validate_property(PropertyInfo &property) const;

	static void _bind_methods();

public:
	virtual int get_output_sequence_port_count() const;
	virtual bool has_input_sequence_port() const;

	virtual String get_output_sequence_port_text(int p_port) const;

	virtual int get_input_value_port_count() const;
	virtual int get_output_value_port_count() const;

	virtual PropertyInfo get_input_value_port_info(int p_idx) const;
	virtual PropertyInfo get_output_value_port_info(int p_idx) const;

	virtual String get_caption() const;
	virtual String get_text() const;
	virtual String get_category() const { return "customs"; }

	void set_custom_mode(CustomMode p_mode);
	CustomMode get_custom_mode() const;

	void set_rpc_call_mode(RPCCallMode p_mode);
	RPCCallMode get_rpc_call_mode() const;

	void set_call_mode(CallMode p_mode);
	CallMode get_call_mode() const;

	void set_basic_type(Variant::Type p_type);
	Variant::Type get_basic_type() const;

	void set_base_type(const StringName &p_type);
	StringName get_base_type() const;

	void set_base_script(const String &p_path);
	String get_base_script() const;

	void set_singleton(const StringName &p_type);
	StringName get_singleton() const;

	void set_function(const StringName &p_type);
	StringName get_function() const;

	void set_base_path(const NodePath &p_type);
	NodePath get_base_path() const;

	void set_use_default_args(int p_amount);
	int get_use_default_args() const;

	void set_validate(bool p_amount);
	bool get_validate() const;

	// 修改点：加入空间触发器signal回调
	void area_trigger_entered_signal_callback(Node *area);
	void area_trigger_exited_signal_callback(Node *area);
	int get_area_trigger_entered_area_num() const;
	bool check_node_in_entered_areas(Node *node);

	void set_task_id(unsigned int id);
	unsigned int get_task_id() const;

	void set_task_split_num(unsigned int num);
	unsigned int get_task_split_num() const;

	void add_sub_task_index_and_objs_state(Vector<RestoreInfo> &state_vec);
	void set_sub_task_cur_index(unsigned int index);
	unsigned int get_sub_task_cur_index() const;
	void restore_sub_task_state(unsigned int index);

	virtual VisualScriptNodeInstance *instance(VisualScriptInstance *p_instance);

	virtual TypeGuess guess_output_type(TypeGuess *p_inputs, int p_output) const;

	GDIVisualScriptCustomNode();
	~GDIVisualScriptCustomNode();

private:
	Vector<Node*> area_trigger_entered_area_vec;
	unsigned int task_id;

	unsigned int task_split_num;
	unsigned int sub_task_cur_index;
	Vector<Vector<RestoreInfo>> sub_tasks_objs_state_vec;

	static unsigned int global_task_id;
};

VARIANT_ENUM_CAST(GDIVisualScriptCustomNode::CallMode);
VARIANT_ENUM_CAST(GDIVisualScriptCustomNode::RPCCallMode);
VARIANT_ENUM_CAST(GDIVisualScriptCustomNode::CustomMode);

#endif // GDI_VISUAL_SCRIPT_CUSTOM_NODES_H
