// ModCatalog.h: モジュールカタログ実装クラス
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

/////////////////////////////////////////////////////////////////////////////
// モジュールカタログ実装クラス
/////////////////////////////////////////////////////////////////////////////

class FriioDriver	:	public CModCatalogBase
{
public:
// CBonObject
	DECLARE_IBONOBJECT(FriioDriver)

// FriioDriver
	FriioDriver(IBonObject *pOwner);
	virtual ~FriioDriver(void);
};


/////////////////////////////////////////////////////////////////////////////
// グローバル変数
/////////////////////////////////////////////////////////////////////////////

extern HMODULE g_hModule;
