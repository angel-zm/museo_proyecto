#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <math.h>
#include <algorithm>
#include <string>

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

// New: lists of mesh indices for parts
std::vector<unsigned int> munecoHeadMeshes;
std::vector<unsigned int> munecoHandMeshes;
std::vector<unsigned int> munecoFootMeshes;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

DirectionalLight mainLight;

static const char* vShader = "shaders/shader_light.vert";

static const char* fShader = "shaders/shader_light.frag";

// New: global position for the model
glm::vec3 munecoPosition(0.0f, -2.0f, 70.0f);
// movement speed (world units per second)
float munecoMoveSpeed = 5.0f;

void CreateObjects()
{
    unsigned int indices[] = {
        0, 3, 1,
        1, 3, 2,
        2, 3, 0,
        0, 1, 2
    };

    GLfloat vertices[] = {
        // x      y      z       u    v       nx   ny   nz
        -1.0f, -1.0f, -0.6f,    0.0f, 0.0f,  0.0f, 0.0f, 0.0f,
         0.0f, -1.0f, 1.0f,     0.5f, 0.0f,  0.0f, 0.0f, 0.0f,
         1.0f, -1.0f, -0.6f,    1.0f, 0.0f,  0.0f, 0.0f, 0.0f,
         0.0f, 1.0f, 0.0f,      0.5f, 1.0f,  0.0f, 0.0f, 0.0f
    };

    unsigned int floorIndices[] = {
        0, 2, 1,
        1, 2, 3
    };

    GLfloat floorVertices[] = {
        -10.0f, 0.0f, -10.0f,   0.0f, 0.0f,   0.0f, -1.0f, 0.0f,
         10.0f, 0.0f, -10.0f,  10.0f, 0.0f,  0.0f, -1.0f, 0.0f,
        -10.0f, 0.0f, 10.0f,    0.0f, 10.0f,  0.0f, -1.0f, 0.0f,
         10.0f, 0.0f, 10.0f,    10.0f, 10.0f, 0.0f, -1.0f, 0.0f
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

// helper to lowercase
static std::string toLower(const std::string &s)
{
	std::string out = s;
	std::transform(out.begin(), out.end(), out.begin(), ::tolower);
	return out;
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

	// Classify meshes once (by name and approximate center) so only head/hands/feet animate
	{
		size_t mc = Muneco_M.GetMeshCount();
		for (unsigned int i = 0; i < mc; ++i)
		{
			std::string name = toLower(Muneco_M.GetMeshName(i));
			glm::vec3 center = Muneco_M.GetMeshCenter(i);
			// by name first
			if (name.find("head") != std::string::npos || name.find("skull") != std::string::npos || center.y > 0.5f)
			{
				munecoHeadMeshes.push_back(i);
			}
			else if (name.find("hand") != std::string::npos || name.find("arm") != std::string::npos || fabs(center.x) > 0.18f)
			{
				munecoHandMeshes.push_back(i);
			}
			else if (name.find("leg") != std::string::npos || name.find("foot") != std::string::npos || center.y < 0.0f)
			{
				munecoFootMeshes.push_back(i);
			}
			// else ignore - will render without animation
		}
	}

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

		// --- Render del mu\xC3\xB1eco (Bart) ---
		{
			// Ajustes de posicionamiento
			// const glm::vec3 munecoWorldPos = glm::vec3(-20.0f, -2.0f, 70.0f);
			const float munecoScale = 48.0f;
			const float munecoRotYdeg = 180.0f;

			// read key states once
			bool* keyState = mainWindow.getsKeys();

			// Movement keys for model (V +x, B -x, N +z, M -z)
			bool vDown = keyState[GLFW_KEY_V];
			bool bDown = keyState[GLFW_KEY_B];
			bool nDown = keyState[GLFW_KEY_N];
			bool mDown = keyState[GLFW_KEY_M];
			glm::vec3 moveDelta(0.0f);
			if (vDown) moveDelta.x += munecoMoveSpeed * deltaTime;
			if (bDown) moveDelta.x -= munecoMoveSpeed * deltaTime;
			if (nDown) moveDelta.z += munecoMoveSpeed * deltaTime;
			if (mDown) moveDelta.z -= munecoMoveSpeed * deltaTime;
			if (moveDelta.x != 0.0f || moveDelta.z != 0.0f)
			{
				munecoPosition += moveDelta;
				printf("Muneco moved to: x=%.2f y=%.2f z=%.2f\n", munecoPosition.x, munecoPosition.y, munecoPosition.z);
			}

			model = glm::mat4(1.0f);
			model = glm::translate(model, munecoPosition); // use global position here
			model = glm::rotate(model, munecoRotYdeg * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(munecoScale, munecoScale, munecoScale));

			// >>> IMPLEMENTACIÓN 3: Configuración del material para que se ilumine <<<
			glUniform1f(uniformSpecularIntensity, 1.0f); // Intensidad alta para que el color se vea
			glUniform1f(uniformShininess, 32.0f);        // Brillo decente
			// >>> FIN IMPLEMENTACIÓN 3 <<<

			munecoTexture.UseTexture();

			// Determine toggle states (rising edge toggles)
			static bool prevZ = false, prevX = false, prevC = false;
			static bool feetEnabled = false, handsEnabled = false, headEnabled = false;
			bool zDown = keyState[GLFW_KEY_Z];
			bool xDown = keyState[GLFW_KEY_X];
			bool cDown = keyState[GLFW_KEY_C];
			if (zDown && !prevZ) feetEnabled = !feetEnabled;
			if (xDown && !prevX) handsEnabled = !handsEnabled;
			if (cDown && !prevC) headEnabled = !headEnabled;
			prevZ = zDown; prevX = xDown; prevC = cDown;

			bool moveFeet = feetEnabled;
			bool moveHands = handsEnabled;
			bool moveHead = headEnabled;

			float headAngle = moveHead ? sinf(now * 2.0f) * 10.0f * toRadians : 0.0f;
			float armAngle = moveHands ? sinf(now * 3.0f) * 25.0f * toRadians : 0.0f;
			float legAngle = moveFeet ? sinf(now * 2.5f + 3.14f) * 30.0f * toRadians : 0.0f;

            size_t meshCount = Muneco_M.GetMeshCount();

            // Render non-animated meshes first
            std::vector<char> animated(meshCount, 0);
            for (auto idx : munecoHeadMeshes) if (idx < meshCount) animated[idx] = 1;
            for (auto idx : munecoHandMeshes) if (idx < meshCount) animated[idx] = 1;
            for (auto idx : munecoFootMeshes) if (idx < meshCount) animated[idx] = 1;

            for (unsigned int i = 0; i < meshCount; ++i)
            {
                if (animated[i]) continue; // skip animated parts here
                glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
                Muneco_M.RenderMesh(i);
            }

			// Render head meshes (if any)
			for (auto idx : munecoHeadMeshes)
			{
				if (idx >= meshCount) continue;
				glm::mat4 meshModel = model;
				meshModel = glm::translate(meshModel, glm::vec3(0.0f, 0.5f, 0.0f));
				meshModel = glm::rotate(meshModel, headAngle, glm::vec3(1.0f, 0.0f, 0.0f));
				meshModel = glm::translate(meshModel, glm::vec3(0.0f, -0.5f, 0.0f));
				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(meshModel));
				Muneco_M.RenderMesh(idx);
			}

			// Render hand meshes
			for (auto idx : munecoHandMeshes)
			{
				if (idx >= meshCount) continue;
				glm::vec3 center = Muneco_M.GetMeshCenter(idx);
				bool left = center.x < 0.0f;
				glm::mat4 meshModel = model;
				meshModel = glm::translate(meshModel, glm::vec3(0.3f * (left ? -1.0f : 1.0f), 0.2f, 0.0f));
				meshModel = glm::rotate(meshModel, (left ? 1.0f : -1.0f) * armAngle, glm::vec3(1.0f, 0.0f, 0.0f));
				meshModel = glm::translate(meshModel, glm::vec3(0.3f * (left ? 1.0f : -1.0f), -0.2f, 0.0f));
				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(meshModel));
				Muneco_M.RenderMesh(idx);
			}

			// Render foot meshes
			for (auto idx : munecoFootMeshes)
			{
				if (idx >= meshCount) continue;
				glm::vec3 center = Muneco_M.GetMeshCenter(idx);
				bool left = center.x < 0.0f;
				glm::mat4 meshModel = model;
				meshModel = glm::translate(meshModel, glm::vec3(0.15f * (left ? -1.0f : 1.0f), -0.5f, 0.0f));
				meshModel = glm::rotate(meshModel, (left ? 1.0f : -1.0f) * legAngle, glm::vec3(1.0f, 0.0f, 0.0f));
				meshModel = glm::translate(meshModel, glm::vec3(0.15f * (left ? 1.0f : -1.0f), 0.5f, 0.0f));
				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(meshModel));
				Muneco_M.RenderMesh(idx);
			}

		}

		glDisable(GL_BLEND);
		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}