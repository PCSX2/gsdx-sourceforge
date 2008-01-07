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

#ifndef RESTRICT
	#ifdef __INTEL_COMPILER
		#define RESTRICT restrict
	#elif _MSC_VER >= 1400
		#define RESTRICT __restrict
	#else
		#define RESTRICT
	#endif
#endif

//
// GSVertexSWFP
//

__declspec(align(16)) union GSVertexSWFP
{
	class __declspec(novtable) Scalar
	{
		float val;

	public:
		Scalar() {}
		explicit Scalar(float f) {val = f;}
		explicit Scalar(int i) {val = (float)i;}

		float GetValue() const {return val;}
		void SetValue(int i) {val = (float)i;}

#if _M_SSE >= 0x200
		void sat() {_mm_store_ss(&val, _mm_min_ss(_mm_max_ss(_mm_set_ss(val), _mm_setzero_ps()), _mm_set_ss(255)));}
		void rcp() {_mm_store_ss(&val, _mm_rcp_ss(_mm_set_ss(val)));}
#else
		void sat() {val = val < 0 ? 0 : val > 255 ? 255 : val;}
		void rcp() {val = 1.0f / val;}
#endif
		void abs() {val = fabs(val);}

		Scalar floor_s() const {return Scalar(floor(val));}
		int floor_i() const {return (int)floor(val);}

		Scalar ceil_s() const {return Scalar(-floor(-val));}
		int ceil_i() const {return -(int)floor(-val);}

		void operator = (float f) {val = f;}
		void operator = (int i) {val = (float)i;}

		operator float() const {return val;}
		operator int() const {return (int)val;}
		operator DWORD() const {return (DWORD)val;}

		void operator += (const Scalar& s) {val += s.val;}
		void operator -= (const Scalar& s) {val -= s.val;}
		void operator *= (const Scalar& s) {val *= s.val;}
		void operator /= (const Scalar& s) {val /= s.val;}

		friend Scalar operator + (const Scalar& s1, const Scalar& s2) {return Scalar(s1.val + s2.val);}
		friend Scalar operator - (const Scalar& s1, const Scalar& s2) {return Scalar(s1.val - s2.val);}
		friend Scalar operator * (const Scalar& s1, const Scalar& s2) {return Scalar(s1.val * s2.val);}
		friend Scalar operator / (const Scalar& s1, const Scalar& s2) {return Scalar(s1.val / s2.val);}

		friend Scalar operator + (const Scalar& s, int i) {return Scalar(s.val + i);}
		friend Scalar operator - (const Scalar& s, int i) {return Scalar(s.val - i);}
		friend Scalar operator * (const Scalar& s, int i) {return Scalar(s.val * i);}
		friend Scalar operator / (const Scalar& s, int i) {return Scalar(s.val / i);}

		friend Scalar operator << (const Scalar& s, int i) {return Scalar(s.val * (1<<i));}
		friend Scalar operator >> (const Scalar& s, int i) {return Scalar(s.val / (1<<i));}

		friend bool operator == (const Scalar& s1, const Scalar& s2) {return s1.val == s2.val;}
		friend bool operator <= (const Scalar& s1, const Scalar& s2) {return s1.val <= s2.val;}
		friend bool operator < (const Scalar& s1, const Scalar& s2) {return s1.val < s2.val;}
		friend bool operator > (const Scalar& s1, const Scalar& s2) {return s1.val > s2.val;}
	};

	__declspec(align(16)) class __declspec(novtable) Vector
	{
	public:
		union
		{
			union {struct {Scalar x, y, z, q;}; struct {Scalar r, g, b, a;};};
			union {struct {Scalar v[4];}; struct {Scalar c[4];};};
			union {__m128 xyzq; __m128 rgba;};
		};

		Vector() {}
		Vector(const Vector& v) {*this = v;}
		Vector(Scalar s) {*this = s;}
		Vector(Scalar s0, Scalar s1, Scalar s2, Scalar s3) {x = s0; y = s1; z = s2; q = s3;}
		explicit Vector(DWORD dw) {*this = dw;}
#if _M_SSE >= 0x200
		Vector(__m128 f0123) {*this = f0123;}
#endif

#if _M_SSE >= 0x200

		void operator = (const Vector& v) {xyzq = v.xyzq;}
		void operator = (Scalar s) {xyzq = _mm_set1_ps(s);}

		void operator = (__m128 f0123) {xyzq = f0123;}
		operator __m128() const {return xyzq;}

		void operator = (DWORD dw) {__m128i zero = _mm_setzero_si128(); xyzq = _mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_unpacklo_epi8(_mm_cvtsi32_si128(dw), zero), zero));}
		operator DWORD() const {__m128i r0 = _mm_cvttps_epi32(xyzq); r0 = _mm_packs_epi32(r0, r0); r0 = _mm_packus_epi16(r0, r0); return (DWORD)_mm_cvtsi128_si32(r0);}
		operator UINT64() const {__m128i r0 = _mm_cvttps_epi32(xyzq); r0 = _mm_packs_epi32(r0, r0); return *(UINT64*)&r0;}

		void sat() {xyzq = _mm_min_ps(_mm_max_ps(xyzq, _mm_setzero_ps()), _mm_set1_ps(255));}
		void rcp() {xyzq = _mm_rcp_ps(xyzq);}

		Vector floor()
		{
			const __m128i _80000000 = _mm_set1_epi32(0x80000000);
			const __m128i _4b000000 = _mm_set1_epi32(0x4b000000);
			const __m128i _3f800000 = _mm_set1_epi32(0x3f800000);

			__m128 sign = _mm_and_ps(xyzq, *(__m128*)&_80000000);
			__m128 r0 = _mm_or_ps(sign, *(__m128*)&_4b000000);
			__m128 r1 = _mm_sub_ps(_mm_add_ps(xyzq, r0), r0);
			__m128 r2 = _mm_sub_ps(r1, xyzq);
			__m128 r3 = _mm_and_ps(_mm_cmpnle_ps(r2, sign), *(__m128*)&_3f800000);
			__m128 r4 = _mm_sub_ps(r1, r3);
			return r4;
		}

		void operator += (const Vector& v) {xyzq = _mm_add_ps(xyzq, v);}
		void operator -= (const Vector& v) {xyzq = _mm_sub_ps(xyzq, v);}
		void operator *= (const Vector& v) {xyzq = _mm_mul_ps(xyzq, v);}
		void operator /= (const Vector& v) {xyzq = _mm_div_ps(xyzq, v);}

#else

		void operator = (const Vector& v) {x = v.x; y = v.y; z = v.z; q = v.q;}
		void operator = (Scalar s) {x = y = z = q = s;}

		void operator = (DWORD dw)
		{
			x = Scalar((int)((dw>>0)&0xff));
			y = Scalar((int)((dw>>8)&0xff));
			z = Scalar((int)((dw>>16)&0xff));
			q = Scalar((int)((dw>>24)&0xff));
		}

		operator DWORD() const
		{
			return (DWORD)(
				(((DWORD)(int)x&0xff)<<0) |
				(((DWORD)(int)y&0xff)<<8) |
				(((DWORD)(int)z&0xff)<<16) |
				(((DWORD)(int)q&0xff)<<24));
		}

		operator UINT64() const
		{
			return (DWORD)(
				(((UINT64)(int)x&0xffff)<<0) |
				(((UINT64)(int)y&0xffff)<<16) |
				(((UINT64)(int)z&0xffff)<<32) |
				(((UINT64)(int)q&0xffff)<<48));
		}

		void sat() {x.sat(); y.sat(); z.sat(); q.sat();}
		void rcp() {x.rcp(); y.rcp(); z.rcp(); q.rcp();}
		
		Vector floor() {return Vector(x.floor_s(), y.floor_s(), z.floor_s(), q.floor_s());}

		void operator += (const Vector& v) {*this = *this + v;}
		void operator -= (const Vector& v) {*this = *this - v;}
		void operator *= (const Vector& v) {*this = *this * v;}
		void operator /= (const Vector& v) {*this = *this / v;}

#endif

		friend Vector operator + (const Vector& v1, const Vector& v2);
		friend Vector operator - (const Vector& v1, const Vector& v2);
		friend Vector operator * (const Vector& v1, const Vector& v2);
		friend Vector operator / (const Vector& v1, const Vector& v2);

		friend Vector operator + (const Vector& v, Scalar s);
		friend Vector operator - (const Vector& v, Scalar s);
		friend Vector operator * (const Vector& v, Scalar s);
		friend Vector operator / (const Vector& v, Scalar s);
	};

	struct {__declspec(align(16)) Vector c, p, t;};
	struct {__declspec(align(16)) Vector sv[3];};
	struct {__declspec(align(16)) Scalar s[12];};

	GSVertexSWFP() {}
	GSVertexSWFP(const GSVertexSWFP& v) {*this = v;}

	void operator = (const GSVertexSWFP& v) {c = v.c; p = v.p; t = v.t;}
	void operator += (const GSVertexSWFP& v) {c += v.c; p += v.p; t += v.t;}

	operator CPoint() const {return CPoint((int)p.x, (int)p.y);}

	__forceinline DWORD GetZ() const 
	{
		return (DWORD)p.z;
	}

	friend GSVertexSWFP operator + (const GSVertexSWFP& v1, const GSVertexSWFP& v2);
	friend GSVertexSWFP operator - (const GSVertexSWFP& v1, const GSVertexSWFP& v2);
	friend GSVertexSWFP operator * (const GSVertexSWFP& v, Scalar s);
	friend GSVertexSWFP operator / (const GSVertexSWFP& v, Scalar s);

	static void Exchange(GSVertexSWFP* RESTRICT v1, GSVertexSWFP* RESTRICT v2)
	{
		Vector c = v1->c, p = v1->p, t = v1->t;
		v1->c = v2->c; v1->p = v2->p; v1->t = v2->t;
		v2->c = c; v2->p = p; v2->t = t;
	}
};

#if _M_SSE >= 0x200

__forceinline GSVertexSWFP::Vector operator + (const GSVertexSWFP::Vector& v1, const GSVertexSWFP::Vector& v2) {return GSVertexSWFP::Vector(_mm_add_ps(v1, v2));}
__forceinline GSVertexSWFP::Vector operator - (const GSVertexSWFP::Vector& v1, const GSVertexSWFP::Vector& v2) {return GSVertexSWFP::Vector(_mm_sub_ps(v1, v2));}
__forceinline GSVertexSWFP::Vector operator * (const GSVertexSWFP::Vector& v1, const GSVertexSWFP::Vector& v2) {return GSVertexSWFP::Vector(_mm_mul_ps(v1, v2));}
__forceinline GSVertexSWFP::Vector operator / (const GSVertexSWFP::Vector& v1, const GSVertexSWFP::Vector& v2) {return GSVertexSWFP::Vector(_mm_div_ps(v1, v2));}

__forceinline GSVertexSWFP::Vector operator + (const GSVertexSWFP::Vector& v, GSVertexSWFP::Scalar s) {return GSVertexSWFP::Vector(_mm_add_ps(v, _mm_set1_ps(s)));}
__forceinline GSVertexSWFP::Vector operator - (const GSVertexSWFP::Vector& v, GSVertexSWFP::Scalar s) {return GSVertexSWFP::Vector(_mm_sub_ps(v, _mm_set1_ps(s)));}
__forceinline GSVertexSWFP::Vector operator * (const GSVertexSWFP::Vector& v, GSVertexSWFP::Scalar s) {return GSVertexSWFP::Vector(_mm_mul_ps(v, _mm_set1_ps(s)));}
__forceinline GSVertexSWFP::Vector operator / (const GSVertexSWFP::Vector& v, GSVertexSWFP::Scalar s) {return GSVertexSWFP::Vector(_mm_div_ps(v, _mm_set1_ps(s)));}

__forceinline GSVertexSWFP::Vector operator << (const GSVertexSWFP::Vector& v, int i) {return GSVertexSWFP::Vector(_mm_mul_ps(v, _mm_set1_ps((float)(1 << i))));}
__forceinline GSVertexSWFP::Vector operator >> (const GSVertexSWFP::Vector& v, int i) {return GSVertexSWFP::Vector(_mm_mul_ps(v, _mm_set1_ps(1.0f / (1 << i))));}

#else

__forceinline GSVertexSWFP::Vector operator + (const GSVertexSWFP::Vector& v1, const GSVertexSWFP::Vector& v2) {return GSVertexSWFP::Vector(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.q + v2.q);}
__forceinline GSVertexSWFP::Vector operator - (const GSVertexSWFP::Vector& v1, const GSVertexSWFP::Vector& v2) {return GSVertexSWFP::Vector(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.q - v2.q);}
__forceinline GSVertexSWFP::Vector operator * (const GSVertexSWFP::Vector& v1, const GSVertexSWFP::Vector& v2) {return GSVertexSWFP::Vector(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.q * v2.q);}
__forceinline GSVertexSWFP::Vector operator / (const GSVertexSWFP::Vector& v1, const GSVertexSWFP::Vector& v2) {return GSVertexSWFP::Vector(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z, v1.q / v2.q);}

__forceinline GSVertexSWFP::Vector operator + (const GSVertexSWFP::Vector& v, GSVertexSWFP::Scalar s) {return GSVertexSWFP::Vector(v.x + s, v.y + s, v.z + s, v.q + s);}
__forceinline GSVertexSWFP::Vector operator - (const GSVertexSWFP::Vector& v, GSVertexSWFP::Scalar s) {return GSVertexSWFP::Vector(v.x - s, v.y - s, v.z - s, v.q - s);}
__forceinline GSVertexSWFP::Vector operator * (const GSVertexSWFP::Vector& v, GSVertexSWFP::Scalar s) {return GSVertexSWFP::Vector(v.x * s, v.y * s, v.z * s, v.q * s);}
__forceinline GSVertexSWFP::Vector operator / (const GSVertexSWFP::Vector& v, GSVertexSWFP::Scalar s) {return GSVertexSWFP::Vector(v.x / s, v.y / s, v.z / s, v.q / s);}

__forceinline GSVertexSWFP::Vector operator << (const GSVertexSWFP::Vector& v, int i) {return GSVertexSWFP::Vector(v.x << i, v.y << i, v.z << i, v.q << i);}
__forceinline GSVertexSWFP::Vector operator >> (const GSVertexSWFP::Vector& v, int i) {return GSVertexSWFP::Vector(v.x >> i, v.y >> i, v.z >> i, v.q >> i);}

#endif

__forceinline GSVertexSWFP operator + (const GSVertexSWFP& v1, const GSVertexSWFP& v2)
{
	GSVertexSWFP v0;
	v0.c = v1.c + v2.c;
	v0.p = v1.p + v2.p;
	v0.t = v1.t + v2.t;
	return v0;
}

__forceinline GSVertexSWFP operator - (const GSVertexSWFP& v1, const GSVertexSWFP& v2)
{
	GSVertexSWFP v0;
	v0.c = v1.c - v2.c;
	v0.p = v1.p - v2.p;
	v0.t = v1.t - v2.t;
	return v0;
}

__forceinline GSVertexSWFP operator * (const GSVertexSWFP& v, GSVertexSWFP::Scalar s)
{
	GSVertexSWFP v0;
	GSVertexSWFP::Vector vs(s);
	v0.c = v.c * vs;
	v0.p = v.p * vs;
	v0.t = v.t * vs;
	return v0;
}

__forceinline GSVertexSWFP operator / (const GSVertexSWFP& v, GSVertexSWFP::Scalar s)
{
	GSVertexSWFP v0;
	GSVertexSWFP::Vector vs(s);
	v0.c = v.c / vs;
	v0.p = v.p / vs;
	v0.t = v.t / vs;
	return v0;
}

// #include "GSVertexSWFX.h"

