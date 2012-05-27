// Emacs style mode select	 -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: r_gl.c 1101 2012-04-08 19:48:22Z svkaiser $
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Author: svkaiser $
// $Revision: 1101 $
// $Date: 2012-04-08 22:48:22 +0300 (нд, 08 кві 2012) $
//
// DESCRIPTION: OpenGL exclusive functions. All OGL initializations are also handled here
//
//-----------------------------------------------------------------------------
#ifdef RCSID
static const char rcsid[] = "$Id: r_gl.c 1101 2012-04-08 19:48:22Z svkaiser $";
#endif

#include <math.h>

#include "SDL.h"
#include "SDL_opengl.h"

#include "doomdef.h"
#include "doomstat.h"
#include "i_video.h"
#include "r_gl.h"
#include "i_system.h"
#include "z_zone.h"
#include "r_main.h"
#include "r_texture.h"
#include "con_console.h"

int	ViewWindowX = 0;
int	ViewWindowY = 0;
int	ViewWidth	= 0;
int	ViewHeight	= 0;

const char *gl_vendor;
const char *gl_renderer;
const char *gl_version;

static float glScaleFactor = 1.0f;

dboolean    usingGL = false;
int         DGL_CLAMP = GL_CLAMP;
float       max_anisotropic = 0;
dboolean    widescreen = false;

CVAR_EXTERNAL(r_filter);
CVAR_EXTERNAL(r_texturecombiner);
CVAR_EXTERNAL(r_anisotropic);
CVAR_EXTERNAL(st_flashoverlay);

//
// R_GLEnable2D
//

static byte checkortho = 0;

void R_GLEnable2D(dboolean filladjust)
{
    float width;
    float height;

    if(checkortho)
    {
        if(widescreen)
        {
            if(filladjust && checkortho == 2)
                return;
        }
        else
            return;
    }

    dglMatrixMode(GL_MODELVIEW);
    dglLoadIdentity();
    dglMatrixMode(GL_PROJECTION);
    dglLoadIdentity();

    if(widescreen && !filladjust)
    {
        const float ratio = (4.0f / 3.0f);
        float fitwidth = ViewHeight * ratio;
        float fitx = (ViewWidth - fitwidth) / 2.0f;

        dglViewport(ViewWindowX + (int)fitx, ViewWindowY, (int)fitwidth, ViewHeight);
    }

    width = SCREENWIDTH;
    height = SCREENHEIGHT;

    if(glScaleFactor != 1.0f)
    {
        width /= glScaleFactor;
        height /= glScaleFactor;
    }
    
    dglOrtho(0, width, height, 0, -1, 1);

    checkortho = (filladjust && widescreen) ? 2 : 1;
}

//
// R_GLDisable2D
//

void R_GLDisable2D(void)
{
    if(widescreen)
        dglViewport(ViewWindowX, ViewWindowY, ViewWidth, ViewHeight);
}

//
// R_GLSetOrthoScale
//

void R_GLSetOrthoScale(float scale)
{
    glScaleFactor = scale;
    checkortho = 0;
}

//
// R_GLGetOrthoScale
//

float R_GLGetOrthoScale(void)
{
    return glScaleFactor;
}

//
// R_GLFinish
//

void R_GLFinish(void)
{
    dglFinish();
    SDL_GL_SwapBuffers();
}

//
// R_GLGetScreen
//

byte* R_GLGetScreen(int x, int y, int width, int height)
{
    byte* buffer;
    byte* data;
    int i;
    int pack;
    int col;

    col     = (width * 3);
    data    = (byte*)Z_Calloc(height * width * 3, PU_STATIC, 0);
    buffer  = (byte*)Z_Calloc(col, PU_STATIC, 0);

    //
    // 20120313 villsa - force pack alignment to 1
    //
    dglGetIntegerv(GL_PACK_ALIGNMENT, &pack);
    dglPixelStorei(GL_PACK_ALIGNMENT, 1);
    dglFlush();
    dglReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
    dglPixelStorei(GL_PACK_ALIGNMENT, pack);

    //
    // Need to vertically flip the image
    // 20120313 villsa - better method to flip image. uses one buffer instead of two
    //
    for(i = 0; i < height / 2; i++)
    {
        dmemcpy(buffer, &data[i * col], col);
        dmemcpy(&data[i * col], &data[(height - (i + 1)) * col], col);
        dmemcpy(&data[(height - (i + 1)) * col], buffer, col);
    }
    
    Z_Free(buffer);

    return data;
}

//
// R_GLSetFilter
//

void R_GLSetFilter(void)
{
    if(!usingGL)
        return;

    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)r_filter.value == 0 ? GL_LINEAR : GL_NEAREST);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)r_filter.value == 0 ? GL_LINEAR : GL_NEAREST);

    if(has_GL_EXT_texture_filter_anisotropic)
    {
        if(r_anisotropic.value)
            dglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_anisotropic);
        else
            dglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 0);
    }
}

//
// R_GLResetCombiners
//

void R_GLResetCombiners(void)
{
    if(has_GL_ARB_multitexture)
    {
        R_SetTextureUnit(1, false);
        R_SetTextureUnit(2, false);
        R_SetTextureUnit(3, false);
        R_SetTextureUnit(0, true);
    }

    R_GLCheckFillMode();

    if(r_texturecombiner.value > 0)
        dglTexCombModulate(GL_TEXTURE0_ARB, GL_PRIMARY_COLOR);
    else
        R_SetTextureMode(GL_MODULATE);
}

//
// R_GLSetupVertex
//

void R_GLSetupVertex(vtx_t *v, float x, float y, int width, int height,
                     float u1, float u2, float v1, float v2, rcolor c)
{
    float left, right, top, bottom;
    
    left = ViewWindowX + x * ViewWidth / video_width;
    right = left + (width * ViewWidth / video_width);
    top = ViewWindowY + y * ViewHeight / video_height;
    bottom = top + (height * ViewHeight / video_height);
    
    v[0].x = v[2].x = left;
    v[1].x = v[3].x = right;
    v[0].y = v[1].y = top;
    v[2].y = v[3].y = bottom;

    v[0].z = v[1].z = v[2].z = v[3].z = 0.0f;
    
    v[0].tu = u1;
    v[2].tu = u1;
    v[1].tu = u2;
    v[3].tu = u2;
    v[0].tv = v1;
    v[1].tv = v1;
    v[2].tv = v2;
    v[3].tv = v2;
    
    dglSetVertexColor(v, c, 4);
}

//
// R_GLRenderVertex
//

void R_GLRenderVertex(vtx_t *v, dboolean filladjust)
{
    R_GLEnable2D(filladjust);

    dglSetVertex(v);
    dglTriangle(0, 1, 2);
    dglTriangle(3, 2, 1);
    dglDrawGeometry(4, v);
    
    R_GLDisable2D();
    
    if(devparm) vertCount += 4;
}

//
// R_GLDraw2DStrip
//

void R_GLDraw2DStrip(float x, float y, int width, int height,
                     float u1, float u2, float v1, float v2, rcolor c, dboolean filladjust)
{
    vtx_t v[4];
    
    R_GLSetupVertex(v, x, y, width, height, u1, u2, v1, v2, c);
    R_GLRenderVertex(v, filladjust);
};

//
// R_GLToggleBlend
//
// Enables alpha blending without any alpha testing
// Mostly used for projected 2D primitives
//

static dboolean rBlended = false;
void R_GLToggleBlend(dboolean enable)
{
    if(enable && rBlended == false)
    {
        dglEnable(GL_BLEND);
        rBlended = true;
    }
    else if(!enable && rBlended == true)
    {
        dglDisable(GL_BLEND);
        rBlended = false;
    }
}

//
// R_GLCheckFillMode
//

static dboolean rFillEnabled = false;
void R_GLCheckFillMode(void)
{
    if(r_fillmode.value <= 0 && rFillEnabled == true)
    {
        dglDisable(GL_TEXTURE_2D);
        rFillEnabled = false;
    }
    else if(r_fillmode.value > 0 && rFillEnabled == false)
    {
        dglEnable(GL_TEXTURE_2D);
        rFillEnabled = true;
    }
}

//
// R_GLEnableCulling
//

static dboolean rCullEnabled = true;
void R_GLEnableCulling(dboolean enable)
{
    if(enable && rCullEnabled == false)
    {
        dglEnable(GL_CULL_FACE);
        rCullEnabled = true;
    }
    else if(!enable && rCullEnabled == true)
    {
        dglDisable(GL_CULL_FACE);
        rCullEnabled = false;
    }
}

//
// R_GLClearFrame
//

void R_GLClearFrame(rcolor clearcolor)
{
    float f[4];

    dglGetColorf(clearcolor, f);
    dglClearColor(f[0], f[1], f[2], f[3]);
    dglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    dglViewport(ViewWindowX, ViewWindowY, ViewWidth, ViewHeight);
    dglScissor(ViewWindowX, ViewWindowY, ViewWidth, ViewHeight);
}

//
// R_GLGet
//

dboolean R_GLGet(int x)
{
    byte b;
    dglGetBooleanv(x, &b);
    
    return (dboolean)b;
}

//
// R_CalcViewSize
//

void R_CalcViewSize(void)
{
    ViewWidth = video_width;
    ViewHeight = video_height;

    widescreen = !dfcmp(((float)ViewWidth / (float)ViewHeight), (4.0f / 3.0f));

    ViewWindowX = (video_width - ViewWidth) / 2;

    if(ViewWidth == video_width)
        ViewWindowY = 0;
    else
        ViewWindowY = (ViewHeight) / 2;
}

//
// R_GLGetVersionInt
// Borrowed from prboom+
//

typedef enum
{
    OPENGL_VERSION_1_0,
    OPENGL_VERSION_1_1,
    OPENGL_VERSION_1_2,
    OPENGL_VERSION_1_3,
    OPENGL_VERSION_1_4,
    OPENGL_VERSION_1_5,
    OPENGL_VERSION_2_0,
    OPENGL_VERSION_2_1,
} glversion_t;

static int R_GLGetVersionInt(const char* version)
{
    int MajorVersion;
    int MinorVersion;
    int versionvar;

    versionvar = OPENGL_VERSION_1_0;

    if(sscanf(version, "%d.%d", &MajorVersion, &MinorVersion) == 2)
    {
        if(MajorVersion > 1)
        {
            versionvar = OPENGL_VERSION_2_0;

            if(MinorVersion > 0)
                versionvar = OPENGL_VERSION_2_1;
        }
        else
        {
            versionvar = OPENGL_VERSION_1_0;

            if(MinorVersion > 0) versionvar = OPENGL_VERSION_1_1;
            if(MinorVersion > 1) versionvar = OPENGL_VERSION_1_2;
            if(MinorVersion > 2) versionvar = OPENGL_VERSION_1_3;
            if(MinorVersion > 3) versionvar = OPENGL_VERSION_1_4;
            if(MinorVersion > 4) versionvar = OPENGL_VERSION_1_5;
        }
    }

    return versionvar;
}

//
// R_GLInitialize
//

void R_GLInitialize(void)
{
    int temp;

    gl_vendor = dglGetString(GL_VENDOR);
    I_Printf("GL_VENDOR: %s\n", gl_vendor);
    gl_renderer = dglGetString(GL_RENDERER);
    I_Printf("GL_RENDERER: %s\n", gl_renderer);
    gl_version = dglGetString(GL_VERSION);
    I_Printf("GL_VERSION: %s\n", gl_version);
    dglGetIntegerv(GL_MAX_TEXTURE_SIZE, &temp);
    I_Printf("GL_MAX_TEXTURE_SIZE: %i\n", temp);
    dglGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &temp);
    I_Printf("GL_MAX_TEXTURE_UNITS_ARB: %i\n", temp);

    if(temp <= 2)
    {
        CON_Warnf("Not enough texture units supported...\n");
        CON_Warnf("Setting st_flashoverlay to 1\n");
        CON_CvarSetValue(st_flashoverlay.name, 1.0f);
    }

    R_CalcViewSize();
    
    dglViewport(0, 0, video_width, video_height);
    dglClearDepth(1.0f);
    dglDisable(GL_TEXTURE_2D);
    dglEnable(GL_CULL_FACE);
    dglCullFace(GL_FRONT);
    dglShadeModel(GL_SMOOTH);
    dglHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    dglDepthFunc(GL_LEQUAL);
    dglAlphaFunc(GL_GEQUAL, ALPHACLEARGLOBAL);
    dglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    dglFogi(GL_FOG_MODE, GL_LINEAR);
    dglHint(GL_FOG_HINT, GL_NICEST);
    dglEnable(GL_SCISSOR_TEST);
    dglEnable(GL_DITHER);
    
    R_GLSetFilter();
    R_GLInitExtensions();
    R_GLResetCombiners();

    if(!has_GL_ARB_multitexture)
        CON_Warnf("Warning: GL_ARB_multitexture not supported...\n");

    if(!(has_GL_ARB_texture_env_combine | has_GL_EXT_texture_env_combine))
    {
        CON_Warnf("Warning: Texture combiners not supported...\n");
        CON_Warnf("Setting r_texturecombiner to 0\n");
        CON_CvarSetValue(r_texturecombiner.name, 0.0f);
    }

    dglEnableClientState(GL_VERTEX_ARRAY);
    dglEnableClientState(GL_TEXTURE_COORD_ARRAY);
    dglEnableClientState(GL_COLOR_ARRAY);

    DGL_CLAMP = (R_GLGetVersionInt(gl_version) >= OPENGL_VERSION_1_2 ? GL_CLAMP_TO_EDGE : GL_CLAMP);
    
    glScaleFactor = 1.0f;

    if(has_GL_EXT_texture_filter_anisotropic)
        dglGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_anisotropic);
}

