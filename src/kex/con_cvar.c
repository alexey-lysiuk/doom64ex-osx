// Emacs style mode select	 -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: con_cvar.c 1043 2012-02-03 20:26:29Z svkaiser $
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
// $Revision: 1043 $
// $Date: 2012-02-03 22:26:29 +0200 (пт, 03 лют 2012) $
//
// DESCRIPTION: Console cvar functionality (from Quake)
//
//-----------------------------------------------------------------------------
#ifdef RCSID
static const char rcsid[] = "$Id: con_cvar.c 1043 2012-02-03 20:26:29Z svkaiser $";
#endif

#include "doomstat.h"
#include "v_sdl.h"
#include "con_console.h"
#include "z_zone.h"
#include "i_system.h"
#include "con_cvar.h"
#include "am_map.h"
#include "s_sound.h"
#include "r_main.h"
#include "m_menu.h"
#include "p_setup.h"
#include "st_stuff.h"
#include "g_game.h"

cvar_t  *cvarcap;

//
// CON_CvarGet
//

cvar_t *CON_CvarGet(char *name)
{
    cvar_t	*var;
    
    for(var = cvarcap; var; var = var->next)
        if(!dstrcmp(name, var->name))
            return var;
        
        return NULL;
}

//
// CON_CvarValue
//

float CON_CvarValue(char *name)
{
    cvar_t	*var;
    
    var = CON_CvarGet(name);
    if(!var)
        return 0;
    
    return datof(var->string);
}

//
// CON_CvarString
//

char *CON_CvarString(char *name)
{
    cvar_t *var;
    
    var = CON_CvarGet(name);
    if(!var)
        return "";
    
    return var->string;
}

//
// CON_CvarAutoComplete
//

void CON_CvarAutoComplete(char *partial)
{
    cvar_t*     cvar;
    int         len;
    char*       name = NULL;
    int         spacinglength;
    dboolean    match = false;
    char*       spacing = NULL;
    
    dstrlwr(partial);
    
    len = dstrlen(partial);
    
    if(!len)
        return;
    
    // check functions
    for(cvar = cvarcap; cvar; cvar = cvar->next)
    {
        if(!dstrncmp(partial, cvar->name, len))
        {
            if(!match)
            {
                match = true;
                CON_Printf(0, "\n");
            }

            name = cvar->name;

            // setup spacing
            spacinglength = 24 - dstrlen(cvar->name);
            spacing = Z_Malloc(spacinglength + 1, PU_STATIC, NULL);
            dmemset(spacing, 0x20, spacinglength);
            spacing[spacinglength] = 0;

            // print all matching cvars
            CON_Printf(AQUA, "%s%s= %s (%s)\n", name, spacing, cvar->string, cvar->defvalue);

            Z_Free(spacing);

            CONCLEARINPUT();
            sprintf(ConsoleInputBuff+1, "%s ", name);
            ConsoleInputLen = dstrlen(ConsoleInputBuff);
        }
    }
}

//
// CON_CvarSet
//

void CON_CvarSet(char *var_name, char *value)
{
    cvar_t	*var;
    dboolean changed;
    
    var = CON_CvarGet(var_name);
    if(!var)
    {	// there is an error in C code if this happens
        CON_Printf(WHITE, "CON_CvarSet: variable %s not found\n", var_name);
        return;
    }
    
    changed = dstrcmp(var->string, value);
    
    Z_Free(var->string);	// free the old value string
    
    var->string = Z_Malloc(dstrlen(value)+1, PU_STATIC, 0);
    dstrcpy(var->string, value);
    var->value = datof(var->string);

    if(var->callback)
        var->callback(var);
}

//
// CON_CvarSetValue
//

void CON_CvarSetValue(char *var_name, float value)
{
    char val[32];
    
    sprintf(val, "%f",value);
    CON_CvarSet(var_name, val);
}

//
// CON_CvarRegister
//

void CON_CvarRegister(cvar_t *variable)
{
    char *oldstr;
    
    // first check to see if it has allready been defined
    if(CON_CvarGet(variable->name))
    {
        CON_Printf(WHITE, "CON_CvarRegister: Can't register variable %s, already defined\n", variable->name);
        return;
    }
    
    // copy the value off, because future sets will Z_Free it
    oldstr = variable->string;
    variable->string = Z_Malloc(dstrlen(variable->string)+1, PU_STATIC, 0);	
    dstrcpy(variable->string, oldstr);
    variable->value = datof(variable->string);
    variable->defvalue = Z_Malloc(dstrlen(variable->string)+1, PU_STATIC, 0);
    dstrcpy(variable->defvalue, variable->string);
    
    // link the variable in
    variable->next = cvarcap;
    cvarcap = variable;
}

//
// CON_CvarInit
//

void CON_CvarInit(void)
{
    AM_RegisterCvars();
    R_RegisterCvars();
    V_RegisterCvars();
    ST_RegisterCvars();
    S_RegisterCvars();
    I_RegisterCvars();
    M_RegisterCvars();
    P_RegisterCvars();
    G_RegisterCvars();
}

