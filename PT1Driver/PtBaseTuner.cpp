// PtBaseTuner.cpp: PT�`���[�i���N���X
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "PtBaseTuner.h"
#include <TChar.h>


/////////////////////////////////////////////////////////////////////////////
// �t�@�C�����[�J���萔�ݒ�
/////////////////////////////////////////////////////////////////////////////

// �f�o�C�X��
#define DEVICENAME_TER	TEXT("Earthsoft PT1 ISDB-T �f�W�^���`���[�i")
#define DEVICENAME_SAT	TEXT("Earthsoft PT1 ISDB-S �f�W�^���`���[�i")

// �o�b�t�@�ݒ�
#define FIFOBUFFTIME	2UL														// �o�b�t�@�� = 2�b
#define FIFOBLOCKSIZE	(CPtBoard::DMABLK_SIZE / CPtBoard::DMACHK_UNIT / 3UL)	// �����u���b�N�T�C�Y
#define FIFOBUFFNUM		(0x3C0000UL / FIFOBLOCKSIZE * FIFOBUFFTIME)				// ����30Mbps�Ƃ���


/////////////////////////////////////////////////////////////////////////////
// PT1�`���[�i�v���p�e�B�N���X
/////////////////////////////////////////////////////////////////////////////

const bool CPtBaseTunerProperty::LoadProperty(IConfigStorage *pStorage)
{
	if(!pStorage)return false;

	// �ݒ�ǂݏo��
	m_bDefaultLnbPower = pStorage->GetBoolItem(TEXT("DefaultLnbPower"	), false);
	m_bEnaleVHF		   = pStorage->GetBoolItem(TEXT("EnaleVHF"			), true	);
	m_bEnaleCATV	   = pStorage->GetBoolItem(TEXT("EnaleCATV"			), true	);
	m_bEnaleCS		   = pStorage->GetBoolItem(TEXT("EnaleCS"			), true	);
	m_bEnaleBS		   = pStorage->GetBoolItem(TEXT("EnaleBS"			), true	);

	return true;
}

const bool CPtBaseTunerProperty::SaveProperty(IConfigStorage *pStorage)
{
	if(!pStorage)return false;
	
	// �ݒ�ۑ�
	try{
		if(!pStorage->SetBoolItem(TEXT("DefaultLnbPower"), m_bDefaultLnbPower	))throw ::BON_EXPECTION();
		if(!pStorage->SetBoolItem(TEXT("EnaleVHF"		), m_bEnaleVHF			))throw ::BON_EXPECTION();
		if(!pStorage->SetBoolItem(TEXT("EnaleCATV"		), m_bEnaleCATV			))throw ::BON_EXPECTION();
		if(!pStorage->SetBoolItem(TEXT("EnaleCS"		), m_bEnaleCS			))throw ::BON_EXPECTION();
		if(!pStorage->SetBoolItem(TEXT("EnaleBS"		), m_bEnaleBS			))throw ::BON_EXPECTION();
		}
	catch(CBonException &Exception){
		// �G���[����
		Exception.Notify();
		return false;
		}
	
	return true;
}

IConfigTarget * CPtBaseTunerProperty::GetConfigTarget(void)
{
	// �R���t�B�O�^�[�Q�b�g��Ԃ�
	return m_pConfigTarget;
}

const bool CPtBaseTunerProperty::CopyProperty(const IConfigProperty *pProperty)
{
	const IPtBaseTunerProperty *pPtBaseTunerProperty = dynamic_cast<const IPtBaseTunerProperty *>(pProperty);
	if(!pPtBaseTunerProperty)return false;
	
	// �v���p�e�B���R�s�[
	*(static_cast<IPtBaseTunerProperty *>(this)) = *pPtBaseTunerProperty;
	
	return true;
}

const DWORD CPtBaseTunerProperty::GetDialogClassName(LPTSTR lpszClassName)
{
	// �v���p�e�B�_�C�A���O�͔����
	return 0UL;
}

CPtBaseTunerProperty::CPtBaseTunerProperty(IBonObject *pOwner)
	: CBonObject(pOwner)
	, m_pConfigTarget(dynamic_cast<IConfigTarget *>(pOwner))
{
	// �f�t�H���g�l��ݒ�
	m_bDefaultLnbPower = false;
	m_bEnaleVHF = true;
	m_bEnaleCATV = true;
	m_bEnaleCS = true;
	m_bEnaleBS = true;
}

CPtBaseTunerProperty::~CPtBaseTunerProperty(void)
{
	// �������Ȃ�
}


/////////////////////////////////////////////////////////////////////////////
// PT1�`���[�i���N���X
/////////////////////////////////////////////////////////////////////////////

CPtManager *CPtBaseTuner::m_pPtManager = NULL;
DWORD CPtBaseTuner::m_dwInstanceCount = 0UL;

const BONGUID CPtBaseTuner::GetDeviceType(void)
{
	// �f�o�C�X�̃^�C�v��Ԃ�
	return ::BON_NAME_TO_GUID(TEXT("IHalTsTuner"));
}

const DWORD CPtBaseTuner::GetDeviceName(LPTSTR lpszName)
{
	static const TCHAR aszDeviceName[][256] = {DEVICENAME_TER, DEVICENAME_SAT};

	// �f�o�C�X����Ԃ�
	if(lpszName)::_tcscpy(lpszName, aszDeviceName[m_dwTunerType]);

	return ::_tcslen(aszDeviceName[m_dwTunerType]);
}

const DWORD CPtBaseTuner::GetTotalDeviceNum(void)
{
	// �`���[�i�̑�����Ԃ�
	return m_pPtManager->GetTotalTunerNum(m_dwTunerType);
}

const DWORD CPtBaseTuner::GetActiveDeviceNum(void)
{
	// �g�p���̃`���[�i����Ԃ�
	return m_pPtManager->GetActiveTunerNum(m_dwTunerType);
}

const bool CPtBaseTuner::OpenTuner(void)
{
	// ��U�N���[�Y
	CloseTuner();
	
	try{
		// �`���[�i���I�[�v������
		if(!(m_pTuner = m_pPtManager->OpenTuner(m_dwTunerType)))throw ::BON_EXPECTION(TEXT("�`���[�i�I�[�v�����s"));

		// LNB�d���ݒ�
		if(m_dwTunerType == Device::ISDB_S){
			if(!SetLnbPower(m_Property.m_bDefaultLnbPower))throw ::BON_EXPECTION(TEXT("����LNB�d���ݒ莸�s"));
			}

		// �f�t�H���g�`�����l���ݒ�
		SetChannel(m_dwCurSpace, m_dwCurChannel);

		// FIFO�o�b�t�@�m��
		m_BlockPool.resize(FIFOBUFFNUM, CMediaData(FIFOBLOCKSIZE));
		m_itFreeBlock = m_BlockPool.begin();

		// �X�g���[����M�J�n
		if(!m_pTuner->StartStream(this))throw ::BON_EXPECTION(TEXT("TS�X�g���[����M�J�n���s"));
		}
	catch(CBonException &Exception){
		// �G���[����
		Exception.Notify();
		CloseTuner();
		return false;
		}
	
	return true;
}

void CPtBaseTuner::CloseTuner(void)
{
	// �`���[�i���N���[�Y����
	if(m_pTuner){
		m_pTuner->StopStream();
		m_pTuner->CloseTuner();
		m_pTuner = NULL;
		}
	
	// FIFO�o�b�t�@�J��
	{
		CBlockLock AutoLock(&m_FifoLock);
	
		while(!m_FifoBuffer.empty())m_FifoBuffer.pop();
		m_BlockPool.clear();
	}
}

const DWORD CPtBaseTuner::EnumTuningSpace(const DWORD dwSpace, LPTSTR lpszSpace)
{
	// �g�p�\�ȃ`���[�j���O��Ԃ�Ԃ�
	if(dwSpace >= m_TuningSpace.size())return 0UL;

	// �`���[�j���O��Ԗ����R�s�[
	if(lpszSpace)::_tcscpy(lpszSpace, m_TuningSpace[dwSpace]->szSpaceName);

	// �`���[�j���O��Ԗ�����Ԃ�
	return ::_tcslen(m_TuningSpace[dwSpace]->szSpaceName);
}

const DWORD CPtBaseTuner::EnumChannelName(const DWORD dwSpace, const DWORD dwChannel, LPTSTR lpszChannel)
{
	// �g�p�\�ȃ`�����l����Ԃ�
	if(dwSpace >= m_TuningSpace.size())return 0UL;

	if(dwChannel >= m_TuningSpace[dwSpace]->dwChNum)return 0UL;

	// �`�����l�������R�s�[
	if(lpszChannel)::_tcscpy(lpszChannel, m_TuningSpace[dwSpace]->pChConfig[dwChannel].szChName);

	// �`�����l��������Ԃ�
	return ::_tcslen(m_TuningSpace[dwSpace]->pChConfig[dwChannel].szChName);
}

const DWORD CPtBaseTuner::GetCurSpace(void)
{
	// ���݂̃`���[�j���O��Ԃ�Ԃ�
	return m_dwCurSpace;
}

const DWORD CPtBaseTuner::GetCurChannel(void)
{
	// ���݂̃`�����l����Ԃ�
	return m_dwCurChannel;
}

const bool CPtBaseTuner::SetChannel(const DWORD dwSpace, const DWORD dwChannel)
{
	// �������Ȃ��A�h���N���X�ŃI�[�o�[���C�h���Ȃ���΂Ȃ�Ȃ�
	::BON_ASSERT(false);
	
	return false;
}

const bool CPtBaseTuner::SetLnbPower(const bool bEnable)
{
	// LNB�d������
	return (m_pTuner)? m_pTuner->SetLnbPower(bEnable) : false;
}

const float CPtBaseTuner::GetSignalLevel(void)
{
	// CNR���擾
	return (m_pTuner)? m_pTuner->GetSignalLevel() : 0.0f;
}

const bool CPtBaseTuner::GetStream(BYTE **ppStream, DWORD *pdwSize, DWORD *pdwRemain)
{
	CBlockLock AutoLock(&m_FifoLock);

	// ��MFIFO�o�b�t�@����X�g���[�������o��
	if(!m_pTuner){
		// �`���[�i���I�[�v������Ă��Ȃ�
		return false;
		}
	else if(m_FifoBuffer.empty()){
		// ���o���\�ȃf�[�^���Ȃ�
		if(ppStream)*ppStream = NULL;
		if(pdwSize)*pdwSize = 0UL;
		if(pdwRemain)*pdwRemain = 0UL;
		}
	else{	
		// �f�[�^�����o��
		if(ppStream)*ppStream = m_FifoBuffer.front()->GetData();
		if(pdwSize)*pdwSize = m_FifoBuffer.front()->GetSize();

		m_FifoBuffer.pop();
		
		if(pdwRemain)*pdwRemain = m_FifoBuffer.size();
		}

	return true;
}

void CPtBaseTuner::PurgeStream(void)
{
	// FIFO�o�b�t�@�N���A
	CBlockLock AutoLock(&m_FifoLock);
	
	while(!m_FifoBuffer.empty())m_FifoBuffer.pop();
}

const DWORD CPtBaseTuner::WaitStream(const DWORD dwTimeOut)
{
	// �X�g���[�������E�F�C�g
	return m_FifoEvent.WaitEvent(dwTimeOut);
}

const DWORD CPtBaseTuner::GetAvailableNum(void)
{
	// ���o���\�X�g���[������Ԃ�
	return m_FifoBuffer.size();
}

const bool CPtBaseTuner::SetProperty(const IConfigProperty *pProperty)
{
	if(!pProperty)return false;

	// �v���p�e�B�ݒ�
	return m_Property.CopyProperty(pProperty);
}

const bool CPtBaseTuner::GetProperty(IConfigProperty *pProperty)
{
	if(!pProperty)return false;

	// �v���p�e�B�擾
	return pProperty->CopyProperty(&m_Property);
}

const DWORD CPtBaseTuner::GetPropertyClassName(LPTSTR lpszClassName)
{
	static const TCHAR szPropertyClassName[] = TEXT("CPtBaseTunerProperty");

	// �v���p�e�B�N���X���擾
	if(lpszClassName)::_tcscpy(lpszClassName, szPropertyClassName);

	return ::_tcslen(szPropertyClassName);
}

CPtBaseTuner::CPtBaseTuner(IBonObject *pOwner, const Device::ISDB dwTunerType)
	: CBonObject(pOwner)
	, m_Property(static_cast<IConfigTarget *>(this))
	, m_pTuner(NULL)
	, m_dwTunerType(dwTunerType)
	, m_dwCurSpace(0UL)
	, m_dwCurChannel(0UL)
{
	// �}�l�[�W���C���X�^���X����
	if(!m_dwInstanceCount++){
		m_pPtManager = new CPtManager;
		}

	::BON_ASSERT(m_pPtManager != NULL);

	// �f�t�H���g�v���p�e�B���f
	SetProperty(&m_Property);
}

CPtBaseTuner::~CPtBaseTuner(void)
{
	CloseTuner();
	
	// �}�l�[�W���C���X�^���X�J��
	if(!--m_dwInstanceCount){
		BON_SAFE_DELETE(m_pPtManager);
		}
}

void CPtBaseTuner::OnTsStream(CPtTuner *pTuner, const BYTE *pData, const DWORD dwSize)
{
	CBlockLock AutoLock(&m_FifoLock);
	
	if(m_FifoBuffer.size() < (FIFOBUFFNUM - 1UL)){
		// FIFO�ɓ���
		m_itFreeBlock->SetData(pData, dwSize);
		m_FifoBuffer.push(&(*m_itFreeBlock));
		if(++m_itFreeBlock == m_BlockPool.end())m_itFreeBlock = m_BlockPool.begin();
		
		// �C�x���g�Z�b�g
		m_FifoEvent.SetEvent();
		}
	else{
		::BON_TRACE(TEXT("FIFO�o�b�t�@�I�[�o�[�t���[\n"));
		}
}
