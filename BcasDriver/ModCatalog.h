// ModCatalog.h: ���W���[���J�^���O�����N���X
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

/////////////////////////////////////////////////////////////////////////////
// ���W���[���J�^���O�����N���X
/////////////////////////////////////////////////////////////////////////////

class BcasDriver	:	public CModCatalogBase
{
public:
// CBonObject
	DECLARE_IBONOBJECT(BcasDriver)

// BcasDriver
	BcasDriver(IBonObject *pOwner);
	virtual ~BcasDriver(void);
};


/////////////////////////////////////////////////////////////////////////////
// �O���[�o���ϐ�
/////////////////////////////////////////////////////////////////////////////

extern HMODULE g_hModule;