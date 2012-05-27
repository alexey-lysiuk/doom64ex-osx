// Emacs style mode select	 -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: g_settings.c 1031 2012-01-15 04:55:14Z svkaiser $
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
// $Log: g_game.c,v $
// Revision 1.1  2008/05/18 22:28:33  svkaiser
//
//
// DESCRIPTION: Doom3D's config file parsing
//
//-----------------------------------------------------------------------------

#ifdef RCSID
static const char
rcsid[] = "$Id: g_settings.c 1031 2012-01-15 04:55:14Z svkaiser $";
#endif

#ifndef _WIN32
// 20120105 bkw: G_GetConfigFileName() needs these, better safe than sorry
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "g_local.h"
#include "z_zone.h"
#include "m_misc.h"
#include "con_console.h"
#include "i_system.h"

static char *ConfigFileName =
#ifdef _WIN32
"config.cfg"
#else
NULL
#endif
;

char	DefaultConfig[] =
#include "defconfig.inc"	// wtf?
;

//
// G_ExecuteMultipleCommands
//

char *G_GetConfigFileName(void) {
#ifdef _WIN32
    return ConfigFileName;
#else
    // 20120105 bkw: Be UNIX-friendly and use ~/.doom64ex/config.cfg
    if(ConfigFileName == NULL) {
        char confdir[PATH_MAX];
        static char conffile[PATH_MAX];
        
        char *homedir = getenv("HOME");
        
        if(!homedir) homedir = "."; // Fall back to ./.doom64ex if HOME not set
        
        // make sure the directory exists
        sprintf(confdir, "%s/.doom64ex", homedir);
        (void)mkdir(confdir, 0755); // ignore return value
        
        sprintf(conffile, "%s/config.cfg", confdir);
        ConfigFileName = conffile;
        I_Printf("G_GetConfigFileName: using config file '%s'\n", ConfigFileName);
    }
    
    return ConfigFileName;
#endif
}

void G_ExecuteMultipleCommands(char *data)
{
    char	*p;
    char	*q;
    char	c;
    char	line[1024];
    
    p=data;
    c=*p;
    while (c)
    {
        q=line;
        c=*(p++);
        while (c&&(c!='\n'))
        {
            if (c!='\r')
                *(q++)=c;
            c=*(p++);
        }
        *q=0;
        if (line[0])
            G_ExecuteCommand(line);
    }
}

//
// G_ExecuteFile
//

void G_ExecuteFile(char *name)
{
    FILE	*fh;
    char	*buff;
    int		len;
    
    if(!name)
        I_Error("G_ExecuteFile: No config name specified");
    
    fh = fopen(name, "rb");
    
    if(!fh)
    {
        fh = fopen(name, "w");
        if(!fh)
            I_Error("G_ExecuteFile: Unable to create %s", name);
        
        fprintf(fh, "%s", DefaultConfig);
        fclose(fh);
        
        fh = fopen(name, "rb");
        
        if(!fh)
            I_Error("G_ExecuteFile: Failed to read %s", name);
    }
    
    fseek(fh, 0, SEEK_END);
    len = ftell(fh);
    fseek(fh, 0, SEEK_SET);
    buff = Z_Malloc(len + 1, PU_STATIC, NULL);
    fread(buff, 1, len, fh);
    buff[len] = 0;
    G_ExecuteMultipleCommands(buff);
    Z_Free(buff);
}

//
// G_LoadSettings
//

void G_LoadSettings(void)
{
    int		p;
    
    p = M_CheckParm("-config");
    if(p && (p < myargc - 1))
    {
        if(myargv[p + 1][0] != '-')
            ConfigFileName = myargv[p + 1];
    }
    
    G_ExecuteFile(G_GetConfigFileName());
}
