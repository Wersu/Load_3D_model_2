#pragma once
#ifndef BILLBOARD_TECHNIQUE_H
#define	BILLBOARD_TECHNIQUE_H

#include "Mesh.h"
//#include "math3d.h"

class BillboardTechnique //: public Technique
{
public:

    BillboardTechnique();

    virtual bool Init();

    void SetVP(const glm::mat4& VP);
    void SetCameraPosition(const glm::vec3& Pos);
    void SetColorTextureUnit(unsigned int TextureUnit);

private:

    GLuint m_VPLocation;
    GLuint m_cameraPosLocation;
    GLuint m_colorMapLocation;
};

#endif	/* BILLBOARD_TECHNIQUE_H */