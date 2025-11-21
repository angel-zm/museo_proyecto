#include "SpotLight.h"
#include <glew.h>  // por si usas glUniform, etc.

// Si ya tienes otros constructores en SpotLight.cpp, déjalos.
// Lo importante es que exista esta función con la misma firma que en SpotLight.h

void SpotLight::UseLight(unsigned int ambientIntensityLocation,
    unsigned int ambientColourLocation,
    unsigned int diffuseIntensityLocation,
    unsigned int positionLocation,
    unsigned int directionLocation,
    unsigned int constantLocation,
    unsigned int linearLocation,
    unsigned int exponentLocation,
    unsigned int edgeLocation)
{
    // Implementación mínima (no hace nada).
    // Si quieres apagar totalmente el spotlight, podrías hacer:
    /*
    glUniform3f(ambientColourLocation, 0.0f, 0.0f, 0.0f);
    glUniform1f(ambientIntensityLocation, 0.0f);
    glUniform1f(diffuseIntensityLocation, 0.0f);
    glUniform1f(constantLocation, 1.0f);
    glUniform1f(linearLocation,   0.0f);
    glUniform1f(exponentLocation, 0.0f);
    glUniform1f(edgeLocation,     0.0f);
    */
}
