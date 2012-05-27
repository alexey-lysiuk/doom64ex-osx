// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: con_cvar.h 1048 2012-02-13 04:08:26Z svkaiser $
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
//---------------------------------------------------------------------

#ifndef CON_CVAR_H
#define CON_CVAR_H

#include "doomtype.h"

typedef struct cvar_s
{
    char*           name;
    char*           string;
    dboolean        nonclient;
    void            (*callback)(void*);
    float           value;
    char*           defvalue;
    struct cvar_s*  next;
} cvar_t;

#define CVAR(name, str)                                     \
    cvar_t name = { # name, # str, 0, NULL }

#define CVAR_CMD(name, str)                                 \
    void CvarCmd_ ## name(cvar_t* cvar);                    \
    cvar_t name = { # name, # str, 0, CvarCmd_ ## name };   \
    void CvarCmd_ ## name(cvar_t* cvar)

#define CVAR_PARAM(name, str, var, flags)                   \
    void CvarCmd_ ## name(cvar_t* cvar);                    \
    cvar_t name = { # name, #str, 0, CvarCmd_ ## name };    \
    void CvarCmd_ ## name(cvar_t* cvar)                     \
    {                                                       \
        if(cvar->value > 0)                                 \
            var |= flags;                                   \
        else                                                \
            var &= ~flags;                                  \
    }

#define NETCVAR(name, str)                                  \
    cvar_t name = { # name, # str, 1, NULL }

#define NETCVAR_CMD(name, str)                              \
    void CvarCmd_ ## name(cvar_t* cvar);                    \
    cvar_t name = { # name, # str, 1, CvarCmd_ ## name };   \
    void CvarCmd_ ## name(cvar_t* cvar)

#define NETCVAR_PARAM(name, str, var, flags)                \
    void CvarCmd_ ## name(cvar_t* cvar);                    \
    cvar_t name = { # name, #str, 0, CvarCmd_ ## name };    \
    void CvarCmd_ ## name(cvar_t* cvar)                     \
    {                                                       \
        if(cvar->value > 0)                                 \
            var |= flags;                                   \
        else                                                \
            var &= ~flags;                                  \
    }

#define CVAR_EXTERNAL(name)                                 \
    extern cvar_t name

extern cvar_t*  cvarcap;

void CON_CvarInit(void);
void CON_CvarRegister(cvar_t *variable);
void CON_CvarSet(char *var_name, char *value);
void CON_CvarSetValue(char *var_name, float value);
void CON_CvarAutoComplete(char *partial);
cvar_t *CON_CvarGet(char *name);

#endif

