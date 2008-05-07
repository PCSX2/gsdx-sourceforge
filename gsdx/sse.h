#pragma once

#if !defined(_M_SSE) && (defined(_M_AMD64) || defined(_M_IX86_FP) && _M_IX86_FP >= 2)

	#define _M_SSE 0x200

#endif

// sse2

#if _M_SSE >= 0x200

	#include <xmmintrin.h>
	#include <emmintrin.h>

	#if  _MSC_VER < 1500

	__forceinline __m128i _mm_castps_si128(__m128 r) {return *(__m128i*)&r;}
	__forceinline __m128 _mm_castsi128_ps(__m128i r) {return *(__m128*)&r;}

	#endif

	__forceinline __m128 _mm_neg_ps(__m128 r)
	{
		const __m128 _80000000 = _mm_castsi128_ps(_mm_set1_epi32(0x80000000));

		r = _mm_xor_ps(_80000000, r);
		
		return r;
	}

	__forceinline __m128 _mm_abs_ps(__m128 r)
	{
		const __m128 _7fffffff = _mm_castsi128_ps(_mm_set1_epi32(0x7fffffff));
		
		r = _mm_and_ps(_7fffffff, r);
		
		return r;
	}

#endif

// sse3

#if _M_SSE >= 0x300

	#include <tmmintrin.h>

#endif

// sse4

#if _M_SSE >= 0x400

	#include <smmintrin.h>

#else

	#define _mm_cvtepu8_epi32(a) _mm_unpacklo_epi16(_mm_unpacklo_epi8(a, _mm_setzero_si128()), _mm_setzero_si128())

	// not an equal replacement for sse4's blend but for our needs it is ok

	#define _mm_blendv_ps(a, b, mask) _mm_or_ps(_mm_andnot_ps(mask, a), _mm_and_ps(mask, b))
	#define _mm_blendv_epi8(a, b, mask) _mm_or_si128(_mm_andnot_si128(mask, a), _mm_and_si128(mask, b))

	__forceinline __m128 _mm_round_ps(__m128 x)
	{
		const __m128 _80000000 = _mm_castsi128_ps(_mm_set1_epi32(0x80000000));
		const __m128 _4b000000 = _mm_castsi128_ps(_mm_set1_epi32(0x4b000000));

		__m128 t = _mm_or_ps(_mm_and_ps(_80000000, x), _4b000000);

		return _mm_sub_ps(_mm_add_ps(x, t), t);
	}

	__forceinline __m128 _mm_floor_ps(__m128 x)
	{
		const __m128 _3f800000 = _mm_castsi128_ps(_mm_set1_epi32(0x3f800000));

		__m128 t = _mm_round_ps(x);

		return _mm_sub_ps(t, _mm_and_ps(_mm_cmplt_ps(x, t), _3f800000));
	}

	__forceinline __m128 _mm_ceil_ps(__m128 x)
	{
		const __m128 _3f800000 = _mm_castsi128_ps(_mm_set1_epi32(0x3f800000));

		__m128 t = _mm_round_ps(x);

		return _mm_add_ps(t, _mm_and_ps(_mm_cmpgt_ps(x, t), _3f800000));
	}

#endif
