// BonEnumerator.cpp: Friio�h���C�o�񋓃N���X
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "BonEnumerator.h"
#include <TChar.h>
#include <SetupApi.h>
#include <CfgMgr32.h>
#include <InitGuid.h>


#pragma comment(lib, "SetupApi.lib")


/////////////////////////////////////////////////////////////////////////////
// �t�@�C�����[�J���萔�ݒ�
/////////////////////////////////////////////////////////////////////////////

// �h���C�o�C���X�^���X��GUID
DEFINE_GUID( GUID_FRIIO_DRIVER,	0x5a56d255L, 0xe23a, 0x4b4d, 0x8c, 0x78, 0xc2, 0x65, 0x93, 0x0b, 0x6c, 0x68 );


/////////////////////////////////////////////////////////////////////////////
// Friio�h���C�o�񋓃N���X
/////////////////////////////////////////////////////////////////////////////

CBonEnumerator::CBonEnumerator(void)
{

}

CBonEnumerator::~CBonEnumerator(void)
{
	m_DeviceLock.Close();
}

const bool CBonEnumerator::EnumDevice(const bool bBlackFriio)
{
	// �f�o�C�X���Z�b�g�̃n���h���擾
	HDEVINFO hDevInfo = ::SetupDiGetClassDevs(&GUID_FRIIO_DRIVER, 0UL, NULL, DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
	if(hDevInfo == INVALID_HANDLE_VALUE)return false;

	// �h���C�o���X�g���N���A
	m_DriverList.clear();

	// �f�o�C�X�C���^�t�F�[�X���
	SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;
	DeviceInterfaceData.cbSize = sizeof(DeviceInterfaceData);

	DWORD dwDeviceIndex = 0UL;
	TCHAR szDriverPath[_MAX_PATH];

	while(::SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &GUID_FRIIO_DRIVER, dwDeviceIndex++, &DeviceInterfaceData)){
		// �f�o�C�X�C���^�t�F�[�X�̏ڍׂ��i�[����̂ɕK�v�ȃo�b�t�@�T�C�Y���擾
		DWORD dwBuffLen = 0UL;
		::SetupDiGetDeviceInterfaceDetail(hDevInfo, &DeviceInterfaceData, NULL, 0, &dwBuffLen, NULL);

		// �o�b�t�@���m��
		dwBuffLen += sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) + 1;
		SP_DEVICE_INTERFACE_DETAIL_DATA *pDeviceInterfaceDetailData = (SP_DEVICE_INTERFACE_DETAIL_DATA *)new BYTE [dwBuffLen];
		if(!pDeviceInterfaceDetailData)continue;
		pDeviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		// �f�o�C�X�̏ڍׂ��擾
		SP_DEVINFO_DATA DevInfoData;
		DevInfoData.cbSize = sizeof(DevInfoData);
		if(!::SetupDiGetDeviceInterfaceDetail(hDevInfo, &DeviceInterfaceData, pDeviceInterfaceDetailData, dwBuffLen, NULL, &DevInfoData))continue;

		// ��ʃf�o�C�X���擾�uUSB �����f�o�C�X�v
		if(::CM_Get_Parent(&DevInfoData.DevInst, DevInfoData.DevInst, 0UL) == CR_SUCCESS){
	
			// ��ʃf�o�C�X���擾�u�ėp USB �n�u�v
			if(::CM_Get_Parent(&DevInfoData.DevInst, DevInfoData.DevInst, 0UL) == CR_SUCCESS){
			
				// ���ʃf�o�C�X���擾�@��:�uMulti-Slots USB SmartCard Reader�v�A��:�uGeneric Usb Smart Card Reader�v
				if(::CM_Get_Child(&DevInfoData.DevInst, DevInfoData.DevInst, 0UL) == CR_SUCCESS){
				
					// ���ʃf�o�C�X���擾�@��:�uChild A Device�v�A��:�u���݂��Ȃ��v
					if(((::CM_Get_Child(&DevInfoData.DevInst, DevInfoData.DevInst, 0UL) == CR_SUCCESS)? false : true) == bBlackFriio){
						
						// DriverPath��ǉ�
						::_tcscpy(szDriverPath, pDeviceInterfaceDetailData->DevicePath);
						m_DriverList.push_back(szDriverPath);
						}
					}
				}
			}		

		// �o�b�t�@���J��
		delete [] ((BYTE *)pDeviceInterfaceDetailData);
		}

	// �f�o�C�X���Z�b�g�̃n���h���J��
	::SetupDiDestroyDeviceInfoList(hDevInfo);

	return true;
}

LPCTSTR CBonEnumerator::GetAvailableDriverPath(void) const
{
	// ���p�\�ȃf�o�C�X������
	for(DWORD dwIndex = 0UL ; dwIndex < m_DriverList.size() ; dwIndex++){
		if(IsAvailableDevice(m_DriverList[dwIndex].c_str()))return m_DriverList[dwIndex].c_str();
		}

	return NULL;
}

const DWORD CBonEnumerator::GetTotalNum(void) const
{
	// �f�o�C�X�̑�����Ԃ�
	return m_DriverList.size();
}

const DWORD CBonEnumerator::GetActiveNum(void) const
{
	DWORD dwActiveNum = 0UL;

	// �g�p���̃f�o�C�X������
	for(DWORD dwIndex = 0UL ; dwIndex < m_DriverList.size() ; dwIndex++){
		if(!IsAvailableDevice(m_DriverList[dwIndex].c_str()))dwActiveNum++;
		}

	return dwActiveNum;
}

const DWORD CBonEnumerator::GetAvailableNum(void) const
{
	DWORD AvailableNum = 0UL;

	// �g�p���łȂ��f�o�C�X������
	for(DWORD dwIndex = 0UL ; dwIndex < m_DriverList.size() ; dwIndex++){
		if(!IsAvailableDevice(m_DriverList[dwIndex].c_str()))AvailableNum++;
		}

	return AvailableNum;
}

const bool CBonEnumerator::LockDevice(LPCTSTR lpszDriverPath)
{
	if(!lpszDriverPath)return false;
	if(!::_tcslen(lpszDriverPath))return false;

	// �h���C�o�p�X����'\'��'/'�ɒu������
	TCHAR szMutexName[_MAX_PATH];
	::_tcscpy(szMutexName, lpszDriverPath);
	
	for(TCHAR *pszPos = szMutexName ; *pszPos ; pszPos++){
		if(*pszPos == TEXT('\\'))*pszPos = TEXT('/');
		}

	// Mutex���擾
	return m_DeviceLock.Create(szMutexName);
}

const bool CBonEnumerator::ReleaseDevice(void)
{
	// Mutex���J��
	return m_DeviceLock.Close();
}

const bool CBonEnumerator::IsAvailableDevice(LPCTSTR lpszDriverPath)
{
	if(!lpszDriverPath)return false;
	if(!::_tcslen(lpszDriverPath))return false;

	// �h���C�o�p�X����'\'��'/'�ɒu������
	TCHAR szMutexName[_MAX_PATH];
	::_tcscpy(szMutexName, lpszDriverPath);

	for(TCHAR *pszPos = szMutexName ; *pszPos ; pszPos++){
		if(*pszPos == TEXT('\\'))*pszPos = TEXT('/');
		}

	// Mutex�̎擾�L����Ԃ�
	return CSmartMutex::IsExist(szMutexName)? false : true;
}
