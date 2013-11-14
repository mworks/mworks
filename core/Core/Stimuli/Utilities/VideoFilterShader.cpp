/*
* XBMC Media Center
* Video Filter Classes
* Copyright (c) 2007 d4rk
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "glew.h"
//#include "system.h"
#include "VideoFilterShader.h"
//#include "utils/log.h"
#include "ConvolutionKernels.h"

#include "Utilities.h"
#include <string>
#include <math.h>

#define EMBED_SHADER_CODE   0


BEGIN_NAMESPACE(Shaders)


//////////////////////////////////////////////////////////////////////
// BaseVideoFilterShader - base class for video filter shaders
//////////////////////////////////////////////////////////////////////

BaseVideoFilterShader::BaseVideoFilterShader()
{
  m_width = 1;
  m_height = 1;
  m_hStepXY = 0;
  m_stepX = 0;
  m_stepY = 0;
  m_sourceTexUnit = 0;
  m_hSourceTex = 0;

  m_stretch = 0.0f;

  string shaderv = 
    "uniform vec2 stepxy;"
    "void main()"
    "{"
    "gl_TexCoord[0].xy = gl_MultiTexCoord0.xy - stepxy * 0.5;"
    "gl_Position = ftransform();"
    "gl_FrontColor = gl_Color;"
    "}";
  VertexShader()->SetSource(shaderv);
}

ConvolutionFilterShader::ConvolutionFilterShader(ESCALINGMETHOD method, bool stretch)
{
  m_method = method;
  m_kernelTex1 = 0;

  string shader;
  string defines = "";

  m_floattex = glewIsSupported("GL_ARB_texture_float");

  if (m_method == VS_SCALINGMETHOD_CUBIC ||
      m_method == VS_SCALINGMETHOD_LANCZOS2 ||
      m_method == VS_SCALINGMETHOD_LANCZOS3_FAST){
      
//      
//#if  EMBED_SHADER_CODE
//      shader = 
//      "uniform sampler2D  img;\n"
//      "uniform sampler1D  kernelTex;\n"
//      "uniform vec2      stepxy;\n"
//      "uniform float     m_stretch;\n"
//      "\n"
//      "#define XBMC_STRETCH    0\n"
//      "#define HAS_FLOAT_TEXTURE   1\n"
//      "\n"
//      "//nvidia's half is a 16 bit float and can bring some speed improvements\n"
//      "//without affecting quality\n"
//      "#ifndef __GLSL_CG_DATA_TYPES\n"
//      "     #define half float\n"
//      "     #define half3 vec3\n"
//      "     #define half4 vec4\n"
//      "#endif\n"
//      "\n"
//      "#if (HAS_FLOAT_TEXTURE)\n"
//      "half4 weight(float pos)\n"
//      "{\n"
//      "return texture1D(kernelTex, pos);\n"
//      "}\n"
//      "#else\n"
//      "half4 weight(float pos)\n"
//      "{\n"
//      "return texture1D(kernelTex, pos) * 2.0 - 1.0;\n"
//      "}\n"
//      "#endif\n"
//      "\n"
//      "vec2 stretch(vec2 pos)\n"
//      "{\n"
//      "#if (XBMC_STRETCH)\n"
//      "// our transform should map [0..1] to itself, with f(0) = 0, f(1) = 1, f(0.5) = 0.5, and f'(0.5) = b.\n"
//      "// a simple curve to do this is g(x) = b(x-0.5) + (1-b)2^(n-1)(x-0.5)^n + 0.5\n"
//      "// where the power preserves sign. n = 2 is the simplest non-linear case (required when b != 1)\n"
//      "float x = pos.x - 0.5;\n"
//      "return vec2(mix(x * abs(x) * 2.0, x, m_stretch) + 0.5, pos.y);\n"
//      "#else\n"
//      "return pos;\n"
//      "#endif\n"
//      "}\n"
//      "\n"
//      "half3 pixel(float xpos, float ypos)\n"
//      "{\n"
//      "return texture2D(img, vec2(xpos, ypos)).rgb;\n"
//      "}\n"
//      "\n"
//      "half3 line (float ypos, vec4 xpos, half4 linetaps)\n"
//      "{\n"
//      "return\n"
//      "pixel(xpos.r, ypos) * linetaps.r +"
//      "pixel(xpos.g, ypos) * linetaps.g +"
//      "pixel(xpos.b, ypos) * linetaps.b +"
//      "pixel(xpos.a, ypos) * linetaps.a;"
//      "}\n"
//      ""
//      "void main()"
//      "{"
//      "vec2 pos = stretch(gl_TexCoord[0].xy);"
//      "vec2 f = fract(pos / stepxy);"
//      ""
//      "half4 linetaps   = weight(1.0 - f.x);"
//      "half4 columntaps = weight(1.0 - f.y);"
//      ""
//      "//make sure all taps added together is exactly 1.0, otherwise some (very small) distortion can occur\n"
//      "linetaps /= linetaps.r + linetaps.g + linetaps.b + linetaps.a;\n"
//      "columntaps /= columntaps.r + columntaps.g + columntaps.b + columntaps.a;\n"
//      "\n"
//      "vec2 xystart = (-0.5 - f) * stepxy + pos;\n"
//      "vec4 xpos = vec4(xystart.x, xystart.x + stepxy.x, xystart.x + stepxy.x * 2.0, xystart.x + stepxy.x * 3.0);\n"
//      "\n"
//      "gl_FragColor.rgb =\n"
//      "line(xystart.y                 , xpos, linetaps) * columntaps.r +"
//      "line(xystart.y + stepxy.y      , xpos, linetaps) * columntaps.g +"
//      "line(xystart.y + stepxy.y * 2.0, xpos, linetaps) * columntaps.b +"
//      "line(xystart.y + stepxy.y * 3.0, xpos, linetaps) * columntaps.a;"
//      "\n"
//      "gl_FragColor.a = gl_Color.a;\n"
//      "}\n";
//#else
      shader="convolution-4x4.glsl";
//#endif
      
      if (m_floattex){
          m_internalformat = GL_RGB16F_ARB;
      } else { 
          m_internalformat = GL_RGB;
      }
  } else if (m_method == VS_SCALINGMETHOD_LANCZOS3){
      //throw SimpleException("Lanczos3 filter: 6x6 Convolution not currently implemented");
    //shader = "";
    shader = "convolution-6x6.glsl";
    if (m_floattex)
      m_internalformat = GL_RGB16F_ARB;
    else
      m_internalformat = GL_RGB;
  }

  //CLog::Log(LOGDEBUG, "GL: ConvolutionFilterShader: using %s defines: %s", shadername.c_str(), defines.c_str());
  
//#if EMBED_SHADER_CODE
//    PixelShader()->SetSource(shader);
//    std::cerr << shader << std::endl;
//#else
    PixelShader()->LoadSource(shader, defines);
//#endif
}

void ConvolutionFilterShader::OnCompiledAndLinked()
{
  // obtain shader attribute handles on successfull compilation
  m_hSourceTex = glGetUniformLocation(ProgramHandle(), "img");
  m_hStepXY    = glGetUniformLocation(ProgramHandle(), "stepxy");
  m_hKernTex   = glGetUniformLocation(ProgramHandle(), "kernelTex");
  m_hStretch   = glGetUniformLocation(ProgramHandle(), "m_stretch");

  CConvolutionKernel kernel(m_method, 256);

  if (m_kernelTex1)
  {
    glDeleteTextures(1, &m_kernelTex1);
    m_kernelTex1 = 0;
  }

  glGenTextures(1, &m_kernelTex1);

  if ((m_kernelTex1<=0))
  {
    mw::merror(mw::M_DISPLAY_MESSAGE_DOMAIN, "GL: ConvolutionFilterShader: Error creating kernel texture");
    return;
  }

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_1D, m_kernelTex1);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  //if float textures are supported, we can load the kernel as a 1d float texture
  //if not we load it as 8 bit unsigned which gets converted back to float in the shader
  if (m_floattex)
    glTexImage1D(GL_TEXTURE_1D, 0, m_internalformat, kernel.GetSize(), 0, GL_RGBA, GL_FLOAT, kernel.GetFloatPixels());
  else
    glTexImage1D(GL_TEXTURE_1D, 0, m_internalformat, kernel.GetSize(), 0, GL_RGBA, GL_UNSIGNED_BYTE, kernel.GetUint8Pixels());
#if defined(HAS_GL)
#elif HAS_GLES == 2
  //TODO: GL_RGBA16F_ARB for GLES!?
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, 1, 0, GL_RGBA, GL_FLOAT, img);
#endif

  glActiveTexture(GL_TEXTURE0);

  //VerifyGLState();
}

bool ConvolutionFilterShader::OnEnabled()
{
  // set shader attributes once enabled
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_1D, m_kernelTex1);

  glActiveTexture(GL_TEXTURE0);
  glUniform1i(m_hSourceTex, m_sourceTexUnit);
  glUniform1i(m_hKernTex, 2);
  glUniform2f(m_hStepXY, m_stepX, m_stepY);
  glUniform1f(m_hStretch, m_stretch);
  //VerifyGLState();
  return true;
}

void ConvolutionFilterShader::Free()
{
  if (m_kernelTex1){
    glDeleteTextures(1, &m_kernelTex1);
  }
  m_kernelTex1 = 0;
  BaseVideoFilterShader::Free();
}

StretchFilterShader::StretchFilterShader()
{
  PixelShader()->LoadSource("stretch.glsl");
}

void StretchFilterShader::OnCompiledAndLinked()
{
  m_hSourceTex = glGetUniformLocation(ProgramHandle(), "img");
  m_hStretch   = glGetUniformLocation(ProgramHandle(), "m_stretch");
}

bool StretchFilterShader::OnEnabled()
{
  glUniform1i(m_hSourceTex, m_sourceTexUnit);
  glUniform1f(m_hStretch, m_stretch);
  //VerifyGLState();
  return true;
}


END_NAMESPACE(Shaders)
