#pragma once

#include "../rn.hpp"

namespace rn {

struct DrawArraysIndirectCmd {
   GLuint count = 0;
   GLuint instanceCount = 0;
   GLuint first = 0;
   GLuint baseInstance = 0;
};

struct DrawElementsIndirectCmd {
    GLuint count = 0;
    GLuint instanceCount = 0;
    GLuint firstIndex = 0;
    GLuint baseVertex = 0;
    GLuint baseInstance = 0;
};

struct RasterizerCmd {
  FillMode fillMode = FillMode_NONE;
  CullMode cullMode = CullMode_NONE;
  GLbool frontCounterClockwise = false;
  GLint depthBias = 0;
  GLfloat depthBiasClamp = 0.f;
  GLfloat slopeScaledDepthBias = 0.f;
  GLbool depthClip = false;
  GLbool scissor = false;
  GLbool multisample = false;
  GLbool antialiasedLine = false;
  // UINT                                  ForcedSampleCount;
  // D3D11_CONSERVATIVE_RASTERIZATION_MODE ConservativeRaster;
};

} // rn