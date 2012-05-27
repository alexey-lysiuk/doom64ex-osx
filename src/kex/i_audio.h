// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: i_audio.h 1089 2012-03-17 05:37:23Z svkaiser $
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
//-----------------------------------------------------------------------------

#ifndef __I_AUDIO_H__
#define __I_AUDIO_H__

// 20120107 bkw: Linux users can change the default FluidSynth backend here:
#ifndef _WIN32
#define DEFAULT_FLUID_DRIVER "alsa"

// 20120203 villsa: add default for windows
#else
#define DEFAULT_FLUID_DRIVER "dsound"

#endif

typedef struct
{
    fixed_t x;
    fixed_t y;
    fixed_t z;
} sndsrc_t;

int I_GetMaxChannels(void);
int I_GetVoiceCount(void);
sndsrc_t* I_GetSoundSource(int c);

void I_InitSequencer(void);
void I_ShutdownSound(void);
void I_UpdateChannel(int c, int volume, int pan);
void I_RemoveSoundSource(int c);
void I_SetMusicVolume(float volume);
void I_SetSoundVolume(float volume);
void I_ResetSound(void);
void I_PauseSound(void);
void I_ResumeSound(void);
void I_SetGain(float db);
void I_StopSound(sndsrc_t* origin, int sfx_id);
void I_StartMusic(int mus_id);
void I_StartSound(int sfx_id, sndsrc_t* origin, int volume, int pan, int reverb);

#endif // __I_AUDIO_H__
