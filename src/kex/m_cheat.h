// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: m_cheat.h 708 2010-05-09 05:32:45Z svkaiser $
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
// DESCRIPTION:
//	Cheat code checking.
//
//-----------------------------------------------------------------------------


#ifndef __M_CHEAT__
#define __M_CHEAT__

#include "doomdef.h"
#include "g_game.h"
#include "doomstat.h"

//
// CHEAT SEQUENCE PACKAGE
//

void M_CheatProcess(player_t* plyr, event_t* ev);
void M_ParseNetCheat(int player, int type, char *buff);

void M_CheatGod(player_t* player, char dat[4]);
void M_CheatClip(player_t* player, char dat[4]);

extern int		amCheating;

#endif
