
	.686
	.model flat
	.mmx
	.xmm

	.const

	align 16
	
	__us16shufmask DB 0, 1, 4, 5, 2, 3, 6, 7, 8, 9, 12, 13, 10, 11, 14, 15
	__us8shufmask DB 0, 4, 2, 6, 8, 12, 10, 14, 1, 5, 3, 7, 9, 13, 11, 15
	__s8shufmask DB 0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15

	.code

;
; swizzling
;

punpck macro op, sd0, sd2, s1, s3, d1, d3

	movdqa					@CatStr(xmm, %d1),	@CatStr(xmm, %sd0)
	pshufd					@CatStr(xmm, %d3),	@CatStr(xmm, %sd2), 0e4h
	
	@CatStr(punpckl, op)	@CatStr(xmm, %sd0),	@CatStr(xmm, %s1)
	@CatStr(punpckh, op)	@CatStr(xmm, %d1),	@CatStr(xmm, %s1)
	@CatStr(punpckl, op)	@CatStr(xmm, %sd2),	@CatStr(xmm, %s3)
	@CatStr(punpckh, op)	@CatStr(xmm, %d3),	@CatStr(xmm, %s3)

	endm
	
punpcknb macro

	movdqa	xmm4, xmm0
	pshufd	xmm5, xmm1, 0e4h

	psllq	xmm1, 4
	psrlq	xmm4, 4

	movdqa	xmm6, xmm7
	pand	xmm0, xmm7
	pandn	xmm6, xmm1
	por		xmm0, xmm6

	movdqa	xmm6, xmm7
	pand	xmm4, xmm7
	pandn	xmm6, xmm5
	por		xmm4, xmm6

	movdqa	xmm1, xmm4

	movdqa	xmm4, xmm2
	pshufd	xmm5, xmm3, 0e4h

	psllq	xmm3, 4
	psrlq	xmm4, 4

	movdqa	xmm6, xmm7
	pand	xmm2, xmm7
	pandn	xmm6, xmm3
	por		xmm2, xmm6

	movdqa	xmm6, xmm7
	pand	xmm4, xmm7
	pandn	xmm6, xmm5
	por		xmm4, xmm6

	movdqa	xmm3, xmm4

	punpck	bw, 0, 2, 1, 3, 4, 6

	endm

;
; unSwizzleBlock32
;

;
; unSwizzleBlock16
;

@unSwizzleBlock16_x86_sse3@12 proc public
	
	push		ebx

	mov			ebx, [esp+4+4]
	mov			eax, 4
	
	movaps		xmm7, xmmword ptr __us16shufmask
	
	align 16
@@:
	movdqa		xmm0, [ecx+16*0]
	movdqa		xmm1, [ecx+16*1]
	movdqa		xmm2, [ecx+16*2]
	movdqa		xmm3, [ecx+16*3]

	pshufb		xmm0, xmm7
	pshufb		xmm1, xmm7
	pshufb		xmm2, xmm7
	pshufb		xmm3, xmm7
	
	punpck		dq, 0, 2, 1, 3, 4, 6
	punpck		qdq, 0, 4, 2, 6, 1, 3

	movdqa		[edx], xmm0
	movdqa		[edx+16], xmm1
	movdqa		[edx+ebx], xmm4
	movdqa		[edx+ebx+16], xmm3

	add			ecx, 64
	lea			edx, [edx+ebx*2]

	dec			eax
	jnz			@B
	
	pop			ebx
	
	ret			4
	
@unSwizzleBlock16_x86_sse3@12 endp

;
; unSwizzleBlock8
;

@unSwizzleBlock8_x86_sse3@12 proc public

	push		ebx

	mov			ebx, [esp+4+4]
	mov			eax, 2

	movaps		xmm7, xmmword ptr __us8shufmask

	align 16
@@:
	; col 0, 2

	movdqa		xmm0, [ecx+16*0]
	movdqa		xmm1, [ecx+16*1]
	movdqa		xmm2, [ecx+16*2]
	movdqa		xmm3, [ecx+16*3]
	
	pshufb		xmm0, xmm7
	pshufb		xmm1, xmm7
	pshufb		xmm2, xmm7
	pshufb		xmm3, xmm7

	punpck		wd, 0, 2, 1, 3, 4, 6
	punpck		dq, 0, 6, 2, 4, 1, 3

	movdqa		[edx], xmm0
	movdqa		[edx+ebx], xmm1
	lea			edx, [edx+ebx*2]

	movdqa		[edx], xmm6
	movdqa		[edx+ebx], xmm3
	lea			edx, [edx+ebx*2]

	; col 1, 3

	movdqa		xmm2, [ecx+16*4]
	movdqa		xmm3, [ecx+16*5]
	movdqa		xmm0, [ecx+16*6]
	movdqa		xmm1, [ecx+16*7]

	pshufb		xmm0, xmm7
	pshufb		xmm1, xmm7
	pshufb		xmm2, xmm7
	pshufb		xmm3, xmm7

	punpck		wd, 0, 2, 1, 3, 4, 6
	punpck		dq, 0, 6, 2, 4, 1, 3

	movdqa		[edx], xmm0
	movdqa		[edx+ebx], xmm1
	lea			edx, [edx+ebx*2]

	movdqa		[edx], xmm6
	movdqa		[edx+ebx], xmm3
	lea			edx, [edx+ebx*2]

	add			ecx, 128

	dec			eax
	jnz			@B

	pop			ebx
	
	ret			4

@unSwizzleBlock8_x86_sse3@12 endp

;
; unSwizzleBlock4
;

;
; unSwizzleBlock8HP
;

;
; unSwizzleBlock4HLP
;

;
; unSwizzleBlock4HHP
;

;
; unSwizzleBlock4P
;

;
; swizzling
;

;
; SwizzleBlock32
;

;
; SwizzleBlock16
;

;
; SwizzleBlock8
;

@SwizzleBlock8_x86_sse3@12 proc public

	push		ebx

	mov			ebx, [esp+4+4]
	mov			eax, 2

	movaps		xmm7, xmmword ptr __s8shufmask

	align 16
@@:
	; col 0, 2

	movdqa		xmm0, [edx]
	movdqa		xmm2, [edx+ebx]
	lea			edx, [edx+ebx*2]

	pshufd		xmm1, [edx], 0b1h
	pshufd		xmm3, [edx+ebx], 0b1h
	lea			edx, [edx+ebx*2]

	pshufb		xmm0, xmm7
	pshufb		xmm1, xmm7
	pshufb		xmm2, xmm7
	pshufb		xmm3, xmm7
	
	punpck		bw, 0, 2, 1, 3, 4, 6
	punpck		qdq, 0, 4, 2, 6, 1, 3

	movdqa		[ecx+16*0], xmm0
	movdqa		[ecx+16*1], xmm1
	movdqa		[ecx+16*2], xmm4
	movdqa		[ecx+16*3], xmm3

	; col 1, 3

	pshufd		xmm0, [edx], 0b1h
	pshufd		xmm2, [edx+ebx], 0b1h
	lea			edx, [edx+ebx*2]

	movdqa		xmm1, [edx]
	movdqa		xmm3, [edx+ebx]
	lea			edx, [edx+ebx*2]

	pshufb		xmm0, xmm7
	pshufb		xmm1, xmm7
	pshufb		xmm2, xmm7
	pshufb		xmm3, xmm7
	
	punpck		bw, 0, 2, 1, 3, 4, 6
	punpck		qdq, 0, 4, 2, 6, 1, 3

	movdqa		[ecx+16*4], xmm0
	movdqa		[ecx+16*5], xmm1
	movdqa		[ecx+16*6], xmm4
	movdqa		[ecx+16*7], xmm3

	add			ecx, 128

	dec			eax
	jnz			@B

	pop			ebx

	ret			4
	
@SwizzleBlock8_x86_sse3@12 endp

;
; SwizzleBlock4
;

;
; swizzling with unaligned reads
;

;
; SwizzleBlock32u
;

;
; SwizzleBlock16u
;

;
; SwizzleBlock8u
;

@SwizzleBlock8u_x86_sse3@12 proc public

	push		ebx

	mov			ebx, [esp+4+4]
	mov			eax, 2

	movaps		xmm7, xmmword ptr __s8shufmask

	align 16
@@:
	; col 0, 2

	movdqu		xmm0, [edx]
	movdqu		xmm2, [edx+ebx]
	lea			edx, [edx+ebx*2]

	movdqu		xmm1, [edx]
	movdqu		xmm3, [edx+ebx]
	pshufd		xmm1, xmm1, 0b1h
	pshufd		xmm3, xmm3, 0b1h
	lea			edx, [edx+ebx*2]

	pshufb		xmm0, xmm7
	pshufb		xmm1, xmm7
	pshufb		xmm2, xmm7
	pshufb		xmm3, xmm7
	
	punpck		bw, 0, 2, 1, 3, 4, 6
	punpck		qdq, 0, 4, 2, 6, 1, 3

	movdqa		[ecx+16*0], xmm0
	movdqa		[ecx+16*1], xmm1
	movdqa		[ecx+16*2], xmm4
	movdqa		[ecx+16*3], xmm3

	; col 1, 3

	movdqu		xmm0, [edx]
	movdqu		xmm2, [edx+ebx]
	pshufd		xmm0, xmm0, 0b1h
	pshufd		xmm2, xmm2, 0b1h
	lea			edx, [edx+ebx*2]

	movdqu		xmm1, [edx]
	movdqu		xmm3, [edx+ebx]
	lea			edx, [edx+ebx*2]

	pshufb		xmm0, xmm7
	pshufb		xmm1, xmm7
	pshufb		xmm2, xmm7
	pshufb		xmm3, xmm7
	
	punpck		bw, 0, 2, 1, 3, 4, 6
	punpck		qdq, 0, 4, 2, 6, 1, 3

	movdqa		[ecx+16*4], xmm0
	movdqa		[ecx+16*5], xmm1
	movdqa		[ecx+16*6], xmm4
	movdqa		[ecx+16*7], xmm3

	add			ecx, 128

	dec			eax
	jnz			@B

	pop			ebx

	ret			4
	
@SwizzleBlock8u_x86_sse3@12 endp

;
; SwizzleBlock4u
;

	end