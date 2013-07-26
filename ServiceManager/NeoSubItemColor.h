#pragma once

#include <list>

using namespace std;

class CNeoSubItemColor
{
public:
	CNeoSubItemColor(void);
	~CNeoSubItemColor(void);

public:
	static COLORREF DefaultColor;
	static COLORREF BlackColor;
	static COLORREF WhiteColor;
	static COLORREF RedColor;
	static COLORREF GreenColor;
	static COLORREF BlueColor;

private:
	list<CNeoSubItemColor> SubItemColors;
	list<CNeoSubItemColor>::iterator SubItemColorsIterator;

private:
	int nIndex;
	int nSubIndex;
	COLORREF colorForSubItem;

public:
	void setSubItemColor(int nItem,int nSubItem,COLORREF color);
	COLORREF getSubItemColor(int nItem,int nSubItem);
};
