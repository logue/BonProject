// ModCatalog.h: ���W���[���J�^���O�����N���X
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

/////////////////////////////////////////////////////////////////////////////
// ���W���[���J�^���O�����N���X
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
// �O���[�o���ϐ�
/////////////////////////////////////////////////////////////////////////////

extern HMODULE g_hModule;