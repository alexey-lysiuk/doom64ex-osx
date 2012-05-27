// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: i_video.h 1101 2012-04-08 19:48:22Z svkaiser $
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
// DESCRIPTION: Global SDL stuff
//
//-----------------------------------------------------------------------------


#ifndef __I_VIDEO_H__
#define __I_VIDEO_H__

#include "SDL.h"

#define SDL_BPP		32

////////////Video///////////////

extern SDL_Surface *screen;

void I_InitVideo(void);
void I_InitGL(void);
void I_NetWaitScreen(void);
void I_ShutdownVideo(void);
//
// Called by D_DoomLoop,
// called before processing each tic in a frame.
// Quick syncronous operations are performed here.
// Can call D_PostEvent.
void I_StartTic(void);

void I_FinishUpdate(void);

int I_ShutdownWait(void);

////////////Input//////////////

extern int	UseMouse[2];
extern int	UseJoystick;

int I_MouseAccel(int val);
void I_MouseAccelChange(void);

void V_RegisterCvars(void);

#endif