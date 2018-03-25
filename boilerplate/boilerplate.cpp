// ==========================================================================
// Barebones OpenGL Core Profile Boilerplate
//    using the GLFW windowing system (http://www.glfw.org)
//
// Loosely based on
//  - Chris Wellons' example (https://github.com/skeeto/opengl-demo) and
//  - Camilla Berglund's example (http://www.glfw.org/docs/latest/quick.html)
//
// Author:  Sonny Chan, University of Calgary
// Co-Authors:
//			Jeremy Hart, University of Calgary
//			John Hall, University of Calgary
// Date:    December 2015
// ==========================================================================

#include <cstdio>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <string>
#include <iterator>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "texture.h"

#include "imagebuffer.h"

using namespace std;
using namespace glm;
// --------------------------------------------------------------------------
// OpenGL utility and support function prototypes

void QueryGLVersion();
bool CheckGLErrors();

string LoadSource(const string &filename);
GLuint CompileShader(GLenum shaderType, const string &source);
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);



// --------------------------------------------------------------------------
// Functions to set up OpenGL shader programs for rendering

// load, compile, and link shaders, returning true if successful
GLuint InitializeShaders()
{
	// load shader source from files
	string vertexSource = LoadSource("shaders/vertex.glsl");
	string fragmentSource = LoadSource("shaders/fragment.glsl");
	if (vertexSource.empty() || fragmentSource.empty()) return false;

	// compile shader source into shader objects
	GLuint vertex = CompileShader(GL_VERTEX_SHADER, vertexSource);
	GLuint fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

	// link shader program
	GLuint program = LinkProgram(vertex, fragment);

	glDeleteShader(vertex);
	glDeleteShader(fragment);

	// check for OpenGL errors and return false if error occurred
	return program;
}

// --------------------------------------------------------------------------
// Functions to set up OpenGL buffers for storing geometry data

struct Geometry
{
	// OpenGL names for array buffer objects, vertex array object
	GLuint  vertexBuffer;
	GLuint  textureBuffer;
	GLuint  colourBuffer;
	GLuint  vertexArray;
	GLsizei elementCount;

	// initialize object names to zero (OpenGL reserved value)
	Geometry() : vertexBuffer(0), colourBuffer(0), vertexArray(0), elementCount(0)
	{}
};

bool InitializeVAO(Geometry *geometry){

	const GLuint VERTEX_INDEX = 0;
	const GLuint COLOUR_INDEX = 1;

	//Generate Vertex Buffer Objects
	// create an array buffer object for storing our vertices
	glGenBuffers(1, &geometry->vertexBuffer);

	// create another one for storing our colours
	glGenBuffers(1, &geometry->colourBuffer);

	//Set up Vertex Array Object
	// create a vertex array object encapsulating all our vertex attributes
	glGenVertexArrays(1, &geometry->vertexArray);
	glBindVertexArray(geometry->vertexArray);

	// associate the position array with the vertex array object
	glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
	glVertexAttribPointer(
		VERTEX_INDEX,		//Attribute index 
		2, 					//# of components
		GL_FLOAT, 			//Type of component
		GL_FALSE, 			//Should be normalized?
		sizeof(vec2),		//Stride - can use 0 if tightly packed
		0);					//Offset to first element
	glEnableVertexAttribArray(VERTEX_INDEX);

	// associate the colour array with the vertex array object
	glBindBuffer(GL_ARRAY_BUFFER, geometry->colourBuffer);
	glVertexAttribPointer(
		COLOUR_INDEX,		//Attribute index 
		3, 					//# of components
		GL_FLOAT, 			//Type of component
		GL_FALSE, 			//Should be normalized?
		sizeof(vec3), 		//Stride - can use 0 if tightly packed
		0);					//Offset to first element
	glEnableVertexAttribArray(COLOUR_INDEX);

	// unbind our buffers, resetting to default state
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return !CheckGLErrors();
}

// create buffers and fill with geometry data, returning true if successful
bool LoadGeometry(Geometry *geometry, vec2 *vertices, vec3 *colours, int elementCount)
{
	geometry->elementCount = elementCount;

	// create an array buffer object for storing our vertices
	glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2)*geometry->elementCount, vertices, GL_STATIC_DRAW);

	// create another one for storing our colours
	glBindBuffer(GL_ARRAY_BUFFER, geometry->colourBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3)*geometry->elementCount, colours, GL_STATIC_DRAW);

	//Unbind buffer to reset to default state
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// check for OpenGL errors and return false if error occurred
	return !CheckGLErrors();
}

// deallocate geometry-related objects
void DestroyGeometry(Geometry *geometry)
{
	// unbind and destroy our vertex array object and associated buffers
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &geometry->vertexArray);
	glDeleteBuffers(1, &geometry->vertexBuffer);
	glDeleteBuffers(1, &geometry->colourBuffer);
}

// --------------------------------------------------------------------------
// Rendering function that draws our scene to the frame buffer

void RenderScene(Geometry *geometry, GLuint program)
{
	// clear screen to a dark grey colour
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// bind our shader program and the vertex array object containing our
	// scene geometry, then tell OpenGL to draw our geometry
	glUseProgram(program);
	glBindVertexArray(geometry->vertexArray);
	glDrawArrays(GL_TRIANGLES, 0, geometry->elementCount);

	// reset state to default (no shader or geometry bound)
	glBindVertexArray(0);
	glUseProgram(0);

	// check for an report any OpenGL errors
	CheckGLErrors();
}

// --------------------------------------------------------------------------
// GLFW callback functions

// reports GLFW errors
void ErrorCallback(int error, const char* description)
{
	cout << "GLFW ERROR " << error << ":" << endl;
	cout << description << endl;
}

// handles keyboard input events
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

//---------------------------------------------------------------------------
// calculate t, r, s?
struct Ray
{
	vec3 origin;
	vec3 dirVector;
	float focalLength;
};

struct Light
{
	vec3 origin;
	vec3 color;
};

struct Shape
{
	int type; // 0 means sphere, 1 means plane; 2 means triangle
	vector<vec3> data;
	float addition = .0f;
	vec3 color;
	int id;
	vec3 specularColor;
	vec3 specularHighLight;
	float PEx;
};

struct IntersectionInfo
{
	float t;
	Shape shape;	
};

vector<Ray> myRayList;
vector<Shape> myShapeList;
vector<Shape> myShadowShapeList;
vector<vec3> myLightList;
vector<vec3> colorList;
vector<vec3> rayList;

Ray generateRay(int x, int y, int width, int height, vec3 origin, float distance){
	Ray aRay;
	aRay.origin = origin;
	aRay.focalLength = distance;
	aRay.dirVector.z = -distance;
	aRay.dirVector.x = (float)x/(width/2)-1.0f-origin.x;
	aRay.dirVector.y = (float)y/(height/2)-1.0f-origin.y;
	return aRay;
}

void readFile(vector<Shape> &shapeList, Light *light, const char* filename){
		
	ifstream f (filename);
	
	const int BUFF_SIZE = 256;
	char buffer [BUFF_SIZE];

	while(f){
		string word;
		f >> word;
		if(word.compare("#") ==0){
			f.getline(buffer,BUFF_SIZE);
		}else if(word.compare("light") ==0){
			f.getline(buffer,BUFF_SIZE);
			
			f.getline(buffer,BUFF_SIZE);
			float x,y,z;
			if(sscanf(buffer,"%f %f %f", &x, &y, &z)==3){				
				light->origin = vec3(x,y,z);
			}
			
			f.getline(buffer,BUFF_SIZE);
			if(sscanf(buffer,"%f %f %f", &x, &y, &z)==3){
				light->color = vec3(x,y,z);
			}

		}else if(word.compare("sphere")==0){
			Shape sphere;
			sphere.type = 0;
			f.getline(buffer,BUFF_SIZE);
			
			f.getline(buffer,BUFF_SIZE);
			float x,y,z;
			if(sscanf(buffer,"%f %f %f", &x, &y, &z)==3){
				sphere.data.push_back(vec3(x,y,z));
			}
			
			float r;
			f >> r;
			sphere.addition = r;
			f.getline(buffer,BUFF_SIZE);
			
			f.getline(buffer,BUFF_SIZE);
			if(sscanf(buffer,"%f %f %f", &x, &y, &z)==3){
				sphere.color = vec3(x,y,z);
			}
			
			f.getline(buffer,BUFF_SIZE);
			if(sscanf(buffer,"%f %f %f", &x, &y, &z)==3){
				sphere.specularColor = vec3(x,y,z);
			}
			
			f.getline(buffer,BUFF_SIZE);
			if(sscanf(buffer,"%f %f %f", &x, &y, &z)==3){
				sphere.specularHighLight = vec3(x,y,z);
			}
			
			f.getline(buffer,BUFF_SIZE);
			if(sscanf(buffer,"%f", &x)==1){
				sphere.PEx = x;
			}
			
			shapeList.push_back(sphere);
							
		}else if(word.compare("triangle")==0){
			Shape triangle;
			triangle.type = 2;
			f.getline(buffer,BUFF_SIZE);
			
			f.getline(buffer,BUFF_SIZE);
			float x,y,z;
			if(sscanf(buffer,"%f %f %f", &x, &y, &z)==3){
				triangle.data.push_back(vec3(x,y,z));
			}
			
			f.getline(buffer,BUFF_SIZE);
			if(sscanf(buffer,"%f %f %f", &x, &y, &z)==3){
				triangle.data.push_back(vec3(x,y,z));
			}
			
			f.getline(buffer,BUFF_SIZE);
			if(sscanf(buffer,"%f %f %f", &x, &y, &z)==3){
				triangle.data.push_back(vec3(x,y,z));
			}
			
			f.getline(buffer,BUFF_SIZE);
			if(sscanf(buffer,"%f %f %f", &x, &y, &z)==3){
				triangle.color= vec3(x,y,z);
			}
			
			f.getline(buffer,BUFF_SIZE);
			if(sscanf(buffer,"%f %f %f", &x, &y, &z)==3){
				triangle.specularColor = vec3(x,y,z);
			}
			
			f.getline(buffer,BUFF_SIZE);
			if(sscanf(buffer,"%f %f %f", &x, &y, &z)==3){
				triangle.specularHighLight = vec3(x,y,z);
			}
			
			f.getline(buffer,BUFF_SIZE);
			if(sscanf(buffer,"%f", &x)==1){
				triangle.PEx = x;
			}
			
			shapeList.push_back(triangle);
			
		}else if(word.compare("plane")==0){
			Shape plane;
			plane.type = 1;
			f.getline(buffer,BUFF_SIZE);
			
			f.getline(buffer,BUFF_SIZE);
			float x,y,z;
			if(sscanf(buffer,"%f %f %f", &x, &y, &z)==3){
				plane.data.push_back(vec3(x,y,z));
			}
			
			f.getline(buffer,BUFF_SIZE);
			if(sscanf(buffer,"%f %f %f", &x, &y, &z)==3){
				plane.data.push_back(vec3(x,y,z));
			}
			
			f.getline(buffer,BUFF_SIZE);
			if(sscanf(buffer,"%f %f %f", &x, &y, &z)==3){
				plane.color = vec3(x,y,z);
			}
			
			f.getline(buffer,BUFF_SIZE);
			if(sscanf(buffer,"%f %f %f", &x, &y, &z)==3){
				plane.specularColor = vec3(x,y,z);
			}
			
			f.getline(buffer,BUFF_SIZE);
			if(sscanf(buffer,"%f %f %f", &x, &y, &z)==3){
				plane.specularHighLight = vec3(x,y,z);
			}
			
			f.getline(buffer,BUFF_SIZE);
			if(sscanf(buffer,"%f", &x)==1){
				plane.PEx = x;
			}
			
			shapeList.push_back(plane);
		}
	}
	
	f.close();
}

bool testIntersectSphere(Ray aRay, Shape aShape, IntersectionInfo *info){
	bool hit = false;
	vec3 e = aRay.origin;
	vec3 d = aRay.dirVector;
	vec3 c = aShape.data[0];
	float r = aShape.addition;
	
	float discriminant = pow(dot(d,e-c),2)-dot(d,d)*(dot(e-c,e-c)-pow(r,2));
	if(discriminant<0){
		return hit;
	}else{
		hit = true;
		float t = (-(dot(d,e-c))-sqrt(discriminant))/dot(d,d);
		info->t = t;
		info->shape = aShape;
	}
	
	return hit;
}

bool testIntersectPlane(Ray aRay, Shape aShape, IntersectionInfo *info){
	bool hit = false;
	vec3 e = aRay.origin;
	vec3 d = aRay.dirVector;
	vec3 n = aShape.data[0];
	vec3 q = aShape.data[1];
	
	if(abs(dot(d,n))<0.0001){
		return hit;
	}else{
		hit = true;
		float t = dot((q-e),n)/dot(d,n);
		info->t = t;
		info->shape = aShape;
	}
	return hit;
}

bool testIntersectTriangle(Ray aRay, Shape aShape, IntersectionInfo *info){
	bool hit = false;
	vec3 e = aRay.origin;
	vec3 d = aRay.dirVector;
	vec3 a = aShape.data[0];
	vec3 b = aShape.data[1];
	vec3 c = aShape.data[2];
	
	float A = a.x-b.x;
	float B = a.y-b.y;
	float C = a.z-b.z;
	float D = a.x-c.x;
	float E = a.y-c.y;
	float F = a.z-c.z;
	float G = d.x;
	float H = d.y;
	float I = d.z;
	float J = a.x-e.x;
	float K = a.y-e.y;
	float L = a.z-e.z;
	
	float M =A*(E*I-H*F)+B*(G*F-D*I)+C*(D*H-E*G);
	
	float beta = (J*(E*I-H*F)+K*(G*F-D*I)+L*(D*H-E*G))/M;
	float gamma = (I*(A*K-J*B)+H*(J*C-A*L)+G*(B*L-K*C))/M;
	float t = -(F*(A*K-J*B)+E*(J*C-A*L)+D*(B*L-K*C))/M;
	
	if(M==0){
		return hit;
	}
	
	if(gamma<0 || gamma>1){
		return hit;
	}else if(beta<0 || beta>(1-gamma)){
		return hit;
	}

	hit = true;
	info->t = t;
	info->shape = aShape;
	
	return hit;
}

bool testIntersection(Ray aRay, Shape aShape, IntersectionInfo *info){ 
	bool hit = false;
	if(aShape.type==0){
		hit = testIntersectSphere(aRay, aShape, info);
	}else if(aShape.type==1){
		hit = testIntersectPlane(aRay, aShape, info);
	}else if(aShape.type==2){
		hit = testIntersectTriangle(aRay, aShape, info);
	}
	return hit;
}



bool testIntersections(Ray aRay, vector<Shape> objectList, IntersectionInfo *resultInfo,float lowerBound,float upperBound){ 
	bool hit = false;
	float t = upperBound;
	for(int i=0; i<objectList.size();i++){
		IntersectionInfo aInfo;
		if(testIntersection(aRay, objectList[i],&aInfo)){
			if(aInfo.t>=lowerBound && aInfo.t<t){
				hit = true;
				t = aInfo.t;
				resultInfo->t=t;
				resultInfo->shape=aInfo.shape;
			}
		}
	}
	
	return hit;
}

vec3 shadingEquation(vec3 intersectionPoint,vec3 view,vec3 surfaceColor, vec3 lightColor, vec3 lightSource, vec3 surfaceNormalVec){
	vec3 l = normalize(lightSource-intersectionPoint);
	vec3 kd = surfaceColor;
	vec3 I = lightColor;
	vec3 n = surfaceNormalVec;
	vec3 ks = vec3(0.7,0.7,0.7);
	vec3 v = view;
	vec3 h = normalize(v+l);
	vec3 ka = surfaceColor;
	vec3 Ia = vec3(.5f,.5f,.5f);
	
	return ka*Ia+kd*I*glm::max(.0f,dot(n,l))+ks*I*(float)(pow(glm::max(.0f,dot(n,h)),500));
}

vec3 surfaceNormalVector(Ray aRay, IntersectionInfo info){
	vec3 result;
	if(info.shape.type == 0){
		result = (aRay.origin + info.t*aRay.dirVector)-info.shape.data[0];
	}else if(info.shape.type == 1){
		result = info.shape.data[0];
	}else if(info.shape.type == 2){
		result = cross(info.shape.data[1]-info.shape.data[0],info.shape.data[2]-info.shape.data[0]);
	}
	return normalize(result);
}

vec3 raycolor(Ray ray, float lowerBound, float upperBound,Light light){
		IntersectionInfo info;
		if(testIntersections(ray,myShapeList,&info,lowerBound,upperBound)){
			vec3 d = ray.dirVector;
			vec3 color = vec3(0,0,0);
			vec3 n = surfaceNormalVector(ray,info);
			vec3 v = normalize(-d);
			vec3 intersectP = ray.origin+info.t*d;
			color = info.shape.color*vec3(.5f,.5f,.5f);
				
			Ray shadowRay;
			shadowRay.origin = intersectP;
			shadowRay.dirVector = (light.origin-shadowRay.origin);
			IntersectionInfo emptyInfo;
			if(!testIntersections(shadowRay,myShapeList,&emptyInfo,0.0001f,1.0f)){				
				vec3 l = normalize(light.origin-intersectP);
				vec3 kd = info.shape.color;
				vec3 I = light.color;					
				vec3 ks = vec3(0.7,0.7,0.7);			
				vec3 h = normalize(v+l);
				color = color+kd*I*glm::max(.0f,dot(n,l))+ks*I*(float)(pow(glm::max(.0f,dot(n,h)),500));
			}
			return color;
		}else{
			return vec3(0,0,0);
		}
	
}

vec3 raycolorRe(Ray ray, float lowerBound, float upperBound,Light light,int times){
		IntersectionInfo info;
		if(testIntersections(ray,myShapeList,&info,lowerBound,upperBound)){
			vec3 d = ray.dirVector;
			vec3 color = vec3(0,0,0);
			vec3 n = surfaceNormalVector(ray,info);
			vec3 v = normalize(-d);
			vec3 intersectP = ray.origin+info.t*d;
			color = info.shape.color*vec3(.4f,.4f,.4f);
				
			Ray shadowRay;
			shadowRay.origin = intersectP;
			shadowRay.dirVector = (light.origin-shadowRay.origin);
			IntersectionInfo emptyInfo;
			if(!testIntersections(shadowRay,myShapeList,&emptyInfo,0.0001f,1.0f)){				
				vec3 l = normalize(light.origin-intersectP);
				vec3 kd = info.shape.color;
				vec3 I = light.color;					
				vec3 ks = info.shape.specularHighLight;			
				vec3 h = normalize(v+l);
				color = color+kd*I*glm::max(.0f,dot(n,l))+ks*I*(float)(pow(glm::max(.0f,dot(n,h)),info.shape.PEx));
			}
			vec3 r = normalize(d) - 2*dot(normalize(d),n)*n;
			vec3 km = info.shape.specularColor;
			Ray reflectionRay;
			reflectionRay.origin = intersectP;
			reflectionRay.dirVector = r;
			if(info.shape.specularColor==vec3(0,0,0))
				times=0;
			if(times>0){
				times--;
				return color+km*raycolorRe(reflectionRay,0.0001,99999.9f,light,times);
			}else{
				return color;
			}
		}else{
			return vec3(0,0,0);
		}
	
}



vec3 reflectionEquation(){
	return vec3(0,0,0);
}

void printLines(const char* filename){
	ifstream f (filename);
	
	const int BUFF_SIZE = 256; // why chose 256
	char buffer [BUFF_SIZE];
	
	while(f){
		f.getline(buffer, BUFF_SIZE);
		cout << buffer << endl;
	}
	
	f.close();
}

//---------------------------------------------------------------------------

// ==========================================================================
// PROGRAM ENTRY POINT

int main(int argc, char *argv[])
{
	// initialize the GLFW windowing system
	if (!glfwInit()) {
		cout << "ERROR: GLFW failed to initialize, TERMINATING" << endl;
		return -1;
	}
	glfwSetErrorCallback(ErrorCallback);

	// attempt to create a window with an OpenGL 4.1 core profile context
	GLFWwindow *window = 0;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	int width = 512, height = 512;
	window = glfwCreateWindow(width, height, "CPSC 453 OpenGL Boilerplate", 0, 0);
	if (!window) {
		cout << "Program failed to create GLFW window, TERMINATING" << endl;
		glfwTerminate();
		return -1;
	}

	// set keyboard callback function and make our context current (active)
	glfwSetKeyCallback(window, KeyCallback);
	glfwMakeContextCurrent(window);

	//Intialize GLAD
	if (!gladLoadGL())
	{
		cout << "GLAD init failed" << endl;
		return -1;
	}

	// query and print out information about our OpenGL environment
	QueryGLVersion();
	
	Light light1;
	readFile(myShapeList,&light1,"scene3.txt");
	cout<<myShapeList.size()<<endl;


	ImageBuffer image = ImageBuffer();
	image.Initialize();
	
	for(int i=0;i<width;i++){
		for(int j=0;j<height;j++){
			Ray ray = generateRay(i,j,width,height,vec3(0,0,0),2.0f);
			vec3 color = vec3(0,0,0);

			//color = raycolor(ray,.0f,99999.9f,light1);
			color = raycolorRe(ray,.0f,9999.9f,light1,10);
			
			image.SetPixel(i,j,color);
		}
	}
	
	//readData("scene2.txt");
	
	
	image.Render();
	
	image.SaveToFile("renderImage.png");
	
	
	// run an event-triggered main loop
	while (!glfwWindowShouldClose(window))
	{


		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	// clean up allocated resources before exit
	glfwDestroyWindow(window);
	glfwTerminate();

	cout << "Goodbye!" << endl;
	return 0;
}

// ==========================================================================
// SUPPORT FUNCTION DEFINITIONS

// --------------------------------------------------------------------------
// OpenGL utility functions

void QueryGLVersion()
{
	// query opengl version and renderer information
	string version = reinterpret_cast<const char *>(glGetString(GL_VERSION));
	string glslver = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
	string renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));

	cout << "OpenGL [ " << version << " ] "
		<< "with GLSL [ " << glslver << " ] "
		<< "on renderer [ " << renderer << " ]" << endl;
}

bool CheckGLErrors()
{
	bool error = false;
	for (GLenum flag = glGetError(); flag != GL_NO_ERROR; flag = glGetError())
	{
		cout << "OpenGL ERROR:  ";
		switch (flag) {
		case GL_INVALID_ENUM:
			cout << "GL_INVALID_ENUM" << endl; break;
		case GL_INVALID_VALUE:
			cout << "GL_INVALID_VALUE" << endl; break;
		case GL_INVALID_OPERATION:
			cout << "GL_INVALID_OPERATION" << endl; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			cout << "GL_INVALID_FRAMEBUFFER_OPERATION" << endl; break;
		case GL_OUT_OF_MEMORY:
			cout << "GL_OUT_OF_MEMORY" << endl; break;
		default:
			cout << "[unknown error code]" << endl;
		}
		error = true;
	}
	return error;
}

// --------------------------------------------------------------------------
// OpenGL shader support functions

// reads a text file with the given name into a string
string LoadSource(const string &filename)
{
	string source;

	ifstream input(filename.c_str());
	if (input) {
		copy(istreambuf_iterator<char>(input),
			istreambuf_iterator<char>(),
			back_inserter(source));
		input.close();
	}
	else {
		cout << "ERROR: Could not load shader source from file "
			<< filename << endl;
	}

	return source;
}

// creates and returns a shader object compiled from the given source
GLuint CompileShader(GLenum shaderType, const string &source)
{
	// allocate shader object name
	GLuint shaderObject = glCreateShader(shaderType);

	// try compiling the source as a shader of the given type
	const GLchar *source_ptr = source.c_str();
	glShaderSource(shaderObject, 1, &source_ptr, 0);
	glCompileShader(shaderObject);

	// retrieve compile status
	GLint status;
	glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint length;
		glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
		string info(length, ' ');
		glGetShaderInfoLog(shaderObject, info.length(), &length, &info[0]);
		cout << "ERROR compiling shader:" << endl << endl;
		cout << source << endl;
		cout << info << endl;
	}

	return shaderObject;
}

// creates and returns a program object linked from vertex and fragment shaders
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader)
{
	// allocate program object name
	GLuint programObject = glCreateProgram();

	// attach provided shader objects to this program
	if (vertexShader)   glAttachShader(programObject, vertexShader);
	if (fragmentShader) glAttachShader(programObject, fragmentShader);

	// try linking the program with given attachments
	glLinkProgram(programObject);

	// retrieve link status
	GLint status;
	glGetProgramiv(programObject, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint length;
		glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &length);
		string info(length, ' ');
		glGetProgramInfoLog(programObject, info.length(), &length, &info[0]);
		cout << "ERROR linking shader program:" << endl;
		cout << info << endl;
	}

	return programObject;
}
