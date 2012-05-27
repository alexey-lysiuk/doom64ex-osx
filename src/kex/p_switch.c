// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: p_switch.c 1039 2012-01-23 00:36:40Z svkaiser $
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
//
// $Author: svkaiser $
// $Revision: 1039 $
// $Date: 2012-01-23 02:36:40 +0200 (пн, 23 січ 2012) $
//
//
// DESCRIPTION:
//	Switches, buttons. Two-state animation. Exits.
//
//-----------------------------------------------------------------------------
#ifdef RCSID
static const char
rcsid[] = "$Id: p_switch.c 1039 2012-01-23 00:36:40Z svkaiser $";
#endif


#include "i_system.h"
#include "doomdef.h"
#include "p_local.h"
#include "w_wad.h"
#include "g_game.h"
#include "s_sound.h"
#include "sounds.h"
#include "doomstat.h"
#include "r_texture.h"
#include "r_local.h"
#include "z_zone.h"


button_t buttonlist[MAXBUTTONS];


//
// P_StartButton
// Start a button counting down till it turns off.
//

void P_StartButton(line_t* line, bwhere_e w, int texture, int time)
{
    int	i;
    
    // See if button is already pressed
    for(i = 0; i < MAXBUTTONS; i++)
    {
        if(buttonlist[i].btimer && buttonlist[i].line == line)
            return;
    }
    
    for(i = 0; i < MAXBUTTONS; i++)
    {
        if(!buttonlist[i].btimer)
        {
            buttonlist[i].line = line;
            buttonlist[i].where = w;
            buttonlist[i].btexture = texture;
            buttonlist[i].btimer = time;
            
            if(SWITCHMASK(line->flags))
                buttonlist[i].soundorg = (mobj_t *)&line->frontsector->soundorg;
            
            return;
        }
    }

    I_Error("P_StartButton: no button slots left!");
}

//
// P_ChangeSwitchTexture
// Function that changes wall texture.
// Tell it if switch is ok to use again (1=yes, it's a button).
//

void P_ChangeSwitchTexture(line_t* line, int useAgain)
{
    int sound;
    int swx;
    
    if(SPECIALMASK(line->special) == 52 || SPECIALMASK(line->special) == 124)
        sound = sfx_switch2;
    else
        sound = sfx_switch1;
    
    if(!useAgain)
        line->special = 0;
    
    if(SWITCHMASK(line->flags) == ML_SWITCHX04)
    {
        swx = swx_start + (sides[line->sidenum[0]].bottomtexture - swx_start) ^ 1;
        
        S_StartSound(buttonlist->soundorg, sound);
        sides[line->sidenum[0]].bottomtexture = swx;
        
        if(useAgain)
            P_StartButton(line, bottom, swx, BUTTONTIME);
        
        return;
    }
    else if(SWITCHMASK(line->flags) == ML_SWITCHX02)
    {
        swx = swx_start + (sides[line->sidenum[0]].toptexture - swx_start) ^ 1;
        
        S_StartSound(buttonlist->soundorg, sound);
        sides[line->sidenum[0]].toptexture = swx;
        
        if(useAgain)
            P_StartButton(line, top, swx, BUTTONTIME);
        
        return;
    }
    else if(SWITCHMASK(line->flags) == (ML_SWITCHX02 | ML_SWITCHX04))
    {
        swx = swx_start + (sides[line->sidenum[0]].midtexture - swx_start) ^ 1;
        
        S_StartSound(buttonlist->soundorg, sound);
        sides[line->sidenum[0]].midtexture = swx;
        
        if(useAgain)
            P_StartButton(line, middle, swx, BUTTONTIME);
        
        return;
    }
}


