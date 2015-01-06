// TsSrcManager.cpp: TS�\�[�X�}�l�[�W���N���X
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "TsSrcManager.h"


/////////////////////////////////////////////////////////////////////////////
// TS�\�[�X�}�l�[�W���N���X
/////////////////////////////////////////////////////////////////////////////

const bool CTsSrcManager::OpenManager(void)
{
	// ��U�N���[�Y
	CloseManager();
	
	// �N���X�񋓃C���X�^���X����
	m_pDriverCollection = ::BON_SAFE_CREATE<IBonClassEnumerator *>(TEXT("CBonClassEnumerator"));
	::BON_ASSERT(m_pDriverCollection != NULL);

	// IHalTsTuner�N���X��
	m_pDriverCollection->EnumBonClass(TEXT("IHalTsTuner"));
	
	
	return true;
}

void CTsSrcManager::CloseManager(void)
{
	// �N���X�񋓃C���X�^���X�J��
	BON_SAFE_RELEASE(m_pDriverCollection);


}

IHalTsTuner * CTsSrcManager::LendTuner(const DWORD dwIndex)
{
	return NULL;
}

const bool CTsSrcManager::RepayTuner(IHalTsTuner *pTsTuner)
{
	return true;
}

const DWORD CTsSrcManager::GetClassName(const DWORD dwIndex, LPTSTR lpszClassName)
{
	return 0UL;
}

const DWORD CTsSrcManager::GetTunerName(const DWORD dwIndex, LPTSTR lpszTunerName)
{
	return 0UL;
}

const DWORD CTsSrcManager::GetTunerIndex(const DWORD dwIndex)
{
	return 0UL;
}

const bool CTsSrcManager::IsTunerLending(const DWORD dwIndex)
{
	return true;
}

CTsSrcManager::CTsSrcManager(IBonObject *pOwner)
	: CBonObject(pOwner)
	, m_pDriverCollection(NULL)
{


}

CTsSrcManager::~CTsSrcManager(void)
{
	// �}�l�[�W���N���[�Y
	CloseManager();
}
