#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <math.h>

#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>

#include "Window.h"
#include "Mesh.h"
#include "Shader_light.h"
#include "Camera.h"
#include "Texture.h"
#include"Model.h"

const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;

Camera camera;

Texture brickTexture;
Texture dirtTexture;
Texture plainTexture;
Texture pisoTexture;
Texture AgaveTexture;
Texture FlechaTexture;
Texture NumerosTexture;
Texture Numero1Texture;
Texture Numero2Texture;
Texture ArcoTexture;
Texture LetreroTexture;
Texture PuertaTexture;
Texture DadoTexture;
Texture GaleriaTexture;

Model Kitt_M;
Model Llanta_M;
Model Dragon_M;
Model alaDerecha_M;
Model alaIzquierda_M;
Model Tiamat_M;
Model Arco_M;
Model Letrero_M;
Model Puerta_Derecha_M;
Model Puerta_Izquierda_M;

Model Galeria;
Model Muneco_M;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

DirectionalLight mainLight;

static const char* vShader = "shaders/shader_light.vert";

static const char* fShader = "shaders/shader_light.frag";

void CreateObjects()
{
	unsigned int indices[] = {
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2
	};

	GLfloat vertices[] = {
		//	x      y      z			u	  v			nx	  ny    nz
			-1.0f, -1.0f, -0.6f,	0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 1.0f,		0.5f, 0.0f,		0.0f, 0.0f, 0.0f,
			1.0f, -1.0f, -0.6f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,		0.5f, 1.0f,		0.0f, 0.0f, 0.0f
	};

	unsigned int floorIndices[] = {
		0, 2, 1,
		1, 2, 3
	};

	GLfloat floorVertices[] = {
		-10.0f, 0.0f, -10.0f,	0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, -10.0f,	10.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		-10.0f, 0.0f, 10.0f,	0.0f, 10.0f,	0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, 10.0f,		10.0f, 10.0f,	0.0f, -1.0f, 0.0f
	};


	Mesh* obj1 = new Mesh();
	obj1->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj1);

	Mesh* obj2 = new Mesh();
	obj2->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj2);

	Mesh* obj3 = new Mesh();
	obj3->CreateMesh(floorVertices, floorIndices, 32, 6);
	meshList.push_back(obj3);
}

void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}

int main()
{
	mainWindow = Window(1366, 768); // 1280, 1024 or 1024, 768
	mainWindow.Initialise();

	CreateObjects();
	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 0.5f, 0.5f);

	// >>> IMPLEMENTACIÓN 1: Inicialización de la Luz Direccional (Blanca, apuntando hacia abajo) <<<
	mainLight = DirectionalLight(1.0f, 1.0f, 1.0f, // Color (R, G, B)
		0.5f, 0.8f,     // Intensidad Ambiente, Intensidad Difusa
		0.0f, -1.0f, 0.0f); // Dirección (Y negativa = hacia abajo)
	// >>> FIN IMPLEMENTACIÓN 1 <<<

	pisoTexture = Texture("Textures/Slate_Floor_Tiles_wfribhq_1K_BaseColor.jpg");
	pisoTexture.LoadTextureA();
	GaleriaTexture = Texture("Textures/181614_Negro.png");
	GaleriaTexture.LoadTextureA();

	Galeria = Model();
	Galeria.LoadModel("Models/Galeria.fbx");

	// Carga del modelo del muñeco
	Muneco_M = Model();
	Muneco_M.LoadModel("Models/bart.obj");

	// >>> IMPLEMENTACIÓN 2: Carga de la textura para el muñeco <<<
	Texture munecoTexture = Texture("Textures/bart.jpg");
	munecoTexture.LoadTextureA();
	// >>> FIN IMPLEMENTACIÓN 2 <<<

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0, uniformTextureOffset = 0;
	GLuint uniformColor = 0;
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);

	glm::mat4 model(1.0);
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec2 toffset = glm::vec2(0.0f, 0.0f);

	////Loop mientras no se cierra la ventana
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;
		//Recibir eventos del usuario
		glfwPollEvents();
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		// Clear the window
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformEyePosition = shaderList[0].GetEyePositionLocation();
		uniformColor = shaderList[0].getColorLocation();
		uniformTextureOffset = shaderList[0].getOffsetLocation();


		uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
		uniformShininess = shaderList[0].GetShininessLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

		shaderList[0].SetDirectionalLight(&mainLight);

		//Reinicializando variables cada ciclo de reloj
		model = glm::mat4(1.0);
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		toffset = glm::vec2(0.0f, 0.0f);

		// --- Renderizado del Piso ---
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		// Configuracion material por defecto para el piso
		glUniform1f(uniformSpecularIntensity, 0.1f);
		glUniform1f(uniformShininess, 2.0f);
		pisoTexture.UseTexture();
		meshList[2]->RenderMesh();

		// --- Carga de la galeria ---
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, 20.0f, 0.0f));
		model = glm::rotate(model, -90.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		// Configuracion material para la galeria
		glUniform1f(uniformSpecularIntensity, 0.5f);
		glUniform1f(uniformShininess, 8.0f);
		GaleriaTexture.UseTexture();
		Galeria.RenderModel();

		// --- Render del muñeco (Bart) ---
		{
			// Ajustes de posicionamiento
			const glm::vec3 munecoWorldPos = glm::vec3(-20.0f, -2.0f, 70.0f);
			const float munecoScale = 48.0f;
			const float munecoRotYdeg = 180.0f;

			model = glm::mat4(1.0f);
			model = glm::translate(model, munecoWorldPos);
			model = glm::rotate(model, munecoRotYdeg * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(munecoScale, munecoScale, munecoScale));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

			// >>> IMPLEMENTACIÓN 3: Configuración del material para que se ilumine <<<
			glUniform1f(uniformSpecularIntensity, 1.0f); // Intensidad alta para que el color se vea
			glUniform1f(uniformShininess, 32.0f);        // Brillo decente
			// >>> FIN IMPLEMENTACIÓN 3 <<<

			munecoTexture.UseTexture();
			Muneco_M.RenderModel();
		}


		glDisable(GL_BLEND);
		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}