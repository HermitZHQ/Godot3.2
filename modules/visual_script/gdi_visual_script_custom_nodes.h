#ifndef GDI_VISUAL_SCRIPT_CUSTOM_NODES_H
#define GDI_VISUAL_SCRIPT_CUSTOM_NODES_H

#include "visual_script.h"

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
		KEYBOARD,
		MOUSE,
		MAT_ALBEDO,
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

	virtual VisualScriptNodeInstance *instance(VisualScriptInstance *p_instance);

	virtual TypeGuess guess_output_type(TypeGuess *p_inputs, int p_output) const;

	GDIVisualScriptCustomNode();
};

VARIANT_ENUM_CAST(GDIVisualScriptCustomNode::CallMode);
VARIANT_ENUM_CAST(GDIVisualScriptCustomNode::RPCCallMode);
VARIANT_ENUM_CAST(GDIVisualScriptCustomNode::CustomMode);

#endif // GDI_VISUAL_SCRIPT_CUSTOM_NODES_H
