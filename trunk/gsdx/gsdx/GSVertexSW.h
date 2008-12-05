/* 
 *	Copyright (C) 2007-2009 Gabest
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

#pragma once

#include "GSVector.h"

__declspec(align(16)) union GSVertexSW
{
	typedef GSVector4 Vector;

	struct {Vector c, p, t;};
	struct {Vector v[3];};
	struct {float f[12];};

	GSVertexSW() {}
	GSVertexSW(const GSVertexSW& v) {*this = v;}

	void operator = (const GSVertexSW& v) {c = v.c; p = v.p; t = v.t;}
	void operator += (const GSVertexSW& v) {c += v.c; p += v.p; t += v.t;}

	friend GSVertexSW operator + (const GSVertexSW& v1, const GSVertexSW& v2);
	friend GSVertexSW operator - (const GSVertexSW& v1, const GSVertexSW& v2);
	friend GSVertexSW operator * (const GSVertexSW& v, const Vector& vv);
	friend GSVertexSW operator / (const GSVertexSW& v, const Vector& vv);
	friend GSVertexSW operator * (const GSVertexSW& v, float f);
	friend GSVertexSW operator / (const GSVertexSW& v, float f);
};

__forceinline GSVertexSW operator + (const GSVertexSW& v1, const GSVertexSW& v2)
{
	GSVertexSW v0;
	v0.c = v1.c + v2.c;
	v0.p = v1.p + v2.p;
	v0.t = v1.t + v2.t;
	return v0;
}

__forceinline GSVertexSW operator - (const GSVertexSW& v1, const GSVertexSW& v2)
{
	GSVertexSW v0;
	v0.c = v1.c - v2.c;
	v0.p = v1.p - v2.p;
	v0.t = v1.t - v2.t;
	return v0;
}

__forceinline GSVertexSW operator * (const GSVertexSW& v, const GSVertexSW::Vector& vv)
{
	GSVertexSW v0;
	v0.c = v.c * vv;
	v0.p = v.p * vv;
	v0.t = v.t * vv;
	return v0;
}

__forceinline GSVertexSW operator / (const GSVertexSW& v, const GSVertexSW::Vector& vv)
{
	GSVertexSW v0;
	v0.c = v.c / vv;
	v0.p = v.p / vv;
	v0.t = v.t / vv;
	return v0;
}

__forceinline GSVertexSW operator * (const GSVertexSW& v, float f)
{
	GSVertexSW v0;
	GSVertexSW::Vector vf(f);
	v0.c = v.c * vf;
	v0.p = v.p * vf;
	v0.t = v.t * vf;
	return v0;
}

__forceinline GSVertexSW operator / (const GSVertexSW& v, float f)
{
	GSVertexSW v0;
	GSVertexSW::Vector vf(f);
	v0.c = v.c / vf;
	v0.p = v.p / vf;
	v0.t = v.t / vf;
	return v0;
}
