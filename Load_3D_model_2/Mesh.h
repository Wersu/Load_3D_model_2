#pragma once

#include <iostream>

#include "GL\glew.h"
#include "glm\glm.hpp"
#include "assimp\Importer.hpp"

#include <vector>
#include <string>


using namespace std;
typedef unsigned int uint;
#define NUM_BONES_PER_VERTEX 4

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 text_coords;
};

struct Texture
{
	GLuint id;
	string type;
	aiString path;
};

struct BoneMatrix
{
	aiMatrix4x4 offset_matrix;
	aiMatrix4x4 final_world_transform;
};

struct VertexBoneData
{
	uint ids[NUM_BONES_PER_VERTEX];   // we have 4 bone ids for EACH vertex & 4 weights for EACH vertex
	float weights[NUM_BONES_PER_VERTEX];

	VertexBoneData()
	{
		memset(ids, 0, sizeof(ids));    // init all values in array = 0
		memset(weights, 0, sizeof(weights));
	}

	void addBoneData(uint bone_id, float weight);
};

class Mesh
{
public:
	Mesh(vector<Vertex> vertic, vector<GLuint> ind, vector<Texture> textur, vector<VertexBoneData> bone_id_weights);
	Mesh() {};
	~Mesh();

	// Render mesh
	void render(GLuint shaders_program);

private:
	//Mesh data
	vector<Vertex> vertices;
	vector<GLuint> indices;
	vector<Texture> textures;
	vector<VertexBoneData> bones_id_weights_for_each_vertex;

	//buffers
	GLuint VAO;
	GLuint VBO_vertices;
	GLuint VBO_bones;
	GLuint EBO_indices;

	//inititalize buffers
	void initialize();
};
