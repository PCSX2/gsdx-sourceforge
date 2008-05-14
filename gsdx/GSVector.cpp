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
#include "GSVector.h"

void GSVector4::operator = (const GSVector4i& v) 
{
	m = _mm_cvtepi32_ps(v);
}

void GSVector4::expand(const GSVector4i& v, GSVector4& a, GSVector4& b, GSVector4& c, GSVector4& d)
{
	GSVector4i mask = GSVector4i(epi32_000000ff);

	a = v & mask;
	b = (v >> 8) & mask;
	c = (v >> 16) & mask;
	d = (v >> 24);
}

void GSVector4i::operator = (const GSVector4& v)
{
	m = _mm_cvttps_epi32(v);
}

