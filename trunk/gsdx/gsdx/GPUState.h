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

#include "GPU.h"
#include "GPUDrawingEnvironment.h"
#include "GPULocalMemory.h"
#include "GPUVertex.h"
#include "GSAlignedClass.h"
#include "GSUtil.h"
#include "GSPerfMon.h"

class GPUState : public GSAlignedClass<16>
{
	typedef void (GPUState::*GPUStatusCommandHandler)(GPUReg* r);

	GPUStatusCommandHandler m_fpGPUStatusCommandHandlers[256];

	void SCH_Null(GPUReg* r);
	void SCH_ResetGPU(GPUReg* r);
	void SCH_ResetCommandBuffer(GPUReg* r);
	void SCH_ResetIRQ(GPUReg* r);
	void SCH_DisplayEnable(GPUReg* r);
	void SCH_DMASetup(GPUReg* r);
	void SCH_StartOfDisplayArea(GPUReg* r);
	void SCH_HorizontalDisplayRange(GPUReg* r);
	void SCH_VerticalDisplayRange(GPUReg* r);
	void SCH_DisplayMode(GPUReg* r);
	void SCH_GPUInfo(GPUReg* r);

	typedef int (GPUState::*GPUPacketHandler)(GPUReg* r, int size);

	GPUPacketHandler m_fpGPUPacketHandler[8];

	int PH_Command(GPUReg* r, int size);
	int PH_Polygon(GPUReg* r, int size);
	int PH_Line(GPUReg* r, int size);
	int PH_Sprite(GPUReg* r, int size);
	int PH_Move(GPUReg* r, int size);
	int PH_Write(GPUReg* r, int size);
	int PH_Read(GPUReg* r, int size);
	int PH_Environment(GPUReg* r, int size);

	class Buffer
	{
	public:
		int bytes;
		int maxbytes;
		BYTE* buff;
		int cur;

	public:
		Buffer();
		~Buffer();
		void Reserve(int size);
		void Append(const BYTE* src, int size);
		void Remove(int size);
		void RemoveAll();
	};

	Buffer m_write;
	Buffer m_read;

	void SetPrim(GPUReg* r);
	void SetCLUT(GPUReg* r);
	void SetTPAGE(GPUReg* r);

protected:

	int s_n;

	void Dump(LPCTSTR s, UINT32 TP, const CRect& r, int inc = true)
	{
		//if(m_perfmon.GetFrame() < 1000) 
		//if((m_env.TWIN.ai32 & 0xfffff) == 0)
		//if(!m_env.STATUS.ME && !m_env.STATUS.MD)
			return;

		if(inc) s_n++;

		if(s_n < 133) return;

		int dir = 1;
#ifdef DEBUG
		dir = 2;
#endif
		CString str;
		str.Format(_T("c:\\temp%d\\%04d_%s.bmp"), dir, s_n, s);
		SaveBMP(str, TP, r);
	}

	void Dump(LPCTSTR s, int inc = true)
	{
		Dump(s, 2, CRect(0, 0, 1024, 512), inc);
	}

	void SaveBMP(LPCTSTR path, UINT32 TP, CRect r);

public:
	GPUDrawingEnvironment m_env;
	GPULocalMemory m_mem;
	GPUVertex m_v;
	GSPerfMon m_perfmon;
	UINT32 m_status[256];

public:
	GPUState();
	virtual ~GPUState();

	virtual void Reset();
	virtual void Flush();
	virtual void FlushPrim() = 0;
	virtual void ResetPrim() = 0;
	virtual void VertexKick() = 0;
	virtual void Invalidate(const CRect& r) {}

	WORD* GetCLUT() {return &m_mem.m_vm16[(m_env.CLUT.Y << 10) + (m_env.CLUT.X << 4)];}

	void WriteData(const BYTE* mem, UINT32 size);
	void ReadData(BYTE* mem, UINT32 size);

	void WriteStatus(UINT32 status);
	UINT32 ReadStatus();

	void Freeze(GPUFreezeData* data);
	void Defrost(const GPUFreezeData* data);
};

