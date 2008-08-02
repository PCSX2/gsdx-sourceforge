#pragma once

// NOTE: x64 version of the _mm_set_* functions are terrible, first they store components into memory then reload in one piece (VS2008 SP1)

#pragma pack(push, 1)

__declspec(align(8)) class GSVector2
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

class GSVector4;

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
		// 4 gprs
		// m = _mm_set_epi32(w, z, y, x); 

		// 2 gprs
		m = _mm_unpacklo_epi32(
			_mm_unpacklo_epi32(_mm_cvtsi32_si128(x), _mm_cvtsi32_si128(z)),
			_mm_unpacklo_epi32(_mm_cvtsi32_si128(y), _mm_cvtsi32_si128(w))
			);
	}

	GSVector4i(char b0, char b1, char b2, char b3, char b4, char b5, char b6, char b7, char b8, char b9, char b10, char b11, char b12, char b13, char b14, char b15) 
	{
		m = _mm_set_epi8(b15, b14, b13, b12, b11, b10, b9, b8, b7, b6, b5, b4, b3, b2, b1, b0);
	}

	GSVector4i(const GSVector4i& v) 
	{
		m = v.m;
	}

	explicit GSVector4i(int i) 
	{
		m = _mm_set1_epi32(i);
	}

	explicit GSVector4i(__m128i m)
	{
		this->m = m;
	}

	explicit GSVector4i(CRect r) 
	{
		m = _mm_set_epi32(r.bottom, r.right, r.top, r.left);
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

	operator CRect() const
	{
		return *(CRect*)&m;
	}

	UINT32 rgba32() const
	{
		__m128i r = m; 
		#if _M_SSE >= 0x401
		r = _mm_packus_epi32(r, r); 
		#else
		r = _mm_packs_epi32(r, r); // good enough for colors...
		#endif
		r = _mm_packus_epi16(r, r); 
		return (UINT32)_mm_cvtsi128_si32(r);
	}

	UINT64 rgba64() const
	{
		__m128i r = m; 
		#if _M_SSE >= 0x401
		r = _mm_packus_epi32(r, r); 
		#else
		r = _mm_packs_epi32(r, r); // good enough for colors...
		#endif
		#ifdef _M_AMD64
		return _mm_cvtsi128_si64(r);
		#else
		return *(UINT64*)&r;
		#endif
	}

	#if _M_SSE >= 0x401

	GSVector4i sat_i8(const GSVector4i& a, const GSVector4i& b) const 
	{
		return GSVector4i(_mm_min_epi8(_mm_max_epi8(m, a), b));
	}

	GSVector4i sat_i8(const GSVector4i& a) const 
	{
		return GSVector4i(_mm_min_epi8(_mm_max_epi8(m, a.xyxy()), a.zwzw()));
	}

	#endif

	GSVector4i sat_i16(const GSVector4i& a, const GSVector4i& b) const 
	{
		return GSVector4i(_mm_min_epi16(_mm_max_epi16(m, a), b));
	}

	GSVector4i sat_i16(const GSVector4i& a) const 
	{
		return GSVector4i(_mm_min_epi16(_mm_max_epi16(m, a.xyxy()), a.zwzw()));
	}

	#if _M_SSE >= 0x401

	GSVector4i sat_i32(const GSVector4i& a, const GSVector4i& b) const 
	{
		return GSVector4i(_mm_min_epi32(_mm_max_epi32(m, a), b));
	}

	GSVector4i sat_i32(const GSVector4i& a) const 
	{
		return GSVector4i(_mm_min_epi32(_mm_max_epi32(m, a.xyxy()), a.zwzw()));
	}

	#endif

	GSVector4i sat_u8(const GSVector4i& a, const GSVector4i& b) const 
	{
		return GSVector4i(_mm_min_epu8(_mm_max_epu8(m, a), b));
	}

	GSVector4i sat_u8(const GSVector4i& a) const 
	{
		return GSVector4i(_mm_min_epu8(_mm_max_epu8(m, a.xyxy()), a.zwzw()));
	}

	#if _M_SSE >= 0x401

	GSVector4i sat_u16(const GSVector4i& a, const GSVector4i& b) const 
	{
		return GSVector4i(_mm_min_epu16(_mm_max_epu16(m, a), b));
	}

	GSVector4i sat_u16(const GSVector4i& a) const 
	{
		return GSVector4i(_mm_min_epu16(_mm_max_epu16(m, a.xyxy()), a.zwzw()));
	}

	#endif

	#if _M_SSE >= 0x401

	GSVector4i sat_u32(const GSVector4i& a, const GSVector4i& b) const 
	{
		return GSVector4i(_mm_min_epu32(_mm_max_epu32(m, a), b));
	}

	GSVector4i sat_u32(const GSVector4i& a) const 
	{
		return GSVector4i(_mm_min_epu32(_mm_max_epu32(m, a.xyxy()), a.zwzw()));
	}

	#endif

	#if _M_SSE >= 0x401

	GSVector4i min_i8(const GSVector4i& a) const
	{
		return GSVector4i(_mm_min_epi8(m, a));
	}

	GSVector4i max_i8(const GSVector4i& a) const
	{
		return GSVector4i(_mm_max_epi8(m, a));
	}

	#endif

	GSVector4i min_i16(const GSVector4i& a) const
	{
		return GSVector4i(_mm_min_epi16(m, a));
	}

	GSVector4i max_i16(const GSVector4i& a) const
	{
		return GSVector4i(_mm_max_epi16(m, a));
	}

	#if _M_SSE >= 0x401

	GSVector4i min_i32(const GSVector4i& a) const
	{
		return GSVector4i(_mm_min_epi32(m, a));
	}

	GSVector4i max_i32(const GSVector4i& a) const
	{
		return GSVector4i(_mm_max_epi32(m, a));
	}

	#endif

	GSVector4i min_u8(const GSVector4i& a) const
	{
		return GSVector4i(_mm_min_epu8(m, a));
	}

	GSVector4i max_u8(const GSVector4i& a) const
	{
		return GSVector4i(_mm_max_epu8(m, a));
	}

	#if _M_SSE >= 0x401

	GSVector4i min_u16(const GSVector4i& a) const
	{
		return GSVector4i(_mm_min_epu16(m, a));
	}

	GSVector4i max_u16(const GSVector4i& a) const
	{
		return GSVector4i(_mm_max_epu16(m, a));
	}

	GSVector4i min_u32(const GSVector4i& a) const
	{
		return GSVector4i(_mm_min_epu32(m, a));
	}

	GSVector4i max_u32(const GSVector4i& a) const
	{
		return GSVector4i(_mm_max_epu32(m, a));
	}

	#endif

	GSVector4i blend8(const GSVector4i& a, const GSVector4i& mask) const
	{
		return GSVector4i(_mm_blendv_epi8(m, a, mask));
	}

	#if _M_SSE >= 0x401

	template<int mask> GSVector4i blend16(const GSVector4i& a) const
	{
		return GSVector4i(_mm_blend_epi16(m, a, mask));
	}

	#endif

	GSVector4i blend(const GSVector4i& a, const GSVector4i& mask) const
	{
		return GSVector4i(_mm_or_si128(_mm_andnot_si128(mask, m), _mm_and_si128(mask, a)));
	}

	#if _M_SSE >= 0x301

	GSVector4i shuffle8(const GSVector4i& mask) const
	{
		return GSVector4i(_mm_shuffle_epi8(m, mask));
	}

	#endif

	GSVector4i ps16(const GSVector4i& a) const
	{
		return GSVector4i(_mm_packs_epi16(m, a));
	}

	GSVector4i pu16(const GSVector4i& a) const
	{
		return GSVector4i(_mm_packus_epi16(m, a));
	}

	GSVector4i ps32(const GSVector4i& a) const
	{
		return GSVector4i(_mm_packs_epi32(m, a));
	}
	
	#if _M_SSE >= 0x401

	GSVector4i pu32(const GSVector4i& a) const
	{
		return GSVector4i(_mm_packus_epi32(m, a));
	}

	#endif

	GSVector4i upl8(const GSVector4i& a) const
	{
		return GSVector4i(_mm_unpacklo_epi8(m, a));
	}

	GSVector4i uph8(const GSVector4i& a) const
	{
		return GSVector4i(_mm_unpackhi_epi8(m, a));
	}

	GSVector4i upl16(const GSVector4i& a) const
	{
		return GSVector4i(_mm_unpacklo_epi16(m, a));
	}

	GSVector4i uph16(const GSVector4i& a) const
	{
		return GSVector4i(_mm_unpackhi_epi16(m, a));
	}

	GSVector4i upl32(const GSVector4i& a) const
	{
		return GSVector4i(_mm_unpacklo_epi32(m, a));
	}

	GSVector4i uph32(const GSVector4i& a) const
	{
		return GSVector4i(_mm_unpackhi_epi32(m, a));
	}

	GSVector4i upl64(const GSVector4i& a) const
	{
		return GSVector4i(_mm_unpacklo_epi64(m, a));
	}

	GSVector4i uph64(const GSVector4i& a) const
	{
		return GSVector4i(_mm_unpackhi_epi64(m, a));
	}

	GSVector4i upl8() const
	{
		return GSVector4i(_mm_unpacklo_epi8(m, _mm_setzero_si128()));
	}

	GSVector4i uph8() const
	{
		return GSVector4i(_mm_unpackhi_epi8(m, _mm_setzero_si128()));
	}

	GSVector4i upl16() const
	{
		return GSVector4i(_mm_unpacklo_epi16(m, _mm_setzero_si128()));
	}

	GSVector4i uph16() const
	{
		return GSVector4i(_mm_unpackhi_epi16(m, _mm_setzero_si128()));
	}

	GSVector4i upl32() const
	{
		return GSVector4i(_mm_unpacklo_epi32(m, _mm_setzero_si128()));
	}

	GSVector4i uph32() const
	{
		return GSVector4i(_mm_unpackhi_epi32(m, _mm_setzero_si128()));
	}

	GSVector4i upl64() const
	{
		return GSVector4i(_mm_unpacklo_epi64(m, _mm_setzero_si128()));
	}

	GSVector4i uph64() const
	{
		return GSVector4i(_mm_unpackhi_epi64(m, _mm_setzero_si128()));
	}

	template<int i> GSVector4i srl() const
	{
		#pragma warning(push)
		#pragma warning(disable: 4556)

		return GSVector4i(_mm_srli_si128(m, i));

		#pragma warning(pop)
	}

	template<int i> GSVector4i srl(const GSVector4i& v)
	{
		#if _M_SSE >= 0x301

		return GSVector4i(_mm_alignr_epi8(v.m, m, i));

		#else

		if(i < 16) return v.sll<i>() | srl<16 - i>();
		else if(i < 32) return v.srl<i - 16>();
		else return zero();

		#endif
	}

	template<int i> GSVector4i sll() const
	{
		return GSVector4i(_mm_slli_si128(m, i));
	}

	GSVector4i sra16(int i) const
	{
		return GSVector4i(_mm_srai_epi16(m, i));
	}

	GSVector4i sra32(int i) const
	{
		return GSVector4i(_mm_srai_epi32(m, i));
	}

	GSVector4i sll16(int i) const
	{
		return GSVector4i(_mm_slli_epi16(m, i));
	}

	GSVector4i sll32(int i) const
	{
		return GSVector4i(_mm_slli_epi32(m, i));
	}

	GSVector4i sll64(int i) const
	{
		return GSVector4i(_mm_slli_epi64(m, i));
	}

	GSVector4i srl16(int i) const
	{
		return GSVector4i(_mm_srli_epi16(m, i));
	}

	GSVector4i srl32(int i) const
	{
		return GSVector4i(_mm_srli_epi32(m, i));
	}

	GSVector4i srl64(int i) const
	{
		return GSVector4i(_mm_srli_epi64(m, i));
	}

	GSVector4i add8(const GSVector4i& v) const
	{
		return GSVector4i(_mm_add_epi8(m, v.m));
	}

	GSVector4i add16(const GSVector4i& v) const
	{
		return GSVector4i(_mm_add_epi16(m, v.m));
	}

	GSVector4i add32(const GSVector4i& v) const
	{
		return GSVector4i(_mm_add_epi32(m, v.m));
	}

	GSVector4i sub8(const GSVector4i& v) const
	{
		return GSVector4i(_mm_sub_epi8(m, v.m));
	}

	GSVector4i sub16(const GSVector4i& v) const
	{
		return GSVector4i(_mm_sub_epi16(m, v.m));
	}

	GSVector4i sub32(const GSVector4i& v) const
	{
		return GSVector4i(_mm_sub_epi32(m, v.m));
	}

	GSVector4i mul16hs(const GSVector4i& v) const
	{
		return GSVector4i(_mm_mulhi_epi16(m, v.m));
	}

	GSVector4i mul16hu(const GSVector4i& v) const
	{
		return GSVector4i(_mm_mulhi_epu16(m, v.m));
	}

	GSVector4i mul16l(const GSVector4i& v) const
	{
		return GSVector4i(_mm_mullo_epi16(m, v.m));
	}

	#if _M_SSE >= 0x301

	GSVector4i mul16hrs(const GSVector4i& v) const
	{
		return GSVector4i(_mm_mulhrs_epi16(m, v.m));
	}

	#endif

	GSVector4i andnot(const GSVector4i& v) const
	{
		return GSVector4i(_mm_andnot_si128(v.m, m));
	}

	int mask() const
	{
		return _mm_movemask_epi8(m);
	}

	bool alltrue() const
	{
		return _mm_movemask_epi8(m) == 0xffff;
	}

	#if _M_SSE >= 0x401

	template<int i> GSVector4i insert8(int a) const
	{
		return GSVector4i(_mm_insert_epi8(m, a, i));
	}

	#endif

	template<int i> int extract8() const
	{
		#if _M_SSE >= 0x401
		return _mm_extract_epi8(m, i);
		#else
		return (int)i8[i];
		#endif
	}

	template<int i> GSVector4i insert16(int a) const
	{
		return GSVector4i(_mm_insert_epi16(m, a, i));
	}

	template<int i> int extract16() const
	{
		return _mm_extract_epi16(m, i);
	}

	#if _M_SSE >= 0x401

	template<int i> GSVector4i insert32(int a) const
	{
		return GSVector4i(_mm_insert_epi32(m, a, i));
	}

	#endif

	template<int i> int extract32() const
	{
		#if _M_SSE >= 0x401
		return _mm_extract_epi32(m, i);
		#else
		return i32[i];
		#endif
	}

	#ifdef _M_AMD64

	#if _M_SSE >= 0x401

	template<int i> GSVector4i insert64(__int64 a) const
	{
		return GSVector4i(_mm_insert_epi64(m, a, i));
	}

	#endif

	template<int i> __int64 extract64() const
	{
		#if _M_SSE >= 0x401
		return _mm_extract_epi64(m, i);
		#else
		return i64[i];
		#endif
	}

	#endif

	#if _M_SSE >= 0x401

	template<int src, class T> __forceinline GSVector4i gather8_4(const T* ptr) const
	{
		GSVector4i v;

		v = load((int)ptr[extract8<src + 0>() & 0xf]);
		v = v.insert8<1>((int)ptr[extract8<src + 0>() >> 4]);
		v = v.insert8<2>((int)ptr[extract8<src + 1>() & 0xf]);
		v = v.insert8<3>((int)ptr[extract8<src + 1>() >> 4]);
		v = v.insert8<4>((int)ptr[extract8<src + 2>() & 0xf]);
		v = v.insert8<5>((int)ptr[extract8<src + 2>() >> 4]);
		v = v.insert8<6>((int)ptr[extract8<src + 3>() & 0xf]);
		v = v.insert8<7>((int)ptr[extract8<src + 3>() >> 4]);
		v = v.insert8<8>((int)ptr[extract8<src + 4>() & 0xf]);
		v = v.insert8<9>((int)ptr[extract8<src + 4>() >> 4]);
		v = v.insert8<10>((int)ptr[extract8<src + 5>() & 0xf]);
		v = v.insert8<11>((int)ptr[extract8<src + 5>() >> 4]);
		v = v.insert8<12>((int)ptr[extract8<src + 6>() & 0xf]);
		v = v.insert8<13>((int)ptr[extract8<src + 6>() >> 4]);
		v = v.insert8<14>((int)ptr[extract8<src + 7>() & 0xf]);
		v = v.insert8<15>((int)ptr[extract8<src + 7>() >> 4]);

		return v;
	}

	template<class T> __forceinline GSVector4i gather8_8(const T* ptr) const
	{
		GSVector4i v;

		v = load((int)ptr[extract8<0>()]);
		v = v.insert8<1>((int)ptr[extract8<1>()]);
		v = v.insert8<2>((int)ptr[extract8<2>()]);
		v = v.insert8<3>((int)ptr[extract8<3>()]);
		v = v.insert8<4>((int)ptr[extract8<4>()]);
		v = v.insert8<5>((int)ptr[extract8<5>()]);
		v = v.insert8<6>((int)ptr[extract8<6>()]);
		v = v.insert8<7>((int)ptr[extract8<7>()]);
		v = v.insert8<8>((int)ptr[extract8<8>()]);
		v = v.insert8<9>((int)ptr[extract8<9>()]);
		v = v.insert8<10>((int)ptr[extract8<10>()]);
		v = v.insert8<11>((int)ptr[extract8<11>()]);
		v = v.insert8<12>((int)ptr[extract8<12>()]);
		v = v.insert8<13>((int)ptr[extract8<13>()]);
		v = v.insert8<14>((int)ptr[extract8<14>()]);
		v = v.insert8<15>((int)ptr[extract8<15>()]);

		return v;
	}

	template<int dst, class T> __forceinline GSVector4i gather8_16(const T* ptr, const GSVector4i& a) const
	{
		GSVector4i v = a;

		v = v.insert8<dst + 0>((int)ptr[extract16<0>()]);
		v = v.insert8<dst + 1>((int)ptr[extract16<1>()]);
		v = v.insert8<dst + 2>((int)ptr[extract16<2>()]);
		v = v.insert8<dst + 3>((int)ptr[extract16<3>()]);
		v = v.insert8<dst + 4>((int)ptr[extract16<4>()]);
		v = v.insert8<dst + 5>((int)ptr[extract16<5>()]);
		v = v.insert8<dst + 6>((int)ptr[extract16<6>()]);
		v = v.insert8<dst + 7>((int)ptr[extract16<7>()]);

		return v;
	}

	template<int dst, class T> __forceinline GSVector4i gather8_32(const T* ptr, const GSVector4i& a) const
	{
		GSVector4i v = a;

		v = v.insert8<dst + 0>((int)ptr[extract32<0>()]);
		v = v.insert8<dst + 1>((int)ptr[extract32<1>()]);
		v = v.insert8<dst + 2>((int)ptr[extract32<2>()]);
		v = v.insert8<dst + 3>((int)ptr[extract32<3>()]);

		return v;
	}

	template<int src, class T> __forceinline GSVector4i gather16_4(const T* ptr) const
	{
		GSVector4i v;

		v = load((int)ptr[extract8<src + 0>() & 0xf]);
		v = v.insert16<1>((int)ptr[extract8<src + 0>() >> 4]);
		v = v.insert16<2>((int)ptr[extract8<src + 1>() & 0xf]);
		v = v.insert16<3>((int)ptr[extract8<src + 1>() >> 4]);
		v = v.insert16<4>((int)ptr[extract8<src + 2>() & 0xf]);
		v = v.insert16<5>((int)ptr[extract8<src + 2>() >> 4]);
		v = v.insert16<6>((int)ptr[extract8<src + 3>() & 0xf]);
		v = v.insert16<7>((int)ptr[extract8<src + 3>() >> 4]);

		return v;
	}

	template<int src, class T> __forceinline GSVector4i gather16_8(const T* ptr) const
	{
		GSVector4i v;

		v = load((int)ptr[extract8<src + 0>()]);
		v = v.insert16<1>((int)ptr[extract8<src + 1>()]);
		v = v.insert16<2>((int)ptr[extract8<src + 2>()]);
		v = v.insert16<3>((int)ptr[extract8<src + 3>()]);
		v = v.insert16<4>((int)ptr[extract8<src + 4>()]);
		v = v.insert16<5>((int)ptr[extract8<src + 5>()]);
		v = v.insert16<6>((int)ptr[extract8<src + 6>()]);
		v = v.insert16<7>((int)ptr[extract8<src + 7>()]);

		return v;
	}

	template<class T>__forceinline GSVector4i gather16_16(const T* ptr) const
	{
		GSVector4i v;

		v = load((int)ptr[extract16<0>()]);
		v = v.insert16<1>((int)ptr[extract16<1>()]);
		v = v.insert16<2>((int)ptr[extract16<2>()]);
		v = v.insert16<3>((int)ptr[extract16<3>()]);
		v = v.insert16<4>((int)ptr[extract16<4>()]);
		v = v.insert16<5>((int)ptr[extract16<5>()]);
		v = v.insert16<6>((int)ptr[extract16<6>()]);
		v = v.insert16<7>((int)ptr[extract16<7>()]);

		return v;
	}

	template<int dst, class T> __forceinline GSVector4i gather16_32(const T* ptr, const GSVector4i& a) const
	{
		GSVector4i v = a;

		v = v.insert16<dst + 0>((int)ptr[extract32<0>()]);
		v = v.insert16<dst + 1>((int)ptr[extract32<1>()]);
		v = v.insert16<dst + 2>((int)ptr[extract32<2>()]);
		v = v.insert16<dst + 3>((int)ptr[extract32<3>()]);

		return v;
	}

	template<int src, class T> __forceinline GSVector4i gather32_4(const T* ptr) const
	{
		GSVector4i v;

		v = load((int)ptr[extract8<src + 0>() & 0xf]);
		v = v.insert32<1>((int)ptr[extract8<src + 0>() >> 4]);
		v = v.insert32<2>((int)ptr[extract8<src + 1>() & 0xf]);
		v = v.insert32<3>((int)ptr[extract8<src + 1>() >> 4]);
		return v;
	}

	template<int src, class T> __forceinline GSVector4i gather32_8(const T* ptr) const
	{
		GSVector4i v;

		v = load((int)ptr[extract8<src + 0>()]);
		v = v.insert32<1>((int)ptr[extract8<src + 1>()]);
		v = v.insert32<2>((int)ptr[extract8<src + 2>()]);
		v = v.insert32<3>((int)ptr[extract8<src + 3>()]);

		return v;
	}

	template<int src, class T> __forceinline GSVector4i gather32_16(const T* ptr) const
	{
		GSVector4i v;

		v = load((int)ptr[extract16<src + 0>()]);
		v = v.insert32<1>((int)ptr[extract16<src + 1>()]);
		v = v.insert32<2>((int)ptr[extract16<src + 2>()]);
		v = v.insert32<3>((int)ptr[extract16<src + 3>()]);

		return v;
	}

	template<class T> __forceinline GSVector4i gather32_32(const T* ptr) const
	{
		GSVector4i v;

		v = load((int)ptr[extract32<0>()]);
		v = v.insert32<1>((int)ptr[extract32<1>()]);
		v = v.insert32<2>((int)ptr[extract32<2>()]);
		v = v.insert32<3>((int)ptr[extract32<3>()]);

		return v;
	}

	#ifdef _M_AMD64

	template<int src, class T> __forceinline GSVector4i gather64_4(const T* ptr) const
	{
		GSVector4i v;

		v = loadq((__int64)ptr[extract8<src + 0>() & 0xf]);
		v = v.insert64<1>((__int64)ptr[extract8<src + 0>() >> 4]);

		return v;
	}

	template<int src, class T> __forceinline GSVector4i gather64_8(const T* ptr) const
	{
		GSVector4i v;

		v = loadq((__int64)ptr[extract8<src + 0>()]);
		v = v.insert64<1>((__int64)ptr[extract8<src + 1>()]);

		return v;
	}

	template<int src, class T> __forceinline GSVector4i gather64_16(const T* ptr) const
	{
		GSVector4i v;

		v = loadq((__int64)ptr[extract16<src + 0>()]);
		v = v.insert64<1>((__int64)ptr[extract16<src + 1>()]);

		return v;
	}

	template<int src, class T> __forceinline GSVector4i gather64_32(const T* ptr) const
	{
		GSVector4i v;

		v = loadq((__int64)ptr[extract32<src + 0>()]);
		v = v.insert64<1>((__int64)ptr[extract32<src + 1>()]);

		return v;
	}

	template<class T> __forceinline GSVector4i gather64_64(const T* ptr) const
	{
		GSVector4i v;

		v = loadq((__int64)ptr[extract64<0>()]);
		v = v.insert64<1>((__int64)ptr[extract64<1>()]);

		return v;
	}

	#else

	template<int src, class T> __forceinline GSVector4i gather64_4(const T* ptr) const
	{
		GSVector4i v;

		v = loadu(&ptr[extract8<src + 0>() & 0xf], &ptr[extract8<src + 0>() >> 4]);

		return v;
	}

	template<int src, class T> __forceinline GSVector4i gather64_8(const T* ptr) const
	{
		GSVector4i v;

		v = load(&ptr[extract8<src + 0>()], &ptr[extract8<src + 1>()]);

		return v;
	}

	template<int src, class T> __forceinline GSVector4i gather64_16(const T* ptr) const
	{
		GSVector4i v;

		v = load(&ptr[extract16<src + 0>()], &ptr[extract16<src + 1>()]);

		return v;
	}

	template<int src, class T> __forceinline GSVector4i gather64_32(const T* ptr) const
	{
		GSVector4i v;

		v = load(&ptr[extract32<src + 0>()], &ptr[extract32<src + 1>()]);

		return v;
	}

	#endif

	template<class T> __forceinline void gather8_4(const T* RESTRICT ptr, GSVector4i* RESTRICT dst) const
	{
		dst[0] = gather8_4<0>(ptr);
		dst[1] = gather8_4<8>(ptr);
	}

	__forceinline void gather8_8(const BYTE* RESTRICT ptr, GSVector4i* RESTRICT dst) const
	{
		dst[0] = gather8_8<>(ptr);
	}

	template<class T> __forceinline void gather16_4(const T* RESTRICT ptr, GSVector4i* RESTRICT dst) const
	{
		dst[0] = gather16_4<0>(ptr);
		dst[1] = gather16_4<4>(ptr);
		dst[2] = gather16_4<8>(ptr);
		dst[3] = gather16_4<12>(ptr);
	}

	template<class T> __forceinline void gather16_8(const T* RESTRICT ptr, GSVector4i* RESTRICT dst) const
	{
		dst[0] = gather16_8<0>(ptr);
		dst[1] = gather16_8<8>(ptr);
	}

	template<class T> __forceinline void gather16_16(const T* RESTRICT ptr, GSVector4i* RESTRICT dst) const
	{
		dst[0] = gather16_16<>(ptr);
	}

	template<class T> __forceinline void gather32_4(const T* RESTRICT ptr, GSVector4i* RESTRICT dst) const
	{
		dst[0] = gather32_4<0>(ptr);
		dst[1] = gather32_4<2>(ptr);
		dst[2] = gather32_4<4>(ptr);
		dst[3] = gather32_4<6>(ptr);
		dst[4] = gather32_4<8>(ptr);
		dst[5] = gather32_4<10>(ptr);
		dst[6] = gather32_4<12>(ptr);
		dst[7] = gather32_4<14>(ptr);
	}

	template<class T> __forceinline void gather32_8(const T* RESTRICT ptr, GSVector4i* RESTRICT dst) const
	{
		dst[0] = gather32_8<0>(ptr);
		dst[1] = gather32_8<4>(ptr);
		dst[2] = gather32_8<8>(ptr);
		dst[3] = gather32_8<12>(ptr);
	}

	template<class T> __forceinline void gather32_16(const T* RESTRICT ptr, GSVector4i* RESTRICT dst) const
	{
		dst[0] = gather32_16<0>(ptr);
		dst[1] = gather32_16<4>(ptr);
	}

	template<class T> __forceinline void gather32_32(const T* RESTRICT ptr, GSVector4i* RESTRICT dst) const
	{
		dst[0] = gather32_32<>(ptr);
	}

	template<class T> __forceinline void gather64_4(const T* RESTRICT ptr, GSVector4i* RESTRICT dst) const
	{
		dst[0] = gather64_4<0>(ptr);
		dst[1] = gather64_4<1>(ptr);
		dst[2] = gather64_4<2>(ptr);
		dst[3] = gather64_4<3>(ptr);
		dst[4] = gather64_4<4>(ptr);
		dst[5] = gather64_4<5>(ptr);
		dst[6] = gather64_4<6>(ptr);
		dst[7] = gather64_4<7>(ptr);
		dst[8] = gather64_4<8>(ptr);
		dst[9] = gather64_4<9>(ptr);
		dst[10] = gather64_4<10>(ptr);
		dst[11] = gather64_4<11>(ptr);
		dst[12] = gather64_4<12>(ptr);
		dst[13] = gather64_4<13>(ptr);
		dst[14] = gather64_4<14>(ptr);
		dst[15] = gather64_4<15>(ptr);
	}

	template<class T> __forceinline void gather64_8(const T* RESTRICT ptr, GSVector4i* RESTRICT dst) const
	{
		dst[0] = gather64_8<0>(ptr);
		dst[1] = gather64_8<2>(ptr);
		dst[2] = gather64_8<4>(ptr);
		dst[3] = gather64_8<6>(ptr);
		dst[4] = gather64_8<8>(ptr);
		dst[5] = gather64_8<10>(ptr);
		dst[6] = gather64_8<12>(ptr);
		dst[7] = gather64_8<14>(ptr);
	}

	template<class T> __forceinline void gather64_16(const T* RESTRICT ptr, GSVector4i* RESTRICT dst) const
	{
		dst[0] = gather64_16<0>(ptr);
		dst[1] = gather64_16<2>(ptr);
		dst[2] = gather64_16<4>(ptr);
		dst[3] = gather64_16<8>(ptr);
	}

	template<class T> __forceinline void gather64_32(const T* RESTRICT ptr, GSVector4i* RESTRICT dst) const
	{
		dst[0] = gather64_32<0>(ptr);
		dst[1] = gather64_32<2>(ptr);
	}

	#ifdef _M_AMD64

	template<class T> __forceinline void gather64_64(const T* RESTRICT ptr, GSVector4i* RESTRICT dst) const
	{
		dst[0] = gather64_64<>(ptr);
	}

	#endif

	#endif

	static GSVector4i zero() 
	{
		return GSVector4i(_mm_setzero_si128());
	}

	static GSVector4i invzero() 
	{
		return zero() == zero();
	}

	static GSVector4i invzero(const GSVector4i& v) 
	{
		// - vc can't generate a simple pxor xmm0, xmm0 / pcmpeqd xmm0, xmm0 
		// - it is better if we just use whatever register there is at the moment, instead of loading the result of _mm_setzero_si128 from memory stored somewhere into a temp earlier (we could also load the final value then...)

		return v == v; 
	}

	static GSVector4i one(const GSVector4i& v)
	{
		return invzero(v).srl32(31);
	}

	static GSVector4i x0001(const GSVector4i& v)
	{
		return invzero(v).srl16(15);
	}

	static GSVector4i x000000ff(const GSVector4i& v)
	{
		return invzero(v).srl32(24);
	}

	static GSVector4i x80000000(const GSVector4i& v)
	{
		return invzero(v).sll32(31);
	}

	static GSVector4i xff000000(const GSVector4i& v)
	{
		return invzero(v).sll32(24);
	}

	static GSVector4i x00ffffff(const GSVector4i& v)
	{
		return invzero(v).srl32(8);
	}

	static GSVector4i x00003fff(const GSVector4i& v)
	{
		return invzero(v).srl32(18);
	}

	#if _M_SSE >= 0x401

	static GSVector4i loadnt(const void* p)
	{
		return GSVector4i(_mm_stream_load_si128((__m128i*)p));
	}

	#endif

	static GSVector4i loadl(const void* p)
	{
		return GSVector4i(_mm_loadl_epi64((__m128i*)p));
	}

	static GSVector4i loadh(const void* p)
	{
		return GSVector4i(_mm_castps_si128(_mm_loadh_pi(_mm_setzero_ps(), (__m64*)p)));
	}

	static GSVector4i loadh(const void* p, const GSVector4i& v)
	{
		return GSVector4i(_mm_castps_si128(_mm_loadh_pi(_mm_castsi128_ps(v.m), (__m64*)p)));
	}

	static GSVector4i load(const void* pl, const void* ph)
	{
		return loadh(ph, loadl(pl));
	}
/*
	static GSVector4i load(const void* pl, const void* ph)
	{
		__m128i lo = _mm_loadl_epi64((__m128i*)pl);
		__m128i hi = _mm_loadl_epi64((__m128i*)ph);

		return GSVector4i(_mm_unpacklo_epi64(lo, hi));
	}
*/
	template<bool aligned> static GSVector4i load(const void* p)
	{
		return GSVector4i(aligned ? _mm_load_si128((__m128i*)p) : _mm_loadu_si128((__m128i*)p));
	}

	static GSVector4i load(int i)
	{
		return GSVector4i(_mm_cvtsi32_si128(i));
	}

	#ifdef _M_AMD64

	static GSVector4i loadq(__int64 i)
	{
		return GSVector4i(_mm_cvtsi64_si128(i));
	}

	#endif

	static void storent(void* p, const GSVector4i& v)
	{
		_mm_stream_si128((__m128i*)p, v.m);
	}

	static void storel(void* p, const GSVector4i& v)
	{
		_mm_storel_epi64((__m128i*)p, v.m);
	}

	static void storeh(void* p, const GSVector4i& v)
	{
		_mm_storeh_pi((__m64*)p, _mm_castsi128_ps(v.m));
	}

	static void store(void* pl, void* ph, const GSVector4i& v)
	{
		GSVector4i::storel(pl, v);
		GSVector4i::storeh(ph, v);
	}

	template<bool aligned> static void store(void* p, const GSVector4i& v)
	{
		if(aligned) _mm_store_si128((__m128i*)p, v.m);
		else _mm_storeu_si128((__m128i*)p, v.m);
	}

	static int store(const GSVector4i& v)
	{
		return _mm_cvtsi128_si32(v.m);
	}

	#ifdef _M_AMD64

	static __int64 storeq(const GSVector4i& v)
	{
		return _mm_cvtsi128_si64(v.m);
	}

	#endif

	__forceinline static void transpose(GSVector4i& a, GSVector4i& b, GSVector4i& c, GSVector4i& d)
	{
		_MM_TRANSPOSE4_SI128(a.m, b.m, c.m, d.m);
	}

	__forceinline static void sw4(GSVector4i& a, GSVector4i& b, GSVector4i& c, GSVector4i& d)
	{
		const __m128i epi32_0f0f0f0f = _mm_set1_epi32(0x0f0f0f0f);

		GSVector4i mask(epi32_0f0f0f0f);

		GSVector4i e = (b << 4).blend(a, mask);
		GSVector4i f = b.blend(a >> 4, mask);
		GSVector4i g = (d << 4).blend(c, mask);
		GSVector4i h = d.blend(c >> 4, mask);

		a = e.upl8(f);
		c = e.uph8(f);
		b = g.upl8(h);
		d = g.uph8(h);
	}

	__forceinline static void sw8(GSVector4i& a, GSVector4i& b, GSVector4i& c, GSVector4i& d)
	{
		GSVector4i e = a;
		GSVector4i f = c;

		a = e.upl8(b);
		c = e.uph8(b);
		b = f.upl8(d);
		d = f.uph8(d);
	}

	__forceinline static void sw16(GSVector4i& a, GSVector4i& b, GSVector4i& c, GSVector4i& d)
	{
		GSVector4i e = a;
		GSVector4i f = c;

		a = e.upl16(b);
		c = e.uph16(b);
		b = f.upl16(d);
		d = f.uph16(d);
	}

	__forceinline static void sw16rl(GSVector4i& a, GSVector4i& b, GSVector4i& c, GSVector4i& d)
	{
		GSVector4i e = a;
		GSVector4i f = c;

		a = b.upl16(e);
		c = e.uph16(b);
		b = d.upl16(f);
		d = f.uph16(d);
	}

	__forceinline static void sw16rh(GSVector4i& a, GSVector4i& b, GSVector4i& c, GSVector4i& d)
	{
		GSVector4i e = a;
		GSVector4i f = c;

		a = e.upl16(b);
		c = b.uph16(e);
		b = f.upl16(d);
		d = d.uph16(f);
	}

	__forceinline static void sw32(GSVector4i& a, GSVector4i& b, GSVector4i& c, GSVector4i& d)
	{
		GSVector4i e = a;
		GSVector4i f = c;

		a = e.upl32(b);
		c = e.uph32(b);
		b = f.upl32(d);
		d = f.uph32(d);
	}

	__forceinline static void sw64(GSVector4i& a, GSVector4i& b, GSVector4i& c, GSVector4i& d)
	{
		GSVector4i e = a;
		GSVector4i f = c;

		a = e.upl64(b);
		c = e.uph64(b);
		b = f.upl64(d);
		d = f.uph64(d);
	}

	__forceinline static bool compare(const void* dst, const void* src, int size)
	{
		ASSERT((size & 15) == 0);

		size >>= 4;

		GSVector4i* s = (GSVector4i*)src;
		GSVector4i* d = (GSVector4i*)dst;

		GSVector4i v = GSVector4i::invzero();

		for(int i = 0; i < size; i++)
		{
			v &= d[i] == s[i];
		}

		return v.alltrue();
	}

	__forceinline static bool update(const void* dst, const void* src, int size)
	{
		ASSERT((size & 15) == 0);

		size >>= 4;

		GSVector4i* s = (GSVector4i*)src;
		GSVector4i* d = (GSVector4i*)dst;

		GSVector4i v = GSVector4i::invzero();

		for(int i = 0; i < size; i++)
		{
			v &= d[i] == s[i];

			d[i] = s[i];
		}

		return v.alltrue();
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

	void operator <<= (const int i) 
	{
		m = _mm_slli_epi32(m, i);
	}

	void operator >>= (const int i) 
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
	
	friend GSVector4i operator << (const GSVector4i& v, const int i) 
	{
		return GSVector4i(_mm_slli_epi32(v, i));
	}
	
	friend GSVector4i operator >> (const GSVector4i& v, const int i) 
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
		return v ^ (v == v);
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

	template<int i> GSVector4i shuffle() const
	{
		return GSVector4i(_mm_shuffle_epi32(m, _MM_SHUFFLE(i, i, i, i)));
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
		GSVector4i xs##4##() const {return GSVector4i(_mm_shuffle_epi32(m, _MM_SHUFFLE(xn, xn, xn, xn)));} \
		GSVector4i xs##4##l() const {return GSVector4i(_mm_shufflelo_epi16(m, _MM_SHUFFLE(xn, xn, xn, xn)));} \
		GSVector4i xs##4##h() const {return GSVector4i(_mm_shufflehi_epi16(m, _MM_SHUFFLE(xn, xn, xn, xn)));} \
		VECTOR4i_SHUFFLE_2(xs, xn, x, 0) \
		VECTOR4i_SHUFFLE_2(xs, xn, y, 1) \
		VECTOR4i_SHUFFLE_2(xs, xn, z, 2) \
		VECTOR4i_SHUFFLE_2(xs, xn, w, 3) \

	VECTOR4i_SHUFFLE_1(x, 0)
	VECTOR4i_SHUFFLE_1(y, 1)
	VECTOR4i_SHUFFLE_1(z, 2)
	VECTOR4i_SHUFFLE_1(w, 3)
};

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

	GSVector4(int x, int y, int z, int w)
	{
		m = _mm_cvtepi32_ps(_mm_set_epi32(w, z, y, x));
	}

	GSVector4(const GSVector4& v) 
	{
		m = v.m;
	}

	explicit GSVector4(const GSVector2& v)
	{
		m = _mm_castsi128_ps(_mm_loadl_epi64((__m128i*)&v));
	}

	explicit GSVector4(float f)
	{
		m = _mm_set1_ps(f);
	}

	explicit GSVector4(__m128 m)
	{
		this->m = m;
	}

	explicit GSVector4(CRect r)
	{
		m = _mm_set_ps((float)r.bottom, (float)r.right, (float)r.top, (float)r.left);
	}

	explicit GSVector4(DWORD dw)
	{
		m = _mm_cvtepi32_ps(_mm_cvtepu8_epi32(_mm_cvtsi32_si128(dw)));
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

	operator __m128() const 
	{
		return m;
	}

	UINT32 rgba32() const
	{
		return GSVector4i(*this).rgba32();
	}

	UINT64 rgba64() const
	{
		return GSVector4i(*this).rgba64();
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

	GSVector4 rcpnr() const 
	{
		return GSVector4(_mm_rcpnr_ps(m));
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

	GSVector4 hadd() const
	{
		#if _M_SSE >= 0x300
		return GSVector4(_mm_hadd_ps(m, m));
		#else
		return xzxz() + ywyw();
		#endif
	}

	GSVector4 hadd(const GSVector4& v) const
	{
		#if _M_SSE >= 0x300
		return GSVector4(_mm_hadd_ps(m, v.m));
		#else
		return xzxz(v) + ywyw(v);
		#endif
	}

	#if _M_SSE >= 0x401
	template<int i> GSVector4 dp(const GSVector4& v) const 
	{
		return GSVector4(_mm_dp_ps(m, v.m, i));
	}
	#endif

	GSVector4 sat(const GSVector4& a, const GSVector4& b) const 
	{
		return GSVector4(_mm_min_ps(_mm_max_ps(m, a), b));
	}

	GSVector4 sat(const GSVector4& a) const 
	{
		return GSVector4(_mm_min_ps(_mm_max_ps(m, a.xyxy()), a.zwzw()));
	}

	GSVector4 sat(const float scale = 255) const 
	{
		return sat(zero(), GSVector4(scale));
	}

	GSVector4 minv(const GSVector4& a) const
	{
		return GSVector4(_mm_min_ps(m, a));
	}

	GSVector4 maxv(const GSVector4& a) const
	{
		return GSVector4(_mm_max_ps(m, a));
	}

	GSVector4 blend8(const GSVector4& a, const GSVector4& mask)  const
	{
		return GSVector4(_mm_blendv_ps(m, a, mask));
	}

	GSVector4 upl(const GSVector4& a) const
	{
		return GSVector4(_mm_unpacklo_ps(m, a));
	}

	GSVector4 uph(const GSVector4& a) const
	{
		return GSVector4(_mm_unpackhi_ps(m, a));
	}

	GSVector4 l2h(const GSVector4& a) const
	{
		return GSVector4(_mm_movelh_ps(m, a));
	}	

	GSVector4 h2l(const GSVector4& a) const
	{
		return GSVector4(_mm_movehl_ps(m, a));
	}	

	GSVector4 andnot(const GSVector4& v) const
	{
		return GSVector4(_mm_andnot_ps(v.m, m));
	}

	int mask() const
	{
		return _mm_movemask_ps(m);
	}

	bool alltrue() const
	{
		return _mm_movemask_ps(m) == 0xf;
	}

	static GSVector4 zero() 
	{
		return GSVector4(_mm_setzero_ps());
	}

	static GSVector4 loadl(const void* p)
	{
		return GSVector4(_mm_castpd_ps(_mm_load_sd((double*)p)));
	}

	template<bool aligned> static GSVector4 load(const void* p)
	{
		return GSVector4i(aligned ? _mm_load_ps((__m128*)p) : _mm_loadu_ps((__m128*)p));
	}

	static void storel(void* p, const GSVector4& v)
	{
		_mm_store_sd((double*)p, _mm_castps_pd(v.m));
	}

	template<bool aligned> static void store(void* p, const GSVector4& v)
	{
		if(aligned) _mm_store_ps((__m128*)p, v.m);
		else _mm_storeu_ps((__m128*)p, v.m);
	}

	__forceinline static void expand(const GSVector4i& v, GSVector4& a, GSVector4& b, GSVector4& c, GSVector4& d)
	{
		GSVector4i mask = GSVector4i::x000000ff(v);

		a = v & mask;
		b = (v >> 8) & mask;
		c = (v >> 16) & mask;
		d = (v >> 24);
	}

	__forceinline static void transpose(GSVector4& a, GSVector4& b, GSVector4& c, GSVector4& d)
	{
		GSVector4 v0 = a.xyxy(b);
		GSVector4 v1 = c.xyxy(d);
		GSVector4 v2 = a.zwzw(b);
		GSVector4 v3 = c.zwzw(d);

		a = v0.xzxz(v1);
		b = v0.ywyw(v1);
		c = v2.xzxz(v3);
		d = v2.ywyw(v3);
/*
		GSVector4 v0 = a.upl(b);
		GSVector4 v1 = a.uph(b);
		GSVector4 v2 = c.upl(d);
		GSVector4 v3 = c.uph(d);

		a = v0.l2h(v2);
		b = v2.h2l(v0);
		c = v1.l2h(v3);
		d = v3.h2l(v1);
*/	}

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

	void operator &= (const GSVector4& v)
	{
		m = _mm_and_ps(m, v);
	}

	void operator |= (const GSVector4& v) 
	{
		m = _mm_or_ps(m, v);
	}

	void operator ^= (const GSVector4& v) 
	{
		m = _mm_xor_ps(m, v);
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

	friend GSVector4 operator & (const GSVector4& v1, const GSVector4& v2)
	{
		return GSVector4(_mm_and_ps(v1, v2));
	}
	
	friend GSVector4 operator | (const GSVector4& v1, const GSVector4& v2)
	{
		return GSVector4(_mm_or_ps(v1, v2));
	}
	
	friend GSVector4 operator ^ (const GSVector4& v1, const GSVector4& v2) 
	{
		return GSVector4(_mm_xor_ps(v1, v2));
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

	template<int i> GSVector4 shuffle() const
	{
		return GSVector4(_mm_shuffle_ps(m, m, _MM_SHUFFLE(i, i, i, i)));
	}

	#define VECTOR4_SHUFFLE_4(xs, xn, ys, yn, zs, zn, ws, wn) \
		GSVector4 xs##ys##zs##ws() const {return GSVector4(_mm_shuffle_ps(m, m, _MM_SHUFFLE(wn, zn, yn, xn)));} \
		GSVector4 xs##ys##zs##ws(const GSVector4& v) const {return GSVector4(_mm_shuffle_ps(m, v.m, _MM_SHUFFLE(wn, zn, yn, xn)));} \

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
		GSVector4 xs##4##() const {return GSVector4(_mm_shuffle_ps(m, m, _MM_SHUFFLE(xn, xn, xn, xn)));} \
		GSVector4 xs##4##(const GSVector4& v) const {return GSVector4(_mm_shuffle_ps(m, v.m, _MM_SHUFFLE(xn, xn, xn, xn)));} \
		VECTOR4_SHUFFLE_2(xs, xn, x, 0) \
		VECTOR4_SHUFFLE_2(xs, xn, y, 1) \
		VECTOR4_SHUFFLE_2(xs, xn, z, 2) \
		VECTOR4_SHUFFLE_2(xs, xn, w, 3) \

	VECTOR4_SHUFFLE_1(x, 0)
	VECTOR4_SHUFFLE_1(y, 1)
	VECTOR4_SHUFFLE_1(z, 2)
	VECTOR4_SHUFFLE_1(w, 3)
};

#pragma pack(pop)
