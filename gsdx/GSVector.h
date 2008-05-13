#pragma once

#pragma pack(push, 1)

class GSVector2
{
public:
	union {struct {float x, y;}; struct {float r, g;}; struct {float v[2];};};

	GSVector2() {}
	GSVector2(float x, float y) {this->x = x; this->y = y;}
};

class GSVector3
{
public:
	union {struct {float x, y, z;}; struct {float r, g, b;}; struct {float v[3];};};

	GSVector3() {}
	GSVector3(float x, float y, float z) {this->x = x; this->y = y; this->z = z;}
};

__declspec(align(16)) class GSVector4
{
public:
	union {struct {float x, y, z, w;}; struct {float r, g, b, a;}; struct {float v[4];}; __m128 m;};

	GSVector4() {}
	GSVector4(float x, float y, float z = 0.5f, float w = 1.0f) {m = _mm_set_ps(w, z, y, x);}
	GSVector4(const GSVector4& v) {*this = v;}	

	explicit GSVector4(float f) {*this = f;}
	explicit GSVector4(__m128 m) {*this = m;}
	explicit GSVector4(__m128i m) {*this = m;}
	explicit GSVector4(CRect r) {*this = r;}
	explicit GSVector4(DWORD dw) {*this = dw;}

	void operator = (const GSVector4& v) {m = v.m;}
	void operator = (float f) {m = _mm_set1_ps(f);}
	void operator = (__m128 m) {this->m = m;}
	void operator = (__m128i m) {this->m = _mm_cvtepi32_ps(m);}
	void operator = (CRect r) {m = _mm_set_ps((float)r.bottom, (float)r.right, (float)r.top, (float)r.left);}
	void operator = (DWORD dw) {m = _mm_cvtepi32_ps(_mm_cvtepu8_epi32(_mm_cvtsi32_si128(dw)));}

	operator DWORD() const {__m128i r = _mm_cvttps_epi32(m); r = _mm_packs_epi32(r, r); r = _mm_packus_epi16(r, r); return (DWORD)_mm_cvtsi128_si32(r);}
	operator UINT64() const {__m128i r = _mm_cvttps_epi32(m); r = _mm_packs_epi32(r, r); return *(UINT64*)&r;} // TODO: _mm_cvtsi128_si64 on x64
	operator __m128() const {return m;}
	operator __m128i() const {return _mm_cvttps_epi32(m);}

	GSVector4 abs() const {return GSVector4(_mm_abs_ps(m));}
	GSVector4 neg() const {return GSVector4(_mm_neg_ps(m));}
	GSVector4 rcp() const {return GSVector4(_mm_rcp_ps(m));}
	GSVector4 floor() const {return GSVector4(_mm_floor_ps(m));}
	GSVector4 ceil() const {return GSVector4(_mm_ceil_ps(m));}
	GSVector4 mod2x(const GSVector4& f, const int scale = 256) const {return *this * (f * (2.0f / scale));}
	GSVector4 mod2x(float f, const int scale = 256) const {return mod2x(GSVector4(f), scale);}
	GSVector4 lerp(const GSVector4& v, const GSVector4& f) const {return *this + (v - *this) * f;}
	GSVector4 lerp(const GSVector4& v, float f) const {return lerp(v, GSVector4(f));}
	GSVector4 sat(const GSVector4& a, const GSVector4& b) const {return GSVector4(_mm_min_ps(_mm_max_ps(m, a), b));}
	GSVector4 sat(const float scale = 255) const {return sat(zero(), GSVector4(scale));}
	GSVector4 blend(const GSVector4& a, const GSVector4& mask) {return GSVector4(_mm_blendv_ps(m, a, mask));}

	static GSVector4 zero() 
	{
		return GSVector4(_mm_castsi128_ps(_mm_setzero_si128())); // pxor is faster than xorps
	}

	static void expand(__m128i m, GSVector4& a, GSVector4& b, GSVector4& c, GSVector4& d)
	{
		__m128i mask = epi32_000000ff;

		a = _mm_cvtepi32_ps(_mm_and_si128(m, mask));
		b = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(m, 8), mask));
		c = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(m, 16), mask));
		d = _mm_cvtepi32_ps(_mm_srli_epi32(m, 24));
	}

	static void transpose(GSVector4& a, GSVector4& b, GSVector4& c, GSVector4& d)
	{
		_MM_TRANSPOSE4_PS(a.m, b.m, c.m, d.m);
	}

	void operator += (const GSVector4& v) {m = _mm_add_ps(m, v);}
	void operator -= (const GSVector4& v) {m = _mm_sub_ps(m, v);}
	void operator *= (const GSVector4& v) {m = _mm_mul_ps(m, v);}
	void operator /= (const GSVector4& v) {m = _mm_div_ps(m, v);}

	void operator += (float f) {*this += GSVector4(f);}
	void operator -= (float f) {*this -= GSVector4(f);}
	void operator *= (float f) {*this *= GSVector4(f);}
	void operator /= (float f) {*this /= GSVector4(f);}

	friend GSVector4 operator + (const GSVector4& v1, const GSVector4& v2) {return GSVector4(_mm_add_ps(v1, v2));}
	friend GSVector4 operator - (const GSVector4& v1, const GSVector4& v2) {return GSVector4(_mm_sub_ps(v1, v2));}
	friend GSVector4 operator * (const GSVector4& v1, const GSVector4& v2) {return GSVector4(_mm_mul_ps(v1, v2));}
	friend GSVector4 operator / (const GSVector4& v1, const GSVector4& v2) {return GSVector4(_mm_div_ps(v1, v2));}

	friend GSVector4 operator + (const GSVector4& v, float f) {return v + GSVector4(f);}
	friend GSVector4 operator - (const GSVector4& v, float f) {return v - GSVector4(f);}
	friend GSVector4 operator * (const GSVector4& v, float f) {return v * GSVector4(f);}
	friend GSVector4 operator / (const GSVector4& v, float f) {return v / GSVector4(f);}

	friend GSVector4 operator == (const GSVector4& v1, const GSVector4& v2) {return GSVector4(_mm_cmpeq_ps(v1, v2));}
	friend GSVector4 operator != (const GSVector4& v1, const GSVector4& v2) {return GSVector4(_mm_cmpneq_ps(v1, v2));}
	friend GSVector4 operator >= (const GSVector4& v1, const GSVector4& v2) {return GSVector4(_mm_cmpge_ps(v1, v2));}
	friend GSVector4 operator <= (const GSVector4& v1, const GSVector4& v2) {return GSVector4(_mm_cmple_ps(v1, v2));}
	friend GSVector4 operator > (const GSVector4& v1, const GSVector4& v2) {return GSVector4(_mm_cmpgt_ps(v1, v2));}
	friend GSVector4 operator < (const GSVector4& v1, const GSVector4& v2) {return GSVector4(_mm_cmplt_ps(v1, v2));}

	#define DEFINE_SHUFFLE_4(xs, xn, ys, yn, zs, zn, ws, wn) \
		GSVector4 xs##ys##zs##ws() const {return GSVector4(_mm_shuffle_ps(m, m, _MM_SHUFFLE(wn, zn, yn, xn)));}

	#define DEFINE_SHUFFLE_3(xs, xn, ys, yn, zs, zn) \
		DEFINE_SHUFFLE_4(xs, xn, ys, yn, zs, zn, x, 0) \
		DEFINE_SHUFFLE_4(xs, xn, ys, yn, zs, zn, y, 1) \
		DEFINE_SHUFFLE_4(xs, xn, ys, yn, zs, zn, z, 2) \
		DEFINE_SHUFFLE_4(xs, xn, ys, yn, zs, zn, w, 3) \

	#define DEFINE_SHUFFLE_2(xs, xn, ys, yn) \
		DEFINE_SHUFFLE_3(xs, xn, ys, yn, x, 0) \
		DEFINE_SHUFFLE_3(xs, xn, ys, yn, y, 1) \
		DEFINE_SHUFFLE_3(xs, xn, ys, yn, z, 2) \
		DEFINE_SHUFFLE_3(xs, xn, ys, yn, w, 3) \

	#define DEFINE_SHUFFLE_1(xs, xn) \
		DEFINE_SHUFFLE_2(xs, xn, x, 0) \
		DEFINE_SHUFFLE_2(xs, xn, y, 1) \
		DEFINE_SHUFFLE_2(xs, xn, z, 2) \
		DEFINE_SHUFFLE_2(xs, xn, w, 3) \

	DEFINE_SHUFFLE_1(x, 0)
	DEFINE_SHUFFLE_1(y, 1)
	DEFINE_SHUFFLE_1(z, 2)
	DEFINE_SHUFFLE_1(w, 3)
};

#pragma pack(pop)
