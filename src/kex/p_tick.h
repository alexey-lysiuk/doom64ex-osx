// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: p_tick.h 771 2010-08-30 04:33:34Z svkaiser $
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
//	?
//
//-----------------------------------------------------------------------------


#ifndef __P_TICK__
#define __P_TICK__


#ifdef __GNUG__
#pragma interface
#endif

void P_Start(void);
void P_Stop(void);
void P_Drawer(void);

// Called by C_Ticker,
// can call G_PlayerExited.
// Carries out all thinking of monsters and players.
int P_Ticker(void);



#endif
