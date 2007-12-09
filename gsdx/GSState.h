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

#include "GS.h"
#include "GSLocalMemory.h"
#include "GSDrawingContext.h"
#include "GSDrawingEnvironment.h"
#include "GSVertex.h"
#include "GSVertexList.h"
#include "GSUtil.h"
#include "GSDirtyRect.h"
#include "GSPerfMon.h"
#include "GSScale.h"

class GSState
{
	typedef void (GSState::*GIFPackedRegHandler)(GIFPackedReg* r);

	GIFPackedRegHandler m_fpGIFPackedRegHandlers[16];

	void GIFPackedRegHandlerNull(GIFPackedReg* r);
	void GIFPackedRegHandlerPRIM(GIFPackedReg* r);
	void GIFPackedRegHandlerRGBA(GIFPackedReg* r);
	void GIFPackedRegHandlerSTQ(GIFPackedReg* r);
	void GIFPackedRegHandlerUV(GIFPackedReg* r);
	void GIFPackedRegHandlerXYZF2(GIFPackedReg* r);
	void GIFPackedRegHandlerXYZ2(GIFPackedReg* r);
	template<int i> void GIFPackedRegHandlerTEX0(GIFPackedReg* r);
	template<int i> void GIFPackedRegHandlerCLAMP(GIFPackedReg* r);
	void GIFPackedRegHandlerFOG(GIFPackedReg* r);
	void GIFPackedRegHandlerXYZF3(GIFPackedReg* r);
	void GIFPackedRegHandlerXYZ3(GIFPackedReg* r);
	void GIFPackedRegHandlerA_D(GIFPackedReg* r);
	void GIFPackedRegHandlerNOP(GIFPackedReg* r);

	typedef void (GSState::*GIFRegHandler)(GIFReg* r);

	GIFRegHandler m_fpGIFRegHandlers[256];

	void GIFRegHandlerNull(GIFReg* r);
	void GIFRegHandlerPRIM(GIFReg* r);
	void GIFRegHandlerRGBAQ(GIFReg* r);
	void GIFRegHandlerST(GIFReg* r);
	void GIFRegHandlerUV(GIFReg* r);
	void GIFRegHandlerXYZF2(GIFReg* r);
	void GIFRegHandlerXYZ2(GIFReg* r);
	template<int i> void GIFRegHandlerTEX0(GIFReg* r);
	template<int i> void GIFRegHandlerCLAMP(GIFReg* r);
	void GIFRegHandlerFOG(GIFReg* r);
	void GIFRegHandlerXYZF3(GIFReg* r);
	void GIFRegHandlerXYZ3(GIFReg* r);
	void GIFRegHandlerNOP(GIFReg* r);
	template<int i> void GIFRegHandlerTEX1(GIFReg* r);
	template<int i> void GIFRegHandlerTEX2(GIFReg* r);
	template<int i> void GIFRegHandlerXYOFFSET(GIFReg* r);
	void GIFRegHandlerPRMODECONT(GIFReg* r);
	void GIFRegHandlerPRMODE(GIFReg* r);
	void GIFRegHandlerTEXCLUT(GIFReg* r);
	void GIFRegHandlerSCANMSK(GIFReg* r);
	template<int i> void GIFRegHandlerMIPTBP1(GIFReg* r);
	template<int i> void GIFRegHandlerMIPTBP2(GIFReg* r);
	void GIFRegHandlerTEXA(GIFReg* r);
	void GIFRegHandlerFOGCOL(GIFReg* r);
	void GIFRegHandlerTEXFLUSH(GIFReg* r);
	template<int i> void GIFRegHandlerSCISSOR(GIFReg* r);
	template<int i> void GIFRegHandlerALPHA(GIFReg* r);
	void GIFRegHandlerDIMX(GIFReg* r);
	void GIFRegHandlerDTHE(GIFReg* r);
	void GIFRegHandlerCOLCLAMP(GIFReg* r);
	template<int i> void GIFRegHandlerTEST(GIFReg* r);
	void GIFRegHandlerPABE(GIFReg* r);
	template<int i> void GIFRegHandlerFBA(GIFReg* r);
	template<int i> void GIFRegHandlerFRAME(GIFReg* r);
	template<int i> void GIFRegHandlerZBUF(GIFReg* r);
	void GIFRegHandlerBITBLTBUF(GIFReg* r);
	void GIFRegHandlerTRXPOS(GIFReg* r);
	void GIFRegHandlerTRXREG(GIFReg* r);
	void GIFRegHandlerTRXDIR(GIFReg* r);
	void GIFRegHandlerHWREG(GIFReg* r);
	void GIFRegHandlerSIGNAL(GIFReg* r);
	void GIFRegHandlerFINISH(GIFReg* r);
	void GIFRegHandlerLABEL(GIFReg* r);

	int m_version;
	int m_vmsize;
	int m_sssize;

	bool m_mt;
	void (*m_irq)();
	bool m_path3hack;

	int m_x, m_y;
	int m_bytes;
	int m_maxbytes;
	BYTE* m_buff;

	void FlushWrite();
	void FlushWrite(BYTE* mem, int len);
	void StepTransfer(int sx, int ex) {if(++m_x == ex) {m_x = sx; m_y++;}}

protected:
	bool DetectBadFrame(int crc, int& skip);

	GSPerfMon m_perfmon;

public:
	GIFRegPRIM*		PRIM;
	GSRegPMODE*		PMODE;
	GSRegSMODE1*	SMODE1;
	GSRegSMODE2*	SMODE2;
	GSRegDISPFB*	DISPFB[2];
	GSRegDISPLAY*	DISPLAY[2];
	GSRegEXTBUF*	EXTBUF;
	GSRegEXTDATA*	EXTDATA;
	GSRegEXTWRITE*	EXTWRITE;
	GSRegBGCOLOR*	BGCOLOR;
	GSRegCSR*		CSR;
	GSRegIMR*		IMR;
	GSRegBUSDIR*	BUSDIR;
	GSRegSIGLBLID*	SIGLBLID;

	GIFPath m_path[3];
	GSLocalMemory m_mem;
	GSDrawingEnvironment m_env;
	GSDrawingContext* m_context;
	GSVertex m_v;
	float m_q;

	bool m_nloophack;

public:
	GSState(BYTE* base, bool mt, void (*irq)(), bool nloophack);
	virtual ~GSState();

	void ResetHandlers();

	CPoint GetDisplayPos(int i);
	CSize GetDisplaySize(int i);
	CRect GetDisplayRect(int i);
	CSize GetDisplayPos();
	CSize GetDisplaySize();
	CRect GetDisplayRect();
	CPoint GetFramePos(int i);
	CSize GetFrameSize(int i);
	CRect GetFrameRect(int i);
	CSize GetFramePos();
	CSize GetFrameSize();
	CRect GetFrameRect();
	bool IsEnabled(int i);
	int GetFPS();

	virtual void Reset();
	virtual void Flush();
	virtual void FlushPrim() = 0;
	virtual void ResetPrim() = 0;
	virtual void VertexKick(bool skip) = 0;
	virtual void InvalidateTexture(const GIFRegBITBLTBUF& BITBLTBUF, CRect r) {}
	virtual void InvalidateLocalMem(const GIFRegBITBLTBUF& BITBLTBUF, CRect r) {}

	void Move();
	void Write(BYTE* mem, int len);
	void Read(BYTE* mem, int len);

	void WriteCSR(UINT32 csr);
	void ReadFIFO(BYTE* mem, int size);
	void Transfer(BYTE* mem, int size, int index);
	void GetLastTag(UINT32* tag) {*tag = m_path3hack; m_path3hack = 0;}
	int Freeze(freezeData* fd, bool sizeonly);
	int Defrost(const freezeData* fd);
};

