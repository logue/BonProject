// DllMain.cpp: DLL�G���g���[�|�C���g�̒�`
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "BonCoreEngine.h"
#include "BonClassEnumerator.h"

#include "MediaBase.h"
#include "SmartFile.h"
#include "SmartSocket.h"
#include "RegCfgStorage.h"
#include "IniCfgStorage.h"


/////////////////////////////////////////////////////////////////////////////
// �t�@�C�����[�J���ϐ�
/////////////////////////////////////////////////////////////////////////////

static CBonCoreEngine BonEngine(NULL);


/////////////////////////////////////////////////////////////////////////////
// �t�@�C�����[�J���֐��v���g�^�C�v
/////////////////////////////////////////////////////////////////////////////

static void RegisterClass(void);


/////////////////////////////////////////////////////////////////////////////
// DLL�G���g���[
/////////////////////////////////////////////////////////////////////////////

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch(ul_reason_for_call){
		case DLL_PROCESS_ATTACH :

			// ���������[�N���o�L��
			_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF); 

			// �R�A�G���W���X�^�[�g�A�b�v
			BonEngine.Startup(hModule);

			// �W���N���X�o�^
			::RegisterClass();

			// Bon���W���[�����[�h
			BonEngine.RegisterBonModule();

			break;

		case DLL_PROCESS_DETACH :

			// �R�A�G���W���V���b�g�_�E��
			BonEngine.Shutdown();

			break;
		}

	return TRUE;
}

static void RegisterClass(void)
{
	// �N���X�t�@�N�g���[�o�^(�W���g�ݍ��݃N���X)
	::BON_REGISTER_CLASS<CBonCoreEngine>();
	::BON_REGISTER_CLASS<CBonClassEnumerator>();

	::BON_REGISTER_CLASS<CMediaData>();
	::BON_REGISTER_CLASS<CSmartFile>();
	::BON_REGISTER_CLASS<CSmartSocket>();
	::BON_REGISTER_CLASS<CRegCfgStorage>();
	::BON_REGISTER_CLASS<CIniCfgStorage>();
}


/////////////////////////////////////////////////////////////////////////////
// �G�N�X�|�[�gAPI
/////////////////////////////////////////////////////////////////////////////

BONAPI const BONGUID BON_NAME_TO_GUID(LPCTSTR lpszName)
{
	// ���W���[����/�N���X��/�C���^�t�F�[�X����GUID�ɕϊ�
	return BonEngine.BonNameToGuid(lpszName);
}

BONAPI IBonObject * BON_CREATE_INSTANCE(LPCTSTR lpszClassName, IBonObject *pOwner)
{
	// �N���X�C���X�^���X����
	return BonEngine.BonCreateInstance(lpszClassName, pOwner);
}

BONAPI const bool QUERY_BON_MODULE(LPCTSTR lpszModuleName)
{
	// ���W���[���̗L����Ԃ�
	return BonEngine.QueryBonModule(lpszModuleName);
}

BONAPI const DWORD GET_BON_MODULE_NAME(const BONGUID ModuleId, LPTSTR lpszModuleName)
{
	// ���W���[����GUID�𖼑O�ɕϊ�
	return BonEngine.GetBonModuleName(ModuleId, lpszModuleName);
}

BONAPI const bool REGISTER_BON_CLASS(LPCTSTR lpszClassName, const CLASSFACTORYMETHOD pfnClassFactory, const DWORD dwPriority)
{
	// �N���X��o�^
	return BonEngine.RegisterBonClass(lpszClassName, pfnClassFactory, dwPriority);
}

BONAPI const bool QUERY_BON_CLASS(LPCTSTR lpszClassName)
{
	// �N���X�̗L����Ԃ�
	return BonEngine.QueryBonClass(lpszClassName);
}

BONAPI const DWORD GET_BON_CLASS_NAME(const BONGUID ClassId, LPTSTR lpszClassName)
{
	// �N���X��GUID�𖼑O�ɕϊ�
	return BonEngine.GetBonClassName(ClassId, lpszClassName);
}

BONAPI const BONGUID GET_BON_CLASS_MODULE(const BONGUID ClassId)
{
	// �N���X��񋟂��郂�W���[����GUID��Ԃ�
	return BonEngine.GetBonClassModule(ClassId);
}

BONAPI IBonObject * GET_STOCK_INSTANCE(LPCTSTR lpszClassName)
{
	// �X�g�b�N�C���X�^���X��Ԃ�
	return BonEngine.GetStockInstance(lpszClassName);
}

BONAPI const bool REGISTER_STOCK_INSTANCE(LPCTSTR lpszClassName, IBonObject *pInstance)
{
	// �X�g�b�N�C���X�^���X�o�^
	return BonEngine.RegisterStockInstance(lpszClassName, pInstance);
}

BONAPI const bool UNREGISTER_STOCK_INSTANCE(LPCTSTR lpszClassName)
{
	// �X�g�b�N�C���X�^���X�폜
	return BonEngine.UnregisterStockInstance(lpszClassName);
}
