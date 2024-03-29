// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1997 Id Software, Inc.
// Copyright(C) 1997 Midway Home Entertainment, Inc
// Copyright(C) 2007-2012 Samuel Villarreal
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
//-----------------------------------------------------------------------------


#ifndef __P_PSPR__
#define __P_PSPR__

#include "doomdef.h"
// Basic data types.
// Needs fixed point, and BAM angles.
//#include "m_fixed.h"
#include "tables.h"
//
// Needs to include the precompiled
//  sprite animation tables.
// Header generated by multigen utility.
// This includes all the data for thing animation,
// i.e. the Thing Atrributes table
// and the Frame Sequence table.
#include "info.h"
#include "m_fixed.h"

#ifdef __GNUG__
#pragma interface
#endif

//
// Frame flags:
// handles maximum brightness (torches, muzzle flare, light sources)
//

#define FF_FULLBRIGHT	0x8000	// flag in thing->frame
#define FF_FRAMEMASK	0x7fff



//
// Overlay psprites are scaled shapes
// drawn directly on the view screen,
// coordinates are given for a 320*200 view screen.
//
typedef enum
{
    ps_weapon,
    ps_flash,
    NUMPSPRITES

} psprnum_t;

typedef struct
{
    state_t*    state;	// a NULL state means not active
    int         tics;
    fixed_t     sx;
    fixed_t     sy;
    int         alpha;  // [d64] for rendering

    // [kex] stuff that happens in between tics
    fixed_t     frame_x;
    fixed_t     frame_y;

} pspdef_t;

typedef struct
{
    ammotype_t	ammo;
    int		upstate;
    int		downstate;
    int		readystate;
    int		atkstate;
    int		flashstate;

} weaponinfo_t;

// Weapon info: sprite frames, ammunition use.
extern  weaponinfo_t    weaponinfo[NUMWEAPONS];

#endif
