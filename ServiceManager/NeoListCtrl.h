#pragma once

#include "NeoSubItemColor.h"

// CNeoListCtrl
class CNeoListCtrl : public CListCtrl,public CNeoSubItemColor
{
	DECLARE_DYNAMIC(CNeoListCtrl)

public:
	CNeoListCtrl();
	virtual ~CNeoListCtrl();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
};


