#ifndef GDI_VISUAL_SCRIPT_CUSTOM_NODES_H
#define GDI_VISUAL_SCRIPT_CUSTOM_NODES_H

#include "visual_script.h"
#include "visual_script_nodes.h"
#include "scene/main/node.h"

class Transfrom;
class Material;
struct Color;

class OS;
class _OS;
class Input;

class NetworkedMultiplayerENet;
class TCP_Server;
class StreamPeerTCP;

class GDICustomNodeBase
{
public:
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
		MULTI_PLAYER,

		INIT_PARTIAL,// preserve
		MAX_COUNT
	};

	GDICustomNodeBase();
	~GDICustomNodeBase();

protected:
	// record singleton to increase the invoke efficiency----
	_OS *_os = nullptr;
	OS *os = nullptr;
	Input *input = nullptr;
};

class GDIVisualScriptCustomNode : public VisualScriptNode, public GDICustomNodeBase {

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
	int combine_task_num;
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
	virtual String get_category() const { return "custom"; }

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

class GDIVisualScriptCustomNodeMouse : public VisualScriptNode, public GDICustomNodeBase {

	GDCLASS(GDIVisualScriptCustomNodeMouse, VisualScriptNode);

protected:
	static void _bind_methods();

public:
	enum MouseKey
	{
		LEFT,
		RIGHT,
		MID,

		MAX_COUNT
	};
	const String mouse_key_list[3] = { TTR("MouseLeftBtn"), TTR("MouseRightBtn"), TTR("MouseMiddleBtn") };

	String get_mouse_key_string(MouseKey key) const;

	virtual int get_output_sequence_port_count() const override;
	virtual bool has_input_sequence_port() const override;
	virtual String get_output_sequence_port_text(int p_port) const override;
	virtual int get_input_value_port_count() const override;
	virtual int get_output_value_port_count() const override;
	virtual PropertyInfo get_input_value_port_info(int p_idx) const override;
	virtual PropertyInfo get_output_value_port_info(int p_idx) const override;
	virtual String get_caption() const override;
	virtual String get_category() const override;
	virtual String get_text() const override;

	void set_mouse_pick_area_path(const NodePath &path);
	NodePath get_mouse_pick_area_path() const;

	virtual VisualScriptNodeInstance *instance(VisualScriptInstance *p_instance);

	GDIVisualScriptCustomNodeMouse();
	~GDIVisualScriptCustomNodeMouse();

private:
	NodePath mouse_pick_area_path;
};

class GDIVisualScriptCustomNodeMultiPlayer : public VisualScriptNode, public GDICustomNodeBase {

	GDCLASS(GDIVisualScriptCustomNodeMultiPlayer, VisualScriptNode);

protected:
	static void _bind_methods();

public:
	enum ConnectionStatus
	{
		CONNECTTING,
		CONNECTED,
		DISCONNECTED,
		CONNECTION_FAILED
	};

	virtual int get_output_sequence_port_count() const override;
	virtual bool has_input_sequence_port() const override;
	virtual String get_output_sequence_port_text(int p_port) const override;
	virtual int get_input_value_port_count() const override;
	virtual int get_output_value_port_count() const override;
	virtual PropertyInfo get_input_value_port_info(int p_idx) const override;
	virtual PropertyInfo get_output_value_port_info(int p_idx) const override;
	virtual String get_caption() const override;
	virtual String get_category() const override;
	virtual String get_text() const override;

	virtual VisualScriptNodeInstance *instance(VisualScriptInstance *p_instance);

	ConnectionStatus get_connection_status() const;
	virtual void rpc_call_test_func();

	~GDIVisualScriptCustomNodeMultiPlayer();
	GDIVisualScriptCustomNodeMultiPlayer();

private:
	ConnectionStatus connection_status;
};

class GDIVisualScriptNodeInstanceCustomMultiPlayer
	: public VisualScriptNodeInstance, public GDICustomNodeBase, public Object {

	GDCLASS(GDIVisualScriptNodeInstanceCustomMultiPlayer, Object);

	enum MultiPlayerSyncProtocol
	{
		S2C_DATA_SYNC,
		C2S_CLIENT_DATA_CHANGE,
		S2C_NEW_INSTANCE_SYNC,// preserve
		C2S_CLIENT_VERIFY_PW,
		S2C_SERVER_VERIFY_PW_FAILED,
		S2C_CLIENT_ID,

		MAX_COUNT
	};

protected:
	static void _bind_methods();

private:
	GDIVisualScriptNodeInstanceCustomMultiPlayer();

public:
	int input_args;
	bool validate;
	int returns;

	GDIVisualScriptCustomNodeMultiPlayer *node;
	VisualScriptInstance *instance;

	// ----multi player sync relevant
	bool already_create_flag = false;
	bool create_succeed_flag = false;
	bool client_disconnected_flag = false;
	bool is_server_flag = false;
	Ref<NetworkedMultiplayerENet> multi_player_enet;
	Ref<TCP_Server> server;
	Map<uint64_t, Ref<StreamPeerTCP>> server_clients_map;// (instance_id, tcp_peer)
	Vector<uint64_t> delete_clients_vec;
	Ref<StreamPeerTCP> client;
	uint64_t client_unique_id;
	struct SyncDataInfo 
	{		
		uint64_t							instance_id;
		String								name;
		String								class_name;
		Transform							transform;
		Transform2D							transform2d;
		bool								visible;
		Vector<Ref<SpatialMaterial>>		surf_mat_vec;
		Vector<Color>						albedo_vec;
		Map<int, String>					albedo_tex_map;

		bool								sync_trans;
		bool								sync_visible;
		bool								sync_albedo;
		bool								sync_albedo_tex;

		SyncDataInfo();
		SyncDataInfo(Node *node);
		bool operator==(const SyncDataInfo &other);

		void set_sync_data(const Transform &trans, const Transform2D &trans2d, bool p_visible, const Vector<Color> &p_albedo_vec, const Map<int, String> &p_albedo_tex_map);
	};
	Map<Node*, SyncDataInfo> stored_sync_data_info_map;
	Vector<SyncDataInfo> changed_data_info_vec;

	// ----
	static GDIVisualScriptNodeInstanceCustomMultiPlayer& get_singleton();
	~GDIVisualScriptNodeInstanceCustomMultiPlayer();

	void peer_connected(int id);
	void peer_disconnected(int id);

	void client_connected_to_server();
	void client_connection_failed();
	void client_server_disconnected();

	void rpc_call_test_func();

	int server_create_func(const Variant **p_inputs, Variant **p_outputs, StartMode p_start_mode, Variant *p_working_mem, Variant::CallError &r_error, String &r_error_str);
	int client_create_func(const Variant **p_inputs, Variant **p_outputs, StartMode p_start_mode, Variant *p_working_mem, Variant::CallError &r_error, String &r_error_str);

	void generate_all_nodes_sync_data_info(Node *node);
	void update_all_nodes_sync_data_info(Node *node);
	void update_stored_nodes_info(SyncDataInfo &stored, SyncDataInfo &changed);
	Node* find_node_with_id_and_name(uint64_t id, const String &name);
	void sync_data_with_node(Node *node, SyncDataInfo &sdi);
	void handle_data_change(Node *root);
	void handle_server_msg(const Variant **p_inputs);
	void handle_client_msg(Variant::CallError &r_error, String &r_error_str);

	virtual int step(const Variant **p_inputs, Variant **p_outputs, StartMode p_start_mode, Variant *p_working_mem, Variant::CallError &r_error, String &r_error_str) override;

	virtual int get_working_memory_size() const override;
};

#endif // GDI_VISUAL_SCRIPT_CUSTOM_NODES_H
