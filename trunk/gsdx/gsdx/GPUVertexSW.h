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

#pragma once

#include "GSVector.h"

__declspec(align(16)) union GPUVertexSW
{
	typedef GSVector4 Vector;

	struct {Vector p, c;};
	struct {Vector v[2];};
	struct {float f[8];};

	GPUVertexSW() {}
	GPUVertexSW(const GPUVertexSW& v) {*this = v;}

	void operator = (const GPUVertexSW& v) {c = v.c; p = v.p;}
	void operator += (const GPUVertexSW& v) {c += v.c; p += v.p;}

	friend GPUVertexSW operator + (const GPUVertexSW& v1, const GPUVertexSW& v2);
	friend GPUVertexSW operator - (const GPUVertexSW& v1, const GPUVertexSW& v2);
	friend GPUVertexSW operator * (const GPUVertexSW& v, const Vector& vv);
	friend GPUVertexSW operator / (const GPUVertexSW& v, const Vector& vv);
	friend GPUVertexSW operator * (const GPUVertexSW& v, float f);
	friend GPUVertexSW operator / (const GPUVertexSW& v, float f);
};

__forceinline GPUVertexSW operator + (const GPUVertexSW& v1, const GPUVertexSW& v2)
{
	GPUVertexSW v0;
	v0.c = v1.c + v2.c;
	v0.p = v1.p + v2.p;
	return v0;
}

__forceinline GPUVertexSW operator - (const GPUVertexSW& v1, const GPUVertexSW& v2)
{
	GPUVertexSW v0;
	v0.c = v1.c - v2.c;
	v0.p = v1.p - v2.p;
	return v0;
}

__forceinline GPUVertexSW operator * (const GPUVertexSW& v, const GPUVertexSW::Vector& vv)
{
	GPUVertexSW v0;
	v0.c = v.c * vv;
	v0.p = v.p * vv;
	return v0;
}

__forceinline GPUVertexSW operator / (const GPUVertexSW& v, const GPUVertexSW::Vector& vv)
{
	GPUVertexSW v0;
	v0.c = v.c / vv;
	v0.p = v.p / vv;
	return v0;
}

__forceinline GPUVertexSW operator * (const GPUVertexSW& v, float f)
{
	GPUVertexSW v0;
	GPUVertexSW::Vector vf(f);
	v0.c = v.c * vf;
	v0.p = v.p * vf;
	return v0;
}

__forceinline GPUVertexSW operator / (const GPUVertexSW& v, float f)
{
	GPUVertexSW v0;
	GPUVertexSW::Vector vf(f);
	v0.c = v.c / vf;
	v0.p = v.p / vf;
	return v0;
}
