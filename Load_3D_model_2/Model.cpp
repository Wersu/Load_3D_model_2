#include "Model.h"
#include "MScene.h"
#include "InputHandler.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Model::Model()
{
	scene = nullptr;
}


Model::~Model()
{
	import.FreeScene();
}

void Model::setShader(GLuint shader_program)
{
	for (uint i = 0; i < MAX_BONES; i++)  
	{
		string name = "bones[" + to_string(i) + "]";
		m_bone_location[i] = glGetUniformLocation(shader_program, name.c_str());
	}
}

void Model::setModel(const string& path)
{
	scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);//загрузка  из файла

	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		cout << "error assimp : " << import.GetErrorString() << endl;
		return;
	}

	m_global_inverse_transform = scene->mRootNode->mTransformation;
	m_global_inverse_transform.Inverse();

	if (scene->mAnimations[0]->mTicksPerSecond != 0.0) //задаем скорость анимации
		ticks_per_second = scene->mAnimations[0]->mTicksPerSecond;
	else
		ticks_per_second = 25.0f;

	directory = path.substr(0, path.find_last_of('/'));

	processNode(scene->mRootNode, scene);
}


void Model::update()
{ 

}

void Model::draw(GLuint shaders_program)
{
	vector<aiMatrix4x4> transforms; //матрица преобразований

	boneTransform((double) SDL_GetTicks() / 1000.0f, transforms);

	for (uint i = 0; i < transforms.size(); i++) 
		glUniformMatrix4fv(m_bone_location[i], 1, GL_TRUE, (const GLfloat*)&transforms[i]);

	for (int i = 0; i < meshes.size(); i++)
		meshes[i].render(shaders_program);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
	Mesh mesh;
	for (uint i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh* ai_mesh = scene->mMeshes[i];
		mesh = processMesh(ai_mesh, scene);
		meshes.push_back(mesh); 
	}
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) //связать меш и модель
{ 
	vector<Vertex> vertices;  
	vector<GLuint> indices;
	vector<Texture> textures;
	vector<VertexBoneData> bones_id_weights_for_each_vertex; //id костей для каждой вершины

	vertices.reserve(mesh->mNumVertices);
	indices.reserve(mesh->mNumVertices); 

	bones_id_weights_for_each_vertex.resize(mesh->mNumVertices);

	for (uint i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		glm::vec3 vector;
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.position = vector;

		if (mesh->mNormals != NULL)
		{
			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.normal = vector;
		}
		else
			vertex.normal = glm::vec3();

		if (mesh->mTextureCoords[0])
		{
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.text_coords = vec;
		}
		else
			vertex.text_coords = glm::vec2(0.0f, 0.0f);

		vertices.push_back(vertex);
	}

	for (uint i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i]; 
		indices.push_back(face.mIndices[0]); 
		indices.push_back(face.mIndices[1]); 
		indices.push_back(face.mIndices[2]);
	}

	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		vector<Texture> diffuse_maps = LoadMaterialTexture(material, aiTextureType_DIFFUSE, "texture_diffuse");

		bool exist = false;

		for (int i = 0; (i < textures.size()) && (diffuse_maps.size() != 0); i++)
			if (textures[i].path ==  diffuse_maps[0].path) 
				exist = true;

		if(!exist && diffuse_maps.size() != 0)
			textures.push_back(diffuse_maps[0]); 

		vector<Texture> specular_maps = LoadMaterialTexture(material, aiTextureType_SPECULAR, "texture_specular");
		exist = false;

		for (int i = 0; (i < textures.size()) && (specular_maps.size() != 0); i++)
			if (textures[i].path == specular_maps[0].path) 
				exist = true;

		if (!exist  && specular_maps.size() != 0) 
			textures.push_back(specular_maps[0]);
	}


	for (uint i = 0; i < mesh->mNumBones; i++)
	{
		uint bone_index = 0;
		string bone_name(mesh->mBones[i]->mName.data);
 
		if (m_bone_mapping.find(bone_name) == m_bone_mapping.end()) 
		{	
			bone_index = m_num_bones;
			m_num_bones++;
			BoneMatrix bi;
			m_bone_matrices.push_back(bi);
			m_bone_matrices[bone_index].offset_matrix = mesh->mBones[i]->mOffsetMatrix;
			m_bone_mapping[bone_name] = bone_index;
		}
		else
			bone_index = m_bone_mapping[bone_name];

		for (uint j = 0; j < mesh->mBones[i]->mNumWeights; j++)
		{
			uint vertex_id = mesh->mBones[i]->mWeights[j].mVertexId;
			float weight = mesh->mBones[i]->mWeights[j].mWeight;
			bones_id_weights_for_each_vertex[vertex_id].addBoneData(bone_index, weight); 
		}
	} 

	return Mesh(vertices, indices, textures, bones_id_weights_for_each_vertex);
}

vector<Texture> Model::LoadMaterialTexture(aiMaterial* mat, aiTextureType type, string type_name)
{
	vector<Texture> textures;
	for (uint i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString ai_str;
		mat->GetTexture(type, i, &ai_str);

		string filename = string(ai_str.C_Str());
		filename = directory + '/' + filename;
		
		Texture texture;
		texture.id = Scene::loadImageToTexture(filename.c_str());
		texture.type = type_name;
		texture.path = ai_str;

		textures.push_back(texture);
	}
	return textures;
}

uint Model::getPosition(float p_animation_time, const aiNodeAnim* p_node_anim)
{
	for (uint i = 0; i < p_node_anim->mNumPositionKeys - 1; i++) 
		if (p_animation_time < (float)p_node_anim->mPositionKeys[i + 1].mTime) 
			return i; 

	assert(0);

	return 0;
}

uint Model::getRotation(float p_animation_time, const aiNodeAnim* p_node_anim)
{
	for (uint i = 0; i < p_node_anim->mNumRotationKeys - 1; i++) 
		if (p_animation_time < (float)p_node_anim->mRotationKeys[i + 1].mTime) 
			return i;

	assert(0);

	return 0;
}

uint Model::getScaling(float p_animation_time, const aiNodeAnim* p_node_anim)
{
	for (uint i = 0; i < p_node_anim->mNumScalingKeys - 1; i++) 
 		if (p_animation_time < (float)p_node_anim->mScalingKeys[i + 1].mTime)
			return i;

	assert(0);

	return 0;
}

aiVector3D Model::calcInterpolatedPosition(float p_animation_time, const aiNodeAnim* p_node_anim)
{
	if (p_node_anim->mNumPositionKeys == 1) 
		return p_node_anim->mPositionKeys[0].mValue;

	uint position_index = getPosition(p_animation_time, p_node_anim); 
	uint next_position_index = position_index + 1; 

	assert(next_position_index < p_node_anim->mNumPositionKeys);

	float delta_time = (float)(p_node_anim->mPositionKeys[next_position_index].mTime - p_node_anim->mPositionKeys[position_index].mTime);
	float factor = (p_animation_time - (float)p_node_anim->mPositionKeys[position_index].mTime) / delta_time;

	assert(factor >= 0.0f && factor <= 1.0f);

	aiVector3D start = p_node_anim->mPositionKeys[position_index].mValue;
	aiVector3D end = p_node_anim->mPositionKeys[next_position_index].mValue;
	aiVector3D delta = end - start;

	return start + factor * delta;
}

aiQuaternion Model::calcInterpolatedRotation(float p_animation_time, const aiNodeAnim* p_node_anim)
{
	if (p_node_anim->mNumRotationKeys == 1) 
		return p_node_anim->mRotationKeys[0].mValue;

	uint rotation_index = getRotation(p_animation_time, p_node_anim);
	uint next_rotation_index = rotation_index + 1; 

	assert(next_rotation_index < p_node_anim->mNumRotationKeys);

	float delta_time = (float)(p_node_anim->mRotationKeys[next_rotation_index].mTime - p_node_anim->mRotationKeys[rotation_index].mTime);
	float factor = (p_animation_time - (float)p_node_anim->mRotationKeys[rotation_index].mTime) / delta_time;

	assert(factor >= 0.0f && factor <= 1.0f);

	aiQuaternion start_quat = p_node_anim->mRotationKeys[rotation_index].mValue;
	aiQuaternion end_quat = p_node_anim->mRotationKeys[next_rotation_index].mValue;

	return nlerp(start_quat, end_quat, factor);
}

aiVector3D Model::calcInterpolatedScaling(float p_animation_time, const aiNodeAnim* p_node_anim)
{
	if (p_node_anim->mNumScalingKeys == 1) 
		return p_node_anim->mScalingKeys[0].mValue;

	uint scaling_index = getScaling(p_animation_time, p_node_anim); 
	uint next_scaling_index = scaling_index + 1;

	assert(next_scaling_index < p_node_anim->mNumScalingKeys);

	float delta_time = (float)(p_node_anim->mScalingKeys[next_scaling_index].mTime - p_node_anim->mScalingKeys[scaling_index].mTime);
	float  factor = (p_animation_time - (float)p_node_anim->mScalingKeys[scaling_index].mTime) / delta_time;

	assert(factor >= 0.0f && factor <= 1.0f);

	aiVector3D start = p_node_anim->mScalingKeys[scaling_index].mValue;
	aiVector3D end = p_node_anim->mScalingKeys[next_scaling_index].mValue;
	aiVector3D delta = end - start;

	return start + factor * delta;
}

const aiNodeAnim * Model::findNodeAnim(const aiAnimation * p_animation, const string p_node_name)
{
	for (uint i = 0; i < p_animation->mNumChannels; i++)
	{
		const aiNodeAnim* node_anim = p_animation->mChannels[i];
		if (string(node_anim->mNodeName.data) == p_node_name)
			return node_anim;
	}

	return nullptr;
}
							
void Model::readNodeHierarchy(float p_animation_time, const aiNode* p_node, const aiMatrix4x4 parent_transform)
{
	string node_name(p_node->mName.data);

	const aiAnimation* animation = scene->mAnimations[0];
	aiMatrix4x4 node_transform = p_node->mTransformation;

	const aiNodeAnim* node_anim = findNodeAnim(animation, node_name); 
	if (node_anim)
	{

		aiVector3D scaling_vector = calcInterpolatedScaling(p_animation_time, node_anim);
		aiMatrix4x4 scaling_matr;
		aiMatrix4x4::Scaling(scaling_vector, scaling_matr);
		aiQuaternion rotate_quat = calcInterpolatedRotation(p_animation_time, node_anim);
		aiMatrix4x4 rotate_matr = aiMatrix4x4(rotate_quat.GetMatrix());
		aiVector3D translate_vector = calcInterpolatedPosition(p_animation_time, node_anim);
		aiMatrix4x4 translate_matr;
		aiMatrix4x4::Translation(translate_vector, translate_matr);

		if ( string(node_anim->mNodeName.data) == string("Head"))
		{
			aiQuaternion rotate_head = aiQuaternion(rotate_head_xz.w, rotate_head_xz.x, rotate_head_xz.y, rotate_head_xz.z);

			node_transform = translate_matr * (rotate_matr * aiMatrix4x4(rotate_head.GetMatrix())) * scaling_matr;
		}
		else
			node_transform = translate_matr * rotate_matr * scaling_matr;
	}

	aiMatrix4x4 global_transform = parent_transform * node_transform;

	if (m_bone_mapping.find(node_name) != m_bone_mapping.end()) 
	{
		uint bone_index = m_bone_mapping[node_name];
		m_bone_matrices[bone_index].final_world_transform = m_global_inverse_transform * global_transform * m_bone_matrices[bone_index].offset_matrix;
	}

	for (uint i = 0; i < p_node->mNumChildren; i++)
		readNodeHierarchy(p_animation_time, p_node->mChildren[i], global_transform);
}

void Model::boneTransform(double time_in_sec, vector<aiMatrix4x4>& transforms) 
{
	aiMatrix4x4 identity_matrix; //матрица преобразований

	double time_in_ticks = time_in_sec * ticks_per_second;
	float animation_time = fmod(time_in_ticks, scene->mAnimations[0]->mDuration); 
	readNodeHierarchy(animation_time, scene->mRootNode, identity_matrix);
	
	transforms.resize(m_num_bones);

	for (uint i = 0; i < m_num_bones; i++)
		transforms[i] = m_bone_matrices[i].final_world_transform;
}

glm::mat4 Model::aiToGlm(aiMatrix4x4 ai_matr) //структурировать матрицу 
{
	glm::mat4 result;

	result[0].x = ai_matr.a1; result[0].y = ai_matr.b1; result[0].z = ai_matr.c1; result[0].w = ai_matr.d1;
	result[1].x = ai_matr.a2; result[1].y = ai_matr.b2; result[1].z = ai_matr.c2; result[1].w = ai_matr.d2;
	result[2].x = ai_matr.a3; result[2].y = ai_matr.b3; result[2].z = ai_matr.c3; result[2].w = ai_matr.d3;
	result[3].x = ai_matr.a4; result[3].y = ai_matr.b4; result[3].z = ai_matr.c4; result[3].w = ai_matr.d4;

	return result;
}

aiQuaternion Model::nlerp(aiQuaternion a, aiQuaternion b, float blend) //финальное умножение матриц преобразований
{
	a.Normalize();
	b.Normalize();

	aiQuaternion result;
	float dot_product = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
	float one_minus_blend = 1.0f - blend;

	if (dot_product < 0.0f)
	{
		result.x = a.x * one_minus_blend + blend * -b.x;
		result.y = a.y * one_minus_blend + blend * -b.y;
		result.z = a.z * one_minus_blend + blend * -b.z;
		result.w = a.w * one_minus_blend + blend * -b.w;
	}
	else
	{
		result.x = a.x * one_minus_blend + blend * b.x;
		result.y = a.y * one_minus_blend + blend * b.y;
		result.z = a.z * one_minus_blend + blend * b.z;
		result.w = a.w * one_minus_blend + blend * b.w;
	}

	return result.Normalize();
}
