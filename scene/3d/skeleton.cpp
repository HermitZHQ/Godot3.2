/*************************************************************************/
/*  skeleton.cpp                                                         */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "skeleton.h"

#include "core/message_queue.h"

#include "core/project_settings.h"
#include "scene/3d/physics_body.h"
#include "scene/resources/surface_tool.h"
#include "core/os/os.h"

#include <functional>
#include "editor/editor_node.h"

void SkinReference::_skin_changed() {
	if (skeleton_node) {
		skeleton_node->_make_dirty();
	}
	skeleton_version = 0;
}

void SkinReference::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_skin_changed"), &SkinReference::_skin_changed);
	ClassDB::bind_method(D_METHOD("get_skeleton"), &SkinReference::get_skeleton);
	ClassDB::bind_method(D_METHOD("get_skin"), &SkinReference::get_skin);
}

RID SkinReference::get_skeleton() const {
	return skeleton;
}

Ref<Skin> SkinReference::get_skin() const {
	return skin;
}

SkinReference::~SkinReference() {
	if (skeleton_node) {
		skeleton_node->skin_bindings.erase(this);
	}

	VS::get_singleton()->free(skeleton);
}

void Skeleton::_gdi_check_load_vec_func(int pos) {

	if (pos >= gdi_anim_node_load_vec.size()) {
		NodeAnim *node = new NodeAnim;
		gdi_anim_node_load_vec.push_back(node);
	}
}

void Skeleton::_gdi_regenerate_anim_node_tree()
{
	if (gdi_anim_node_load_vec.size() == 0) {
		return;
	}

	for (int i = 0; i < gdi_anim_node_load_vec.size(); ++i) {
		gdi_anim_node_load_vec.ptrw()[i]->parent = (gdi_anim_node_load_vec.ptrw()[i]->parent == 0) ?
			0 : gdi_anim_node_load_vec.ptrw()[(int64_t)gdi_anim_node_load_vec.ptrw()[i]->parent];

		for (int j = 0; j < gdi_anim_node_load_vec.ptrw()[i]->childs.size(); ++j) {
			gdi_anim_node_load_vec.ptrw()[i]->childs.ptrw()[j] = gdi_anim_node_load_vec.ptrw()[(int64_t)gdi_anim_node_load_vec.ptrw()[i]->childs[j]];
		}
	}

	gdi_anim_node_root = gdi_anim_node_load_vec.ptrw()[0];
	gdi_anim_node_addr = (int64_t)gdi_anim_node_root;
}

bool Skeleton::_set(const StringName &p_path, const Variant &p_value) {

	String path = p_path;

	if (path == "edit bones") {
		gdi_set_show_bones_property_flag((bool)p_value);
		EditorNode::get_singleton()->get_inspector()->call_deferred("_property_changed_update_all"); 
		return true;
	}

	if (!path.begins_with("bones/"))
		return false;

	int which = path.get_slicec('/', 1).to_int();
	String what = path.get_slicec('/', 2);
	int pos = path.get_slicec('/', 3).to_int();

	if (which == bones.size() && what == "name") {

		add_bone(p_value);
		return true;
	}

	ERR_FAIL_INDEX_V(which, bones.size(), false);

	if (what == "parent")
		set_bone_parent(which, p_value);
	else if (what == "rest")
		set_bone_rest(which, p_value);
	else if (what == "enabled")
		set_bone_enabled(which, p_value);
	else if (what == "pose")
		set_bone_pose(which, p_value);
	else if (what == "import_file_format") {
		gdi_import_file_format = (ImportFileFormat)((int)p_value);
		gdi_set_import_file_format((int)p_value);
	}
	// 修改点：增加属性存储
	else if (what == "anim_node_name") {
		_gdi_check_load_vec_func(pos);
		gdi_anim_node_load_vec.ptrw()[pos]->name = p_value;
	}
	else if (what == "anim_node_child_id_array") {
		_gdi_check_load_vec_func(pos);
		PoolVector<int> id_vec = p_value;
		// get out the parent id
		gdi_anim_node_load_vec.ptrw()[pos]->parent = (NodeAnim*)(id_vec[0] == -1 ? (int64_t)0 : (int64_t)id_vec[0]);
		// get out the childs id
		for (int64_t i = 1; i < id_vec.size(); ++i)
		{
			// 先按照数量填充无效child，后面再进行组合
			// 其实也不是无效，只是相当于先把ID转换为无效的指针保存了起来
			// 后面重新生成时，再把指针转化为索引即可
			gdi_anim_node_load_vec.ptrw()[pos]->childs.push_back((NodeAnim*)(((int64_t)id_vec[i])));
		}
	}
	else if (what == "anim_node_local_transform") {
		_gdi_check_load_vec_func(pos);
		gdi_anim_node_load_vec.ptrw()[pos]->local_transform = p_value;
	}
	else if (what == "anim_node_global_transform") {
		_gdi_check_load_vec_func(pos);
		gdi_anim_node_load_vec.ptrw()[pos]->global_transform = p_value;
	}
	else if (what == "anim_node_channel_id") {
		_gdi_check_load_vec_func(pos);
		gdi_anim_node_load_vec.ptrw()[pos]->channel_id = p_value;
	}
	else if (what == "anim_node_is_bone") {
		_gdi_check_load_vec_func(pos);
		gdi_anim_node_load_vec.ptrw()[pos]->is_bone = p_value;
	}
	else if (what == "anim_node_bone_num") {
		_gdi_check_load_vec_func(pos);
		gdi_anim_node_load_vec.ptrw()[pos]->bone_num = p_value;
	}
	else if (what == "anim_node_is_mesh") {
		_gdi_check_load_vec_func(pos);
		gdi_anim_node_load_vec.ptrw()[pos]->is_mesh = p_value;
	}
	else if (what == "anim_node_mesh_num") {
		_gdi_check_load_vec_func(pos);
		gdi_anim_node_load_vec.ptrw()[pos]->mesh_num = p_value;
	}
	else if (what == "bound_children") {
		Array children = p_value;

		if (is_inside_tree()) {
			bones.write[which].nodes_bound.clear();

			for (int i = 0; i < children.size(); i++) {

				NodePath npath = children[i];
				ERR_CONTINUE(npath.operator String() == "");
				Node *node = get_node(npath);
				ERR_CONTINUE(!node);
				bind_child_node_to_bone(which, node);
			}
		}
	} else {
		return false;
	}

	return true;
}

bool Skeleton::_get(const StringName &p_path, Variant &r_ret) const {

	String path = p_path;

	if (p_path == "edit bones") {
		r_ret = gdi_get_show_bones_property_flag();
		return true;
	}

	if (!path.begins_with("bones/"))
		return false;

	int which = path.get_slicec('/', 1).to_int();
	String what = path.get_slicec('/', 2);
	int pos = path.get_slicec('/', 3).to_int();

	ERR_FAIL_INDEX_V(which, bones.size(), false);

	if (what == "name")
		r_ret = get_bone_name(which);
	else if (what == "parent")
		r_ret = get_bone_parent(which);
	else if (what == "rest")
		r_ret = get_bone_rest(which);
	else if (what == "enabled")
		r_ret = is_bone_enabled(which);
	else if (what == "pose")
		r_ret = get_bone_pose(which);
	else if (what == "import_file_format")
		r_ret = (int)gdi_get_import_file_format();
	// 修改点：增加animNode相关属性存储
	else if (what == "anim_node_name") {
		r_ret = gdi_anim_node_save_vec.ptr()[pos]->name;
	}
	else if (what == "anim_node_child_id_array") {
		PoolVector<int> id_vec;
		// parent id
		id_vec.push_back((int)(gdi_anim_node_save_vec.ptr()[pos]->parent ?
			gdi_anim_node_save_vec.ptr()[pos]->parent->node_id :
			-1));
		// child ids
		for (int i = 0; i < gdi_anim_node_save_vec.ptr()[pos]->childs.size(); ++i) {
			id_vec.push_back(gdi_anim_node_save_vec.ptr()[pos]->childs[i]->node_id);
		}

		r_ret = id_vec;
	}
	else if (what == "anim_node_local_transform") {
		r_ret = gdi_anim_node_save_vec.ptr()[pos]->local_transform;
	}
	else if (what == "anim_node_global_transform") {
		r_ret = gdi_anim_node_save_vec.ptr()[pos]->global_transform;
	}
	else if (what == "anim_node_channel_id") {
		r_ret = (int64_t)gdi_anim_node_save_vec.ptr()[pos]->channel_id;
	}
	else if (what == "anim_node_is_bone") {
		r_ret = gdi_anim_node_save_vec.ptr()[pos]->is_bone;
	}
	else if (what == "anim_node_bone_num") {
		r_ret = gdi_anim_node_save_vec.ptr()[pos]->bone_num;
	}
	else if (what == "anim_node_is_mesh") {
		r_ret = gdi_anim_node_save_vec.ptr()[pos]->is_mesh;
	}
	else if (what == "anim_node_mesh_num") {
		r_ret = gdi_anim_node_save_vec.ptr()[pos]->mesh_num;
	}
	else if (what == "bound_children") {
		Array children;

		for (const List<uint32_t>::Element *E = bones[which].nodes_bound.front(); E; E = E->next()) {

			Object *obj = ObjectDB::get_instance(E->get());
			ERR_CONTINUE(!obj);
			Node *node = Object::cast_to<Node>(obj);
			ERR_CONTINUE(!node);
			NodePath npath = get_path_to(node);
			children.push_back(npath);
		}

		r_ret = children;
	} else
		return false;

	return true;
}
void Skeleton::_get_property_list(List<PropertyInfo> *p_list) const {
	// 修改点：整理整个animNodeTree的顺序ID，并按照顺序存入property中，最后再尝试取出

	// add show bones flag
	p_list->push_back(PropertyInfo(Variant::BOOL, "edit bones", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_UPDATE_ALL_IF_MODIFIED));

	for (int i = 0; i < bones.size(); i++) {

		String prep = "bones/" + itos(i) + "/";
		p_list->push_back(PropertyInfo(Variant::STRING, prep + "name"));
		p_list->push_back(PropertyInfo(Variant::INT, prep + "parent", PROPERTY_HINT_RANGE, "-1," + itos(bones.size() - 1) + ",1"));
		p_list->push_back(PropertyInfo(Variant::TRANSFORM, prep + "rest"));
		p_list->push_back(PropertyInfo(Variant::BOOL, prep + "enabled"));
		p_list->push_back(PropertyInfo(Variant::TRANSFORM, prep + "pose", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR));
		p_list->push_back(PropertyInfo(Variant::ARRAY, prep + "bound_children"));

		// 修改点：增加animNode相关属性
		if (0 == i) {
			// save import file format
			p_list->push_back(PropertyInfo(Variant::INT, prep + "import_file_format"));

			for (int a = 0; a < gdi_anim_node_save_vec.size(); ++a) {
				NodeAnim *node = gdi_anim_node_save_vec[a];

				p_list->push_back(PropertyInfo(Variant::STRING, prep + "anim_node_name/" + itos(a)));
				p_list->push_back(PropertyInfo(Variant::ARRAY, prep + "anim_node_child_id_array/" + itos(a)));
				p_list->push_back(PropertyInfo(Variant::TRANSFORM, prep + "anim_node_local_transform/" + itos(a)));
				p_list->push_back(PropertyInfo(Variant::TRANSFORM, prep + "anim_node_global_transform/" + itos(a)));
				p_list->push_back(PropertyInfo(Variant::INT, prep + "anim_node_channel_id/" + itos(a)));
				p_list->push_back(PropertyInfo(Variant::BOOL, prep + "anim_node_is_bone/" + itos(a)));
				p_list->push_back(PropertyInfo(Variant::INT, prep + "anim_node_bone_num/" + itos(a)));
				p_list->push_back(PropertyInfo(Variant::BOOL, prep + "anim_node_is_mesh/" + itos(a)));
				p_list->push_back(PropertyInfo(Variant::INT, prep + "anim_node_mesh_num/" + itos(a)));
			}
		}
	}
}

void Skeleton::_update_process_order() {

	if (!process_order_dirty)
		return;

	Bone *bonesptr = bones.ptrw();
	int len = bones.size();

	process_order.resize(len);
	int *order = process_order.ptrw();
	for (int i = 0; i < len; i++) {

		if (bonesptr[i].parent >= len) {
			//validate this just in case
			ERR_PRINTS("Bone " + itos(i) + " has invalid parent: " + itos(bonesptr[i].parent));
			bonesptr[i].parent = -1;
		}
		order[i] = i;
		bonesptr[i].sort_index = i;
	}
	//now check process order
	int pass_count = 0;
	while (pass_count < len * len) {
		//using bubblesort because of simplicity, it won't run every frame though.
		//bublesort worst case is O(n^2), and this may be an infinite loop if cyclic
		bool swapped = false;
		for (int i = 0; i < len; i++) {
			int parent_idx = bonesptr[order[i]].parent;
			if (parent_idx < 0)
				continue; //do nothing because it has no parent
			//swap indices
			int parent_order = bonesptr[parent_idx].sort_index;
			if (parent_order > i) {
				bonesptr[order[i]].sort_index = parent_order;
				bonesptr[parent_idx].sort_index = i;
				//swap order
				SWAP(order[i], order[parent_order]);
				swapped = true;
			}
		}

		if (!swapped)
			break;
		pass_count++;
	}

	if (pass_count == len * len) {
		ERR_PRINT("Skeleton parenthood graph is cyclic");
	}

	process_order_dirty = false;
}

void Skeleton::_notification(int p_what) {

	switch (p_what) {

		case NOTIFICATION_UPDATE_SKELETON: {

			VisualServer *vs = VisualServer::get_singleton();
			Bone *bonesptr = bones.ptrw();
			int len = bones.size();

			if (ImportFileFormat::ASSIMP_FBX == gdi_import_file_format) {
				if (nullptr == gdi_anim_node_root) {
					// 修改点：update所有非bone节点的global transform，否则有些节点不会转变
					gdi_anim_node_root = (NodeAnim*)gdi_get_anim_root_node_addr();
					
					// 修改点：检测是anim_node_root为null，是的话，从load的anim node vec中重新组合出tree
					if (nullptr == gdi_anim_node_root) {
						_gdi_regenerate_anim_node_tree();
					}
				}
			}


			_update_process_order();
			const int *order = process_order.ptr();

			for (int i = 0; i < len; i++) {

				Bone &b = bonesptr[order[i]];

				if (b.global_pose_override_amount >= 0.999) {
					b.pose_global = b.global_pose_override;
				} else {
					// 注释点：不明所以的rest矩阵，不曾出现在assimp-viewer或者教程中
					// rest已经清楚了，用来保持原始矩阵，并且绘制skin用的
					if (b.disable_rest) {
						if (b.enabled) {

							Transform pose = b.pose;
							if (b.custom_pose_enable) {
								pose = b.custom_pose * pose;
							}
							if (b.parent >= 0) {

								b.pose_global = bonesptr[b.parent].pose_global * pose;
							} else {

								b.pose_global = pose;
							}
						} else {

							if (b.parent >= 0) {

								b.pose_global = bonesptr[b.parent].pose_global;
							} else {

								b.pose_global = Transform();
							}
						}

					} else {
						if (b.enabled) {

							Transform pose = b.pose;
							if (b.custom_pose_enable) {
								pose = b.custom_pose * pose;
							}

							// 修改点：尝试使用assimp-viewer的处理流程，它这里的处理流程意义不明，注释掉了原有处理流程
							if (ImportFileFormat::DEFAULT == gdi_import_file_format) {
								if (b.parent >= 0) {
	
									b.pose_global = bonesptr[b.parent].pose_global * (b.rest * pose);
								} else {
	
									b.pose_global = b.rest * pose;
								}
							}
							else if (ImportFileFormat::ASSIMP_FBX == gdi_import_file_format) {

								NodeAnim *node = b.gdi_node_anim ? b.gdi_node_anim :
									(gdi_anim_node_root ?
										gdi_find_anim_node_by_name(gdi_anim_node_root, b.name) : nullptr);
								if (nullptr == b.gdi_node_anim && node) {
									b.gdi_node_anim = node;
								}

								// 这个pose还不能随意设置，否则不能兼容其他tscn
								b.pose_global = b.gdi_node_anim ? b.gdi_node_anim->local_transform : b.rest;

								NodeAnim *parent = nullptr;
								if (node) {
									parent = node->parent;
								}

								// 兼容处理其他tscn过来的模型，如果没有从nodeAnim中找到parent（不正常情况）
								// 那么我们采用原有流程的global计算，进行兼容
								// 主要兼容的是gdi_anim_node_root，因为其他版本的文件是读取不到这个东西的
								if (nullptr == parent && -1 != b.parent && !gdi_anim_node_root) {
									b.pose_global = bonesptr[b.parent].pose_global * (b.rest * pose);
								}
								else if (nullptr == parent && -1 == b.parent && !gdi_anim_node_root) {
									b.pose_global = (b.rest * pose);
								}

								while (nullptr != parent) {
									b.pose_global = parent->local_transform * b.pose_global;
									parent = parent->parent;

									// 测试点：对比每次生成的global矩阵
	// 								if (b.name == "Bip001 L Forearm")
	// 								{
	// 									OS::get_singleton()->print("test parent[%S] mat:[%f], [%f], [%f]\n",
	// 										parent->name.ptr(),
	// 										b.pose_global.basis.elements[0].x,
	// 										b.pose_global.basis.elements[0].y,
	// 										b.pose_global.basis.elements[0].z);
	// 								}
								}
							}

						}
						else {

							if (b.parent >= 0) {

								b.pose_global = bonesptr[b.parent].pose_global * b.rest;
							} else {

								b.pose_global = b.rest;
							}
						}
					}

					if (b.global_pose_override_amount >= CMP_EPSILON) {
						b.pose_global = b.pose_global.interpolate_with(b.global_pose_override, b.global_pose_override_amount);
					}
				}

				if (b.global_pose_override_reset) {
					b.global_pose_override_amount = 0.0;
				}

				for (List<uint32_t>::Element *E = b.nodes_bound.front(); E; E = E->next()) {

					Object *obj = ObjectDB::get_instance(E->get());
					ERR_CONTINUE(!obj);
					Spatial *sp = Object::cast_to<Spatial>(obj);
					ERR_CONTINUE(!sp);
					sp->set_transform(b.pose_global);
				}
			}

			if (ImportFileFormat::ASSIMP_FBX == gdi_import_file_format && gdi_anim_node_root) {
				gdi_update_all_none_bone_anim_node(gdi_anim_node_root);
			}

			// 测试点：只更新一个skin，观察差异
			int iTest = 0;
			//update skins
			for (Set<SkinReference *>::Element *E = skin_bindings.front(); E; E = E->next()) {

				//测试点：
// 				if (0 != iTest) {
// 					continue;
// 				}
// 				if (0 == iTest) {
// 					iTest = 1;
// 				}

				const Skin *skin = E->get()->skin.operator->();
				RID skeleton = E->get()->skeleton;
				uint32_t bind_count = skin->get_bind_count();

				if (E->get()->bind_count != bind_count) {
					VS::get_singleton()->skeleton_allocate(skeleton, bind_count);
					E->get()->bind_count = bind_count;
					E->get()->skin_bone_indices.resize(bind_count);
					E->get()->skin_bone_indices_ptrs = E->get()->skin_bone_indices.ptrw();
				}

				if (E->get()->skeleton_version != version) {

					for (uint32_t i = 0; i < bind_count; i++) {
						StringName bind_name = skin->get_bind_name(i);

						if (bind_name != StringName()) {
							//bind name used, use this
							bool found = false;
							for (int j = 0; j < len; j++) {
								if (bonesptr[j].name == bind_name) {
									E->get()->skin_bone_indices_ptrs[i] = j;
									found = true;
									break;
								}
							}

							if (!found) {
								ERR_PRINT("Skin bind #" + itos(i) + " contains named bind '" + String(bind_name) + "' but Skeleton has no bone by that name.");
								E->get()->skin_bone_indices_ptrs[i] = 0;
							}
						} else if (skin->get_bind_bone(i) >= 0) {
							int bind_index = skin->get_bind_bone(i);
							if (bind_index >= len) {
								ERR_PRINT("Skin bind #" + itos(i) + " contains bone index bind: " + itos(bind_index) + " , which is greater than the skeleton bone count: " + itos(len) + ".");
								E->get()->skin_bone_indices_ptrs[i] = 0;
							} else {
								E->get()->skin_bone_indices_ptrs[i] = bind_index;
							}
						} else {
							ERR_PRINT("Skin bind #" + itos(i) + " does not contain a name nor a bone index.");
							E->get()->skin_bone_indices_ptrs[i] = 0;
						}
					}

					E->get()->skeleton_version = version;
				}

				for (uint32_t i = 0; i < bind_count; i++) {
					uint32_t bone_index = E->get()->skin_bone_indices_ptrs[i];
					ERR_CONTINUE(bone_index >= (uint32_t)len);

					if (ImportFileFormat::DEFAULT == gdi_import_file_format) {
						vs->skeleton_bone_set_transform(skeleton, i, bonesptr[bone_index].pose_global * skin->get_bind_pose(i));
					} 
					else if (ImportFileFormat::ASSIMP_FBX == gdi_import_file_format) {
						// 测试点：查看这里的骨骼transform影响
						auto pose = skin->get_bind_pose(i);
						auto global_pose = bonesptr[bone_index].pose_global;
						auto final_pose = global_pose * pose;
						vs->skeleton_bone_set_transform(skeleton, i, final_pose);
					}
				}
			}

			dirty = false;
		} break;
	}
}

void Skeleton::clear_bones_global_pose_override() {
	for (int i = 0; i < bones.size(); i += 1) {
		bones.write[i].global_pose_override_amount = 0;
	}
	_make_dirty();
}

void Skeleton::set_bone_global_pose_override(int p_bone, const Transform &p_pose, float p_amount, bool p_persistent) {

	ERR_FAIL_INDEX(p_bone, bones.size());
	bones.write[p_bone].global_pose_override_amount = p_amount;
	bones.write[p_bone].global_pose_override = p_pose;
	bones.write[p_bone].global_pose_override_reset = !p_persistent;
	_make_dirty();
}

Transform Skeleton::get_bone_global_pose(int p_bone) const {

	ERR_FAIL_INDEX_V(p_bone, bones.size(), Transform());
	if (dirty)
		const_cast<Skeleton *>(this)->notification(NOTIFICATION_UPDATE_SKELETON);
	return bones[p_bone].pose_global;
}

// skeleton creation api
void Skeleton::add_bone(const String &p_name)
{
	ERR_FAIL_COND(p_name == "" || p_name.find(":") != -1 || p_name.find("/") != -1);

	for (int i = 0; i < bones.size(); i++) {

		ERR_FAIL_COND(bones[i].name == p_name);
	}

	// 测试点：查看weapon sub拷贝时，到底发生了什么
	if (p_name == "weapon sub") {
		int i = 0;
		++i;
	}

	Bone b;
	b.name = p_name;
	bones.push_back(b);
	process_order_dirty = true;
	version++;
	_make_dirty();
	update_gizmo();
}

int Skeleton::find_bone(const String &p_name) const {

	for (int i = 0; i < bones.size(); i++) {

		if (bones[i].name == p_name)
			return i;
	}

	return -1;
}

String Skeleton::get_bone_name(int p_bone) const {

	ERR_FAIL_INDEX_V(p_bone, bones.size(), "");

	return bones[p_bone].name;
}

// void Skeleton::SetNodeAnimRoot(int64_t rootAddr)
// {
// 	anim_node_root = static_cast<NodeAnim*>((void*)rootAddr);
// }
// 
// int64_t Skeleton::GetNodeAnimRoot() const
// {
// 	return (int64_t)(anim_node_root);
// }

Skeleton::NodeAnim* Skeleton::gdi_find_anim_node_by_name(NodeAnim *root, String boneName)
{
	Skeleton::NodeAnim *node = nullptr;
	_gdi_find_anim_node_recursive(root, boneName, &node);
	return node;
}

void Skeleton::_gdi_find_anim_node_recursive(NodeAnim *nodeAnim, String boneName, NodeAnim **node) {

	if (nullptr == nodeAnim) {
		return;
	}

	if (nodeAnim->name == boneName) {
		*node = nodeAnim;
		return;
	}

	for (int i = 0; i < nodeAnim->childs.size(); ++i) {
		_gdi_find_anim_node_recursive(nodeAnim->childs[i], boneName, node);
	}
}

void Skeleton::gdi_update_all_none_bone_anim_node(NodeAnim *node) {

	//if (node->name == String("renwu_1")) {
	//	int i = 0;
	//	++i;
	//}

// 	if (!node->is_bone) {
		node->global_transform = node->local_transform;
		NodeAnim *parentNode = node->parent;
		while (parentNode) {
			node->global_transform = parentNode->local_transform * node->global_transform;
			parentNode = parentNode->parent;
		}

		// mesh都应该进行联动？？目前看来的情况好像是这样
		if (nullptr != gdi_editor_root &&
			((node->is_mesh && 0 == node->bone_num) /*|| (node->is_bone && -1 == node->channel_id)*/)) {
			Spatial *spa = Object::cast_to<Spatial>(gdi_editor_root->find_node(node->name));
			if (nullptr != spa) {
				spa->set_global_transform(node->global_transform);
			}
		}
// 	}

	for (int i = 0; i < node->childs.size(); ++i) {
		gdi_update_all_none_bone_anim_node(node->childs[i]);
	}
}

bool Skeleton::gdi_update_mesh_anim_node(const String &p_old_name, const String &p_new_name) {

	int size = gdi_anim_node_save_vec.size();
	for (int i = 0; i < size; ++i) {
		if (gdi_anim_node_save_vec[i]->name == p_old_name) {
			gdi_anim_node_save_vec.ptrw()[i]->name = p_new_name;
			return true;
		}
	}

	return false;
}

bool Skeleton::is_bone_parent_of(int p_bone, int p_parent_bone_id) const {

	int parent_of_bone = get_bone_parent(p_bone);

	if (-1 == parent_of_bone)
		return false;

	if (parent_of_bone == p_parent_bone_id)
		return true;

	return is_bone_parent_of(parent_of_bone, p_parent_bone_id);
}

int Skeleton::get_bone_count() const {

	return bones.size();
}

void Skeleton::set_bone_parent(int p_bone, int p_parent) {

	ERR_FAIL_INDEX(p_bone, bones.size());
	ERR_FAIL_COND(p_parent != -1 && (p_parent < 0));

	bones.write[p_bone].parent = p_parent;
	process_order_dirty = true;
	_make_dirty();
}

void Skeleton::unparent_bone_and_rest(int p_bone) {

	ERR_FAIL_INDEX(p_bone, bones.size());

	_update_process_order();

	int parent = bones[p_bone].parent;
	while (parent >= 0) {
		bones.write[p_bone].rest = bones[parent].rest * bones[p_bone].rest;
		parent = bones[parent].parent;
	}

	bones.write[p_bone].parent = -1;
	process_order_dirty = true;

	_make_dirty();
}

void Skeleton::set_bone_disable_rest(int p_bone, bool p_disable) {

	ERR_FAIL_INDEX(p_bone, bones.size());
	bones.write[p_bone].disable_rest = p_disable;
}

bool Skeleton::is_bone_rest_disabled(int p_bone) const {

	ERR_FAIL_INDEX_V(p_bone, bones.size(), false);
	return bones[p_bone].disable_rest;
}

int Skeleton::get_bone_parent(int p_bone) const {

	ERR_FAIL_INDEX_V(p_bone, bones.size(), -1);

	return bones[p_bone].parent;
}

void Skeleton::set_bone_rest(int p_bone, const Transform &p_rest) {

	ERR_FAIL_INDEX(p_bone, bones.size());

	bones.write[p_bone].rest = p_rest;
	_make_dirty();
}
Transform Skeleton::get_bone_rest(int p_bone) const {

	ERR_FAIL_INDEX_V(p_bone, bones.size(), Transform());

	return bones[p_bone].rest;
}

void Skeleton::set_bone_enabled(int p_bone, bool p_enabled) {

	ERR_FAIL_INDEX(p_bone, bones.size());

	bones.write[p_bone].enabled = p_enabled;
	_make_dirty();
}
bool Skeleton::is_bone_enabled(int p_bone) const {

	ERR_FAIL_INDEX_V(p_bone, bones.size(), false);
	return bones[p_bone].enabled;
}

void Skeleton::bind_child_node_to_bone(int p_bone, Node *p_node) {

	ERR_FAIL_NULL(p_node);
	ERR_FAIL_INDEX(p_bone, bones.size());

	uint32_t id = p_node->get_instance_id();

	for (const List<uint32_t>::Element *E = bones[p_bone].nodes_bound.front(); E; E = E->next()) {

		if (E->get() == id)
			return; // already here
	}

	bones.write[p_bone].nodes_bound.push_back(id);
}
void Skeleton::unbind_child_node_from_bone(int p_bone, Node *p_node) {

	ERR_FAIL_NULL(p_node);
	ERR_FAIL_INDEX(p_bone, bones.size());

	uint32_t id = p_node->get_instance_id();
	bones.write[p_bone].nodes_bound.erase(id);
}
void Skeleton::get_bound_child_nodes_to_bone(int p_bone, List<Node *> *p_bound) const {

	ERR_FAIL_INDEX(p_bone, bones.size());

	for (const List<uint32_t>::Element *E = bones[p_bone].nodes_bound.front(); E; E = E->next()) {

		Object *obj = ObjectDB::get_instance(E->get());
		ERR_CONTINUE(!obj);
		p_bound->push_back(Object::cast_to<Node>(obj));
	}
}

void Skeleton::clear_bones() {

	bones.clear();
	process_order_dirty = true;
	version++;
	_make_dirty();
}

// posing api

void Skeleton::set_bone_pose(int p_bone, const Transform &p_pose) {

	ERR_FAIL_INDEX(p_bone, bones.size());

	bones.write[p_bone].pose = p_pose;
	// 修改点：同时更新animNode的tranfrom（local和global）
	// 错误：经过测试后发现不能在这里进行更新global，因为这里所有的channels没有更新完毕，这样的话有些父节点的local trans就没有更新
	// 这里只能更新对应nodeAnim的local trans
	//if (bones.write[p_bone].name == "weapon sub") {
	//	int i = 0;
	//	++i;
	//}

	if (nullptr == bones.write[p_bone].gdi_node_anim) {
		NodeAnim *root = (NodeAnim*)gdi_get_anim_root_node_addr();
		bones.write[p_bone].gdi_node_anim = (nullptr == root) ? nullptr : gdi_find_anim_node_by_name(root, bones.write[p_bone].name);
	}

	NodeAnim *nodeAnim = bones.write[p_bone].gdi_node_anim;
	if (nodeAnim /*&& nodeAnim->channelId != -1*/) {
		nodeAnim->local_transform = p_pose;
	}

	if (is_inside_tree()) {
		_make_dirty();
	}
}
Transform Skeleton::get_bone_pose(int p_bone) const {

	ERR_FAIL_INDEX_V(p_bone, bones.size(), Transform());
	return bones[p_bone].pose;
}

void Skeleton::gdi_set_none_bone_pose(StringName name, const Transform &p_pose) {

	NodeAnim *root = (NodeAnim*)gdi_get_anim_root_node_addr();
	NodeAnim *nodeAnim = (nullptr == root) ? nullptr : gdi_find_anim_node_by_name(root, name);

	if (nodeAnim) {
		nodeAnim->local_transform = p_pose;
	}

	_make_dirty();
}

int64_t Skeleton::gdi_get_anim_root_node_addr() const {

	return gdi_anim_node_addr;
}

void Skeleton::gdi_set_anim_root_node_addr(int64_t addr) {

	gdi_anim_node_addr = addr;
	gdi_anim_node_root = (NodeAnim *)addr;

	static std::function<void(NodeAnim *, unsigned int &, Vector<NodeAnim*> &)> gen_anim_node_tree_id_func = [&](NodeAnim *node, unsigned int &startId, Vector<NodeAnim*> &animNodeVec) {
		node->node_id = startId++;
		animNodeVec.push_back(node);

		for (int i = 0; i < node->childs.size(); ++i) {
			gen_anim_node_tree_id_func(node->childs[i], startId, animNodeVec);
		}
	};
	unsigned int Id = 0;
	gdi_anim_node_save_vec.clear();
	if (gdi_anim_node_root) {
		gen_anim_node_tree_id_func(gdi_anim_node_root, Id, gdi_anim_node_save_vec);
	}
}

Skeleton::NodeAnim* Skeleton::gdi_get_bone_anim_node(int p_bone) const {

	return bones.ptr()[p_bone].gdi_node_anim;
}

void Skeleton::gdi_set_bone_anim_node(int p_bone, int64_t addr) {

	bones.ptrw()[p_bone].gdi_node_anim = (NodeAnim *)addr;
}

void Skeleton::gdi_set_editor_scene_root(Node *root) {

	gdi_editor_root = root;
}

Node* Skeleton::gdi_get_editor_scene_root() const {

	return gdi_editor_root;
}

Transform Skeleton::gdi_get_bone_pose_by_name(const String &name, bool &find_flag) const {

	find_flag = false;
	auto size = bones.size();
	for (auto i = 0; i < size; ++i) {
		if (bones.ptr()[i].name == name) {
			find_flag = true;
			return bones.ptr()[i].pose;
		}
	}

	return Transform();
}

void Skeleton::gdi_set_show_bones_property_flag(bool flag) {
	gdi_show_bones_property_flag = flag;
}

bool Skeleton::gdi_get_show_bones_property_flag() const {
	return gdi_show_bones_property_flag;
}

void Skeleton::set_bone_custom_pose(int p_bone, const Transform &p_custom_pose) {

	ERR_FAIL_INDEX(p_bone, bones.size());
	//ERR_FAIL_COND( !is_inside_scene() );

	bones.write[p_bone].custom_pose_enable = (p_custom_pose != Transform());
	bones.write[p_bone].custom_pose = p_custom_pose;

	_make_dirty();
}

Transform Skeleton::get_bone_custom_pose(int p_bone) const {

	ERR_FAIL_INDEX_V(p_bone, bones.size(), Transform());
	return bones[p_bone].custom_pose;
}

void Skeleton::_make_dirty() {

	if (dirty)
		return;

	MessageQueue::get_singleton()->push_notification(this, NOTIFICATION_UPDATE_SKELETON);
	dirty = true;
}

int Skeleton::get_process_order(int p_idx) {
	ERR_FAIL_INDEX_V(p_idx, bones.size(), -1);
	_update_process_order();
	return process_order[p_idx];
}

void Skeleton::localize_rests() {

	_update_process_order();

	for (int i = bones.size() - 1; i >= 0; i--) {
		int idx = process_order[i];
		if (bones[idx].parent >= 0) {
			set_bone_rest(idx, bones[bones[idx].parent].rest.affine_inverse() * bones[idx].rest);
		}
	}
}

#ifndef _3D_DISABLED

void Skeleton::bind_physical_bone_to_bone(int p_bone, PhysicalBone *p_physical_bone) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	ERR_FAIL_COND(bones[p_bone].physical_bone);
	ERR_FAIL_COND(!p_physical_bone);
	bones.write[p_bone].physical_bone = p_physical_bone;

	_rebuild_physical_bones_cache();
}

void Skeleton::unbind_physical_bone_from_bone(int p_bone) {
	ERR_FAIL_INDEX(p_bone, bones.size());
	bones.write[p_bone].physical_bone = NULL;

	_rebuild_physical_bones_cache();
}

PhysicalBone *Skeleton::get_physical_bone(int p_bone) {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), NULL);

	return bones[p_bone].physical_bone;
}

PhysicalBone *Skeleton::get_physical_bone_parent(int p_bone) {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), NULL);

	if (bones[p_bone].cache_parent_physical_bone) {
		return bones[p_bone].cache_parent_physical_bone;
	}

	return _get_physical_bone_parent(p_bone);
}

PhysicalBone *Skeleton::_get_physical_bone_parent(int p_bone) {
	ERR_FAIL_INDEX_V(p_bone, bones.size(), NULL);

	const int parent_bone = bones[p_bone].parent;
	if (0 > parent_bone) {
		return NULL;
	}

	PhysicalBone *pb = bones[parent_bone].physical_bone;
	if (pb) {
		return pb;
	} else {
		return get_physical_bone_parent(parent_bone);
	}
}

void Skeleton::_rebuild_physical_bones_cache() {
	const int b_size = bones.size();
	for (int i = 0; i < b_size; ++i) {
		PhysicalBone *parent_pb = _get_physical_bone_parent(i);
		if (parent_pb != bones[i].physical_bone) {
			bones.write[i].cache_parent_physical_bone = parent_pb;
			if (bones[i].physical_bone)
				bones[i].physical_bone->_on_bone_parent_changed();
		}
	}
}

void _pb_stop_simulation(Node *p_node) {

	for (int i = p_node->get_child_count() - 1; 0 <= i; --i) {
		_pb_stop_simulation(p_node->get_child(i));
	}

	PhysicalBone *pb = Object::cast_to<PhysicalBone>(p_node);
	if (pb) {
		pb->set_simulate_physics(false);
		pb->set_static_body(false);
	}
}

void Skeleton::physical_bones_stop_simulation() {
	_pb_stop_simulation(this);
}

void _pb_start_simulation(const Skeleton *p_skeleton, Node *p_node, const Vector<int> &p_sim_bones) {

	for (int i = p_node->get_child_count() - 1; 0 <= i; --i) {
		_pb_start_simulation(p_skeleton, p_node->get_child(i), p_sim_bones);
	}

	PhysicalBone *pb = Object::cast_to<PhysicalBone>(p_node);
	if (pb) {
		bool sim = false;
		for (int i = p_sim_bones.size() - 1; 0 <= i; --i) {
			if (p_sim_bones[i] == pb->get_bone_id() || p_skeleton->is_bone_parent_of(pb->get_bone_id(), p_sim_bones[i])) {
				sim = true;
				break;
			}
		}

		pb->set_simulate_physics(true);
		if (sim) {
			pb->set_static_body(false);
		} else {
			pb->set_static_body(true);
		}
	}
}

void Skeleton::physical_bones_start_simulation_on(const Array &p_bones) {

	Vector<int> sim_bones;
	if (p_bones.size() <= 0) {
		sim_bones.push_back(0); // if no bones is specified, activate ragdoll on full body
	} else {
		sim_bones.resize(p_bones.size());
		int c = 0;
		for (int i = sim_bones.size() - 1; 0 <= i; --i) {
			if (Variant::STRING == p_bones.get(i).get_type()) {
				int bone_id = find_bone(p_bones.get(i));
				if (bone_id != -1)
					sim_bones.write[c++] = bone_id;
			}
		}
		sim_bones.resize(c);
	}

	_pb_start_simulation(this, this, sim_bones);
}

void _physical_bones_add_remove_collision_exception(bool p_add, Node *p_node, RID p_exception) {

	for (int i = p_node->get_child_count() - 1; 0 <= i; --i) {
		_physical_bones_add_remove_collision_exception(p_add, p_node->get_child(i), p_exception);
	}

	CollisionObject *co = Object::cast_to<CollisionObject>(p_node);
	if (co) {
		if (p_add) {
			PhysicsServer::get_singleton()->body_add_collision_exception(co->get_rid(), p_exception);
		} else {
			PhysicsServer::get_singleton()->body_remove_collision_exception(co->get_rid(), p_exception);
		}
	}
}

void Skeleton::physical_bones_add_collision_exception(RID p_exception) {
	_physical_bones_add_remove_collision_exception(true, this, p_exception);
}

void Skeleton::physical_bones_remove_collision_exception(RID p_exception) {
	_physical_bones_add_remove_collision_exception(false, this, p_exception);
}

#endif // _3D_DISABLED

void Skeleton::_skin_changed() {
	_make_dirty();
}

Ref<SkinReference> Skeleton::register_skin(const Ref<Skin> &p_skin) {

	for (Set<SkinReference *>::Element *E = skin_bindings.front(); E; E = E->next()) {
		if (E->get()->skin == p_skin) {
			return Ref<SkinReference>(E->get());
		}
	}

	Ref<Skin> skin = p_skin;

	if (skin.is_null()) {
		//need to create one from existing code, this is for compatibility only
		//when skeletons did not support skins. It is also used by gizmo
		//to display the skeleton.

		skin.instance();
		skin->set_bind_count(bones.size());
		_update_process_order(); //just in case

		// pose changed, rebuild cache of inverses
		const Bone *bonesptr = bones.ptr();
		int len = bones.size();
		const int *order = process_order.ptr();

		// calculate global rests and invert them
		for (int i = 0; i < len; i++) {
			const Bone &b = bonesptr[order[i]];
			if (b.parent >= 0) {
				skin->set_bind_pose(order[i], skin->get_bind_pose(b.parent) * b.rest);
			} else {
				skin->set_bind_pose(order[i], b.rest);
			}
		}

		for (int i = 0; i < len; i++) {
			//the inverse is what is actually required
			skin->set_bind_bone(i, i);
			skin->set_bind_pose(i, skin->get_bind_pose(i).affine_inverse());
		}
	}

	ERR_FAIL_COND_V(skin.is_null(), Ref<SkinReference>());

	Ref<SkinReference> skin_ref;
	skin_ref.instance();

	skin_ref->skeleton_node = this;
	skin_ref->bind_count = 0;
	skin_ref->skeleton = VisualServer::get_singleton()->skeleton_create();
	skin_ref->skeleton_node = this;
	skin_ref->skin = skin;

	skin_bindings.insert(skin_ref.operator->());

	skin->connect("changed", skin_ref.operator->(), "_skin_changed");
	_make_dirty();
	return skin_ref;
}

void Skeleton::_bind_methods() {

	ClassDB::bind_method(D_METHOD("add_bone", "name"), &Skeleton::add_bone);
	ClassDB::bind_method(D_METHOD("find_bone", "name"), &Skeleton::find_bone);
	ClassDB::bind_method(D_METHOD("get_bone_name", "bone_idx"), &Skeleton::get_bone_name);

	ClassDB::bind_method(D_METHOD("get_bone_parent", "bone_idx"), &Skeleton::get_bone_parent);
	ClassDB::bind_method(D_METHOD("set_bone_parent", "bone_idx", "parent_idx"), &Skeleton::set_bone_parent);

	ClassDB::bind_method(D_METHOD("get_bone_count"), &Skeleton::get_bone_count);

	ClassDB::bind_method(D_METHOD("unparent_bone_and_rest", "bone_idx"), &Skeleton::unparent_bone_and_rest);

	ClassDB::bind_method(D_METHOD("get_bone_rest", "bone_idx"), &Skeleton::get_bone_rest);
	ClassDB::bind_method(D_METHOD("set_bone_rest", "bone_idx", "rest"), &Skeleton::set_bone_rest);

	ClassDB::bind_method(D_METHOD("register_skin", "skin"), &Skeleton::register_skin);

	ClassDB::bind_method(D_METHOD("localize_rests"), &Skeleton::localize_rests);

	ClassDB::bind_method(D_METHOD("set_bone_disable_rest", "bone_idx", "disable"), &Skeleton::set_bone_disable_rest);
	ClassDB::bind_method(D_METHOD("is_bone_rest_disabled", "bone_idx"), &Skeleton::is_bone_rest_disabled);

	ClassDB::bind_method(D_METHOD("bind_child_node_to_bone", "bone_idx", "node"), &Skeleton::bind_child_node_to_bone);
	ClassDB::bind_method(D_METHOD("unbind_child_node_from_bone", "bone_idx", "node"), &Skeleton::unbind_child_node_from_bone);
	ClassDB::bind_method(D_METHOD("get_bound_child_nodes_to_bone", "bone_idx"), &Skeleton::_get_bound_child_nodes_to_bone);

	ClassDB::bind_method(D_METHOD("clear_bones"), &Skeleton::clear_bones);

	ClassDB::bind_method(D_METHOD("get_bone_pose", "bone_idx"), &Skeleton::get_bone_pose);
	ClassDB::bind_method(D_METHOD("set_bone_pose", "bone_idx", "pose"), &Skeleton::set_bone_pose);

	ClassDB::bind_method(D_METHOD("clear_bones_global_pose_override"), &Skeleton::clear_bones_global_pose_override);
	ClassDB::bind_method(D_METHOD("set_bone_global_pose_override", "bone_idx", "pose", "amount", "persistent"), &Skeleton::set_bone_global_pose_override, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("get_bone_global_pose", "bone_idx"), &Skeleton::get_bone_global_pose);

	ClassDB::bind_method(D_METHOD("get_bone_custom_pose", "bone_idx"), &Skeleton::get_bone_custom_pose);
	ClassDB::bind_method(D_METHOD("set_bone_custom_pose", "bone_idx", "custom_pose"), &Skeleton::set_bone_custom_pose);

	ClassDB::bind_method(D_METHOD("gdi_get_show_bones_property_flag"), &Skeleton::gdi_get_show_bones_property_flag);
	ClassDB::bind_method(D_METHOD("gdi_set_show_bones_property_flag", "flag"), &Skeleton::gdi_set_show_bones_property_flag);

#ifndef _3D_DISABLED

	ClassDB::bind_method(D_METHOD("physical_bones_stop_simulation"), &Skeleton::physical_bones_stop_simulation);
	ClassDB::bind_method(D_METHOD("physical_bones_start_simulation", "bones"), &Skeleton::physical_bones_start_simulation_on, DEFVAL(Array()));
	ClassDB::bind_method(D_METHOD("physical_bones_add_collision_exception", "exception"), &Skeleton::physical_bones_add_collision_exception);
	ClassDB::bind_method(D_METHOD("physical_bones_remove_collision_exception", "exception"), &Skeleton::physical_bones_remove_collision_exception);

#endif // _3D_DISABLED

	BIND_CONSTANT(NOTIFICATION_UPDATE_SKELETON);
}

Skeleton::Skeleton()
	:gdi_anim_node_root(nullptr)
	, gdi_anim_node_addr(0)
	, gdi_import_file_format(ImportFileFormat::DEFAULT)
{

	dirty = false;
	version = 1;
	process_order_dirty = true;

	// 修改点：
	gdi_anim_node_load_vec.clear();
	gdi_anim_node_save_vec.clear();
	gdi_editor_root = nullptr;
	gdi_show_bones_property_flag = false;
}

Skeleton::~Skeleton() {

	//some skins may remain bound
	for (Set<SkinReference *>::Element *E = skin_bindings.front(); E; E = E->next()) {
		E->get()->skeleton_node = nullptr;
	}
}
