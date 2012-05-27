// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: wi_stuff.c 1036 2012-01-22 21:34:26Z svkaiser $
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
// $Revision: 1036 $
// $Date: 2012-01-22 23:34:26 +0200 (нд, 22 січ 2012) $
//
//
// DESCRIPTION:
//      Intermission screens.
//
//-----------------------------------------------------------------------------
#ifdef RCSID
static const char
rcsid[] = "$Id: wi_stuff.c 1036 2012-01-22 21:34:26Z svkaiser $";
#endif

#include <stdio.h>

#include "z_zone.h"
#include "m_misc.h"
#include "g_game.h"
#include "s_sound.h"
#include "doomstat.h"
#include "sounds.h"
#include "wi_stuff.h"
#include "d_englsh.h"
#include "m_password.h"
#include "p_setup.h"
#include "st_stuff.h"
#include "r_wipe.h"

#define WIALPHARED      D_RGBA(0xC0, 0, 0, 0xFF)

static int itempercent[MAXPLAYERS];
static int itemvalue[MAXPLAYERS];

static int killpercent[MAXPLAYERS];
static int killvalue[MAXPLAYERS];

static int secretpercent[MAXPLAYERS];
static int secretvalue[MAXPLAYERS];

static char timevalue[16];

static int wi_stage = 0;
static int wi_counter = 0;
static int wi_advance = 0;

//
// WI_Start
//

void WI_Start(void)
{
    int i;
    int minutes = 0;
    int seconds = 0;

    // initialize player stats
    for(i = 0; i < MAXPLAYERS; i++)
    {
        itemvalue[i] = killvalue[i] = secretvalue[i] = -1;

        if(!totalkills)
            killpercent[i] = 100;
        else
            killpercent[i] = (players[i].killcount * 100) / totalkills;

        if(!totalitems)
            itempercent[i] = 100;
        else
            itempercent[i] = (players[i].itemcount * 100) / totalitems;

        if(!totalsecret)
            secretpercent[i] = 100;
        else
            secretpercent[i] = (players[i].secretcount * 100) / totalsecret;
    }

    // setup level time
    if(leveltime)
    {
        minutes = (leveltime / TICRATE) / 60;
        seconds = (leveltime / TICRATE) % 60;
    }

    dsnprintf(timevalue, 16, "%2.2d:%2.2d", minutes, seconds);

    // generate password
    if(nextmap < 34)
        M_EncodePassword();

    // clear variables
    wi_counter = 0;
    wi_stage = 0;
    wi_advance = 0;

    // start music
    S_StartMusic(mus_complete);

    allowmenu = true;
}

//
// WI_Stop
//

void WI_Stop(void)
{
    wi_counter = 0;
    wi_stage = 0;
    wi_advance = 0;

    allowmenu = false;

    S_StopMusic();
    WIPE_FadeScreen(6);
}

//
// WI_Ticker
//

int WI_Ticker(void)
{
    dboolean    state = false;
    player_t*   player;
    int         i;
    dboolean    next = false;

    if(wi_advance <= 3)
    {
        // check for button presses to skip delays
        for(i = 0, player = players; i < MAXPLAYERS; i++, player++)
        {
            if(playeringame[i])
            {
                if(player->cmd.buttons & BT_ATTACK)
                {
                    if(!player->attackdown)
                    {
                        S_StartSound(NULL, sfx_explode);
                        wi_advance++;
                    }
                    player->attackdown = true;
                }
                else
                    player->attackdown = false;

                if(player->cmd.buttons & BT_USE)
                {
                    if(!player->usedown)
                    {
                        S_StartSound(NULL, sfx_explode);
                        wi_advance++;
                    }
                    player->usedown = true;
                }
                else
                    player->usedown = false;
            }
        }
    }

    // accelerate counters
    if(wi_advance == 1)
    {
        wi_stage = 5;
        wi_counter = 0;
        wi_advance = 2;

        for(i = 0; i < MAXPLAYERS; i++)
        {
            killvalue[i] = killpercent[i];
            itemvalue[i] = itempercent[i];
            secretvalue[i] = secretpercent[i];
        }
    }

    if(wi_advance == 2)
        return 0;

    if(wi_advance == 3)
    {
        //S_StartSound(NULL, sfx_explode);
        wi_advance = 4;
    }

    // fade out, complete world
    if(wi_advance >= 4)
    {
        clusterdef_t* cluster;
        clusterdef_t* nextcluster;

        cluster = P_GetCluster(gamemap);
        nextcluster = P_GetCluster(nextmap);

        if((nextcluster && cluster != nextcluster && nextcluster->enteronly) ||
            (cluster && cluster != nextcluster && !cluster->enteronly))
            return ga_victory;

        return 1;
    }

    if(wi_counter)
    {
        if((gametic - wi_counter) <= 60)
            return 0;
    }

    next = true;

    // counter ticks
    switch(wi_stage)
    {
    case 0:
        S_StartSound(NULL, sfx_explode);
        wi_stage = 1;
        state = false;
        break;

    case 1:     // kills
        for(i = 0; i < MAXPLAYERS; i++)
        {
            killvalue[i] += 4;
            if(killvalue[i] > killpercent[i])
                killvalue[i] = killpercent[i];
            else
                next = false;
        }

        if(next)
        {
            S_StartSound(NULL, sfx_explode);

            wi_counter = gametic;
            wi_stage = 2;
        }

        state = true;
        break;

    case 2:     // item
        for(i = 0; i < MAXPLAYERS; i++)
        {
            itemvalue[i] += 4;
            if(itemvalue[i] > itempercent[i])
                itemvalue[i] = itempercent[i];
            else
                next = false;
        }

        if(next)
        {
            S_StartSound(NULL, sfx_explode);

            wi_counter = gametic;
            wi_stage = 3;
        }

        state = true;
        break;

    case 3:     // secret
        for(i = 0; i < MAXPLAYERS; i++)
        {
            secretvalue[i] += 4;
            if(secretvalue[i] > secretpercent[i])
                secretvalue[i] = secretpercent[i];
            else
                next = false;
        }

        if(next)
        {
            S_StartSound(NULL, sfx_explode);

            wi_counter = gametic;
            wi_stage = 4;
        }

        state = true;
        break;

    case 4:
        if(gamemap > 33 && nextmap > 33)
            S_StartSound(NULL, sfx_explode);

        wi_counter = gametic;
        wi_stage = 5;
        state = false;
        break;
    }

    if(!wi_advance && !state)
    {
        if(wi_stage == 5)
            wi_advance = 1;
    }

    // play sound as counter increases
    if(state && !(gametic & 3))
        S_StartSound(NULL, sfx_pistol);

    return 0;
}

//
// WI_Drawer
//

void WI_Drawer(void)
{
    int currentmap = gamemap;

    R_GLClearFrame(0xFF000000);

    if(currentmap < 0)
        currentmap = 0;

    if(currentmap > 33)
        currentmap = 33;

    if(wi_advance >= 4)
        return;

    // draw background
    R_DrawGfx(63, 25, "EVIL", WHITE, false);

    // draw 'mapname' Finished text
    M_DrawSmbText(-1, 20, WHITE, P_GetMapInfo(currentmap)->mapname);
	M_DrawSmbText(-1, 36, WHITE, "Finished");

    if(!netgame)
    {
        // draw kills
        M_DrawSmbText(57, 60, WIALPHARED, "Kills");
        M_DrawSmbText(248, 60, WIALPHARED, "%");
        if(wi_stage > 0 && killvalue[0] > -1)
            M_DrawNumber(210, 60, killvalue[0], 1, WIALPHARED);

        // draw items
        M_DrawSmbText(57, 78, WIALPHARED, "Items");
        M_DrawSmbText(248, 78, WIALPHARED, "%");
        if(wi_stage > 1 && itemvalue[0] > -1)
            M_DrawNumber(210, 78, itemvalue[0], 1, WIALPHARED);

        // draw secrets
        M_DrawSmbText(57, 99, WIALPHARED, "Secrets");
        M_DrawSmbText(248, 99, WIALPHARED, "%");
        if(wi_stage > 2 && secretvalue[0] > -1)
            M_DrawNumber(210, 99, secretvalue[0], 1, WIALPHARED);

        // draw time
        if(wi_stage > 3)
        {
            M_DrawSmbText(57, 120, WIALPHARED, "Time");
            M_DrawSmbText(210, 120, WIALPHARED, timevalue);
        }
    }
    else
    {
        int i;
        int y = 160;

        R_GLSetOrthoScale(0.5f);

        M_DrawSmbText(180, 140, WHITE, "Kills");
        M_DrawSmbText(300, 140, WHITE, "Items");
        M_DrawSmbText(412, 140, WHITE, "Secrets");

        for(i = 0; i < MAXPLAYERS; i++)
        {
            if(!playeringame[i])
                continue;

            M_DrawSmbText(57, y, WIALPHARED, player_names[i]);
            M_DrawSmbText(232, y, WIALPHARED, "%");
            M_DrawSmbText(352, y, WIALPHARED, "%");
            M_DrawSmbText(464, y, WIALPHARED, "%");

            if(wi_stage > 0 && killvalue[i] > -1)
                M_DrawNumber(180, y, killvalue[i], 1, WIALPHARED);

            if(wi_stage > 1 && itemvalue[i] > -1)
                M_DrawNumber(300, y, itemvalue[i], 1, WIALPHARED);

            if(wi_stage > 2 && secretvalue[i] > -1)
                M_DrawNumber(412, y, secretvalue[i], 1, WIALPHARED);

            y += 16;
        }

        // draw time
        if(wi_stage > 3)
        {
            M_DrawSmbText(248, y + 32, WIALPHARED, "Time");
            M_DrawSmbText(324, y + 32, WIALPHARED, timevalue);
        }

        R_GLSetOrthoScale(1.0f);
    }

    // draw password and name of next map
    if(wi_stage > 4 && (P_GetMapInfo(nextmap) != NULL))
    {
        char password[20];
	    byte *passData;
        int i = 0;
        int y = 145;

        if(netgame)
            y = 187;

        M_DrawSmbText(-1, y, WHITE, "Entering");
        M_DrawSmbText(-1, y + 16, WHITE, P_GetMapInfo(nextmap)->mapname);

        if(netgame)	// don't bother drawing the password on netgames
		    return;

        M_DrawSmbText(-1, 187, WHITE, "Password");

        dmemset(password, 0, 20);
	    passData = passwordData;

        // draw actual password
	    do
	    {
		    if(i && !((passData - passwordData) & 3))
			    password[i++] = 0x20;

		    if(i >= 20)
			    break;

		    password[i++] = passwordChar[*(passData++)];
	    } while(i < 20);

	    password[19] = 0;

        M_DrawSmbText(-1, 203, WHITE, password);
    }
}

