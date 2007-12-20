/* 
 *	Copyright (C) 2007 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "StdAfx.h"
#include "GSRenderer.h"

GSSetting g_interlace[] =
{
	{0, _T("None"), NULL},
	{1, _T("Weave tff"), _T("saw-tooth")},
	{2, _T("Weave bff"), _T("saw-tooth")},
	{3, _T("Bob tff"), _T("use blend if shaking")},
	{4, _T("Bob bff"), _T("use blend if shaking")},
	{5, _T("Blend tff"), _T("slight blur, 1/2 fps")},
	{6, _T("Blend bff"), _T("slight blur, 1/2 fps")},
};

GSSetting g_aspectratio[] =
{
	{0, _T("Stretch"), NULL},
	{1, _T("4:3"), NULL},
	{2, _T("16:9"), NULL},
};
