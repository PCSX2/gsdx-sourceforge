#pragma once

#pragma pack(push, 1)

class GSVector2
{
public:
	union 
	{
		struct {float x, y;}; 
		struct {float r, g;}; 
		struct {float v[2];};
	};

	GSVector2()
	{
	}

	GSVector2(float x, float y) 
	{
		this->x = x; 
		this->y = y;
	}
};

class GSVector3
{
public:
	union 
	{
		struct {float x, y, z;}; 
		struct {float r, g, b;}; 
		struct {float v[3];};
	};

	GSVector3()
	{
	}

	GSVector3(float x, float y, float z) 
	{
		this->x = x; 
		this->y = y; 
		this->z = z;
	}
};

class GSVector4i;

__declspec(align(16)) class GSVector4
{
public:
	union 
	{
		struct {float x, y, z, w;}; 
		struct {float r, g, b, a;}; 
		float v[4];
		float f32[4];
		unsigned __int64 u64[2];
		__int8 i8[16];
		__int16 i16[8];
		__int32 i32[4];
		__int64  i64[2];
		unsigned __int8 u8[16];
		unsigned __int16 u16[8];
		unsigned __int32 u32[4];
		__m128 m;
	};

	GSVector4()
	{
	}

	GSVector4(float x, float y, float z = 0.5f, float w = 1.0f)
	{
		m = _mm_set_ps(w, z, y, x);
	}

	GSVector4(const GSVector4& v) 
	{
		*this = v;
	}

	explicit GSVector4(float f)
	{
		*this = f;
	}

	explicit GSVector4(__m128 m)
	{
		*this = m;
	}

	explicit GSVector4(CRect r)
	{
		*this = r;
	}

	explicit GSVector4(DWORD dw)
	{
		*this = dw;
	}

	explicit GSVector4(const GSVector4i& v)
	{
		*this = v;
	}

	void operator = (const GSVector4& v)
	{
		m = v.m;
	}

	void operator = (const GSVector4i& v);

	void operator = (float f)
	{
		m = _mm_set1_ps(f);
	}

	void operator = (__m128 m)
	{
		this->m = m;
	}

	void operator = (DWORD dw)
	{
		m = _mm_cvtepi32_ps(_mm_cvtepu8_epi32(_mm_cvtsi32_si128(dw)));
	}

	void operator = (CRect r)
	{
		m = _mm_set_ps((float)r.bottom, (float)r.right, (float)r.top, (float)r.left);
	}

	operator DWORD() const 
	{
		__m128i r = _mm_cvttps_epi32(m); 
		r = _mm_packs_epi32(r, r); 
		r = _mm_packus_epi16(r, r); 
		return (DWORD)_mm_cvtsi128_si32(r);
	}

	operator UINT64() const 
	{
		__m128i r = _mm_cvttps_epi32(m); 
		r = _mm_packs_epi32(r, r); 
		return *(UINT64*)&r; // TODO: _mm_cvtsi128_si64 on x64
	}

	operator __m128() const 
	{
		return m;
	}

	GSVector4 abs() const 
	{
		return GSVector4(_mm_abs_ps(m));
	}

	GSVector4 neg() const 
	{
		return GSVector4(_mm_neg_ps(m));
	}

	GSVector4 rcp() const 
	{
		return GSVector4(_mm_rcp_ps(m));
	}

	GSVector4 floor() const 
	{
		return GSVector4(_mm_floor_ps(m));
	}

	GSVector4 ceil() const 
	{
		return GSVector4(_mm_ceil_ps(m));
	}

	GSVector4 mod2x(const GSVector4& f, const int scale = 256) const 
	{
		return *this * (f * (2.0f / scale));
	}

	GSVector4 mod2x(float f, const int scale = 256) const 
	{
		return mod2x(GSVector4(f), scale);
	}

	GSVector4 lerp(const GSVector4& v, const GSVector4& f) const 
	{
		return *this + (v - *this) * f;
	}

	GSVector4 lerp(const GSVector4& v, float f) const 
	{
		return lerp(v, GSVector4(f));
	}

	GSVector4 sat(const GSVector4& a, const GSVector4& b) const 
	{
		return GSVector4(_mm_min_ps(_mm_max_ps(m, a), b));
	}

	GSVector4 sat(const float scale = 255) const 
	{
		return sat(zero(), GSVector4(scale));
	}

	GSVector4 blend8(const GSVector4& a, const GSVector4& mask) 
	{
		return GSVector4(_mm_blendv_ps(m, a, mask));
	}

	GSVector4 upl(const GSVector4& a)
	{
		return GSVector4(_mm_unpacklo_ps(m, a));
	}

	GSVector4 uph(const GSVector4& a)
	{
		return GSVector4(_mm_unpackhi_ps(m, a));
	}

	static GSVector4 zero() 
	{
		return GSVector4(_mm_castsi128_ps(_mm_setzero_si128())); // pxor is faster than xorps
	}

	static void expand(const GSVector4i& v, GSVector4& a, GSVector4& b, GSVector4& c, GSVector4& d);

	static void transpose(GSVector4& a, GSVector4& b, GSVector4& c, GSVector4& d)
	{
		_MM_TRANSPOSE4_PS(a.m, b.m, c.m, d.m);
	}

	void operator += (const GSVector4& v) 
	{
		m = _mm_add_ps(m, v);
	}

	void operator -= (const GSVector4& v) 
	{
		m = _mm_sub_ps(m, v);
	}

	void operator *= (const GSVector4& v) 
	{
		m = _mm_mul_ps(m, v);
	}

	void operator /= (const GSVector4& v) 
	{
		m = _mm_div_ps(m, v);
	}

	void operator += (float f) 
	{
		*this += GSVector4(f);
	}

	void operator -= (float f) 
	{
		*this -= GSVector4(f);
	}

	void operator *= (float f) 
	{
		*this *= GSVector4(f);
	}

	void operator /= (float f) 
	{
		*this /= GSVector4(f);
	}

	friend GSVector4 operator + (const GSVector4& v1, const GSVector4& v2) 
	{
		return GSVector4(_mm_add_ps(v1, v2));
	}

	friend GSVector4 operator - (const GSVector4& v1, const GSVector4& v2) 
	{
		return GSVector4(_mm_sub_ps(v1, v2));
	}

	friend GSVector4 operator * (const GSVector4& v1, const GSVector4& v2)
	{
		return GSVector4(_mm_mul_ps(v1, v2));
	}

	friend GSVector4 operator / (const GSVector4& v1, const GSVector4& v2) 
	{
		return GSVector4(_mm_div_ps(v1, v2));
	}

	friend GSVector4 operator + (const GSVector4& v, float f) 
	{
		return v + GSVector4(f);
	}

	friend GSVector4 operator - (const GSVector4& v, float f) 
	{
		return v - GSVector4(f);
	}

	friend GSVector4 operator * (const GSVector4& v, float f) 
	{
		return v * GSVector4(f);
	}

	friend GSVector4 operator / (const GSVector4& v, float f) 
	{
		return v / GSVector4(f);
	}

	friend GSVector4 operator == (const GSVector4& v1, const GSVector4& v2) 
	{
		return GSVector4(_mm_cmpeq_ps(v1, v2));
	}

	friend GSVector4 operator != (const GSVector4& v1, const GSVector4& v2) 
	{
		return GSVector4(_mm_cmpneq_ps(v1, v2));
	}

	friend GSVector4 operator > (const GSVector4& v1, const GSVector4& v2) 
	{
		return GSVector4(_mm_cmpgt_ps(v1, v2));
	}

	friend GSVector4 operator < (const GSVector4& v1, const GSVector4& v2) 
	{
		return GSVector4(_mm_cmplt_ps(v1, v2));
	}

	friend GSVector4 operator >= (const GSVector4& v1, const GSVector4& v2) 
	{
		return GSVector4(_mm_cmpge_ps(v1, v2));
	}

	friend GSVector4 operator <= (const GSVector4& v1, const GSVector4& v2) 
	{
		return GSVector4(_mm_cmple_ps(v1, v2));
	}

	#define VECTOR4_SHUFFLE_4(xs, xn, ys, yn, zs, zn, ws, wn) \
		GSVector4 xs##ys##zs##ws() const {return GSVector4(_mm_shuffle_ps(m, m, _MM_SHUFFLE(wn, zn, yn, xn)));}

	#define VECTOR4_SHUFFLE_3(xs, xn, ys, yn, zs, zn) \
		VECTOR4_SHUFFLE_4(xs, xn, ys, yn, zs, zn, x, 0) \
		VECTOR4_SHUFFLE_4(xs, xn, ys, yn, zs, zn, y, 1) \
		VECTOR4_SHUFFLE_4(xs, xn, ys, yn, zs, zn, z, 2) \
		VECTOR4_SHUFFLE_4(xs, xn, ys, yn, zs, zn, w, 3) \

	#define VECTOR4_SHUFFLE_2(xs, xn, ys, yn) \
		VECTOR4_SHUFFLE_3(xs, xn, ys, yn, x, 0) \
		VECTOR4_SHUFFLE_3(xs, xn, ys, yn, y, 1) \
		VECTOR4_SHUFFLE_3(xs, xn, ys, yn, z, 2) \
		VECTOR4_SHUFFLE_3(xs, xn, ys, yn, w, 3) \

	#define VECTOR4_SHUFFLE_1(xs, xn) \
		VECTOR4_SHUFFLE_2(xs, xn, x, 0) \
		VECTOR4_SHUFFLE_2(xs, xn, y, 1) \
		VECTOR4_SHUFFLE_2(xs, xn, z, 2) \
		VECTOR4_SHUFFLE_2(xs, xn, w, 3) \

	VECTOR4_SHUFFLE_1(x, 0)
	VECTOR4_SHUFFLE_1(y, 1)
	VECTOR4_SHUFFLE_1(z, 2)
	VECTOR4_SHUFFLE_1(w, 3)
};

__declspec(align(16)) class GSVector4i
{
public:
	union 
	{
		struct {int x, y, z, w;}; 
		struct {int r, g, b, a;};
		int v[4];
		float f32[4];
		unsigned __int64 u64[2];
		__int8 i8[16];
		__int16 i16[8];
		__int32 i32[4];
		__int64  i64[2];
		unsigned __int8 u8[16];
		unsigned __int16 u16[8];
		unsigned __int32 u32[4];
		__m128i m;
	};

	GSVector4i() 
	{
	}

	GSVector4i(int x, int y, int z, int w) 
	{
		m = _mm_set_epi32(w, z, y, x);
	}

	GSVector4i(const GSVector4i& v) 
	{
		*this = v;
	}

	explicit GSVector4i(int i) 
	{
		*this = i;
	}

	explicit GSVector4i(__m128i m)
	{
		*this = m;
	}

	explicit GSVector4i(CRect r) 
	{
		*this = r;
	}

	explicit GSVector4i(const GSVector4& v)
	{
		*this = v;
	}

	void operator = (const GSVector4i& v) 
	{
		m = v.m;
	}

	void operator = (const GSVector4& v);

	void operator = (int i) 
	{
		m = _mm_set1_epi32(i);
	}

	void operator = (__m128i m) 
	{
		this->m = m;
	}

	void operator = (CRect r)
	{
		m = _mm_set_epi32(r.bottom, r.right, r.top, r.left);
	}

	operator __m128i() const 
	{
		return m;
	}

	GSVector4i sat_i8(const GSVector4i& a, const GSVector4i& b) const 
	{
		return GSVector4i(_mm_min_epi8(_mm_max_epi8(m, a), b));
	}

	GSVector4i sat_i16(const GSVector4i& a, const GSVector4i& b) const 
	{
		return GSVector4i(_mm_min_epi16(_mm_max_epi16(m, a), b));
	}

	GSVector4i sat_i32(const GSVector4i& a, const GSVector4i& b) const 
	{
		return GSVector4i(_mm_min_epi32(_mm_max_epi32(m, a), b));
	}

	GSVector4i sat_u8(const GSVector4i& a, const GSVector4i& b) const 
	{
		return GSVector4i(_mm_min_epu8(_mm_max_epu8(m, a), b));
	}

	GSVector4i sat_u16(const GSVector4i& a, const GSVector4i& b) const 
	{
		return GSVector4i(_mm_min_epu16(_mm_max_epu16(m, a), b));
	}

	GSVector4i sat_u32(const GSVector4i& a, const GSVector4i& b) const 
	{
		return GSVector4i(_mm_min_epu32(_mm_max_epu32(m, a), b));
	}

	GSVector4i blend8(const GSVector4i& a, const GSVector4i& mask) 
	{
		return GSVector4i(_mm_blendv_epi8(m, a, mask));
	}

	GSVector4i blend(const GSVector4i& a, const GSVector4i& mask) 
	{
		return GSVector4i(_mm_or_si128(_mm_andnot_si128(mask, m), _mm_and_si128(mask, a)));
	}

	GSVector4i ps32(const GSVector4i& a)
	{
		return GSVector4i(_mm_packs_epi32(m, a));
	}
	
	#if _M_SSE >= 0x400
	GSVector4i pu32(const GSVector4i& a)
	{
		return GSVector4i(_mm_packus_epi32(m, a));
	}
	#endif

	GSVector4i ps16(const GSVector4i& a)
	{
		return GSVector4i(_mm_packs_epi16(m, a));
	}

	GSVector4i pu16(const GSVector4i& a)
	{
		return GSVector4i(_mm_packus_epi16(m, a));
	}

	GSVector4i upl8(const GSVector4i& a)
	{
		return GSVector4i(_mm_unpacklo_epi8(m, a));
	}

	GSVector4i uph8(const GSVector4i& a)
	{
		return GSVector4i(_mm_unpackhi_epi8(m, a));
	}

	GSVector4i upl16(const GSVector4i& a)
	{
		return GSVector4i(_mm_unpacklo_epi16(m, a));
	}

	GSVector4i uph16(const GSVector4i& a)
	{
		return GSVector4i(_mm_unpackhi_epi16(m, a));
	}

	GSVector4i upl32(const GSVector4i& a)
	{
		return GSVector4i(_mm_unpacklo_epi32(m, a));
	}

	GSVector4i uph32(const GSVector4i& a)
	{
		return GSVector4i(_mm_unpackhi_epi32(m, a));
	}

	GSVector4i upl64(const GSVector4i& a)
	{
		return GSVector4i(_mm_unpacklo_epi64(m, a));
	}

	GSVector4i uph64(const GSVector4i& a)
	{
		return GSVector4i(_mm_unpackhi_epi64(m, a));
	}

	static GSVector4i zero() 
	{
		return GSVector4i(_mm_setzero_si128());
	}

	static GSVector4i invzero() 
	{
		return GSVector4i(0) == GSVector4i(0);
	}

	static GSVector4i loadu(const void* v)
	{
		return GSVector4i(_mm_loadu_si128((__m128i*)v));
	}

	void operator += (const GSVector4i& v) 
	{
		m = _mm_add_epi32(m, v);
	}

	void operator -= (const GSVector4i& v) 
	{
		m = _mm_sub_epi32(m, v);
	}

	void operator += (int i) 
	{
		*this += GSVector4i(i);
	}

	void operator -= (int i) 
	{
		*this -= GSVector4i(i);
	}

	void operator <<= (int i) 
	{
		m = _mm_slli_epi32(m, i);
	}

	void operator >>= (int i) 
	{
		m = _mm_srli_epi32(m, i);
	}

	void operator &= (const GSVector4i& v)
	{
		m = _mm_and_si128(m, v);
	}

	void operator |= (const GSVector4i& v) 
	{
		m = _mm_or_si128(m, v);
	}

	void operator ^= (const GSVector4i& v) 
	{
		m = _mm_xor_si128(m, v);
	}

	friend GSVector4i operator + (const GSVector4i& v1, const GSVector4i& v2)
	{
		return GSVector4i(_mm_add_epi32(v1, v2));
	}

	friend GSVector4i operator - (const GSVector4i& v1, const GSVector4i& v2) 
	{
		return GSVector4i(_mm_sub_epi32(v1, v2));
	}

	friend GSVector4i operator + (const GSVector4i& v, int i)
	{
		return v + GSVector4i(i);
	}
	
	friend GSVector4i operator - (const GSVector4i& v, int i)
	{
		return v - GSVector4i(i);
	}
	
	friend GSVector4i operator << (const GSVector4i& v, int i) 
	{
		return GSVector4i(_mm_slli_epi32(v, i));
	}
	
	friend GSVector4i operator >> (const GSVector4i& v, int i) 
	{
		return GSVector4i(_mm_srli_epi32(v, i));
	}
	
	friend GSVector4i operator & (const GSVector4i& v1, const GSVector4i& v2)
	{
		return GSVector4i(_mm_and_si128(v1, v2));
	}
	
	friend GSVector4i operator | (const GSVector4i& v1, const GSVector4i& v2)
	{
		return GSVector4i(_mm_or_si128(v1, v2));
	}
	
	friend GSVector4i operator ^ (const GSVector4i& v1, const GSVector4i& v2) 
	{
		return GSVector4i(_mm_xor_si128(v1, v2));
	}
	
	friend GSVector4i operator & (const GSVector4i& v, int i) 
	{
		return v & GSVector4i(i);
	}
	
	friend GSVector4i operator | (const GSVector4i& v, int i) 
	{
		return v | GSVector4i(i);
	}
	
	friend GSVector4i operator ^ (const GSVector4i& v, int i) 
	{
		return v ^ GSVector4i(i);
	}
	
	friend GSVector4i operator ~ (const GSVector4i& v) 
	{
		return v ^ GSVector4i::invzero();
	}

	friend GSVector4i operator == (const GSVector4i& v1, const GSVector4i& v2) 
	{
		return GSVector4i(_mm_cmpeq_epi32(v1, v2));
	}
	
	friend GSVector4i operator != (const GSVector4i& v1, const GSVector4i& v2) 
	{
		return ~(v1 == v2);
	}
	
	friend GSVector4i operator > (const GSVector4i& v1, const GSVector4i& v2) 
	{
		return GSVector4i(_mm_cmpgt_epi32(v1, v2));
	}
	
	friend GSVector4i operator < (const GSVector4i& v1, const GSVector4i& v2) 
	{
		return GSVector4i(_mm_cmplt_epi32(v1, v2));
	}
	
	friend GSVector4i operator >= (const GSVector4i& v1, const GSVector4i& v2) 
	{
		return (v1 > v2) | (v1 == v2);
	}
	
	friend GSVector4i operator <= (const GSVector4i& v1, const GSVector4i& v2) 
	{
		return (v1 < v2) | (v1 == v2);
	}

	#define VECTOR4i_SHUFFLE_4(xs, xn, ys, yn, zs, zn, ws, wn) \
		GSVector4i xs##ys##zs##ws() const {return GSVector4i(_mm_shuffle_epi32(m, _MM_SHUFFLE(wn, zn, yn, xn)));} \
		GSVector4i xs##ys##zs##ws##l() const {return GSVector4i(_mm_shufflelo_epi16(m, _MM_SHUFFLE(wn, zn, yn, xn)));} \
		GSVector4i xs##ys##zs##ws##h() const {return GSVector4i(_mm_shufflehi_epi16(m, _MM_SHUFFLE(wn, zn, yn, xn)));} \

	#define VECTOR4i_SHUFFLE_3(xs, xn, ys, yn, zs, zn) \
		VECTOR4i_SHUFFLE_4(xs, xn, ys, yn, zs, zn, x, 0) \
		VECTOR4i_SHUFFLE_4(xs, xn, ys, yn, zs, zn, y, 1) \
		VECTOR4i_SHUFFLE_4(xs, xn, ys, yn, zs, zn, z, 2) \
		VECTOR4i_SHUFFLE_4(xs, xn, ys, yn, zs, zn, w, 3) \

	#define VECTOR4i_SHUFFLE_2(xs, xn, ys, yn) \
		VECTOR4i_SHUFFLE_3(xs, xn, ys, yn, x, 0) \
		VECTOR4i_SHUFFLE_3(xs, xn, ys, yn, y, 1) \
		VECTOR4i_SHUFFLE_3(xs, xn, ys, yn, z, 2) \
		VECTOR4i_SHUFFLE_3(xs, xn, ys, yn, w, 3) \

	#define VECTOR4i_SHUFFLE_1(xs, xn) \
		VECTOR4i_SHUFFLE_2(xs, xn, x, 0) \
		VECTOR4i_SHUFFLE_2(xs, xn, y, 1) \
		VECTOR4i_SHUFFLE_2(xs, xn, z, 2) \
		VECTOR4i_SHUFFLE_2(xs, xn, w, 3) \

	VECTOR4i_SHUFFLE_1(x, 0)
	VECTOR4i_SHUFFLE_1(y, 1)
	VECTOR4i_SHUFFLE_1(z, 2)
	VECTOR4i_SHUFFLE_1(w, 3)
};

#pragma pack(pop)
