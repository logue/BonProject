// DllMain.cpp: DLL�G���g���[�|�C���g�̒�`
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "BonWhiteTuner.h"
#include "BonBlackTuner.h"
#include "ModCatalog.h"


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

			// ���W���[���n���h���o�^
			CBonWhiteTuner::SetModuleHandle(hModule);
			CBonBlackTuner::SetModuleHandle(hModule);

			// �N���X�o�^
			g_hModule = hModule;
			::RegisterClass();

			break;

		case DLL_PROCESS_DETACH :
			
			break;
		}

	return TRUE;
}


static void RegisterClass(void)
{
	// �N���X�o�^
	::BON_REGISTER_CLASS<FriioDriver>();
	::BON_REGISTER_CLASS<CBonWhiteTuner>();
	::BON_REGISTER_CLASS<CBonBlackTuner>();
}
