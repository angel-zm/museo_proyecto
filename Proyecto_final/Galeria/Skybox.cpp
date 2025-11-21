#include "Skybox.h"
#include "stb_image.h"   // para stbi_load
#include <iostream>

Skybox::Skybox()
{
    // MUY IMPORTANTE: inicializar punteros
    skyMesh = nullptr;
    skyShader = nullptr;
    textureId = 0;
    uniformProjection = 0;
    uniformView = 0;
}

Skybox::Skybox(std::vector<std::string> faceLocations)
{
    // Inicializa primero
    skyMesh = nullptr;
    skyShader = nullptr;
    textureId = 0;
    uniformProjection = 0;
    uniformView = 0;

    // 1) Crear shader del skybox
    skyShader = new Shader();
    skyShader->CreateFromFiles("shaders/skybox.vert", "shaders/skybox.frag");
    uniformProjection = skyShader->GetProjectionLocation();
    uniformView = skyShader->GetViewLocation();

    // 2) Cargar cubemap
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

    int width, height, bitDepth;

    // por si las caras no son exactamente 6, usamos size()
    for (size_t i = 0; i < faceLocations.size(); i++)
    {
        unsigned char* texData = stbi_load(faceLocations[i].c_str(),
            &width, &height, &bitDepth, 0);

        if (!texData)
        {
            std::cout << "No se encontró la textura del skybox: "
                << faceLocations[i] << std::endl;
            continue;  // seguimos con las demás caras
        }

        // ojo: para cubemap normalmente NO se hace flip
        // si las ves invertidas, quita esta línea o ponla antes de cargar todas
        // stbi_set_flip_vertically_on_load(true);

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<GLenum>(i),
            0, GL_RGB, width, height, 0,
            GL_RGB, GL_UNSIGNED_BYTE, texData);

        stbi_image_free(texData);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 3) Crear la malla (cubo) del skybox
    unsigned int skyboxIndices[] = {
        // front
        0, 1, 2,
        2, 1, 3,
        // right
        2, 3, 5,
        5, 3, 7,
        // back
        5, 7, 4,
        4, 7, 6,
        // left
        4, 6, 0,
        0, 6, 1,
        // top
        4, 0, 5,
        5, 0, 2,
        // bottom
        1, 6, 3,
        3, 6, 7
    };

    float skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f,   0.0f, 0.0f,   0.0f, 0.0f, 0.0f, // v0
        -1.0f, -1.0f, -1.0f,   0.0f, 0.0f,   0.0f, 0.0f, 0.0f, // v1
         1.0f,  1.0f, -1.0f,   0.0f, 0.0f,   0.0f, 0.0f, 0.0f, // v2
         1.0f, -1.0f, -1.0f,   0.0f, 0.0f,   0.0f, 0.0f, 0.0f, // v3
        -1.0f,  1.0f,  1.0f,   0.0f, 0.0f,   0.0f, 0.0f, 0.0f, // v4
         1.0f,  1.0f,  1.0f,   0.0f, 0.0f,   0.0f, 0.0f, 0.0f, // v5
        -1.0f, -1.0f,  1.0f,   0.0f, 0.0f,   0.0f, 0.0f, 0.0f, // v6
         1.0f, -1.0f,  1.0f,   0.0f, 0.0f,   0.0f, 0.0f, 0.0f  // v7
    };

    skyMesh = new Mesh();
    skyMesh->CreateMesh(skyboxVertices, skyboxIndices, 64, 36);
}

void Skybox::DrawSkybox(glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
{
    // Protección: si algo falló en el ctor, no dibujes
    if (!skyMesh || !skyShader)
        return;

    // solo rotación de la cámara (el skybox no se traslada)
    viewMatrix = glm::mat4(glm::mat3(viewMatrix));

    glDepthMask(GL_FALSE);
    skyShader->UseShader();

    glUniformMatrix4fv(uniformProjection, 1, GL_FALSE,
        glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(uniformView, 1, GL_FALSE,
        glm::value_ptr(viewMatrix));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

    skyMesh->RenderMesh();

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glDepthMask(GL_TRUE);
}

Skybox::~Skybox()
{
    if (skyMesh)
    {
        skyMesh->ClearMesh();
        delete skyMesh;
        skyMesh = nullptr;
    }

    if (skyShader)
    {
        delete skyShader;
        skyShader = nullptr;
    }
}
