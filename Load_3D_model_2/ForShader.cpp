#include "ForShader.h"


ForShader::ForShader()
{
}


ForShader::~ForShader()
{
}


const GLchar* ForShader::readShader(const char* filename)
{

#ifdef WIN32
	FILE* infile;
	fopen_s(&infile, filename, "rb");
#else
	FILE* infile = fopen(filename, "rb");
#endif 

	if (!infile) {

#ifdef _DEBUG
		std::cerr << "Unable to open file '" << filename << "'" << std::endl;
#endif 

		return NULL;
	}

	fseek(infile, 0, SEEK_END);
	int len = ftell(infile);
	fseek(infile, 0, SEEK_SET);

	GLchar* source = new GLchar[len + 1];

	fread(source, 1, len, infile);
	fclose(infile);

	source[len] = 0;

	return const_cast<const GLchar*>(source);
}

GLuint ForShader::createProgram(const char* vertex, const char* fragment)
{
	const char* vertexShaderCode = readShader(vertex);
	const char* fragmentShaderCode = readShader(fragment);

	GLuint vertexID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentID = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vertexID, 1, &vertexShaderCode, 0);
	delete[] vertexShaderCode;

	glShaderSource(fragmentID, 1, &fragmentShaderCode, 0);
	delete[] fragmentShaderCode;

	glCompileShader(vertexID);

	GLint compiled;
	glGetShaderiv(vertexID, GL_COMPILE_STATUS, &compiled);

	if (compiled == GL_FALSE) 
	{
		GLsizei len;

		glGetShaderiv(vertexID, GL_INFO_LOG_LENGTH, &len);

		GLchar* log = new GLchar[len + 1];
		glGetShaderInfoLog(vertexID, len, &len, log);

		std::cout << "Vertex Shader compilation failed: " << log << std::endl;

		delete[] log;
	}

	glCompileShader(fragmentID);

	glGetShaderiv(fragmentID, GL_COMPILE_STATUS, &compiled);

	if (compiled == GL_FALSE) 
	{
		GLsizei len;

		glGetShaderiv(fragmentID, GL_INFO_LOG_LENGTH, &len);

		GLchar* log = new GLchar[len + 1];
		glGetShaderInfoLog(fragmentID, len, &len, log);

		std::cout << "Fragment Shader compilation failed: " << log << std::endl;

		delete[] log;
	}

	GLuint programID = glCreateProgram();
	glAttachShader(programID, vertexID);
	glAttachShader(programID, fragmentID);

	glLinkProgram(programID);

	GLint linked;
	glGetProgramiv(programID, GL_LINK_STATUS, &linked);

	if (linked == GL_FALSE)
	{
		GLsizei len;

		glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &len);

		GLchar* log = new GLchar[len + 1];
		glGetProgramInfoLog(programID, len, &len, log);

		std::cerr << "glLinkProgram(programID) failed: " << log << std::endl;

		delete[] log;
	}


	glDeleteShader(vertexID);
	glDeleteShader(fragmentID);

	return programID;
}