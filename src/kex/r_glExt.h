// Emacs style mode select	 -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: r_glExt.h 1050 2012-02-16 03:01:58Z svkaiser $
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
// $Revision: 1050 $
// $Date: 2012-02-16 05:01:58 +0200 (чт, 16 лют 2012) $
//
// DESCRIPTION: OpenGL extensions
//
//-----------------------------------------------------------------------------

#ifndef __R_GLEXT_H__
#define __R_GLEXT_H__

void R_GLInitExtensions(void);
dboolean R_GLCheckExt(const char *ext);

#endif