// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: i_video.c 1101 2012-04-08 19:48:22Z svkaiser $
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
// $Revision: 1101 $
// $Date: 2012-04-08 22:48:22 +0300 (нд, 08 кві 2012) $
//
//
// DESCRIPTION:
//	SDL Stuff
//
//-----------------------------------------------------------------------------
#ifdef RCSID
static const char rcsid[] = "$Id: i_video.c 1101 2012-04-08 19:48:22Z svkaiser $";
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "SDL.h"
#include "SDL_opengl.h"

#include "m_misc.h"
#include "doomdef.h"
#include "doomstat.h"
#include "i_system.h"
#include "i_video.h"
#include "d_main.h"
#include "r_gl.h"

#ifdef _WIN32
#include "i_xinput.h"
#endif

CVAR(v_msensitivityx, 5);
CVAR(v_msensitivityy, 5);
CVAR(v_macceleration, 0);
CVAR(v_mlook, 0);
CVAR(v_mlookinvert, 0);
CVAR(v_width, 640);
CVAR(v_height, 480);
CVAR(v_windowed, 1);
CVAR(v_vsync, 1);
CVAR(v_depthsize, 24);
CVAR(v_buffersize, 32);

static void I_GetEvent(SDL_Event *Event);
static void I_ReadMouse(void);
static void I_InitInputs(void);
void I_UpdateGrab(void);

//================================================================================
// Video
//================================================================================

SDL_Surface *screen;
int	video_width;
int	video_height;
dboolean window_focused;

//
// I_InitScreen
//

void I_InitScreen(void)
{
    int		newwidth;
    int		newheight;
    int		p;
    
    InWindow = (int)v_windowed.value;
    video_width = (int)v_width.value;
    video_height = (int)v_height.value;
    
    if(M_CheckParm("-window"))		InWindow=true;
    if(M_CheckParm("-fullscreen"))	InWindow=false;
    
    newwidth = newheight = 0;
    
    p = M_CheckParm("-width");
    if(p && p < myargc - 1)
        newwidth = datoi(myargv[p+1]);
    
    p = M_CheckParm("-height");
    if(p && p < myargc - 1)
        newheight = datoi(myargv[p+1]);
    
    if(newwidth && newheight)
    {
        video_width = newwidth;
        video_height = newheight;
        CON_CvarSetValue(v_width.name, (float)video_width);
        CON_CvarSetValue(v_height.name, (float)video_height);
    }
    
    usingGL = false;
}

//
// I_ShutdownWait
//

int I_ShutdownWait(void)
{
    static SDL_Event event;
        
    while(SDL_PollEvent(&event))
    {
        if(event.type == SDL_QUIT || 
            (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
        {
            I_ShutdownVideo();
#ifndef USESYSCONSOLE
            exit(0);
#else
            return 1;
#endif
        }
    }

    return 0;
}

//
// I_ShutdownVideo
//

void I_ShutdownVideo(void)
{
    SDL_Quit();
}

//
// I_NetWaitScreen
// Blank screen display while waiting for players to join
//

void I_NetWaitScreen(void)
{
    uint32	flags = 0;
    
    I_InitScreen();
    flags |= SDL_SWSURFACE;
    
    if (!(screen = SDL_SetVideoMode(320, 240, 0, flags)))
    {
        I_ShutdownVideo();
        exit(1);
    }
    
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
}

//
// I_InitGL
//

void I_InitGL(void)
{
    uint32	flags = 0;
    
    I_InitScreen();
    
    if(v_depthsize.value != 8 &&
        v_depthsize.value != 16 &&
        v_depthsize.value != 24)
    {
        CON_CvarSetValue(v_depthsize.name, 24);
    }
    
    if(v_buffersize.value != 8 &&
        v_buffersize.value != 16 &&
        v_buffersize.value != 24
        && v_buffersize.value != 32)
    {
        CON_CvarSetValue(v_buffersize.name, 32);
    }
    
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, (int)v_buffersize.value);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, (int)v_depthsize.value);
    SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, (int)v_vsync.value);
    
    flags |= SDL_OPENGL;
    
    if(!InWindow)
        flags |= SDL_FULLSCREEN;
    
    if (SDL_SetVideoMode(video_width, video_height, SDL_BPP, flags) == NULL)
        I_Error("I_Init: Failed to set opengl");
    
    R_GLInitialize();
    
    usingGL = true;
    
#ifdef USESYSCONSOLE
    I_ShowSysConsole(false);
#endif
}

//
// I_InitVideo
//

void I_InitVideo(void)
{
    char title[256];
    
    uint32 f = SDL_INIT_VIDEO;
    
#ifdef _DEBUG
    f |= SDL_INIT_NOPARACHUTE;
#endif
    
    putenv("SDL_VIDEO_CENTERED=1");
    
    if(SDL_Init(f) < 0)
    {
        printf("ERROR - Failed to initialize SDL");
        exit(1);
    }
    
    sprintf(title, "Doom64 - Version Date: %s", version_date);
    SDL_WM_SetCaption(title, "Doom64");
    
    I_InitInputs();
}

//
// I_StartTic
//

void I_StartTic (void)
{
    SDL_Event Event;
    
    while(SDL_PollEvent(&Event))
        I_GetEvent(&Event);
    
#ifdef _USE_XINPUT
    I_XInputPollEvent();
#endif
    
    I_ReadMouse();
}

//
// I_FinishUpdate
//

void I_FinishUpdate(void)
{
    I_UpdateGrab();
    R_GLFinish();

    BusyDisk = false;
}

//================================================================================
// Input
//================================================================================

static SDL_Cursor* cursors[2] = { NULL, NULL };
float mouse_accelfactor;

int			UseJoystick;
int			UseMouse[2];
dboolean	DigiJoy;
int			DualMouse;

dboolean	MouseMode;//false=microsoft, true=mouse systems

//
// I_TranslateKey
//

static int I_TranslateKey(SDL_keysym* key)
{
    int rc = 0;
    
    switch (key->sym)
    {
    case SDLK_LEFT:			rc = KEY_LEFTARROW;			break;
    case SDLK_RIGHT:		rc = KEY_RIGHTARROW;		break;
    case SDLK_DOWN:			rc = KEY_DOWNARROW;			break;
    case SDLK_UP:			rc = KEY_UPARROW;			break;
    case SDLK_ESCAPE:		rc = KEY_ESCAPE;			break;
    case SDLK_RETURN:		rc = KEY_ENTER;				break;
    case SDLK_TAB:			rc = KEY_TAB;				break;
    case SDLK_F1:			rc = KEY_F1;				break;
    case SDLK_F2:			rc = KEY_F2;				break;
    case SDLK_F3:			rc = KEY_F3;				break;
    case SDLK_F4:			rc = KEY_F4;				break;
    case SDLK_F5:			rc = KEY_F5;				break;
    case SDLK_F6:			rc = KEY_F6;				break;
    case SDLK_F7:			rc = KEY_F7;				break;
    case SDLK_F8:			rc = KEY_F8;				break;
    case SDLK_F9:			rc = KEY_F9;				break;
    case SDLK_F10:			rc = KEY_F10;				break;
    case SDLK_F11:			rc = KEY_F11;				break;
    case SDLK_F12:			rc = KEY_F12;				break;
    case SDLK_BACKSPACE:	rc = KEY_BACKSPACE;			break;
    case SDLK_DELETE:		rc = KEY_DEL;				break;
    case SDLK_INSERT:		rc = KEY_INSERT;			break;
    case SDLK_PAGEUP:		rc = KEY_PAGEUP;			break;
    case SDLK_PAGEDOWN:		rc = KEY_PAGEDOWN;			break;
    case SDLK_HOME:			rc = KEY_HOME;				break;
    case SDLK_END:			rc = KEY_END;				break;
    case SDLK_PAUSE:		rc = KEY_PAUSE;				break;
    case SDLK_EQUALS:		rc = KEY_EQUALS;			break;
    case SDLK_MINUS:		rc = KEY_MINUS;				break;
    case SDLK_KP0:			rc = KEY_KEYPAD0;			break;
    case SDLK_KP1:			rc = KEY_KEYPAD1;			break;
    case SDLK_KP2:			rc = KEY_KEYPAD2;			break;
    case SDLK_KP3:			rc = KEY_KEYPAD3;			break;
    case SDLK_KP4:			rc = KEY_KEYPAD4;			break;
    case SDLK_KP5:			rc = KEY_KEYPAD5;			break;
    case SDLK_KP6:			rc = KEY_KEYPAD6;			break;
    case SDLK_KP7:			rc = KEY_KEYPAD7;			break;
    case SDLK_KP8:			rc = KEY_KEYPAD8;			break;
    case SDLK_KP9:			rc = KEY_KEYPAD9;			break;
    case SDLK_KP_PLUS:		rc = KEY_KEYPADPLUS;		break;
    case SDLK_KP_MINUS:		rc = KEY_KEYPADMINUS;		break;
    case SDLK_KP_DIVIDE:	rc = KEY_KEYPADDIVIDE;		break;
    case SDLK_KP_MULTIPLY:	rc = KEY_KEYPADMULTIPLY;	break;
    case SDLK_KP_ENTER:		rc = KEY_KEYPADENTER;		break;
    case SDLK_KP_PERIOD:	rc = KEY_KEYPADPERIOD;		break;
    case SDLK_LSHIFT:
    case SDLK_RSHIFT:		rc = KEY_RSHIFT;			break;
    case SDLK_LCTRL:
    case SDLK_RCTRL:		rc = KEY_RCTRL;				break;
    case SDLK_LALT:
    case SDLK_LMETA:
    case SDLK_RALT:
    case SDLK_RMETA:		rc = KEY_RALT;				break;
    case SDLK_CAPSLOCK:		rc = KEY_CAPS;				break;
    default:				rc = key->sym;				break;
    }
    
    return rc;
    
}

//
// I_SDLtoDoomMouseState
//

static int I_SDLtoDoomMouseState(Uint8 buttonstate)
{
    return 0
        | (buttonstate & SDL_BUTTON(SDL_BUTTON_LEFT)      ? 1 : 0)
        | (buttonstate & SDL_BUTTON(SDL_BUTTON_MIDDLE)    ? 2 : 0)
        | (buttonstate & SDL_BUTTON(SDL_BUTTON_RIGHT)     ? 4 : 0);
}

//
// I_UpdateFocus
//

static void I_UpdateFocus(void)
{
    Uint8 state;
    state = SDL_GetAppState();
    
    // We should have input (keyboard) focus and be visible 
    // (not minimised)
    window_focused = (state & SDL_APPINPUTFOCUS) && (state & SDL_APPACTIVE);
}

// I_CenterMouse
// Warp the mouse back to the middle of the screen
//

static void I_CenterMouse(void)
{
    // Warp the the screen center
    SDL_WarpMouse((unsigned short)(video_width/2), (unsigned short)(video_height/2));
    
    // Clear any relative movement caused by warping
    SDL_PumpEvents();
    SDL_GetRelativeMouseState(NULL, NULL);
}

//
// I_MouseShouldBeGrabbed
//

static dboolean I_MouseShouldBeGrabbed()
{
#ifndef _WIN32
    // 20120105 bkw: Always grab the mouse in fullscreen mode
    if(!InWindow)
        return true;
#endif
    
    // if the window doesnt have focus, never grab it
    if(!window_focused)
        return false;
    
#ifdef _WIN32
    if(!InWindow)
        return true;
#endif
    
    // when menu is active or game is paused, release the mouse 
    if(menuactive || paused)
        return false;
    
    // only grab mouse when playing levels (but not demos)
    return (gamestate == GS_LEVEL) && !demoplayback;
}

//
// I_ReadMouse
//

static void I_ReadMouse(void)
{
    int x, y;
    event_t ev;
    
    SDL_GetRelativeMouseState(&x, &y);
    
    // 20120105 bkw: Update the mouse even if the pointer hasn't been
    // moved. Otherwise, we can't shoot while standing still!
    
    // if(x != 0 || y != 0) 
    // {
    ev.type = ev_mouse;
    ev.data1 = I_SDLtoDoomMouseState(SDL_GetMouseState(NULL, NULL));
    ev.data2 = x << 5;
    ev.data3 = (-y) << 5;
    D_PostEvent(&ev);
    // }
    
    if(I_MouseShouldBeGrabbed())
        I_CenterMouse();
}

//
// I_MouseAccelChange
//

void I_MouseAccelChange(void)
{
    mouse_accelfactor = v_macceleration.value / 200.0f + 1.0f;
}

//
// I_MouseAccel
//

int I_MouseAccel(int val)
{
    if(!v_macceleration.value)
        return val;
    
    if(val < 0)
        return -I_MouseAccel(-val);
    
    return (int)(pow((double)val, (double)mouse_accelfactor));
}

//
// I_ActivateMouse
//

static void I_ActivateMouse(void)
{
    SDL_SetCursor(cursors[1]);
    SDL_WM_GrabInput(SDL_GRAB_ON);
    SDL_ShowCursor(1);
}

//
// I_DeactivateMouse
//

static void I_DeactivateMouse(void)
{
    SDL_SetCursor(cursors[0]);
    SDL_WM_GrabInput(SDL_GRAB_OFF);
    SDL_ShowCursor(1);
}

//
// I_UpdateGrab
//

void I_UpdateGrab(void)
{
    static dboolean currently_grabbed = false;
    dboolean grab;
    
    grab = I_MouseShouldBeGrabbed();
    if (grab && !currently_grabbed)
    {
        I_ActivateMouse();
    }
    
    if (!grab && currently_grabbed)
    {
        I_DeactivateMouse();
    }
    
    currently_grabbed = grab;
}

//
// I_GetEvent
//

static void I_GetEvent(SDL_Event *Event)
{
    event_t event;
    uint32 mwheeluptic = 0, mwheeldowntic = 0;
    uint32 tic = gametic;
    
    switch(Event->type)
    {
    case SDL_KEYDOWN:
        event.type = ev_keydown;
        event.data1 = I_TranslateKey(&Event->key.keysym);
        D_PostEvent(&event);
        break;
        
    case SDL_KEYUP:
        event.type = ev_keyup;
        event.data1 = I_TranslateKey(&Event->key.keysym);
        D_PostEvent(&event);
        break;
        
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
        if(!window_focused)
            break;
        
        if(Event->button.button == SDL_BUTTON_WHEELUP)
        {
            event.type = ev_keydown;
            event.data1 = KEY_MWHEELUP;
            mwheeluptic = tic;
        }
        else if(Event->button.button == SDL_BUTTON_WHEELDOWN)
        {
            event.type = ev_keydown;
            event.data1 = KEY_MWHEELDOWN;
            mwheeldowntic = tic;
        }
        else
        {
            event.type = ev_mouse;
            event.data1 = I_SDLtoDoomMouseState(SDL_GetMouseState(NULL, NULL));
        }
        
        event.data2 = event.data3 = 0;
        D_PostEvent(&event);
        break;
        
    case SDL_ACTIVEEVENT:
    case SDL_VIDEOEXPOSE:
        I_UpdateFocus();
        break;
        
    case SDL_QUIT:
        I_Quit();
        break;
        
    default:
        break;
    }
    
    if(mwheeluptic && mwheeluptic + 1 < tic)
    {
        event.type = ev_keyup;
        event.data1 = KEY_MWHEELUP;
        D_PostEvent(&event);
        mwheeluptic = 0;
    }
    
    if(mwheeldowntic && mwheeldowntic + 1 < tic)
    {
        event.type = ev_keyup;
        event.data1 = KEY_MWHEELDOWN;
        D_PostEvent(&event);
        mwheeldowntic = 0;
    }
}

//
// I_InitInputs
//

static void I_InitInputs(void)
{
    Uint8 data[1] = { 0x00 };
    
    SDL_PumpEvents();
    cursors[0] = SDL_GetCursor();
    cursors[1] = SDL_CreateCursor(data, data, 8, 1, 0, 0);
    
    UseMouse[0] = 1;
    UseMouse[1] = 2;
    
    I_CenterMouse();
    I_MouseAccelChange();

#ifdef _USE_XINPUT
    I_XInputInit();
#endif
}


//
// V_RegisterCvars
//

void V_RegisterCvars(void)
{
    CON_CvarRegister(&v_msensitivityx);
    CON_CvarRegister(&v_msensitivityy);
    CON_CvarRegister(&v_macceleration);
    CON_CvarRegister(&v_mlook);
    CON_CvarRegister(&v_mlookinvert);
    CON_CvarRegister(&v_width);
    CON_CvarRegister(&v_height);
    CON_CvarRegister(&v_windowed);
    CON_CvarRegister(&v_vsync);
    CON_CvarRegister(&v_depthsize);
    CON_CvarRegister(&v_buffersize);
}

