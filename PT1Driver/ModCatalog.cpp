// ModCatalog.cpp: ���W���[���J�^���O�����N���X
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "ModCatalog.h"


/////////////////////////////////////////////////////////////////////////////
// �O���[�o���ϐ�
/////////////////////////////////////////////////////////////////////////////

HMODULE g_hModule = NULL;


/////////////////////////////////////////////////////////////////////////////
// �t�@�C�����[�J���ϐ�
/////////////////////////////////////////////////////////////////////////////

static const CModCatalogBase::MOD_CLASS_INFO f_aClassInfo[] =
{
	{TEXT("IHalTsTuner"), TEXT("CPtTerTuner"), TEXT("Earthsoft PT1 ISDB-T �f�W�^���`���[�i"	 )},
	{TEXT("IHalTsTuner"), TEXT("CPtSatTuner"), TEXT("Earthsoft PT1 ISDB-S �f�W�^���`���[�i"	 )},
	{TEXT("IHalTsTuner"), TEXT("CPtTriTuner"), TEXT("Earthsoft PT1 ISDB-T/S �f�W�^���`���[�i")}
};


/////////////////////////////////////////////////////////////////////////////
// ���W���[���J�^���O�����N���X
/////////////////////////////////////////////////////////////////////////////

PT1Driver::PT1Driver(IBonObject *pOwner)
	: CModCatalogBase(pOwner, f_aClassInfo, sizeof(f_aClassInfo) / sizeof(*f_aClassInfo), g_hModule, TEXT("PT1�h���C�o"), TEXT("�g���c�[�����̐l"), TEXT("http://2sen.dip.jp/dtv/"))
{
	// �������Ȃ�
}

PT1Driver::~PT1Driver(void)
{
	// �������Ȃ�
}
