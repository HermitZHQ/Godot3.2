/*************************************************************************/
/*  editor_scene_importer_assimp.cpp                                     */
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

#include "editor_scene_importer_assimp.h"
#include "core/io/image_loader.h"
#include "editor/import/resource_importer_scene.h"
#include "import_utils.h"
#include "scene/3d/camera.h"
#include "scene/3d/light.h"
#include "scene/3d/mesh_instance.h"
#include "scene/main/node.h"
#include "scene/resources/material.h"
#include "scene/resources/surface_tool.h"

#include <assimp/matrix4x4.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/LogStream.hpp>
#include "thirdparty/assimp/code/FBX/FBXCommon.h"
#include "core/math/quat.h"

#include "assimp/mesh.h"

// move into assimp
aiBone *get_bone_by_name(const aiScene *scene, aiString bone_name) {
	for (unsigned int mesh_id = 0; mesh_id < scene->mNumMeshes; ++mesh_id) {
		aiMesh *mesh = scene->mMeshes[mesh_id];

		// iterate over all the bones on the mesh for this node only!
		for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++) {

			aiBone *bone = mesh->mBones[boneIndex];
			if (bone->mName == bone_name) {
// 				printf("matched bone by name: %s\n", bone->mName.C_Str());
				return bone;
			}
		}
	}

	return NULL;
}

void EditorSceneImporterAssimp::get_extensions(List<String> *r_extensions) const {

	const String import_setting_string = "filesystem/import/open_asset_import/";

	Map<String, ImportFormat> import_format;
	{
		Vector<String> exts;
		exts.push_back("fbx");
		ImportFormat import = { exts, true };
		import_format.insert("fbx", import);
	}
	for (Map<String, ImportFormat>::Element *E = import_format.front(); E; E = E->next()) {
		_register_project_setting_import(E->key(), import_setting_string, E->get().extensions, r_extensions,
				E->get().is_default);
	}
}

void EditorSceneImporterAssimp::_register_project_setting_import(const String generic, const String import_setting_string,
		const Vector<String> &exts, List<String> *r_extensions,
		const bool p_enabled) const {
	const String use_generic = "use_" + generic;
	_GLOBAL_DEF(import_setting_string + use_generic, p_enabled, true);
	if (ProjectSettings::get_singleton()->get(import_setting_string + use_generic)) {
		for (int32_t i = 0; i < exts.size(); i++) {
			r_extensions->push_back(exts[i]);
		}
	}
}

uint32_t EditorSceneImporterAssimp::get_import_flags() const {
	return IMPORT_SCENE;
}

void EditorSceneImporterAssimp::_bind_methods() {
}

Node *EditorSceneImporterAssimp::import_scene(const String &p_path, uint32_t p_flags, int p_bake_fps,
		List<String> *r_missing_deps, Error *r_err) {
	Assimp::Importer importer;
	//importer.SetPropertyBool(AI_CONFIG_PP_FD_REMOVE, true);
	// Cannot remove pivot points because the static mesh will be in the wrong place
	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, p_flags & GDI_IMPORT_ANIMATION_ASSIMP_PIVOT);
	//importer.SetPropertyBool(AI_CONFIG_PP_OG_EXCLUDE_LIST, false);
	//importer.SetPropertyBool(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, true);
	int32_t max_bone_weights = 4;
	//if (p_flags & IMPORT_ANIMATION_EIGHT_WEIGHTS) {
	//	const int eight_bones = 8;
	//	importer.SetPropertyBool(AI_CONFIG_PP_LBW_MAX_WEIGHTS, eight_bones);
	//	max_bone_weights = eight_bones;
	//}

	importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);

	//importer.SetPropertyFloat(AI_CONFIG_PP_DB_THRESHOLD, 1.0f);
	int32_t post_process_Steps = aiProcess_CalcTangentSpace |
								 aiProcess_GlobalScale |
								 // imports models and listens to their file scale for CM to M conversions
								 //aiProcess_FlipUVs |
								 aiProcess_FlipWindingOrder |
								 // very important for culling so that it is done in the correct order.
								 //aiProcess_DropNormals |
								 aiProcess_GenSmoothNormals |
								 //aiProcess_JoinIdenticalVertices |
								 aiProcess_ImproveCacheLocality |
								 //aiProcess_RemoveRedundantMaterials | // Causes a crash
								 //aiProcess_SplitLargeMeshes |
								 aiProcess_Triangulate |
								 aiProcess_GenUVCoords |
								 //aiProcess_FindDegenerates |
								 //aiProcess_SortByPType |
								 // aiProcess_FindInvalidData |
								 aiProcess_TransformUVCoords |
								 aiProcess_FindInstances |
								 //aiProcess_FixInfacingNormals |
								 //aiProcess_ValidateDataStructure |
								 aiProcess_OptimizeMeshes |
								 //aiProcess_PopulateArmatureData |
								 //aiProcess_OptimizeGraph |
								 //aiProcess_Debone |
								 //aiProcess_EmbedTextures |
								 //aiProcess_SplitByBoneCount |
								 0;
	//uint64_t time = OS::get_singleton()->get_system_time_msecs();
	String g_path = ProjectSettings::get_singleton()->globalize_path(p_path);
// 	post_process_Steps = 62889679;// assimp viewer steps
	aiScene *scene = (aiScene *)importer.ReadFile(g_path.utf8().ptr(), post_process_Steps);

	ERR_FAIL_COND_V_MSG(scene == NULL, NULL, String("Open Asset Import failed to open: ") + String(importer.GetErrorString()));

	Spatial *spa = _generate_scene(p_path, scene, p_flags, p_bake_fps, max_bone_weights);
	//OS::get_singleton()->print("import fbx cast time[%d]\n", OS::get_singleton()->get_system_time_msecs() - time);
	return spa;
}

template <class T>
struct EditorSceneImporterAssetImportInterpolate {

	T lerp(const T &a, const T &b, float c) const {

		return a + (b - a) * c;
	}

	T catmull_rom(const T &p0, const T &p1, const T &p2, const T &p3, float t) {

		float t2 = t * t;
		float t3 = t2 * t;

		return 0.5f * ((2.0f * p1) + (-p0 + p2) * t + (2.0f * p0 - 5.0f * p1 + 4 * p2 - p3) * t2 +
							  (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
	}

	T bezier(T start, T control_1, T control_2, T end, float t) {
		/* Formula from Wikipedia article on Bezier curves. */
		real_t omt = (1.0 - t);
		real_t omt2 = omt * omt;
		real_t omt3 = omt2 * omt;
		real_t t2 = t * t;
		real_t t3 = t2 * t;

		return start * omt3 + control_1 * omt2 * t * 3.0 + control_2 * omt * t2 * 3.0 + end * t3;
	}
};

//thank you for existing, partial specialization
template <>
struct EditorSceneImporterAssetImportInterpolate<Quat> {

	Quat lerp(const Quat &a, const Quat &b, float c) const {
		ERR_FAIL_COND_V_MSG(!a.is_normalized(), Quat(), "The quaternion \"a\" must be normalized.");
		ERR_FAIL_COND_V_MSG(!b.is_normalized(), Quat(), "The quaternion \"b\" must be normalized.");

		return a.slerp(b, c).normalized();
	}

	Quat catmull_rom(const Quat &p0, const Quat &p1, const Quat &p2, const Quat &p3, float c) {
		ERR_FAIL_COND_V_MSG(!p1.is_normalized(), Quat(), "The quaternion \"p1\" must be normalized.");
		ERR_FAIL_COND_V_MSG(!p2.is_normalized(), Quat(), "The quaternion \"p2\" must be normalized.");

		return p1.slerp(p2, c).normalized();
	}

	Quat bezier(Quat start, Quat control_1, Quat control_2, Quat end, float t) {
		ERR_FAIL_COND_V_MSG(!start.is_normalized(), Quat(), "The start quaternion must be normalized.");
		ERR_FAIL_COND_V_MSG(!end.is_normalized(), Quat(), "The end quaternion must be normalized.");

		return start.slerp(end, t).normalized();
	}
};

template <class T>
T EditorSceneImporterAssimp::_interpolate_track(const Vector<float> &p_times, const Vector<T> &p_values, float p_time,
		AssetImportAnimation::Interpolation p_interp) {
	//could use binary search, worth it?
	int idx = -1;
	for (int i = 0; i < p_times.size(); i++) {
		if (p_times[i] > p_time)
			break;
		idx++;
	}

	EditorSceneImporterAssetImportInterpolate<T> interp;

	switch (p_interp) {
		case AssetImportAnimation::INTERP_LINEAR: {

			if (idx == -1) {
				return p_values[0];
			} else if (idx >= p_times.size() - 1) {
				return p_values[p_times.size() - 1];
			}

			float c = (p_time - p_times[idx]) / (p_times[idx + 1] - p_times[idx]);

			return interp.lerp(p_values[idx], p_values[idx + 1], c);

		} break;
		case AssetImportAnimation::INTERP_STEP: {

			if (idx == -1) {
				return p_values[0];
			} else if (idx >= p_times.size() - 1) {
				return p_values[p_times.size() - 1];
			}

			return p_values[idx];

		} break;
		case AssetImportAnimation::INTERP_CATMULLROMSPLINE: {

			if (idx == -1) {
				return p_values[1];
			} else if (idx >= p_times.size() - 1) {
				return p_values[1 + p_times.size() - 1];
			}

			float c = (p_time - p_times[idx]) / (p_times[idx + 1] - p_times[idx]);

			return interp.catmull_rom(p_values[idx - 1], p_values[idx], p_values[idx + 1], p_values[idx + 3], c);

		} break;
		case AssetImportAnimation::INTERP_CUBIC_SPLINE: {

			if (idx == -1) {
				return p_values[1];
			} else if (idx >= p_times.size() - 1) {
				return p_values[(p_times.size() - 1) * 3 + 1];
			}

			float c = (p_time - p_times[idx]) / (p_times[idx + 1] - p_times[idx]);

			T from = p_values[idx * 3 + 1];
			T c1 = from + p_values[idx * 3 + 2];
			T to = p_values[idx * 3 + 4];
			T c2 = to + p_values[idx * 3 + 3];

			return interp.bezier(from, c1, c2, to, c);

		} break;
	}

	ERR_FAIL_V(p_values[0]);
}

aiBone *EditorSceneImporterAssimp::get_bone_from_stack(ImportState &state, aiString name) {

	List<aiBone *>::Element *iter;
	aiBone *bone = NULL;
	for (iter = state.bone_stack.front(); iter; iter = iter->next()) {
		bone = (aiBone *)iter->get();

		if (bone && bone->mName == name) {
			state.bone_stack.erase(bone);
			return bone;
		}
	}

	return NULL;
}

aiBone* EditorSceneImporterAssimp::gdi_find_bone_from_stack(ImportState &state, const aiString &name) {

	List<aiBone *>::Element *iter;
	aiBone *bone = NULL;
	for (iter = state.bone_stack.front(); iter; iter = iter->next()) {
		bone = (aiBone *)iter->get();

		if (bone && bone->mName == name) {
			return bone;
		}
	}

	return nullptr;
}

bool EditorSceneImporterAssimp::_gdi_optimize_track_data(aiNodeAnim *track) {

	bool optimized_flag = false;
	// optimize the track data
	Vector<aiVectorKey> track_pos_vec;
	Vector<aiQuatKey> track_quat_vec;
	Vector<aiVectorKey> track_scale_vec;

	// optimize position keys----
	aiVector3D last_dir, tmp_dir;
	ai_real last_len = 0, tmp_len = 0;
	if (track->mNumPositionKeys > 0 && nullptr != track->mPositionKeys) {
		track_pos_vec.push_back(track->mPositionKeys[0]);

		for (int pos_idx = 0; pos_idx < track->mNumPositionKeys - 1; ++pos_idx) {
			tmp_len = (track->mPositionKeys[pos_idx + 1].mValue - track->mPositionKeys[pos_idx].mValue).Length();
			tmp_dir = (track->mPositionKeys[pos_idx + 1].mValue - track->mPositionKeys[pos_idx].mValue).NormalizeSafe();

			if (pos_idx > 0) {
				if (last_len != 0 && last_dir != tmp_dir) {
					track_pos_vec.push_back(track->mPositionKeys[pos_idx]);
				}
			}

			last_len = tmp_len;
			last_dir = tmp_dir;
		}
		track_pos_vec.push_back(track->mPositionKeys[track->mNumPositionKeys - 1]);

		if (track_pos_vec.size() != track->mNumPositionKeys) {
			if (track->mNumPositionKeys > 1) {
				delete[] track->mPositionKeys;
			}
			else {
				delete track->mPositionKeys;
			}

			if (2 != track_pos_vec.size()) {
				track->mNumPositionKeys = track_pos_vec.size();
				track->mPositionKeys = new aiVectorKey[track->mNumPositionKeys];
				for (int pos_idx = 0; pos_idx < track->mNumPositionKeys; ++pos_idx) {
					track->mPositionKeys[pos_idx] = track_pos_vec[pos_idx];
				}
			} 
			else {
				track->mNumPositionKeys = 0;
				track->mPositionKeys = nullptr;
			}
			optimized_flag = true;
		}
	}

	// optimize rotation keys----
	if (track->mNumRotationKeys > 0 && nullptr != track->mRotationKeys) {
		track_quat_vec.push_back(track->mRotationKeys[0]);

		bool tmp_quat_equal_flag = false, last_quat_equal_flag = false;
		for (int rot_idx = 0; rot_idx < track->mNumRotationKeys - 1; ++rot_idx) {
			tmp_quat_equal_flag = (track->mRotationKeys[rot_idx + 1] == track->mRotationKeys[rot_idx]);

			if (rot_idx > 0 && !last_quat_equal_flag) {
				track_quat_vec.push_back(track->mRotationKeys[rot_idx]);
			}

			last_quat_equal_flag = tmp_quat_equal_flag;
		}
		track_quat_vec.push_back(track->mRotationKeys[track->mNumRotationKeys - 1]);

		if (track_quat_vec.size() != track->mNumRotationKeys) {
			if (track->mNumRotationKeys > 1) {
				delete[] track->mRotationKeys;
			}
			else {
				delete track->mRotationKeys;
			}

			if (2 != track_quat_vec.size()) {
				track->mNumRotationKeys = track_quat_vec.size();
				track->mRotationKeys = new aiQuatKey[track->mNumRotationKeys];
				for (int rot_idx = 0; rot_idx < track->mNumRotationKeys; ++rot_idx) {
					track->mRotationKeys[rot_idx] = track_quat_vec[rot_idx];

					//if (abs(abs(track->mRotationKeys[rot_idx].mValue.x) - 0.707) < 0.001 &&
					//	abs(abs(track->mRotationKeys[rot_idx].mValue.w) - 0.707) < 0.001) {
					//	track->mRotationKeys[rot_idx].mValue.x = 0;
					//	track->mRotationKeys[rot_idx].mValue.y = 0;
					//	track->mRotationKeys[rot_idx].mValue.z = 0;
					//	track->mRotationKeys[rot_idx].mValue.w = 1;
					//}
				}
			} 
			else {
				track->mNumRotationKeys = 0;
				track->mRotationKeys = nullptr;
			}
			optimized_flag = true;
		}
	}

	// optimize scaling keys----
	if (track->mNumScalingKeys > 0 && nullptr != track->mScalingKeys) {
		track_scale_vec.push_back(track->mScalingKeys[0]);

		bool tmp_scale_equal_flag = false, last_scale_equal_flag = false;
		for (int scale_idx = 0; scale_idx < track->mNumScalingKeys - 1; ++scale_idx) {
			tmp_scale_equal_flag = (track->mScalingKeys[scale_idx + 1] == track->mScalingKeys[scale_idx]);

			if (scale_idx > 0 && !last_scale_equal_flag) {
				track_scale_vec.push_back(track->mScalingKeys[scale_idx]);
			}

			last_scale_equal_flag = tmp_scale_equal_flag;
		}
		track_scale_vec.push_back(track->mScalingKeys[track->mNumScalingKeys - 1]);

		if (track_scale_vec.size() != track->mNumScalingKeys) {
			if (track->mNumScalingKeys > 1) {
				delete[] track->mScalingKeys;
			}
			else {
				delete track->mScalingKeys;
			}

			if (2 != track_scale_vec.size()) {
				track->mNumScalingKeys = track_scale_vec.size();
				track->mScalingKeys = new aiVectorKey[track->mNumScalingKeys];
				for (int scale_idx = 0; scale_idx < track->mNumScalingKeys; ++scale_idx) {
					track->mScalingKeys[scale_idx] = track_scale_vec[scale_idx];
				}
			} 
			else {
				track->mNumScalingKeys = 0;
				track->mScalingKeys = nullptr;
			}
			optimized_flag = true;
		}
	}

	return optimized_flag;
}

Spatial *
EditorSceneImporterAssimp::_generate_scene(const String &p_path, aiScene *scene, const uint32_t p_flags, int p_bake_fps,
		const int32_t p_max_bone_weights) {

	ERR_FAIL_COND_V(scene == NULL, NULL);

	ImportState state;
	state.path = p_path;
	state.assimp_scene = scene;
	state.max_bone_weights = p_max_bone_weights;
	state.animation_player = NULL;
	state.import_flags = p_flags;
	state.gdi_armature_index_map.clear();

	// populate light map
	for (unsigned int l = 0; l < scene->mNumLights; l++) {

		aiLight *ai_light = scene->mLights[l];
		ERR_CONTINUE(ai_light == NULL);
		state.light_cache[AssimpUtils::get_assimp_string(ai_light->mName)] = l;
	}

	// fill camera cache
	for (unsigned int c = 0; c < scene->mNumCameras; c++) {
		aiCamera *ai_camera = scene->mCameras[c];
		ERR_CONTINUE(ai_camera == NULL);
		state.camera_cache[AssimpUtils::get_assimp_string(ai_camera->mName)] = c;
	}

	if (scene->mRootNode) {
		state.nodes.push_back(scene->mRootNode);

		// 注释点：生成所有节点数据，内部主要处理了如何生成Armature节点（生成方式见函数内部）
		// make flat node tree - in order to make processing deterministic
		for (unsigned int i = 0; i < scene->mRootNode->mNumChildren; i++) {
			_generate_node(state, scene->mRootNode->mChildren[i]);
		}

		// 注释点：根据scene重新获取所有bone节点，存入stack_bone下
		RegenerateBoneStack(state);

#ifdef GDI_ENABLE_ASSIMP_MODIFICATION
		// 修改点：创建animNode的所有节点链表用于动画寻找父节点
		gdi_create_anim_node_from_scene(scene, &gdi_node_anim_root);

		// 修改点：手动赋值总的armature节点给所有bone node，并给mNode赋值有效地址
		// 修改2：分开处理不同的mesh中的bone，绑定不同的armature
		int bone_stack_size = state.bone_stack.size();
		for (int i = 0; i < bone_stack_size; ++i) {
			state.bone_stack[i]->mArmature = state.armature_nodes[state.gdi_bone_stack_mesh_index[i]];

			for (auto n = state.nodes.front(); n; n = n->next()) {
				if (n->get()->mName == state.bone_stack[i]->mName) {
					state.bone_stack[i]->mNode = (aiNode*)n->get();
					break;
				}
			}
		}
#endif

		Node *last_valid_parent = NULL;

		List<const aiNode *>::Element *iter;
		// 循环创建所有节点，之前创建为Armature的节点转换为Skeleton（空间节点）节点（该节点中可以存放子bones）
		// 有父节点的会把子节点加入父节点的显示中
		// 之前不为armature的都会先生成spatial节点，后面的代码会进一步检测是否为mesh，是的话则转换为MeshInstance，之前的spatial节点会延迟删除
		// 要注意的是，新转换的mesh，可能名称会发生变化，这是之前导致bug的一个原因
		for (iter = state.nodes.front(); iter; iter = iter->next()) {
			const aiNode *element_assimp_node = iter->get();
			const aiNode *parent_assimp_node = element_assimp_node->mParent;

			String node_name = AssimpUtils::get_assimp_string(element_assimp_node->mName);
			//print_verbose("node: " + node_name);

			//if (element_assimp_node->mName == aiString("Maca")) {
			//	int i = 0;
			//	++i;
			//}

			Spatial *spatial = NULL;
			Transform transform = AssimpUtils::assimp_matrix_transform(element_assimp_node->mTransformation);

			// retrieve this node bone
			aiBone *bone = get_bone_from_stack(state, element_assimp_node->mName);

#ifdef GDI_ENABLE_ASSIMP_MODIFICATION
			// 修改点：如果mesh不匹配的话，bone应该重置为0
			// TODO：这里修改的不完善，应该还需要匹配mesh是否一致（因为bone也存在重名的情况）
			// 总结：这里其实不需要去get_bone，因为这个阶段的bone获取没有意义，可能是错误的
			// 全部添加到场景树用于更新即可
			// 此处的改动需要后续流程的支持，比如非BoneTrack的改动支持，以及Skeleton中update所有非bone的animNode的支持
			bone = (element_assimp_node->mNumMeshes == 0) ? nullptr : bone;
			bone = nullptr;
#endif

			if (state.light_cache.has(node_name)) {
				spatial = create_light(state, node_name, transform);
			} else if (state.camera_cache.has(node_name)) {
				spatial = create_camera(state, node_name, transform);
			} else if (state.armature_nodes.find(element_assimp_node)) {
				// create skeleton
				print_verbose("Making skeleton: " + node_name);
				Skeleton *skeleton = memnew(Skeleton);
				skeleton->gdi_set_import_file_format(Object::ASSIMP_FBX);
				spatial = skeleton;

#ifdef GDI_ENABLE_ASSIMP_MODIFICATION
				skeleton->gdi_set_anim_root_node_addr((int64_t)gdi_node_anim_root);
#endif

				if (!state.armature_skeletons.has(element_assimp_node)) {
					state.armature_skeletons.insert(element_assimp_node, skeleton);
				}

				// 修改点：尝试使用spatial节点替换skeleton，替换后不显示骨骼，可以显示静态模型
// 				spatial = memnew(Spatial);
			} else if (bone != NULL) {
				continue;
			} else {
				spatial = memnew(Spatial);
			}

			ERR_CONTINUE_MSG(spatial == NULL, "FBX Import - are we out of ram?");
			// we on purpose set the transform and name after creating the node.

			spatial->set_name(node_name);
			spatial->set_global_transform(transform);

			// first element is root
			if (iter == state.nodes.front()) {
				state.root = spatial;
			}

			// flat node map parent lookup tool
			state.flat_node_map.insert(element_assimp_node, spatial);

			Map<const aiNode *, Spatial *>::Element *parent_lookup = state.flat_node_map.find(parent_assimp_node);

			// note: this always fails on the root node :) keep that in mind this is by design
			if (parent_lookup) {
				Spatial *parent_node = parent_lookup->value();

				ERR_FAIL_COND_V_MSG(parent_node == NULL, state.root,
						"Parent node invalid even though lookup successful, out of ram?")

				if (spatial != state.root /*&& !bSkeletonFlag*/) {
					parent_node->add_child(spatial);
					spatial->set_owner(state.root);
				}
				else {
					// required - think about it root never has a parent yet is valid, anything else without a parent is not valid.
				}
			} else if (spatial != state.root) {
				// if the ainode is not in the tree
				// parent it to the last good parent found
				if (last_valid_parent) {
					last_valid_parent->add_child(spatial);
					spatial->set_owner(state.root);
				} else {
					// this is a serious error?
					memdelete(spatial);
				}
			}

			// update last valid parent
			last_valid_parent = spatial;
		}
		print_verbose("node counts: " + itos(state.nodes.size()));

		// make clean bone stack----------------------------------------------------
		RegenerateBoneStack(state);
		print_verbose("generating godot bone data");
		print_verbose("Godot bone stack count: " + itos(state.bone_stack.size()));

		// This is a list of bones, duplicates are from other meshes and must be dealt with properly
		for (List<aiBone *>::Element *element = state.bone_stack.front(); element; element = element->next()) {
			aiBone *bone = element->get();
			// 测试点：关闭骨骼信息录入，还需要关闭后面生成mesh时的地方
// 			continue;

			ERR_CONTINUE_MSG(!bone, "invalid bone read from assimp?");

			// Utilities for armature lookup - for now only FBX makes these
			aiNode *armature_for_bone = bone->mArmature;

			// Utilities for bone node lookup - for now only FBX makes these
			aiNode *bone_node = bone->mNode;
			aiNode *parent_node = bone_node->mParent;

			String bone_name = AssimpUtils::get_anim_string_from_assimp(bone->mName);
			//if (armature_for_bone == NULL) {
			//	ai_assert(0);
			//}
			ERR_CONTINUE_MSG(armature_for_bone == NULL, "Armature for bone invalid: " + bone_name);
			Skeleton *skeleton = state.armature_skeletons[armature_for_bone];

			state.skeleton_bone_map[bone] = skeleton;

			if (bone_name.empty()) {
				bone_name = "untitled_bone_name";
				WARN_PRINT("Untitled bone name detected... report with file please");
			}

			// todo: this is where skin support goescreate
			if (skeleton && skeleton->find_bone(bone_name) == -1) {
				print_verbose("[Godot Glue] Imported bone" + bone_name);
				int boneIdx = skeleton->get_bone_count();

				Transform pform = AssimpUtils::assimp_matrix_transform(bone->mNode->mTransformation);
				skeleton->add_bone(bone_name);
				skeleton->set_bone_rest(boneIdx, pform);
				skeleton->set_bone_pose(boneIdx, pform);

				//ai_assert(skeleton->get_bone_count() <= 110);

				if (parent_node != NULL) {
					int parent_bone_id = skeleton->find_bone(AssimpUtils::get_anim_string_from_assimp(parent_node->mName));
					int current_bone_id = boneIdx;

					// 修改点：尝试屏蔽父节点设置，这里不能随意改动，强行-1父节点会导致skin的骨架不显示
					// 这里的父节点的设置，其实是为了重新计算（校正）骨骼的offsetMat，很有用
					skeleton->set_bone_parent(current_bone_id, parent_bone_id);
				}
			}
		}

		//----------------------------------------------------------------------------------------
		print_verbose("generating mesh phase from skeletal mesh");
		List<Spatial *> cleanup_template_nodes;

		// 必须保持节点树的创建顺序，否则reparent时会出现问题，此问题也是引擎底层架构的问题
		int mesh_num = state.gdi_mesh_node_vec.size();
		for (unsigned int i = 0; i < mesh_num; ++i) {
			const aiNode *mesh_node = state.gdi_mesh_node_vec[i];

			for (Map<const aiNode *, Spatial *>::Element *key_value_pair = state.flat_node_map.front(); key_value_pair; key_value_pair = key_value_pair->next()) {
				const aiNode *assimp_node = key_value_pair->key();
				if (assimp_node != mesh_node) {
					continue;
				}
				if (0 == assimp_node->mNumMeshes) {
					continue;
				}

				Spatial *mesh_template = key_value_pair->value();
	
				//if (assimp_node->mName == aiString("Maca")) {
				//	int i = 0;
				//	++i;
				//}
	
				ERR_CONTINUE(assimp_node == NULL);
				ERR_CONTINUE(mesh_template == NULL);
	
				Node *parent_node = mesh_template->get_parent();
	
				if (mesh_template == state.root) {
					continue;
				}
	
				if (parent_node == NULL) {
					print_error("Found invalid parent node!");
					continue; // root node
				}
	
				String node_name = AssimpUtils::get_assimp_string(assimp_node->mName);
				Transform node_transform = AssimpUtils::assimp_matrix_transform(assimp_node->mTransformation);
	
				if (assimp_node->mNumMeshes > 0) {
					// 注释点：这里要格外注意，有些节点会因为有mesh，而被转换为meshInstance，之前的Spatial节点会被删除，且同名的话会被加上2，3这样的后缀
					MeshInstance *mesh = create_mesh(state, assimp_node, node_name, parent_node, node_transform);
					// 出现这种情况时，应该更新所有skeleton中保存的节点信息，否则无法对该mesh进行实时的更新
					auto e = state.armature_skeletons.front();
					if (nullptr != e && !e->value()->gdi_update_mesh_anim_node(node_name, mesh->get_name())) {
						printf("[GDI]assimp update mesh anim node failed, mesh[%S], should not happen\n", String(mesh->get_name()).c_str());
					}
	
					if (mesh) {
	
						parent_node->remove_child(mesh_template);
	
						// re-parent children
						List<Node *> children;
						// re-parent all children to new node
						// note: since get_child_count will change during execution we must build a list first to be safe.
						for (int childId = 0; childId < mesh_template->get_child_count(); childId++) {
							// get child
							Node *child = mesh_template->get_child(childId);
							children.push_back(child);
						}
	
						for (List<Node *>::Element *element = children.front(); element; element = element->next()) {
							// reparent the children to the real mesh node.
							mesh_template->remove_child(element->get());
							mesh->add_child(element->get());
							element->get()->set_owner(state.root);
						}
	
						// update mesh in list so that each mesh node is available
						// this makes the template unavailable which is the desired behaviour
						state.flat_node_map[assimp_node] = mesh;
	
						cleanup_template_nodes.push_back(mesh_template);
	
						// clean up this list we don't need it
						children.clear();
					}
				}

				break;
			}
		}

		// 修改点：屏蔽删除numMeshes>0的mesh节点，测试创建多Skeleton节点（基于这个ainode）
		for (List<Spatial *>::Element *element = cleanup_template_nodes.front(); element; element = element->next()) {
			if (element->get()) {
				memdelete(element->get());
			}
		}
	}

	// ---------------------------------------Handle Animation infos
#ifdef GDI_ENABLE_ASSIMP_MODIFICATION
	state.gdi_none_bone_track_vec.clear();
	if (p_flags & IMPORT_ANIMATION && scene->mNumAnimations) {
		state.animation_player = memnew(AnimationPlayer);
		state.animation_player->gdi_set_import_file_format(Object::ASSIMP_FBX);
		//String anim_player_name = "AnimPlayer-" + String(scene->mAnimations[i]->mName.data);
		state.animation_player->set_name("AnimPlayer");
		state.root->add_child(state.animation_player);
		state.animation_player->set_owner(state.root);

			// 修改点：导入动画时，需要映射正确的mesh idx
			for (uint32_t i = 0; i < scene->mNumAnimations; i++) {
				// 骨骼动画相关
				if (state.armature_skeletons.size() > 0) {
					for (auto e = state.armature_skeletons.front(); e; e = e->next()) {
						auto arm_index = state.gdi_armature_index_map.find(e->key());
						if (nullptr != arm_index) {
							_import_animation(state, i, p_bake_fps, arm_index->value());
						}
						else {
							_import_animation(state, i, p_bake_fps, 0);
							print_error("[GDI-Error]assimp import anim, wrong case");
						}
					}
				} 
				else {
					// 路径动画相关
					_import_animation(state, i, p_bake_fps);
				}
			}
	}
#else
	if (p_flags & IMPORT_ANIMATION && scene->mNumAnimations) {

		state.animation_player = memnew(AnimationPlayer);
		state.root->add_child(state.animation_player);
		state.animation_player->set_owner(state.root);

		for (uint32_t i = 0; i < scene->mNumAnimations; i++) {
			_import_animation(state, i, p_bake_fps);
		}
	}
#endif


	//
	// Cleanup operations
	//

	state.mesh_cache.clear();
	state.material_cache.clear();
	state.light_cache.clear();
	state.camera_cache.clear();
	state.assimp_node_map.clear();
	state.path_to_image_cache.clear();
	state.nodes.clear();
	state.flat_node_map.clear();
	state.armature_skeletons.clear();
	state.bone_stack.clear();
	return state.root;
}

void EditorSceneImporterAssimp::gdi_create_anim_node_from_scene(const aiScene *scene, Skeleton::NodeAnim **root) {
	*root = new Skeleton::NodeAnim();
	(*root)->name = AssimpUtils::get_assimp_string(scene->mRootNode->mName);
	(*root)->parent = nullptr;
	(*root)->local_transform = AssimpUtils::assimp_matrix_transform(scene->mRootNode->mTransformation);

	for (int i = 0; i < scene->mRootNode->mNumChildren; ++i) {
		(*root)->childs.push_back(gdi_create_anim_nodes(scene, scene->mRootNode->mChildren[i], *root));
	}
}

Skeleton::NodeAnim* EditorSceneImporterAssimp::gdi_create_anim_nodes(const aiScene *scene,
	const aiNode *node,
	Skeleton::NodeAnim *parent) {

	Skeleton::NodeAnim *animNode = new Skeleton::NodeAnim;
	animNode->name = AssimpUtils::get_assimp_string(node->mName);
	animNode->parent = parent;
	animNode->is_bone = get_bone_by_name(scene, node->mName) ? true : false;
	animNode->is_mesh = node->mNumMeshes > 0 ? true : false;
	animNode->bone_num = animNode->is_bone ? 1 : (animNode->is_mesh ? scene->mMeshes[node->mMeshes[0]]->mNumBones : 0);
	animNode->mesh_num = animNode->is_mesh ? node->mNumMeshes : 0;
	animNode->local_transform = AssimpUtils::assimp_matrix_transform(node->mTransformation);

	//if (node->mName == aiString("Dummy33_end")) {
	//	int i = 0;
	//	++i;
	//}

	// default to use animation 0, for now(todo)
	if (scene->mNumAnimations > 0) {
		if (scene->mNumAnimations > 1) {
			// 这里我们是应该创建多套不同的AnimNodes吗？还需要验证，比如Piety这种动画
			// 目前暂时验证的结果是不需要，但是还是保持警告的存在，万一还有其他特殊的动画
			OS::get_singleton()->print("[GDI-Warning]animation num over 1..., what you handled maybe wrong\n");
		}
		aiAnimation *anim = scene->mAnimations[0];
		for (int i = 0; i < anim->mNumChannels; ++i) {
			if (anim->mChannels[i]->mNodeName == node->mName) {
				animNode->channel_id = i;
				break;
			}
		}
	}

	for (int i = 0; i < node->mNumChildren; ++i) {
		auto childNode = gdi_create_anim_nodes(scene, node->mChildren[i], animNode);
		animNode->childs.push_back(childNode);
	}

	return animNode;
}

void EditorSceneImporterAssimp::gdi_find_anim_node_by_name(Skeleton::NodeAnim *node, const String &name, __out Skeleton::NodeAnim **res_node) {

	if (nullptr == node || nullptr != *res_node) {
		return;
	}

	if (node->name == name) {
		*res_node = node;
		return;
	}

	int child_num = node->childs.size();
	for (int i = 0; i < child_num; ++i)	{
		gdi_find_anim_node_by_name(node->childs[i], name, res_node);
	}
}

void EditorSceneImporterAssimp::_insert_animation_track(ImportState &scene, const aiAnimation *assimp_anim, int track_id,
		int anim_fps, Ref<Animation> animation, float ticks_per_second,
		Skeleton *skeleton, const NodePath &node_path,
		const String &node_name, aiBone *track_bone) {

	const aiNodeAnim *assimp_track = assimp_anim->mChannels[track_id];

#ifdef GDI_ENABLE_ASSIMP_MODIFICATION
	// 修改点：是否可以取消只有一个key的track？？正常的track貌似至少有两个有效key
	unsigned int gdi_assimp_max_track_count = assimp_track->mNumPositionKeys > assimp_track->mNumRotationKeys ? assimp_track->mNumPositionKeys : assimp_track->mNumRotationKeys;
	gdi_assimp_max_track_count = gdi_assimp_max_track_count > assimp_track->mNumScalingKeys ? gdi_assimp_max_track_count : assimp_track->mNumScalingKeys;
	if (gdi_assimp_max_track_count <= 1) {
		return;
	}
#endif

	Skeleton::NodeAnim *node_anim = nullptr;
	gdi_find_anim_node_by_name(gdi_node_anim_root, node_name, &node_anim);

	//make transform track
	int track_idx = animation->get_track_count();
	animation->add_track(Animation::TYPE_TRANSFORM);
	animation->track_set_path(track_idx, node_path);
	//first determine animation length

	float increment = 1.0 / float(anim_fps);
	float time = 0.0;
	unsigned int gdi_key_count = 0;

	bool last = false;

	Vector<Vector3> pos_values;
	Vector<float> pos_times;
	Vector<Vector3> scale_values;
	Vector<float> scale_times;
	Vector<Quat> rot_values;
	Vector<float> rot_times;

	for (size_t p = 0; p < assimp_track->mNumPositionKeys; p++) {
		aiVector3D pos = assimp_track->mPositionKeys[p].mValue;
		pos_values.push_back(Vector3(pos.x, pos.y, pos.z));
		pos_times.push_back(assimp_track->mPositionKeys[p].mTime / ticks_per_second);
	}

	for (size_t r = 0; r < assimp_track->mNumRotationKeys; r++) {
		aiQuaternion quat = assimp_track->mRotationKeys[r].mValue;
		rot_values.push_back(Quat(quat.x, quat.y, quat.z, quat.w).normalized());
		rot_times.push_back(assimp_track->mRotationKeys[r].mTime / ticks_per_second);
	}

	for (size_t sc = 0; sc < assimp_track->mNumScalingKeys; sc++) {
		aiVector3D scale = assimp_track->mScalingKeys[sc].mValue;
		scale_values.push_back(Vector3(scale.x, scale.y, scale.z));
		scale_times.push_back(assimp_track->mScalingKeys[sc].mTime / ticks_per_second);
	}

	while (true) {
		
#ifdef GDI_ENABLE_ASSIMP_MODIFICATION
		Vector3 pos = (nullptr == node_anim) ? Vector3() : node_anim->local_transform.get_origin();
		Quat rot = (nullptr == node_anim) ? Quat() : node_anim->local_transform.get_basis().get_rotation_quat();
		Vector3 scale = (nullptr == node_anim) ? Vector3(1, 1, 1) : node_anim->local_transform.get_basis().get_scale();
#else
		Vector3 pos;
		Quat rot;
		Vector3 scale(1, 1, 1);
#endif

		if (pos_values.size()) {
			pos = _interpolate_track<Vector3>(pos_times, pos_values, time, AssetImportAnimation::INTERP_LINEAR);
		}

		if (rot_values.size()) {
			rot = _interpolate_track<Quat>(rot_times, rot_values, time,
					AssetImportAnimation::INTERP_LINEAR)
						  .normalized();
		}

		if (scale_values.size()) {
			scale = _interpolate_track<Vector3>(scale_times, scale_values, time, AssetImportAnimation::INTERP_LINEAR);
		}

		if (skeleton) {
			int skeleton_bone = skeleton->find_bone(node_name);

			if (skeleton_bone >= 0 && track_bone) {

				Transform xform;
				xform.basis.set_quat_scale(rot, scale);
				xform.origin = pos;

				// 修改点：取消此矩阵计算，意义不明
				// 此矩阵的注释，在目前看来是必须的，否则会导致后面的骨骼动画不正常
				// 此处修改，应该只会影响到骨骼动画流程，不会影响到其他地方
#ifndef GDI_ENABLE_ASSIMP_MODIFICATION
				// 加入此矩阵计算后会消除平移的效果
				xform = skeleton->get_bone_pose(skeleton_bone).inverse() * xform;
#endif

				rot = xform.basis.get_rotation_quat();
				rot.normalize();
				scale = xform.basis.get_scale();
				pos = xform.origin;
			} else {
#ifndef GDI_ENABLE_ASSIMP_MODIFICATION
				ERR_FAIL_MSG("Skeleton bone lookup failed for skeleton: " + skeleton->get_name());
#else
				Transform xform;
				xform.basis.set_quat_scale(rot, scale);
				xform.origin = pos;

				rot = xform.basis.get_rotation_quat();
				rot.normalize();
				scale = xform.basis.get_scale();
				pos = xform.origin;
#endif
			}
		}

		animation->track_set_interpolation_type(track_idx, Animation::INTERPOLATION_LINEAR);
		animation->transform_track_insert_key(track_idx, time, pos, rot, scale);

		if (last) { //done this way so a key is always inserted past the end (for proper interpolation)
			break;
		}
		time += increment;
		if (time >= animation->get_length()) {
			last = true;
		}
	}
}

// I really do not like this but need to figure out a better way of removing it later.
Node *EditorSceneImporterAssimp::get_node_by_name(ImportState &state, String name) {

	for (Map<const aiNode *, Spatial *>::Element *key_value_pair = state.flat_node_map.front(); key_value_pair; key_value_pair = key_value_pair->next()) {
		const aiNode *assimp_node = key_value_pair->key();
		Spatial *node = key_value_pair->value();

		String node_name = AssimpUtils::get_assimp_string(assimp_node->mName);
		if (name == node_name && node) {
			return node;
		}
	}
	return NULL;
}

/* Bone stack is a fifo handler for multiple armatures since armatures aren't a thing in assimp (yet) */
void EditorSceneImporterAssimp::RegenerateBoneStack(ImportState &state) {

	state.bone_stack.clear();
	state.gdi_bone_stack_mesh_index.clear();
	auto num_armature = state.armature_nodes.size();

	// build bone stack list
	for (unsigned int mesh_id = 0; mesh_id < state.assimp_scene->mNumMeshes; ++mesh_id) {
		aiMesh *mesh = state.assimp_scene->mMeshes[mesh_id];

		aiString str_tmp_mesh_name = mesh->mName;
		str_tmp_mesh_name.Append("_ArmatureNode");
		// iterate over all the bones on the mesh for this node only!
		for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++) {
			aiBone *bone = mesh->mBones[boneIndex];

			// doubtful this is required right now but best to check
			if (!state.bone_stack.find(bone)) {
				state.bone_stack.push_back(bone);

				// 修改点：把bone在对应mesh armature的索引存入
				// 后面需要在导入动画时使用对应的索引
				for (int arm_index = 0; arm_index < num_armature; ++arm_index) {
					if (state.armature_nodes[arm_index]->mName == str_tmp_mesh_name) {
						state.gdi_bone_stack_mesh_index.push_back(arm_index);
						break;
					}
				}
			}
		}
	}
}

/* Bone stack is a fifo handler for multiple armatures since armatures aren't a thing in assimp (yet) */
void EditorSceneImporterAssimp::RegenerateBoneStack(ImportState &state, aiMesh *mesh) {

	state.bone_stack.clear();
	if (nullptr == mesh) {
		return;
	}

	// iterate over all the bones on the mesh for this node only!
	for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++) {
		aiBone *bone = mesh->mBones[boneIndex];
		if (state.bone_stack.find(bone) == NULL) {
			state.bone_stack.push_back(bone);
		}
	}
}

// animation tracks are per bone

void EditorSceneImporterAssimp::_import_animation(ImportState &state, int p_animation_index, int p_bake_fps, unsigned int mesh_id) {

	ERR_FAIL_INDEX(p_animation_index, (int)state.assimp_scene->mNumAnimations);
	// for test
// 	if (0 != mesh_id) {
// 		return;
// 	}

	const aiAnimation *anim = state.assimp_scene->mAnimations[p_animation_index];
	String name = AssimpUtils::get_anim_string_from_assimp(anim->mName);

#ifdef GDI_ENABLE_ASSIMP_MODIFICATION
	// 修改点：尝试根据不同mesh id，改变不同的anim mesh track name
	aiMesh *mesh = -1 == mesh_id ? nullptr : state.assimp_scene->mMeshes[mesh_id];
	// 多mesh轨道分离操作，使用不同的anim名称，就可以建立多条轨道
	//name = name + "-" + "tracks-" + mesh->mName.data + "-" + itos(mesh_id);
#endif

	if (name == String()) {
		name = "Animation " + itos(p_animation_index + 1);
	}
	print_verbose("import animation: " + name);
	float ticks_per_second = anim->mTicksPerSecond;

	if (state.assimp_scene->mMetaData != NULL && Math::is_equal_approx(ticks_per_second, 0.0f)) {
		int32_t time_mode = 0;
		state.assimp_scene->mMetaData->Get("TimeMode", time_mode);
		ticks_per_second = AssimpUtils::get_fbx_fps(time_mode, state.assimp_scene);
	}

	//?
	//if ((p_path.get_file().get_extension().to_lower() == "glb" || p_path.get_file().get_extension().to_lower() == "gltf") && Math::is_equal_approx(ticks_per_second, 0.0f)) {
	//	ticks_per_second = 1000.0f;
	//}

	if (Math::is_equal_approx(ticks_per_second, 0.0f)) {
		ticks_per_second = 25.0f;
	}

	Ref<Animation> animation;
	bool anim_existed_flag = state.animation_player->has_animation(name);
	if (!anim_existed_flag) {
		animation.instance();
		animation->set_name(name);
		animation->set_length(anim->mDuration / ticks_per_second);
	}
	else {
		animation = state.animation_player->get_animation(name);
	}
	animation->gdi_set_import_file_format(Object::ASSIMP_FBX);

	if (name.begins_with("loop") || name.ends_with("loop") || name.begins_with("cycle") || name.ends_with("cycle")) {
		animation->set_loop(true);
	}

	// for debug
	printf("import anim[%S]------------------>>>>>>mesh[%s] mesh-id[%d] bone-num[%d]\n", name.c_str(), (nullptr != mesh ? mesh->mName.C_Str() : "Null"), mesh_id, (nullptr != mesh ? mesh->mNumBones : 0));

	// 修改点：使用mesh指定的bone生成接口
	// generate bone stack for animation import
#ifdef GDI_ENABLE_ASSIMP_MODIFICATION
	// 使用所有meshes来生成的bone信息的会有问题，最大的问题就是重名覆盖，而引擎又会去通过名称找bone，所以肯定是有问题的
	//RegenerateBoneStack(state);
	// 此处如果不设置正确的对应mesh来产生bones信息的话，是无法正常驱动动画的
	RegenerateBoneStack(state, mesh);

	// 修改点：首先get到有效的armature，因为目前处理是不同的mesh有不同的armature，所以都是同一个arm
	Skeleton *skeleton = NULL;
	for (size_t i = 0; i < anim->mNumChannels; i++) {
		const aiNodeAnim *track = anim->mChannels[i];
		String node_name = AssimpUtils::get_assimp_string(track->mNodeName);
		// check magic node
		bool magic_flag = false;
		int magic_pos = node_name.find(Assimp::FBX::MAGIC_NODE_TAG.c_str());
		if (-1 != magic_pos) {
			magic_flag = true;
			node_name = node_name.substr(0, magic_pos);
		}
		aiBone *bone = gdi_find_bone_from_stack(state, magic_flag ? aiString(node_name.ascii().get_data()) : track->mNodeName);

		if (bone) {
			// get skeleton by bone
			skeleton = state.armature_skeletons[bone->mArmature];
			break;// get到有效arm就可以退出循环了
		}
	}

	// 检测是否为特殊skeleton
	auto check_special_skeleton_func = [&]()->bool {

		if (nullptr == mesh) {
			return false;
		}

		int bone_num = mesh->mNumBones;
		for (int bone_idx = 0; bone_idx < bone_num; ++bone_idx) {
			aiBone *bone = mesh->mBones[bone_idx];
			for (size_t i = 0; i < anim->mNumChannels; i++) {
				if (anim->mChannels[i]->mNodeName == bone->mName) {
					return false;
				}
			}
		}

		return true;
	};

	bool special_skeleton_flag = check_special_skeleton_func();
	if (special_skeleton_flag) {
		if (nullptr == skeleton) {
			for (auto e = state.gdi_armature_index_map.front(); e; e = e->next()) {
				if (e->value() == mesh_id) {
					skeleton = state.armature_skeletons[e->key()];
					break;
				}
			}
		}
		printf("[GDI-Warning] found special skeleton[%S]\n", String(skeleton->get_name()).c_str());

		state.animation_player->gdi_set_special_skeleton_only_with_none_track_bone(skeleton->get_name());
	}
#else
	RegenerateBoneStack(state);
#endif

	//regular tracks
	printf("[import-anim] total channels num[%d]\n", anim->mNumChannels);
	for (size_t i = 0; i < anim->mNumChannels; i++) {
		aiNodeAnim *track = anim->mChannels[i];
		String node_name = AssimpUtils::get_assimp_string(track->mNodeName);
		// for test
		//if (node_name.find("jshenfa_01_$AssimpFbx$_Translation") != -1) {
		//	int i = 0;
		//	++i;
		//}

		// check magic node
		bool magic_flag = false;
		bool magic_comb_flag = false;
		int magic_pos = node_name.find(Assimp::FBX::MAGIC_NODE_TAG.c_str());
		//Node *tree_node = get_node_by_name(state, node_name);
		//if (-1 == magic_pos) {
		//	Node *parent = tree_node->get_parent();
		//	Node *last_parent = nullptr;
		//	bool remove_child_flag = false;
		//	while (nullptr != parent) {
		//		if (!remove_child_flag) {
		//			//parent->remove_child(tree_node);
		//			//remove_child_flag = true;
		//		}
		//		int tmp_pos = String(parent->get_name()).find(Assimp::FBX::MAGIC_NODE_TAG.c_str());

		//		if (-1 != tmp_pos) {
		//			last_parent = parent;
		//			tree_node->raise();
		//			parent = parent->get_parent();
		//		}
		//		else {
		//			break;
		//		}
		//	}
		//}

		if (-1 != magic_pos && 0) {
			magic_flag = true;
			node_name = node_name.substr(0, magic_pos);
			// recomb the whole magic track..., I don't see where shows the magic...
// 			if (-1 != String(track->mNodeName.C_Str()).find("Translation")) {
// 				aiNodeAnim *track_rot = ((i + 1) < anim->mNumChannels) ? anim->mChannels[i + 1] : nullptr;
// 				if (nullptr != track_rot && -1 != String(track_rot->mNodeName.C_Str()).find("Rotation")) {
// 					track->mNumRotationKeys = track_rot->mNumRotationKeys;
// 					if (nullptr != track->mRotationKeys) {
// 						delete[] track->mRotationKeys;
// 						track->mRotationKeys = nullptr;
// 					}
// 					track->mRotationKeys = track_rot->mRotationKeys;
// 
// 					track_rot->mRotationKeys = nullptr;
// 					track_rot->mNumRotationKeys = 0;
// 				}
// 
// 				aiNodeAnim *track_scale = ((i + 2) < anim->mNumChannels) ? anim->mChannels[i + 2] : nullptr;
// 				if (nullptr != track_scale && -1 != String(track_scale->mNodeName.C_Str()).find("Scaling")) {
// 					track->mNumScalingKeys = track_scale->mNumScalingKeys;
// 					if (nullptr != track->mScalingKeys) {
// 						delete[] track->mScalingKeys;
// 						track->mScalingKeys = nullptr;
// 					}
// 					track->mScalingKeys = track_scale->mScalingKeys;
// 
// 					track_scale->mScalingKeys = nullptr;
// 					track_scale->mNumScalingKeys = 0;
// 				}
// 
// 				magic_comb_flag = true;
// 				printf("[import-anim] found and combine the magic track\n");
// 			}
		}

		// for test
		//if (node_name.find("HengGang") != -1) {
		//	int i = 0;
		//	++i;
		//}

		bool optimized_flag = false;
 		if (magic_flag && 0) {
 			optimized_flag = _gdi_optimize_track_data(track);
 		}

		print_verbose("track name import: " + node_name);
		if (track->mNumRotationKeys == 0 && track->mNumPositionKeys == 0 && track->mNumScalingKeys == 0) {
			continue; //do not bother
		}

#ifndef GDI_ENABLE_ASSIMP_MODIFICATION
		Skeleton *skeleton = NULL;
#endif
		NodePath node_path;
		aiBone *bone = NULL;

		// Import skeleton bone animation for this track
		// Any bone will do, no point in processing more than just what is in the skeleton
		{
			bone = get_bone_from_stack(state, magic_flag ? aiString(node_name.ascii().get_data()) : track->mNodeName);

			if (bone) {
#ifndef GDI_ENABLE_ASSIMP_MODIFICATION
				// get skeleton by bone
				skeleton = state.armature_skeletons[bone->mArmature];
#endif

				// for debug
				printf("[import-anim] track&bone:[%s]-index[%d]\n", bone->mName.C_Str(), (int)i);

				if (skeleton) {
					String path = state.root->get_path_to(skeleton);
					path += ":" + node_name;
					node_path = path;

					if (node_path != NodePath()) {
						_insert_animation_track(state, anim, i, p_bake_fps, animation, ticks_per_second, skeleton,
								node_path, node_name, bone);
					} else {
						print_error("Failed to find valid node path for animation");
					}
				}
			}
		}

		// not a bone
		// note this is flaky it uses node names which is unreliable
		Node *allocated_node = get_node_by_name(state, node_name);
		// todo: implement skeleton grabbing for node based animations too :)
		// check if node exists, if it does then also apply animation track for node and bones above are all handled.
		// this is now inclusive animation handling so that
		// we import all the data and do not miss anything.
		// 公共非BoneTrack只添加第一次
		// 因为加入了非Bone的Track支持，所以这里需要判断一下bone，获取不到的才添加
		// 同时需要注意对同名track的处理
		if (allocated_node && !bone /*&& !anim_existed_flag*/) {
			node_path = state.root->get_path_to(allocated_node);

#ifndef GDI_ENABLE_ASSIMP_MODIFICATION
			if (node_path != NodePath()) {
				_insert_animation_track(state, anim, i, p_bake_fps, animation, ticks_per_second, skeleton,
						node_path, node_name, nullptr);
			}
#else
			if (node_path != NodePath() && -1 == state.gdi_none_bone_track_vec.find(node_path)) {
				// repair the track data first
				Spatial *spa = Object::cast_to<Spatial>(allocated_node);
				if (0 && optimized_flag && nullptr != spa) {
					auto trans = spa->get_transform();
					// for test
					//if (String(spa->get_name()).find("HengGang") != -1) {
					//	int i = 0;
					//	++i;
					//}

					if (0 == track->mNumPositionKeys) {
						track->mNumPositionKeys = 1;
						track->mPositionKeys = new aiVectorKey(0, aiVector3D(trans.origin.x, trans.origin.y, trans.origin.z));
					}

					if (0 == track->mNumRotationKeys) {
						auto quat = trans.basis.get_rotation_quat();
						auto q2 = trans.basis.get_rotation_quat();
						track->mNumRotationKeys = 1;
						track->mRotationKeys = new aiQuatKey(0, aiQuaternion(quat.w, quat.x, quat.y, quat.z));
					}

					if (0 == track->mNumScalingKeys) {
						auto scale = trans.basis.get_scale();
						auto s2 = trans.basis.get_scale_abs();
						auto s3 = trans.basis.get_scale_local();

						track->mNumScalingKeys = 1;
						track->mScalingKeys = new aiVectorKey(0, aiVector3D(scale.x, scale.y, scale.z));
					}
				}

				state.gdi_none_bone_track_vec.push_back(node_path);
				_insert_animation_track(state, anim, i, p_bake_fps, animation, ticks_per_second, skeleton,
					node_path, node_name, nullptr);

				// for debug
				printf("[import-anim] track&none-bone:[%S]-index[%d]\n", String(allocated_node->get_name()).c_str(), (int)i);
			}
#endif
		}
#ifdef GDI_ENABLE_ASSIMP_MODIFICATION
		else if (bone == nullptr) {
			printf("[GDI-warning]:mesh[%s], missing track[%s], can't find it in the nodes vec, mesh id[%d], track id[%d]\n", mesh ? mesh->mName.C_Str() : "null",track->mNodeName.data, mesh_id, (int)i);

			// for test
			//if (String(track->mNodeName.data) == String("Bip001 L Finger1Nub")) {
			//	int i = 0;
			//	++i;
			//}

			//String path = state.root->get_path_to(skeleton);
			//path = "TestNoneBone";
			//path += ":" + node_name;
			//node_path = path;
			//node_path = state.root->get_path_to(allocated_node);

			//if (node_path != NodePath()) {
			//	_insert_animation_track(state, anim, i, p_bake_fps, animation, ticks_per_second, skeleton,
			//		node_path, node_name, nullptr);
			//}
		}

		//if (magic_flag && magic_comb_flag) {
		//	track->mRotationKeys = nullptr;
		//	track->mScalingKeys = nullptr;
		//}
#endif
	}

	//blend shape tracks

	for (size_t i = 0; i < anim->mNumMorphMeshChannels; i++) {

		//printf("[Skeleton]:enter morph mesh animation.....(TODO)\n");
		const aiMeshMorphAnim *anim_mesh = anim->mMorphMeshChannels[i];

		const String prop_name = AssimpUtils::get_assimp_string(anim_mesh->mName);
		const String mesh_name = prop_name.split("*")[0];

		ERR_CONTINUE(prop_name.split("*").size() != 2);

		Node *item = get_node_by_name(state, mesh_name);
		ERR_CONTINUE_MSG(!item, "failed to look up node by name");
		const MeshInstance *mesh_instance = Object::cast_to<MeshInstance>(item);
		ERR_CONTINUE(mesh_instance == NULL);

		String base_path = state.root->get_path_to(mesh_instance);

		Ref<Mesh> mesh = mesh_instance->get_mesh();
		ERR_CONTINUE(mesh.is_null());

		//add the tracks for this mesh
		int base_track = animation->get_track_count();
		for (int j = 0; j < mesh->get_blend_shape_count(); j++) {

			animation->add_track(Animation::TYPE_VALUE);
			animation->track_set_path(base_track + j, base_path + ":blend_shapes/" + mesh->get_blend_shape_name(j));
		}

		for (size_t k = 0; k < anim_mesh->mNumKeys; k++) {
			for (size_t j = 0; j < anim_mesh->mKeys[k].mNumValuesAndWeights; j++) {

				float t = anim_mesh->mKeys[k].mTime / ticks_per_second;
				float w = anim_mesh->mKeys[k].mWeights[j];

				animation->track_insert_key(base_track + j, t, w);
			}
		}
	}

	if (animation->get_track_count() && !anim_existed_flag) {
		state.animation_player->add_animation(name, animation);
	}
}
//
// Mesh Generation from indices ? why do we need so much mesh code
// [debt needs looked into]
Ref<Mesh>
EditorSceneImporterAssimp::_generate_mesh_from_surface_indices(ImportState &state, const Vector<int> &p_surface_indices,
		const aiNode *assimp_node, Ref<Skin> &skin,
		Skeleton *&skeleton_assigned) {

	Ref<ArrayMesh> mesh;
	mesh.instance();
	bool has_uvs = false;
	bool compress_vert_data = state.import_flags & IMPORT_USE_COMPRESSION;
	uint32_t mesh_flags = compress_vert_data ? Mesh::ARRAY_COMPRESS_DEFAULT : 0;

	Map<String, uint32_t> morph_mesh_string_lookup;

	for (int i = 0; i < p_surface_indices.size(); i++) {
		const unsigned int mesh_idx = p_surface_indices[0];
		const aiMesh *ai_mesh = state.assimp_scene->mMeshes[mesh_idx];
		for (size_t j = 0; j < ai_mesh->mNumAnimMeshes; j++) {
			String ai_anim_mesh_name = AssimpUtils::get_assimp_string(ai_mesh->mAnimMeshes[j]->mName);
			if (!morph_mesh_string_lookup.has(ai_anim_mesh_name)) {
				morph_mesh_string_lookup.insert(ai_anim_mesh_name, j);
				mesh->set_blend_shape_mode(Mesh::BLEND_SHAPE_MODE_NORMALIZED);
				if (ai_anim_mesh_name.empty()) {
					ai_anim_mesh_name = String("morph_") + itos(j);
				}
				mesh->add_blend_shape(ai_anim_mesh_name);
			}
		}
	}
	//
	// Process Vertex Weights
	//
	// 修改点：记录多surface的组合name
	String multi_surf_name;
	for (int i = 0; i < p_surface_indices.size(); i++) {
		const unsigned int mesh_idx = p_surface_indices[i];
		const aiMesh *ai_mesh = state.assimp_scene->mMeshes[mesh_idx];
		multi_surf_name += ai_mesh->mName.data;
		if (i < p_surface_indices.size() - 1) {
			multi_surf_name += "-";
		}

		Map<uint32_t, Vector<BoneInfo> > vertex_weights;

		if (ai_mesh->mNumBones > 0) {
			for (size_t b = 0; b < ai_mesh->mNumBones; b++) {
				aiBone *bone = ai_mesh->mBones[b];
// 				continue;

				if (!skeleton_assigned) {
					print_verbose("Assigned mesh skeleton during mesh creation");
					skeleton_assigned = state.skeleton_bone_map[bone];
					if (nullptr == skeleton_assigned) {
						continue;
					}

					if (!skin.is_valid()) {
						print_verbose("Configured new skin");
						skin.instance();
					} else {
						print_verbose("Reusing existing skin!");
					}
				}
				
				String bone_name = AssimpUtils::get_assimp_string(bone->mName);
				int bone_index = skeleton_assigned->find_bone(bone_name);
				ERR_CONTINUE(bone_index == -1);
				for (size_t w = 0; w < bone->mNumWeights; w++) {

					aiVertexWeight ai_weights = bone->mWeights[w];

					BoneInfo bi;
					uint32_t vertex_index = ai_weights.mVertexId;
					bi.bone = bone_index;
					bi.weight = ai_weights.mWeight;

					if (!vertex_weights.has(vertex_index)) {
						vertex_weights[vertex_index] = Vector<BoneInfo>();
					}

					vertex_weights[vertex_index].push_back(bi);
				}
			}
		}

		//
		// Create mesh from data from assimp
		//

		Ref<SurfaceTool> st;
		st.instance();
		st->begin(Mesh::PRIMITIVE_TRIANGLES);

		for (size_t j = 0; j < ai_mesh->mNumVertices; j++) {

			// Get the texture coordinates if they exist
			if (ai_mesh->HasTextureCoords(0)) {
				has_uvs = true;
				st->add_uv(Vector2(ai_mesh->mTextureCoords[0][j].x, 1.0f - ai_mesh->mTextureCoords[0][j].y));
			}

			if (ai_mesh->HasTextureCoords(1)) {
				has_uvs = true;
				st->add_uv2(Vector2(ai_mesh->mTextureCoords[1][j].x, 1.0f - ai_mesh->mTextureCoords[1][j].y));
			}

			// Assign vertex colors
			if (ai_mesh->HasVertexColors(0)) {
				Color color = Color(ai_mesh->mColors[0]->r, ai_mesh->mColors[0]->g, ai_mesh->mColors[0]->b,
						ai_mesh->mColors[0]->a);
				st->add_color(color);
			}

			// Work out normal calculations? - this needs work it doesn't work properly on huestos
			if (ai_mesh->mNormals != NULL) {
				const aiVector3D normals = ai_mesh->mNormals[j];
				const Vector3 godot_normal = Vector3(normals.x, normals.y, normals.z);
				st->add_normal(godot_normal);
				if (ai_mesh->HasTangentsAndBitangents()) {
					const aiVector3D tangents = ai_mesh->mTangents[j];
					const Vector3 godot_tangent = Vector3(tangents.x, tangents.y, tangents.z);
					const aiVector3D bitangent = ai_mesh->mBitangents[j];
					const Vector3 godot_bitangent = Vector3(bitangent.x, bitangent.y, bitangent.z);
					float d = godot_normal.cross(godot_tangent).dot(godot_bitangent) > 0.0f ? 1.0f : -1.0f;
					st->add_tangent(Plane(tangents.x, tangents.y, tangents.z, d));
				}
			}

			// We have vertex weights right?
			if (vertex_weights.has(j)) {

				Vector<BoneInfo> bone_info = vertex_weights[j];
				Vector<int> bones;
				bones.resize(bone_info.size());
				Vector<float> weights;
				weights.resize(bone_info.size());

				// todo? do we really need to loop over all bones? - assimp may have helper to find all influences on this vertex.
				for (int k = 0; k < bone_info.size(); k++) {
					bones.write[k] = bone_info[k].bone;
					weights.write[k] = bone_info[k].weight;
				}

				st->add_bones(bones);
				st->add_weights(weights);
			}

			// Assign vertex
			const aiVector3D pos = ai_mesh->mVertices[j];

			// note we must include node offset transform as this is relative to world space not local space.
			Vector3 godot_pos = Vector3(pos.x, pos.y, pos.z);
			st->add_vertex(godot_pos);
		}

		// fire replacement for face handling
		for (size_t j = 0; j < ai_mesh->mNumFaces; j++) {
			const aiFace face = ai_mesh->mFaces[j];
			for (unsigned int k = 0; k < face.mNumIndices; k++) {
				st->add_index(face.mIndices[k]);
			}
		}

		if (ai_mesh->HasTangentsAndBitangents() == false && has_uvs) {
			st->generate_tangents();
		}

		aiMaterial *ai_material = state.assimp_scene->mMaterials[ai_mesh->mMaterialIndex];
		Ref<SpatialMaterial> mat;
		mat.instance();

		int32_t mat_two_sided = 0;
		if (AI_SUCCESS == ai_material->Get(AI_MATKEY_TWOSIDED, mat_two_sided)) {
			if (mat_two_sided > 0) {
				mat->set_cull_mode(SpatialMaterial::CULL_DISABLED);
			} else {
				mat->set_cull_mode(SpatialMaterial::CULL_BACK);
			}
		}

		aiString mat_name;
		if (AI_SUCCESS == ai_material->Get(AI_MATKEY_NAME, mat_name)) {
			mat->set_name(AssimpUtils::get_assimp_string(mat_name));
		}

		// Culling handling for meshes

		// cull all back faces
		mat->set_cull_mode(SpatialMaterial::CULL_DISABLED);

		// Now process materials
		aiTextureType base_color = aiTextureType_BASE_COLOR;
		{
			String filename, path;
			AssimpImageData image_data;

			if (AssimpUtils::GetAssimpTexture(state, ai_material, base_color, filename, path, image_data)) {
				AssimpUtils::set_texture_mapping_mode(image_data.map_mode, image_data.texture);

				// anything transparent must be culled
				if (image_data.raw_image->detect_alpha() != Image::ALPHA_NONE) {
					mat->set_feature(SpatialMaterial::FEATURE_TRANSPARENT, true);
					mat->set_depth_draw_mode(SpatialMaterial::DepthDrawMode::DEPTH_DRAW_ALPHA_OPAQUE_PREPASS);
					mat->set_cull_mode(
							SpatialMaterial::CULL_DISABLED); // since you can see both sides in transparent mode
				}

				mat->set_texture(SpatialMaterial::TEXTURE_ALBEDO, image_data.texture);
			}
		}

		aiTextureType tex_diffuse = aiTextureType_DIFFUSE;
		{
			String filename, path;
			AssimpImageData image_data;

			if (AssimpUtils::GetAssimpTexture(state, ai_material, tex_diffuse, filename, path, image_data)) {
				AssimpUtils::set_texture_mapping_mode(image_data.map_mode, image_data.texture);

				// anything transparent must be culled
				if (image_data.raw_image->detect_alpha() != Image::ALPHA_NONE) {
					mat->set_feature(SpatialMaterial::FEATURE_TRANSPARENT, true);
					mat->set_depth_draw_mode(SpatialMaterial::DepthDrawMode::DEPTH_DRAW_ALPHA_OPAQUE_PREPASS);
					mat->set_cull_mode(
							SpatialMaterial::CULL_DISABLED); // since you can see both sides in transparent mode
				}

				mat->set_texture(SpatialMaterial::TEXTURE_ALBEDO, image_data.texture);
			}

			aiColor4D clr_diffuse;
			if (AI_SUCCESS == ai_material->Get(AI_MATKEY_COLOR_DIFFUSE, clr_diffuse)) {
				if (Math::is_equal_approx(clr_diffuse.a, 1.0f) == false) {
					mat->set_feature(SpatialMaterial::FEATURE_TRANSPARENT, true);
					mat->set_depth_draw_mode(SpatialMaterial::DepthDrawMode::DEPTH_DRAW_ALPHA_OPAQUE_PREPASS);
					mat->set_cull_mode(
							SpatialMaterial::CULL_DISABLED); // since you can see both sides in transparent mode
				}
				mat->set_albedo(Color(clr_diffuse.r, clr_diffuse.g, clr_diffuse.b, clr_diffuse.a));
			}
		}

		aiTextureType tex_normal = aiTextureType_NORMALS;
		{
			String filename, path;
			Ref<ImageTexture> texture;
			AssimpImageData image_data;

			// Process texture normal map
			if (AssimpUtils::GetAssimpTexture(state, ai_material, tex_normal, filename, path, image_data)) {
				AssimpUtils::set_texture_mapping_mode(image_data.map_mode, image_data.texture);
				mat->set_feature(SpatialMaterial::Feature::FEATURE_NORMAL_MAPPING, true);
				mat->set_texture(SpatialMaterial::TEXTURE_NORMAL, image_data.texture);
			} else {
				aiString texture_path;
				if (AI_SUCCESS == ai_material->Get(AI_MATKEY_FBX_NORMAL_TEXTURE, AI_PROPERTIES, texture_path)) {
					if (AssimpUtils::CreateAssimpTexture(state, texture_path, filename, path, image_data)) {
						mat->set_feature(SpatialMaterial::Feature::FEATURE_NORMAL_MAPPING, true);
						mat->set_texture(SpatialMaterial::TEXTURE_NORMAL, image_data.texture);
					}
				}
			}
		}

		aiTextureType tex_normal_camera = aiTextureType_NORMAL_CAMERA;
		{
			String filename, path;
			Ref<ImageTexture> texture;
			AssimpImageData image_data;

			// Process texture normal map
			if (AssimpUtils::GetAssimpTexture(state, ai_material, tex_normal_camera, filename, path, image_data)) {
				AssimpUtils::set_texture_mapping_mode(image_data.map_mode, image_data.texture);
				mat->set_feature(SpatialMaterial::Feature::FEATURE_NORMAL_MAPPING, true);
				mat->set_texture(SpatialMaterial::TEXTURE_NORMAL, image_data.texture);
			}
		}

		aiTextureType tex_emission_color = aiTextureType_EMISSION_COLOR;
		{
			String filename, path;
			Ref<ImageTexture> texture;
			AssimpImageData image_data;

			// Process texture normal map
			if (AssimpUtils::GetAssimpTexture(state, ai_material, tex_emission_color, filename, path, image_data)) {
				AssimpUtils::set_texture_mapping_mode(image_data.map_mode, image_data.texture);
				mat->set_feature(SpatialMaterial::Feature::FEATURE_NORMAL_MAPPING, true);
				mat->set_texture(SpatialMaterial::TEXTURE_NORMAL, image_data.texture);
			}
		}

		aiTextureType tex_metalness = aiTextureType_METALNESS;
		{
			String filename, path;
			Ref<ImageTexture> texture;
			AssimpImageData image_data;

			// Process texture normal map
			if (AssimpUtils::GetAssimpTexture(state, ai_material, tex_metalness, filename, path, image_data)) {
				AssimpUtils::set_texture_mapping_mode(image_data.map_mode, image_data.texture);
				mat->set_texture(SpatialMaterial::TEXTURE_METALLIC, image_data.texture);
			}
		}

		aiTextureType tex_roughness = aiTextureType_DIFFUSE_ROUGHNESS;
		{
			String filename, path;
			Ref<ImageTexture> texture;
			AssimpImageData image_data;

			// Process texture normal map
			if (AssimpUtils::GetAssimpTexture(state, ai_material, tex_roughness, filename, path, image_data)) {
				AssimpUtils::set_texture_mapping_mode(image_data.map_mode, image_data.texture);
				mat->set_texture(SpatialMaterial::TEXTURE_ROUGHNESS, image_data.texture);
			}
		}

		aiTextureType tex_emissive = aiTextureType_EMISSIVE;
		{
			String filename = "";
			String path = "";
			Ref<Image> texture;
			AssimpImageData image_data;

			if (AssimpUtils::GetAssimpTexture(state, ai_material, tex_emissive, filename, path, image_data)) {
				AssimpUtils::set_texture_mapping_mode(image_data.map_mode, image_data.texture);
				mat->set_feature(SpatialMaterial::FEATURE_EMISSION, true);
				mat->set_texture(SpatialMaterial::TEXTURE_EMISSION, image_data.texture);
			} else {
				// Process emission textures
				aiString texture_emissive_path;
				if (AI_SUCCESS ==
						ai_material->Get(AI_MATKEY_FBX_MAYA_EMISSION_TEXTURE, AI_PROPERTIES, texture_emissive_path)) {
					if (AssimpUtils::CreateAssimpTexture(state, texture_emissive_path, filename, path, image_data)) {
						mat->set_feature(SpatialMaterial::FEATURE_EMISSION, true);
						mat->set_texture(SpatialMaterial::TEXTURE_EMISSION, image_data.texture);
					}
				} else {
					float pbr_emission = 0.0f;
					if (AI_SUCCESS == ai_material->Get(AI_MATKEY_FBX_MAYA_EMISSIVE_FACTOR, AI_NULL, pbr_emission)) {
						mat->set_emission(Color(pbr_emission, pbr_emission, pbr_emission, 1.0f));
					}
				}
			}
		}

		aiTextureType tex_specular = aiTextureType_SPECULAR;
		{
			String filename, path;
			Ref<ImageTexture> texture;
			AssimpImageData image_data;

			// Process texture normal map
			if (AssimpUtils::GetAssimpTexture(state, ai_material, tex_specular, filename, path, image_data)) {
				AssimpUtils::set_texture_mapping_mode(image_data.map_mode, image_data.texture);
				mat->set_texture(SpatialMaterial::TEXTURE_METALLIC, image_data.texture);
			}
		}

		aiTextureType tex_ao_map = aiTextureType_AMBIENT_OCCLUSION;
		{
			String filename, path;
			Ref<ImageTexture> texture;
			AssimpImageData image_data;

			// Process texture normal map
			if (AssimpUtils::GetAssimpTexture(state, ai_material, tex_ao_map, filename, path, image_data)) {
				AssimpUtils::set_texture_mapping_mode(image_data.map_mode, image_data.texture);
				mat->set_feature(SpatialMaterial::FEATURE_AMBIENT_OCCLUSION, true);
				mat->set_texture(SpatialMaterial::TEXTURE_AMBIENT_OCCLUSION, image_data.texture);
			}
		}

		Array array_mesh = st->commit_to_arrays();
		Array morphs;
		morphs.resize(ai_mesh->mNumAnimMeshes);
		// 修改点：为什么要固定成三角呢....node里不是有类型吗，有毛病吗
		// 这些原生引擎里的BUG写法真的是到处都是
		Mesh::PrimitiveType primitive = Mesh::PRIMITIVE_TRIANGLES;
		switch (ai_mesh->mPrimitiveTypes)
		{
		case 2: {
			primitive = Mesh::PRIMITIVE_LINES;
			break;
		}
		case 4: {
			primitive = Mesh::PRIMITIVE_TRIANGLES;
			break;
		}
		default:
			primitive = Mesh::PRIMITIVE_TRIANGLES;
			break;
		}

		for (size_t j = 0; j < ai_mesh->mNumAnimMeshes; j++) {
// 			continue;
			String ai_anim_mesh_name = AssimpUtils::get_assimp_string(ai_mesh->mAnimMeshes[j]->mName);

			if (ai_anim_mesh_name.empty()) {
				ai_anim_mesh_name = String("morph_") + itos(j);
			}

			Array array_copy;
			array_copy.resize(VisualServer::ARRAY_MAX);

			for (int l = 0; l < VisualServer::ARRAY_MAX; l++) {
				array_copy[l] = array_mesh[l].duplicate(true);
			}

			const size_t num_vertices = ai_mesh->mAnimMeshes[j]->mNumVertices;
			array_copy[Mesh::ARRAY_INDEX] = Variant();
			if (ai_mesh->mAnimMeshes[j]->HasPositions()) {
				PoolVector3Array vertices;
				vertices.resize(num_vertices);
				for (size_t l = 0; l < num_vertices; l++) {
					const aiVector3D ai_pos = ai_mesh->mAnimMeshes[j]->mVertices[l];
					Vector3 position = Vector3(ai_pos.x, ai_pos.y, ai_pos.z);
					vertices.write()[l] = position;
				}
				PoolVector3Array new_vertices = array_copy[VisualServer::ARRAY_VERTEX].duplicate(true);
				ERR_CONTINUE(vertices.size() != new_vertices.size());
				for (int32_t l = 0; l < new_vertices.size(); l++) {
					PoolVector3Array::Write w = new_vertices.write();
					w[l] = vertices[l];
				}
				array_copy[VisualServer::ARRAY_VERTEX] = new_vertices;
			}

			int32_t color_set = 0;
			if (ai_mesh->mAnimMeshes[j]->HasVertexColors(color_set)) {
				PoolColorArray colors;
				colors.resize(num_vertices);
				for (size_t l = 0; l < num_vertices; l++) {
					const aiColor4D ai_color = ai_mesh->mAnimMeshes[j]->mColors[color_set][l];
					Color color = Color(ai_color.r, ai_color.g, ai_color.b, ai_color.a);
					colors.write()[l] = color;
				}
				PoolColorArray new_colors = array_copy[VisualServer::ARRAY_COLOR].duplicate(true);
				ERR_CONTINUE(colors.size() != new_colors.size());
				for (int32_t l = 0; l < colors.size(); l++) {
					PoolColorArray::Write w = new_colors.write();
					w[l] = colors[l];
				}
				array_copy[VisualServer::ARRAY_COLOR] = new_colors;
			}

			if (ai_mesh->mAnimMeshes[j]->HasNormals()) {
				PoolVector3Array normals;
				normals.resize(num_vertices);
				for (size_t l = 0; l < num_vertices; l++) {
					const aiVector3D ai_normal = ai_mesh->mAnimMeshes[j]->mNormals[l];
					Vector3 normal = Vector3(ai_normal.x, ai_normal.y, ai_normal.z);
					normals.write()[l] = normal;
				}
				PoolVector3Array new_normals = array_copy[VisualServer::ARRAY_NORMAL].duplicate(true);
				ERR_CONTINUE(normals.size() != new_normals.size());
				for (int l = 0; l < normals.size(); l++) {
					PoolVector3Array::Write w = new_normals.write();
					w[l] = normals[l];
				}
				array_copy[VisualServer::ARRAY_NORMAL] = new_normals;
			}

			if (ai_mesh->mAnimMeshes[j]->HasTangentsAndBitangents()) {
				PoolColorArray tangents;
				tangents.resize(num_vertices);
				PoolColorArray::Write w = tangents.write();
				for (size_t l = 0; l < num_vertices; l++) {
					AssimpUtils::calc_tangent_from_mesh(ai_mesh, j, l, l, w);
				}
				PoolRealArray new_tangents = array_copy[VisualServer::ARRAY_TANGENT].duplicate(true);
				ERR_CONTINUE(new_tangents.size() != tangents.size() * 4);
				for (int32_t l = 0; l < tangents.size(); l++) {
					new_tangents.write()[l + 0] = tangents[l].r;
					new_tangents.write()[l + 1] = tangents[l].g;
					new_tangents.write()[l + 2] = tangents[l].b;
					new_tangents.write()[l + 3] = tangents[l].a;
				}
				array_copy[VisualServer::ARRAY_TANGENT] = new_tangents;
			}

			morphs[j] = array_copy;
		}
		mesh->add_surface_from_arrays(primitive, array_mesh, morphs, mesh_flags);
		mesh->surface_set_material(i, mat);
		mesh->surface_set_name(i, AssimpUtils::get_assimp_string(ai_mesh->mName));
	}

	// 修改点：必须设置mesh的resource name，否则无法正常reimport
	mesh->set_name(multi_surf_name);
	return mesh;
}

/**
 * Create a new mesh for the node supplied
 */
MeshInstance *
EditorSceneImporterAssimp::create_mesh(ImportState &state, const aiNode *assimp_node, const String &node_name, Node *active_node, Transform node_transform) {
	/* MESH NODE */
	Ref<Mesh> mesh;
	Ref<Skin> skin;
	// see if we have mesh cache for this.
	Vector<int> surface_indices;

	RegenerateBoneStack(state);

	// Configure indices
	for (uint32_t i = 0; i < assimp_node->mNumMeshes; i++) {
		int mesh_index = assimp_node->mMeshes[i];
		// create list of mesh indexes
		surface_indices.push_back(mesh_index);
	}

	//surface_indices.sort();
	String mesh_key;
	for (int i = 0; i < surface_indices.size(); i++) {
		if (i > 0) {
			mesh_key += ":";
		}
		mesh_key += itos(surface_indices[i]);
	}

	Skeleton *skeleton = NULL;
	aiNode *armature = NULL;

	if (!state.mesh_cache.has(mesh_key)) {
		mesh = _generate_mesh_from_surface_indices(state, surface_indices, assimp_node, skin, skeleton);
		state.mesh_cache[mesh_key] = mesh;
	}

	MeshInstance *mesh_node = memnew(MeshInstance);
	mesh = state.mesh_cache[mesh_key];
	mesh_node->set_mesh(mesh);

	// 修改点：拷贝mesh的mat到MeshInstance中
	auto surface_num = mesh->get_surface_count();
	for (int i = 0; i < surface_num; ++i) {
		auto mat = mesh->surface_get_material(i);
		mesh_node->set_surface_material(i, mat);
	}

	// if we have a valid skeleton set it up
	if (skin.is_valid()) {
		for (uint32_t i = 0; i < assimp_node->mNumMeshes; i++) {
			unsigned int mesh_index = assimp_node->mMeshes[i];
			const aiMesh *ai_mesh = state.assimp_scene->mMeshes[mesh_index];

			// please remember bone id relative to the skin is NOT the mesh relative index.
			// it is the index relative to the skeleton that is why
			// we have state.bone_id_map, it allows for duplicate bone id's too :)
			// hope this makes sense

			int bind_count = 0;
			for (unsigned int boneId = 0; boneId < ai_mesh->mNumBones; ++boneId) {
				aiBone *iterBone = ai_mesh->mBones[boneId];

				// used to reparent mesh to the correct armature later on if assigned.
				if (!armature) {
					print_verbose("Configured mesh armature, will reparent later to armature");
					armature = iterBone->mArmature;
				}

				if (skeleton) {
					int id = skeleton->find_bone(AssimpUtils::get_assimp_string(iterBone->mName));
					if (id != -1) {
						print_verbose("Set bind bone: mesh: " + itos(mesh_index) + " bone index: " + itos(id));
						Transform t = AssimpUtils::assimp_matrix_transform(iterBone->mOffsetMatrix);

						// 注释点：add_bind会调用set_bind_pose，也就是offset mat
						skin->add_bind(bind_count, t);
						skin->set_bind_bone(bind_count, id);
						bind_count++;
					}
				}
			}
		}

		print_verbose("Finished configuring bind pose for skin mesh");
	}

	// this code parents all meshes with bones to the armature they are for
	// GLTF2 specification relies on this and we are enforcing it for FBX.
	if (armature && state.flat_node_map[armature]) {
		Node *armature_parent = state.flat_node_map[armature];
		print_verbose("Parented mesh " + node_name + " to armature " + armature_parent->get_name());
		// static mesh handling
		armature_parent->add_child(mesh_node);
		// transform must be identity
		mesh_node->set_global_transform(Transform());
		mesh_node->set_name(node_name);
		mesh_node->set_owner(state.root);
	} else {
		// static mesh handling
		active_node->add_child(mesh_node);
		mesh_node->set_global_transform(node_transform);
		mesh_node->set_name(node_name);
		mesh_node->set_owner(state.root);
	}

	if (skeleton) {
		print_verbose("Attempted to set skeleton path!");
		mesh_node->set_skeleton_path(mesh_node->get_path_to(skeleton));
		mesh_node->set_skin(skin);
	}

	return mesh_node;
}

/**
 * Create a light for the scene
 * Automatically caches lights for lookup later
 */
Spatial *EditorSceneImporterAssimp::create_light(
		ImportState &state,
		const String &node_name,
		Transform &look_at_transform) {
	Light *light = NULL;
	aiLight *assimp_light = state.assimp_scene->mLights[state.light_cache[node_name]];
	ERR_FAIL_COND_V(!assimp_light, NULL);

	if (assimp_light->mType == aiLightSource_DIRECTIONAL) {
		light = memnew(DirectionalLight);
	} else if (assimp_light->mType == aiLightSource_POINT) {
		light = memnew(OmniLight);
	} else if (assimp_light->mType == aiLightSource_SPOT) {
		light = memnew(SpotLight);
	}
	ERR_FAIL_COND_V(light == NULL, NULL);

	if (assimp_light->mType != aiLightSource_POINT) {
		Vector3 pos = Vector3(
				assimp_light->mPosition.x,
				assimp_light->mPosition.y,
				assimp_light->mPosition.z);
		Vector3 look_at = Vector3(
				assimp_light->mDirection.y,
				assimp_light->mDirection.x,
				assimp_light->mDirection.z)
								  .normalized();
		Vector3 up = Vector3(
				assimp_light->mUp.x,
				assimp_light->mUp.y,
				assimp_light->mUp.z);

		look_at_transform.set_look_at(pos, look_at, up);
	}
	// properties for light variables should be put here.
	// not really hugely important yet but we will need them in the future

	light->set_color(
			Color(assimp_light->mColorDiffuse.r, assimp_light->mColorDiffuse.g, assimp_light->mColorDiffuse.b));

	return light;
}

/**
 * Create camera for the scene
 */
Spatial *EditorSceneImporterAssimp::create_camera(
		ImportState &state,
		const String &node_name,
		Transform &look_at_transform) {
	aiCamera *camera = state.assimp_scene->mCameras[state.camera_cache[node_name]];
	ERR_FAIL_COND_V(!camera, NULL);

	Camera *camera_node = memnew(Camera);
	ERR_FAIL_COND_V(!camera_node, NULL);
	float near = camera->mClipPlaneNear;
	if (Math::is_equal_approx(near, 0.0f)) {
		near = 0.1f;
	}
	camera_node->set_perspective(Math::rad2deg(camera->mHorizontalFOV) * 2.0f, near, camera->mClipPlaneFar);
	Vector3 pos = Vector3(camera->mPosition.x, camera->mPosition.y, camera->mPosition.z);
	Vector3 look_at = Vector3(camera->mLookAt.y, camera->mLookAt.x, camera->mLookAt.z).normalized();
	Vector3 up = Vector3(camera->mUp.x, camera->mUp.y, camera->mUp.z);

	look_at_transform.set_look_at(pos + look_at_transform.origin, look_at, up);
	return camera_node;
}

/**
 * Generate node
 * Recursive call to iterate over all nodes
 */
void EditorSceneImporterAssimp::_generate_node(
		ImportState &state,
		const aiNode *assimp_node) {

	ERR_FAIL_COND(assimp_node == NULL);
	String parent_name = AssimpUtils::get_assimp_string(assimp_node->mParent->mName);

#ifdef GDI_ENABLE_ASSIMP_MODIFICATION
	//String node_name = AssimpUtils::get_assimp_string(assimp_node->mName);
	//String original_name = node_name;
	//// check repeat
	//bool repeat_flag = false, changed_name_flag = false;
	//int node_size = state.nodes.size();
	//for (int i = 0; i < node_size; ++i) {
	//	if (AssimpUtils::get_assimp_string(state.nodes[i]->mName) == node_name) {
	//		repeat_flag = true;
	//		changed_name_flag = true;
	//		break;
	//	}
	//}

	//while (repeat_flag) {
	//	repeat_flag = false;
	//	node_name = node_name.insert(node_name.length(), "_x");
	//	for (int i = 0; i < node_size; ++i) {
	//		if (AssimpUtils::get_assimp_string(state.nodes[i]->mName) == node_name) {
	//			repeat_flag = true;
	//			break;
	//		}
	//	}
	//}
	//if (changed_name_flag) {
	//	aiNode *tmp_assimp_node = const_cast<aiNode*>(assimp_node);
	//	tmp_assimp_node->mName = aiString(node_name.ascii().get_data());

	//	// modify parent's(mesh) bone name
	//	aiNode *parent = tmp_assimp_node->mParent;
	//	while (parent) {
	//		if (parent->mNumMeshes > 0 && state.assimp_scene->mMeshes[parent->mMeshes[0]]->mNumBones > 0) {
	//			int bone_num = state.assimp_scene->mMeshes[parent->mMeshes[0]]->mNumBones;
	//			aiMesh *mesh = state.assimp_scene->mMeshes[parent->mMeshes[0]];
	//			for (int i = 0; i < bone_num; ++i) {
	//				if (mesh->mBones[i]->mName == aiString(original_name.ascii().get_data())) {
	//					mesh->mBones[i]->mName = aiString(node_name.ascii().get_data());
	//					break;
	//				}
	//			}
	//			break;
	//		}

	//		parent = parent->mParent;
	//	}
	//}
	state.nodes.push_back(assimp_node);
#else
	state.nodes.push_back(assimp_node);
#endif

	//if (assimp_node->mName == aiString("SM_JiaZi_002")) {
	//	int i = 0;
	//	++i;
	//}

	// please note
	// duplicate bone names exist
	// this is why we only check if the bone exists
	// so everything else is useless but the name
	// please do not copy any other values from get_bone_by_name.
	aiBone *parent_bone = get_bone_by_name(state.assimp_scene, assimp_node->mParent->mName);
	aiBone *current_bone = get_bone_by_name(state.assimp_scene, assimp_node->mName);

#ifndef GDI_ENABLE_ASSIMP_MODIFICATION

	// is this an armature
	// parent null
	// and this is the first bone :)
	if (parent_bone == NULL && current_bone) {
		state.armature_nodes.push_back(assimp_node->mParent);
		print_verbose("found valid armature: " + parent_name);
	}
#else

	// 由于需要对多Mesh对应一个animation player做处理，而不同mesh的bone对应顺序并不一样，必须单独分开处理，所以Armature可以创立到有mesh的节点下
	if (/*state.armature_nodes.size() == 0 &&*/ assimp_node->mNumMeshes > 0 /*&& assimp_node->mName == aiString("Bip001")*/) {
		// 伪造和mesh节点同级的新aiNode用作Armature节点
		if (assimp_node->mNumMeshes > 1) {
			OS::get_singleton()->print("[GDI-Warning]Gen node to armature with num meshes:[%d], node name[%s]\n", assimp_node->mNumMeshes, assimp_node->mName.data);
		}

		// 目前看来，这种多mesh的node，只用生成一次arm即可，不过上面的警告我还是保留
		// 避免以后出现bug没有任何提示
		unsigned int last_mesh_bone_num = 0;
		for (int i = 0; i < assimp_node->mNumMeshes; ++i) {
			aiMesh *mesh = state.assimp_scene->mMeshes[assimp_node->mMeshes[i]];
			// 如果mesh的bone数量不一致的话，需要发出警告，因为目前凭经验只生成第一个arm，暂时还未发现同node下bone数量不一致的多mesh
			if (0 == i) {
				last_mesh_bone_num = mesh->mNumBones;
			}
			else {
				if (last_mesh_bone_num != mesh->mNumBones) {
					printf("[GDI-Warning] assimp load node, found multi mesh node with diff bone num\n");
				}
				last_mesh_bone_num = mesh->mNumBones;
			}

			// 目前i > 0表示只手动生成一次arm
			if (i > 0) {
				continue;
			}
			if (mesh->mNumBones <= 0 && 1 == assimp_node->mNumMeshes) {
				state.gdi_mesh_node_vec.push_back(assimp_node);
				printf("----[gen mesh node]%s\n", assimp_node->mName.C_Str());
				continue;
			}

			Vector<aiString> str_vec;
			for (int i = 0; i < mesh->mNumBones; ++i) {
				// test code: check the bones info
				//str_vec.push_back(mesh->mBones[i]->mName);
				//if (mesh->mBones[i]->mName == aiString("Bone001")) {
				//	int i = 0;
				//	++i;
				//}
			}

			aiNode *parent = assimp_node->mParent;
			aiNode *newMeshNode = new aiNode();
			newMeshNode->mParent = parent;
			newMeshNode->mNumChildren = 0;
			aiString name = assimp_node->mName;
			name = mesh->mName;
			name.Append("_ArmatureNode");
			newMeshNode->mName = name;
			newMeshNode->mNumMeshes = 0;
			newMeshNode->mTransformation = aiMatrix4x4();
			parent->addChildren(1, &newMeshNode);

			state.armature_nodes.push_back((aiNode *const)newMeshNode);
			state.gdi_armature_index_map.insert((aiNode *const)newMeshNode, assimp_node->mMeshes[i]);
			state.gdi_mesh_node_vec.push_back(assimp_node);
			printf("----[gen mesh arm node]%s\n", name.C_Str());
		}
		print_verbose("use root node be the only one armature node");
	}
#endif

	for (size_t i = 0; i < assimp_node->mNumChildren; i++) {
		_generate_node(state, assimp_node->mChildren[i]);
	}
}
