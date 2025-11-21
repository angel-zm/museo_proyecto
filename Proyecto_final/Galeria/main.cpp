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
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "Window.h"
#include "Mesh.h"
#include "Shader_light.h"
#include "Camera.h"
#include "Texture.h"
#include "Model.h"
#include "Skybox.h"   // <<--- skybox

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

// --- NUEVAS DECLARACIONES DE MODELOS DE LA ESCENA ---
Model Estatua_M;
Model Basura_M;
Model Hotdog_M;
Model Flores_M;
Model Pino_M;

// --- ESTRUCTURA PARA ALMACENAR INSTANCIAS ---
struct InstanceData {
    glm::vec3 position;
    glm::vec3 rotationDeg; // Rotación en grados (Pitch, Yaw, Roll)
    glm::vec3 scale;
};

std::vector<InstanceData> estatuaInstances;
std::vector<InstanceData> basuraInstances;
std::vector<InstanceData> floresInstances;
std::vector<InstanceData> pinoInstances;
InstanceData hotdogInstance; // El carrito es una instancia única

// Partes del muñeco
std::vector<unsigned int> munecoHeadMeshes;
std::vector<unsigned int> munecoHandMeshes;
std::vector<unsigned int> munecoFootMeshes;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

DirectionalLight mainLight;
PointLight pointLights[2];
float lightMoveSpeed = 5.0f; // speed to move lights

static const char* vShader = "shaders/shader_light.vert";
static const char* fShader = "shaders/shader_light.frag";

// Posición global del muñeco
glm::vec3 munecoPosition(0.0f, -2.0f, 70.0f);
float munecoMoveSpeed = 5.0f;
// velocidad de marcha usada por la patrulla (más lenta que el movimiento manual)
float munecoWalkSpeed = 1.0f; // ajustar para 'caminar'

// Skybox global
//Skybox skybox;

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
         0.0f, -1.0f,  1.0f,    0.5f, 0.0f,  0.0f, 0.0f, 0.0f,
         1.0f, -1.0f, -0.6f,    1.0f, 0.0f,  0.0f, 0.0f, 0.0f,
         0.0f,  1.0f,  0.0f,    0.5f, 1.0f,  0.0f, 0.0f, 0.0f
    };

    unsigned int floorIndices[] = {
        0, 2, 1,
        1, 2, 3
    };

    GLfloat floorVertices[] = {
        -10.0f, 0.0f, -10.0f,   0.0f,  0.0f,  0.0f, -1.0f, 0.0f,
         10.0f, 0.0f, -10.0f,  10.0f,  0.0f,  0.0f, -1.0f, 0.0f,
        -10.0f, 0.0f,  10.0f,   0.0f, 10.0f,  0.0f, -1.0f, 0.0f,
         10.0f, 0.0f,  10.0f,  10.0f, 10.0f,  0.0f, -1.0f, 0.0f
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
static std::string toLower(const std::string& s)
{
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(), ::tolower);
    return out;
}

int main()
{
    mainWindow = Window(1366, 768);
    mainWindow.Initialise();

    CreateObjects();
    CreateShaders();

    camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        -60.0f, 0.0f, 0.5f, 0.5f);

    // Luz direccional
    mainLight = DirectionalLight(1.0f, 1.0f, 1.0f,
        0.5f, 0.8f,
        0.0f, -1.0f, 0.0f);

    // Inicializar dos luces puntuales situadas cerca del muñeco, elevadas sobre el piso
    pointLights[0] = PointLight(1.0f, 0.9f, 0.7f,   // color cálido
        0.2f, 1.0f,
        munecoPosition.x + 5.0f, munecoPosition.y + 3.0f, munecoPosition.z + 5.0f,
        1.0f, 0.01f, 0.002f);

    pointLights[1] = PointLight(0.6f, 0.8f, 1.0f,   // color frío
        0.15f, 0.9f,
        munecoPosition.x - 5.0f, munecoPosition.y + 3.0f, munecoPosition.z - 5.0f,
        1.0f, 0.01f, 0.002f);

    // Texturas
    pisoTexture = Texture("Textures/Slate_Floor_Tiles_wfribhq_1K_BaseColor.jpg");
    pisoTexture.LoadTextureA();
    GaleriaTexture = Texture("Textures/181614_Negro.png");
    GaleriaTexture.LoadTextureA();

    // Modelos
    Galeria = Model();
    Galeria.LoadModel("Models/Galeria.fbx");

    Muneco_M = Model();
    Muneco_M.LoadModel("Models/BartSimpson.obj");

    // Clasificación de meshes del muñeco
    {
        size_t mc = Muneco_M.GetMeshCount();
        for (unsigned int i = 0; i < mc; ++i)
        {
            std::string name = toLower(Muneco_M.GetMeshName(i));
            glm::vec3 center = Muneco_M.GetMeshCenter(i);

            if (name.find("head") != std::string::npos ||
                name.find("skull") != std::string::npos ||
                center.y > 0.5f)
            {
                munecoHeadMeshes.push_back(i);
            }
            else if (name.find("hand") != std::string::npos ||
                name.find("arm") != std::string::npos ||
                fabs(center.x) > 0.18f)
            {
                munecoHandMeshes.push_back(i);
            }
            else if (name.find("leg") != std::string::npos ||
                name.find("foot") != std::string::npos ||
                center.y < 0.0f)
            {
                munecoFootMeshes.push_back(i);
            }
        }
    }

    // Textura del muñeco
    Texture munecoTexture = Texture("Textures/bart.jpg");
    munecoTexture.LoadTextureA();

    // -------------------------------------------------------------------
    // --- CARGA DE TEXTURAS Y MODELOS DE ESCENA ---
    // -------------------------------------------------------------------

    // Estatua
    Texture estatuaTexture = Texture("Textures/ESTATUA0.jpg, ESTATUARoughness.jpg, ESTATUARoughness.jpg, ESTATUAMetallic.jpg, ESTATUAMetallic.jpg, ESTATUA2.jpg");
    estatuaTexture.LoadTextureA();
    Estatua_M = Model();
    Estatua_M.LoadModel("Models/ESTATUA.obj");

    // Basura
    Texture basuraTexture = Texture("Textures/BASURA0.jpg");
    basuraTexture.LoadTextureA();
    Basura_M = Model();
    Basura_M.LoadModel("Models/BASURA.obj");

    // Hotdog (Carrito)
    Texture hotdogTexture = Texture("Textures/HOTDOG0.jpg");
    hotdogTexture.LoadTextureA();
    Hotdog_M = Model();
    Hotdog_M.LoadModel("Models/HOTDOG.obj");

    // Flores
    Texture floresTexture = Texture("Textures/FLORES0.jpg");
    floresTexture.LoadTextureA();
    Flores_M = Model();
    Flores_M.LoadModel("Models/FLORES.obj");

    // Pino
    Texture pinoTexture = Texture("Textures/PINO0.jpg");
    pinoTexture.LoadTextureA();
    Pino_M = Model();
    Pino_M.LoadModel("Models/PINO.obj");

    // -------------------------------------------------------------------
    // --- DEFINICIÓN DE INSTANCIAS (POSICIONES) EDITABLES ---
    // -------------------------------------------------------------------

    estatuaInstances.clear();
    basuraInstances.clear();
    floresInstances.clear();
    pinoInstances.clear();

    // Escalas base (puedes cambiarlas también)
    float estatuaScale = 50.0f;
    float basuraScale = 10.0f;
    float floresScale = 20.0f;
    float pinoScale = 50.0f;

    // ===================== ESTATUAS =====================
    // <<< AQUÍ PUEDES CAMBIAR POSICIONES Y ROTACIONES DE LAS ESTATUAS >>>
    // position = (x, y, z), rotationDeg = (pitch, yaw, roll), scale = (sx, sy, sz)

    estatuaInstances.push_back({
        glm::vec3(90.0f, 1.0f,  30.0f),   // POSICIÓN ESTATUA 1
        glm::vec3(0.0f,   -180.0f,  0.0f),   // ROTACIÓN EN GRADOS
        glm::vec3(estatuaScale)           // ESCALA
        });

    estatuaInstances.push_back({
        glm::vec3(90.0f, 1.0f,  -30.0f),   // POSICIÓN ESTATUA 2
        glm::vec3(0.0f,  -180.0f,  0.0f),
        glm::vec3(estatuaScale)
        });

    // ===================== BASURA (OPCIONAL) =====================
    // No lo pediste, pero lo dejo simple también por si quieres moverlos.
    basuraInstances.push_back({
        glm::vec3(-120.0f, 1.0f, -70.0f),
        glm::vec3(0.0f,    0.0f,  0.0f),
        glm::vec3(basuraScale)
        });

    basuraInstances.push_back({
        glm::vec3(120.0f, 1.0f, -70.0f),
        glm::vec3(0.0f,   90.0f,  0.0f),
        glm::vec3(basuraScale)
        });

    // ===================== FLORES =====================
    // <<< CAMBIA ESTAS POSICIONES PARA PONER TUS MACETAS/FLORES DONDE QUIERAS >>>
    floresInstances.push_back({
        glm::vec3(-145.0f, 3.0f,  195.0f),   // FLORES 1
        glm::vec3(0.0f,  0.0f,   0.0f),
        glm::vec3(floresScale)
        });

    floresInstances.push_back({
        glm::vec3(170.0f, 1.0f,  105.0f),   // FLORES 2
        glm::vec3(0.0f, 15.0f,   0.0f),
        glm::vec3(floresScale)
        });

    floresInstances.push_back({
        glm::vec3(-98.0f, 1.0f, 95.0f),   // FLORES 3
        glm::vec3(0.0f, -30.0f, 0.0f),
        glm::vec3(floresScale)
        });

    floresInstances.push_back({
        glm::vec3(118.0f, 1.0f, 79.0f),   // FLORES 4
        glm::vec3(0.0f,  45.0f, 0.0f),
        glm::vec3(floresScale)
        });

    // Si quieres más flores, copia/pega bloques como este:
    /*
    floresInstances.push_back({
        glm::vec3(x, y, z),
        glm::vec3(pitch, yaw, roll),
        glm::vec3(floresScale)
    });
    */

    // ===================== PINOS =====================
    // <<< CAMBIA ESTAS POSICIONES PARA COLOCAR LOS ÁRBOLES >>>
    pinoInstances.push_back({
        glm::vec3(-25.0f, 23.0f, -90.0f), // PINO 1
        glm::vec3(0.0f,    0.0f,   0.0f),
        glm::vec3(pinoScale)
        });

    pinoInstances.push_back({
        glm::vec3(70.0f, 23.0f, -90.0f), // PINO 2
        glm::vec3(0.0f,   0.0f,   0.0f),
        glm::vec3(pinoScale)
        });

    pinoInstances.push_back({
        glm::vec3(-50.0f, 23.0f,  90.0f), // PINO 3
        glm::vec3(0.0f,   0.0f,   0.0f),
        glm::vec3(pinoScale)
        });

    pinoInstances.push_back({
        glm::vec3(90.0f, 23.0f,  90.0f), // PINO 4
        glm::vec3(0.0f,   0.0f,   0.0f),
        glm::vec3(pinoScale)
        });

    // ===================== HOTDOG (CARRO) =====================
    // <<< ESTE ES EL CARRO DE HOTDOG, MUEVELO CAMBIANDO LA POSICIÓN >>>
    hotdogInstance = {
        glm::vec3(0.0f, 10.0f, 70.0f),  // POSICIÓN DEL CARRO0
        glm::vec3(0.0f,  45.0f,  0.0f),  // ROTACIÓN (solo usamos .y aquí)
        glm::vec3(25.0f)                  // ESCALA
    };

    // Skybox
    std::vector<std::string> skyboxFaces = {
        "Skybox/cupertin-lake_rt.tga",
        "Skybox/cupertin-lake_lf.tga",
        "Skybox/cupertin-lake_dn.tga",
        "Skybox/cupertin-lake_up.tga",
        "Skybox/cupertin-lake_bk.tga",
        "Skybox/cupertin-lake_ft.tga"
    };
    Skybox  Skybox(skyboxFaces);

    // Uniforms
    GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
        uniformSpecularIntensity = 0, uniformShininess = 0, uniformTextureOffset = 0;
    GLuint uniformColor = 0;

    glm::mat4 projection = glm::perspective(
        45.0f,
        (GLfloat)mainWindow.getBufferWidth() / (GLfloat)mainWindow.getBufferHeight(),
        0.1f, 1000.0f);

    glm::mat4 model(1.0f);
    glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec2 toffset = glm::vec2(0.0f, 0.0f);

    // -------------------------------------------------------------------
    // --- FUNCIÓN LAMBDA PARA RENDERIZAR INSTANCIAS ---
    // -------------------------------------------------------------------
    auto RenderInstances = [&](
        Model& modelToRender,
        Texture& textureToUse,
        const std::vector<InstanceData>& instances,
        float specIntensity,
        float shininess)
        {
            glUniform1f(uniformSpecularIntensity, specIntensity);
            glUniform1f(uniformShininess, shininess);
            textureToUse.UseTexture();

            for (const auto& instance : instances)
            {
                model = glm::mat4(1.0f);
                // 1. Traslación (Posición)
                model = glm::translate(model, instance.position);
                // 2. Rotación (Y-X-Z)
                model = glm::rotate(model, instance.rotationDeg.y * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::rotate(model, instance.rotationDeg.x * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
                model = glm::rotate(model, instance.rotationDeg.z * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
                // 3. Escala
                model = glm::scale(model, instance.scale);

                glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
                modelToRender.RenderModel();
            }
        };
    // -------------------------------------------------------------------

    // Loop principal
    while (!mainWindow.getShouldClose())
    {
        GLfloat now = glfwGetTime();
        deltaTime = now - lastTime;
        deltaTime += (now - lastTime) / limitFPS;
        lastTime = now;

        glfwPollEvents();
        camera.keyControl(mainWindow.getsKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

        bool* keyState = mainWindow.getsKeys();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- SKYBOX ---
        Skybox.DrawSkybox(camera.calculateViewMatrix(), projection);

        // --- SHADER PRINCIPAL ---
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
        glUniform3f(uniformEyePosition,
            camera.getCameraPosition().x,
            camera.getCameraPosition().y,
            camera.getCameraPosition().z);

        shaderList[0].SetDirectionalLight(&mainLight);
        shaderList[0].SetPointLights(pointLights, 2);

        // --- Piso ---
        model = glm::mat4(1.0f);
        color = glm::vec3(1.0f, 1.0f, 1.0f);
        toffset = glm::vec2(0.0f, 0.0f);

        glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
        model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
        model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        glUniform1f(uniformSpecularIntensity, 0.1f);
        glUniform1f(uniformShininess, 2.0f);
        pisoTexture.UseTexture();
        meshList[2]->RenderMesh();

        // --- Galería ---
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 20.0f, 0.0f));
        model = glm::rotate(model, -90.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1f(uniformSpecularIntensity, 0.5f);
        glUniform1f(uniformShininess, 8.0f);
        GaleriaTexture.UseTexture();
        Galeria.RenderModel();

        // -------------------------------------------------------------------
        // --- RENDERIZADO DE INSTANCIAS DE ESCENA ---
        // -------------------------------------------------------------------

        // 1. Estatuas
        RenderInstances(Estatua_M, estatuaTexture, estatuaInstances, 0.8f, 64.0f);

        // 2. Basura
        RenderInstances(Basura_M, basuraTexture, basuraInstances, 0.1f, 4.0f);

        // 3. Flores
        RenderInstances(Flores_M, floresTexture, floresInstances, 0.05f, 2.0f);

        // 4. Pinos
        RenderInstances(Pino_M, pinoTexture, pinoInstances, 0.1f, 8.0f);

        // 5. Carrito de Hotdog (Instancia única)
        glUniform1f(uniformSpecularIntensity, 0.5f);
        glUniform1f(uniformShininess, 16.0f);
        hotdogTexture.UseTexture();
        model = glm::mat4(1.0f);
        model = glm::translate(model, hotdogInstance.position);
        model = glm::rotate(model, hotdogInstance.rotationDeg.y * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, hotdogInstance.scale);
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Hotdog_M.RenderModel();

        // --- Muñeco (Bart) ---
        {
            const float munecoScale = 8.0f;
            float munecoRotYdeg = 180.0f;

            const bool autoPatrol = true;
            static std::vector<glm::vec3> patrolPoints;
            static size_t patrolIndex = 0;
            static float patrolSide = 20.0f;
            float patrolSpeed = munecoWalkSpeed;

            if (autoPatrol && patrolPoints.empty()) {
                glm::vec3 center = munecoPosition;
                float half = patrolSide * 0.5f;
                patrolPoints.push_back(center + glm::vec3(-half, 0.0f, -half));
                patrolPoints.push_back(center + glm::vec3(half, 0.0f, -half));
                patrolPoints.push_back(center + glm::vec3(half, 0.0f, half));
                patrolPoints.push_back(center + glm::vec3(-half, 0.0f, half));
                patrolIndex = 0;
            }

            glm::vec3 moveDelta(0.0f);
            if (autoPatrol && !patrolPoints.empty()) {
                glm::vec3 target = patrolPoints[patrolIndex];
                target.y = munecoPosition.y;
                glm::vec3 toTarget = target - munecoPosition;
                toTarget.y = 0.0f;
                float dist = glm::length(toTarget);
                if (dist > 0.001f) {
                    glm::vec3 dir = toTarget / dist;
                    float step = patrolSpeed * deltaTime;
                    if (step >= dist) {
                        munecoPosition = target;
                        patrolIndex = (patrolIndex + 1) % patrolPoints.size();
                    }
                    else {
                        munecoPosition += dir * step;
                    }
                    munecoRotYdeg = atan2(dir.x, dir.z) * (180.0f / 3.14159265f);
                }
            }
            else {
                bool vDown = keyState[GLFW_KEY_V];
                bool bDown = keyState[GLFW_KEY_B];
                bool nDown = keyState[GLFW_KEY_N];
                bool mDown = keyState[GLFW_KEY_M];

                if (vDown) moveDelta.x += munecoMoveSpeed * deltaTime;
                if (bDown) moveDelta.x -= munecoMoveSpeed * deltaTime;
                if (nDown) moveDelta.z += munecoMoveSpeed * deltaTime;
                if (mDown) moveDelta.z -= munecoMoveSpeed * deltaTime;

                if (moveDelta.x != 0.0f || moveDelta.z != 0.0f) {
                    munecoPosition += moveDelta;
                    munecoRotYdeg = atan2(moveDelta.x, moveDelta.z) * (180.0f / 3.14159265f);
                }
            }

            static float printTimer = 0.0f;
            printTimer += deltaTime;
            if (printTimer >= 0.5f) {
                printTimer = 0.0f;
                printf("Muneco position: x=%.2f y=%.2f z=%.2f\n",
                    munecoPosition.x, munecoPosition.y, munecoPosition.z);
            }

            model = glm::mat4(1.0f);
            model = glm::translate(model, munecoPosition);
            model = glm::rotate(model, munecoRotYdeg * toRadians,
                glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(munecoScale));

            glUniform1f(uniformSpecularIntensity, 1.0f);
            glUniform1f(uniformShininess, 32.0f);

            munecoTexture.UseTexture();

            static bool prevZ = false, prevX = false, prevC = false;
            static bool feetEnabled = false, handsEnabled = false, headEnabled = false;
            bool zDown = keyState[GLFW_KEY_Z];
            bool xDown = keyState[GLFW_KEY_X];
            bool cDown = keyState[GLFW_KEY_C];
            if (zDown && !prevZ) feetEnabled = !feetEnabled;
            if (xDown && !prevX) handsEnabled = !handsEnabled;
            if (cDown && !prevC) headEnabled = !headEnabled;
            prevZ = zDown; prevX = xDown; prevC = cDown;

            bool moveFeet = feetEnabled || autoPatrol;
            bool moveHands = handsEnabled;
            bool moveHead = headEnabled;

            float headAngle = moveHead ? sinf(now * 2.0f) * 10.0f * toRadians : 0.0f;
            float armAngle = moveHands ? sinf(now * 3.0f) * 25.0f * toRadians : 0.0f;
            float legAngle = moveFeet ? sinf(now * 2.5f + 3.14f) * 30.0f * toRadians : 0.0f;

            size_t meshCount = Muneco_M.GetMeshCount();

            std::vector<char> animated(meshCount, 0);
            for (auto idx : munecoHeadMeshes)  if (idx < meshCount) animated[idx] = 1;
            for (auto idx : munecoHandMeshes)  if (idx < meshCount) animated[idx] = 1;
            for (auto idx : munecoFootMeshes)  if (idx < meshCount) animated[idx] = 1;

            // Mallas no animadas
            for (unsigned int i = 0; i < meshCount; ++i)
            {
                if (animated[i]) continue;
                glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
                Muneco_M.RenderMesh(i);
            }

            // Cabeza
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

            // Manos
            for (auto idx : munecoHandMeshes)
            {
                if (idx >= meshCount) continue;
                glm::vec3 center = Muneco_M.GetMeshCenter(idx);
                bool left = center.x < 0.0f;
                glm::mat4 meshModel = model;
                meshModel = glm::translate(meshModel,
                    glm::vec3(0.3f * (left ? -1.0f : 1.0f), 0.2f, 0.0f));
                meshModel = glm::rotate(meshModel,
                    (left ? 1.0f : -1.0f) * armAngle,
                    glm::vec3(1.0f, 0.0f, 0.0f));
                meshModel = glm::translate(meshModel,
                    glm::vec3(0.3f * (left ? 1.0f : -1.0f), -0.2f, 0.0f));
                glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(meshModel));
                Muneco_M.RenderMesh(idx);
            }

            // Pies
            for (auto idx : munecoFootMeshes)
            {
                if (idx >= meshCount) continue;
                glm::vec3 center = Muneco_M.GetMeshCenter(idx);
                bool left = center.x < 0.0f;
                glm::mat4 meshModel = model;
                meshModel = glm::translate(meshModel,
                    glm::vec3(0.15f * (left ? -1.0f : 1.0f), -0.5f, 0.0f));
                meshModel = glm::rotate(meshModel,
                    (left ? 1.0f : -1.0f) * legAngle,
                    glm::vec3(1.0f, 0.0f, 0.0f));
                meshModel = glm::translate(meshModel,
                    glm::vec3(0.15f * (left ? 1.0f : -1.0f), 0.5f, 0.0f));
                glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(meshModel));
                Muneco_M.RenderMesh(idx);
            }
        }
        // --- FIN DEL CÓDIGO DE BART SIMPSON ---

        glDisable(GL_BLEND);
        glUseProgram(0);

        mainWindow.swapBuffers();
    }

    return 0;
}
