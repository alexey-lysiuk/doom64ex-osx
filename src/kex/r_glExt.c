// Emacs style mode select	 -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: r_glExt.c 1077 2012-03-05 18:26:15Z svkaiser $
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
// $Revision: 1077 $
// $Date: 2012-03-05 20:26:15 +0200 (пн, 05 бер 2012) $
//
// DESCRIPTION: OpenGL extensions
//
//-----------------------------------------------------------------------------
#ifdef RCSID
static const char rcsid[] = "$Id: r_glExt.c 1077 2012-03-05 18:26:15Z svkaiser $";
#endif

#include "SDL.h"
#include "SDL_opengl.h"

#include "doomdef.h"
#include "doomstat.h"
#include "r_gl.h"
#include "i_system.h"
#include "r_main.h"
#include "con_console.h"

// ======================== OGL Extensions ===================================

GL_ARB_multitexture_Define();
GL_EXT_compiled_vertex_array_Define();
//GL_EXT_multi_draw_arrays_Define();
//GL_EXT_fog_coord_Define();
//GL_ARB_vertex_buffer_object_Define();
GL_ARB_texture_non_power_of_two_Define();
GL_ARB_texture_env_combine_Define();
GL_EXT_texture_env_combine_Define();
GL_EXT_texture_filter_anisotropic_Define();

//
// R_GLCheckExtension
//

dboolean R_GLCheckExtension(const char *ext)
{
    if(R_GLCheckExt(ext))
    {
        CON_Printf(WHITE, "GL Extension: %s = true\n", ext);
        return true;
    }
    else
        CON_Printf(YELLOW, "GL Extension: %s = false\n", ext);
    
    return false;
}

//
// R_GLRegisterProc
//

void* R_GLRegisterProc(const char *address)
{
    void *proc = SDL_GL_GetProcAddress(address);
    
    if(!proc)
    {
        CON_Warnf("R_GLRegisterProc: Failed to get proc address: %s", address);
        return NULL;
    }
    
    return proc;
}

//
// R_GLInitExtensions
//

void R_GLInitExtensions(void)
{
    GL_ARB_multitexture_Init();
    GL_EXT_compiled_vertex_array_Init();
    //GL_EXT_multi_draw_arrays_Init();
    //GL_EXT_fog_coord_Init();
    //GL_ARB_vertex_buffer_object_Init();
    GL_ARB_texture_non_power_of_two_Init();
    GL_ARB_texture_env_combine_Init();
    GL_EXT_texture_env_combine_Init();
    GL_EXT_texture_filter_anisotropic_Init();
}

//
// R_GLCheckExt
//

dboolean R_GLCheckExt(const char *ext)
{
    const byte *extensions = NULL;
    const byte *start;
    byte *where, *terminator;
    
    // Extension names should not have spaces.
    where = (byte *) strchr(ext, ' ');
    if (where || *ext == '\0')
        return 0;
    
    extensions = dglGetString(GL_EXTENSIONS);
    
    start = extensions;
    for(;;)
    {
        where = (byte *)strstr((const char *) start, ext);
        if(!where)
            break;
        terminator = where + dstrlen(ext);
        if(where == start || *(where - 1) == ' ')
        {
            if(*terminator == ' ' || *terminator == '\0')
                return true;
            start = terminator;
        }
    }
    return false;
}
