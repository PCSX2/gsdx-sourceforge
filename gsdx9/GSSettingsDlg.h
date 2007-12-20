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

#include "resource.h"

extern GSSetting g_renderers[]; 
extern GSSetting g_psversion[];

class GSSettingsDlg : public CDialog
{
	DECLARE_DYNAMIC(GSSettingsDlg)

private:
	CAtlList<D3DDISPLAYMODE> m_modes;

	void InitComboBox(CComboBox& combobox, const GSSetting* settings, int count, DWORD sel, DWORD maxid = ~0);

public:
	GSSettingsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~GSSettingsDlg();

// Dialog Data
	enum { IDD = IDD_CONFIG };
	CComboBox m_resolution;
	CComboBox m_renderer;
	CComboBox m_psversion;
	CComboBox m_interlace;
	CComboBox m_aspectratio;
	BOOL m_tvout;
	int m_filter;
	int m_nloophack;
	CSpinButtonCtrl m_resx;
	CSpinButtonCtrl m_resy;
	BOOL m_nativeres;
	CEdit m_resxedit;
	CEdit m_resyedit;
	BOOL m_vsync;
	BOOL m_logz;
	BOOL m_fba;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedCheck1();
};

