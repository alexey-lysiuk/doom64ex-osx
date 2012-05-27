// Emacs style mode select	 -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: d_devstat.c 1092 2012-03-20 06:20:48Z svkaiser $
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
// $Revision: 1092 $
// $Date: 2012-03-20 08:20:48 +0200 (вт, 20 бер 2012) $
//
// DESCRIPTION: Developer stuff
//
//-----------------------------------------------------------------------------
#ifdef RCSID
static const char rcsid[] = "$Id: d_devstat.c 1092 2012-03-20 06:20:48Z svkaiser $";
#endif

#include <stdlib.h>

#include "doomdef.h"
#include "doomstat.h"
#include "p_local.h"
#include "i_system.h"
#include "r_local.h"
#include "z_zone.h"
#include "m_misc.h"
#include "s_sound.h"
#include "d_englsh.h"

static dboolean showstats = true;

extern word statindice;

CVAR_EXTERNAL(v_mlook);
CVAR_EXTERNAL(v_mlookinvert);
CVAR_EXTERNAL(sv_lockmonsters);

//
// ST_DrawFPS
//

void ST_DrawFPS(int offset)
{
	static int	frames;
    static int	lasttick=0;
    static int	fps;
    int		ticks;
    int		n;
	
	ticks = I_GetTime();
    if(!lasttick)
    {
		lasttick = ticks;
		frames = fps = 0;
    }

    frames++;

    if(ticks - lasttick >= TICRATE)
    {
		lasttick = ticks;
		fps = frames;
		frames = 0;
		if(fps > 99)
			fps = 99;
    }
    n = fps;
	M_DrawText(0, offset, WHITE, 0.35f, false, "FPS: %i", n);
}

//
// D_DeveloperDisplay
//

void D_DeveloperDisplay(void)
{
	rcolor	sevclr = WHITE;
	int		p_nummobjthinkers = 0;
	fixed_t px, py, pz, pa, pp;
	int		y = 8;
	mobj_t* mo;

    if(!showstats)
    {
        glBindCalls = 0;
        vertCount = 0;
        statindice = 0;

        return;
    }

	/*PLAYER INFORMATION*/

	px=py=pz=pa=pp=0;
	if(players[0].cameratarget && gamestate == GS_LEVEL)
	{
		px=players[0].cameratarget->x/FRACUNIT;
		py=players[0].cameratarget->y/FRACUNIT;
		pz=players[0].cameratarget->z/FRACUNIT;
		pa=players[0].cameratarget->angle>>24;
		pp=players[0].cameratarget->pitch>>24;

		M_DrawText(0, y, WHITE, 0.35f, false, "x: %i, y: %i, z: %i, a: %i, p: %i", px, py, pz, pa, pp);
		y+=16;
	}


	/*Z_MALLOC INFORMATION*/

    M_DrawText(0, y, WHITE, 0.35f, false, "Zone PU_STATIC Usage: %6d kb", Z_TagUsage(PU_STATIC) >> 10);
    y+=16;

    M_DrawText(0, y, WHITE, 0.35f, false, "Zone PU_LEVEL Usage: %7d kb", Z_TagUsage(PU_LEVEL) >> 10);
    y+=16;

    M_DrawText(0, y, WHITE, 0.35f, false, "Zone PU_CACHE Usage: %7d kb", Z_TagUsage(PU_CACHE) >> 10);
    y+=16;

    M_DrawText(0, y, WHITE, 0.35f, false, "Zone PU_LEVSPEC Usage: %5d kb", Z_TagUsage(PU_LEVSPEC) >> 10);
    y+=16;

    M_DrawText(0, y, WHITE, 0.35f, false, "Zone PU_AUTO Usage: %8d kb", Z_TagUsage(PU_AUTO) >> 10);
    y+=16;

	if(gamestate == GS_LEVEL)
	{
		ST_DrawFPS(y);
		y+=16;
	}


	/*MOBJ INFORMATION*/

	if(gamestate == GS_LEVEL)
	{
		for(mo = mobjhead.next; mo != &mobjhead; mo = mo->next)
		{
			if(!mo->player)
				p_nummobjthinkers++;
		}

		M_DrawText(0, y, WHITE, 0.35f, false, "P_Mobj Total Things: %i", p_nummobjthinkers);
		y+=16;
	}

	/*RENDERING INFORMATION*/

    sevclr = vertCount >= 4000 ? YELLOW : WHITE;
    M_DrawText(0, y, sevclr, 0.35f, false, "Total Vertices: %i", vertCount);
    y+=16;

    sevclr = (vertCount/2) >= 2000 ? YELLOW : WHITE;
    M_DrawText(0, y, sevclr, 0.35f, false, "Total Tris: %i", vertCount/2);
    y+=16;

    sevclr = glBindCalls >= 100 ? YELLOW : WHITE;
    M_DrawText(0, y, sevclr, 0.35f, false, "Texture Bind Calls: %i", glBindCalls);
    y+=16;

    M_DrawText(0, y, WHITE, 0.35f, false, "Draw Indices: %i", statindice);
    y+=16;

	if(gamestate == GS_LEVEL && !automapactive)
	{
		M_DrawText(0, y, WHITE, 0.35f, false, "PlayerView Render Time: %ims", renderTic);
		y+=16;

		M_DrawText(0, y, WHITE, 0.35f, false, "Sprite Render Time: %ims", spriteRenderTic);
		y+=16;

		M_DrawText(0, y, WHITE, 0.35f, false, "Draw List Size: %fkb", (float)drawListSize / 1000000.0);
		y+=16;
	}

    M_DrawText(0, y, WHITE, 0.35f, false, "Active Sounds: %i", S_GetActiveSounds());

#ifdef INSTRUMENTED
	Z_PrintStats();
#endif

	glBindCalls = 0;
    vertCount = 0;
    statindice = 0;
}

//
// D_BoyISuck
//

void D_BoyISuck(void)
{
	mobj_t* mo;

	for(mo = mobjhead.next; mo != &mobjhead; mo = mo->next)
    {
		if(mo->player)
			continue;

		if(mo->health > 0 && mo->flags & MF_COUNTKILL)
		{
			P_DamageMobj (mo, NULL, NULL, 99999);
			continue;
		}
    }
}

//
// D_DevKeyResponder
// Special debug actions when pressing the F## keys
//

static dboolean freelook = false;
dboolean D_DevKeyResponder(event_t* ev)
{
    if(gamestate != GS_LEVEL)
        return false;

    if(ev->type == ev_keydown)
    {
        switch(ev->data1)
        {
        case KEY_F1:    // toggle demo mode - take damage but never die
            players[consoleplayer].cheats |= CF_UNDYING;
            players[consoleplayer].message = "Demo Mode On";
            players[consoleplayer].health = 100;
            players[consoleplayer].mo->health = 100;
            return true;

        case KEY_F2:    // toggle spectator mode
            if(!(players[consoleplayer].cheats & CF_SPECTATOR))
            {
                players[consoleplayer].cheats |= CF_SPECTATOR;
                players[consoleplayer].message = "Spectator Mode On";
                      
                freelook = (int)v_mlook.value;
                CON_CvarSetValue(v_mlook.name, 1);
                      
                players[consoleplayer].mo->flags |= MF_FLOAT;
                players[consoleplayer].mo->flags |= MF_NOCLIP;
                players[consoleplayer].mo->flags &= ~MF_GRAVITY;
            }
            else
            {
                players[consoleplayer].cheats &= ~CF_SPECTATOR;
                players[consoleplayer].message = "Spectator Mode Off";
                      
                CON_CvarSetValue(v_mlook.name, (float)freelook);
                freelook = false;
                      
                players[consoleplayer].mo->flags &= ~MF_FLOAT;
                players[consoleplayer].mo->flags &= ~MF_NOCLIP;
                players[consoleplayer].mo->flags |= MF_GRAVITY;
            }
            return true;

        case KEY_F3:    // freeze all mobj thinkers
            CON_CvarSetValue(sv_lockmonsters.name, (float)((int)sv_lockmonsters.value ^ 1));
            players[consoleplayer].message = sv_lockmonsters.value?"Lock Monsters On":"Lock Monsters Off";
            return true;

        case KEY_F4:    // kill everything
            D_BoyISuck();
            players[consoleplayer].message = DEVKILLALL;
            return true;

        case KEY_F7:    // toggle wireframe
            players[consoleplayer].message = r_fillmode.value?"Wireframe Mode On":"Wireframe Mode Off";
            R_DrawWireframe(r_fillmode.value?1:0);
            return true;

        case KEY_F8:    // toggle fullbright
            nolights ^= 1;
            players[consoleplayer].message = nolights?"Lights Disabled":"Lights Enabled";
            return true;

        case KEY_F9:    // toggle display stats
            showstats ^= 1;
            players[consoleplayer].message = showstats?"Display Stats On":"Display Stats Off";
            return true;
        }
    }

    return false;
}


