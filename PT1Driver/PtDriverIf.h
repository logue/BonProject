// PtDriverIf.h: PT1ドライバカスタムインタフェース
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


/////////////////////////////////////////////////////////////////////////////
// PT1チューナプロパティインタフェース
/////////////////////////////////////////////////////////////////////////////

class IPtBaseTunerProperty :	public IBonObject
{
public:
	bool m_bDefaultLnbPower;	// LNB電源デフォルト状態設定
	bool m_bEnaleVHF;			// VHF  チューニング空間有効
	bool m_bEnaleCATV;			// CATV チューニング空間有効
	bool m_bEnaleCS;			// 110CSチューニング空間有効
	bool m_bEnaleBS;			// BSF  チューニング空間有効
};
