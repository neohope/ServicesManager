#include "StdAfx.h"
#include "NeoSubItemColor.h"

CNeoSubItemColor::CNeoSubItemColor(void)
{
	nIndex = 0;
	nSubIndex = 0;
	colorForSubItem = RGB(0,0,0);
}

CNeoSubItemColor::~CNeoSubItemColor(void)
{
}

COLORREF CNeoSubItemColor::DefaultColor=RGB(0,0,0);
COLORREF CNeoSubItemColor::BlackColor=RGB(0,0,0);
COLORREF CNeoSubItemColor::WhiteColor=RGB(255,255,255);
COLORREF CNeoSubItemColor::RedColor=RGB(200,0,0);
COLORREF CNeoSubItemColor::GreenColor=RGB(0,200,0);
COLORREF CNeoSubItemColor::BlueColor=RGB(0,0,200);

void CNeoSubItemColor::setSubItemColor(int nItem,int nSubItem,COLORREF color)
{
	for(SubItemColorsIterator=SubItemColors.begin();SubItemColorsIterator!=SubItemColors.end();SubItemColorsIterator++)
	{
		if(SubItemColorsIterator->nIndex==nItem && SubItemColorsIterator->nSubIndex==nSubItem)
		{
			SubItemColorsIterator->colorForSubItem=color;
			return;
		}
	}

	CNeoSubItemColor subItemColor;
	subItemColor.nIndex = nItem;
	subItemColor.nSubIndex = nSubItem;
	subItemColor.colorForSubItem = color;
	SubItemColors.push_back(subItemColor);
}

COLORREF CNeoSubItemColor::getSubItemColor(int nItem,int nSubItem)
{
	for(SubItemColorsIterator=SubItemColors.begin();SubItemColorsIterator!=SubItemColors.end();SubItemColorsIterator++)
	{
		if(SubItemColorsIterator->nIndex==nItem && SubItemColorsIterator->nSubIndex==nSubItem)
		{
			return SubItemColorsIterator->colorForSubItem;
		}
	}

	return CNeoSubItemColor::DefaultColor;
}