// BonWhiteTuner.cpp: ��Friio�`���[�i�[�N���X
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Resource.h"
#include "BonSDK.h"
#include "BonWhiteTuner.h"
#include <Math.h>
#include <TChar.h>


/////////////////////////////////////////////////////////////////////////////
// �t�@�C�����[�J���萔�ݒ�
/////////////////////////////////////////////////////////////////////////////

// �f�o�C�X��
#define DEVICENAME	TEXT("�� Friio USB 2.0 Digital TV Receiver")

// �o�b�t�@�ݒ�
#define ASYNCBUFFTIME		2UL											// �o�b�t�@�� = 2�b
#define ASYNCBUFFNUM		(0x400000UL / TSRECVSIZE * ASYNCBUFFTIME)	// ����32Mbps�Ƃ���

#define REQRESERVNUM		16UL			// �񓯊����N�G�X�g�\��
#define REQPOLLINGWAIT		10UL			// �񓯊����N�G�X�g�|�[�����O�Ԋu(ms)

// I/O�R���g���[���R�[�h
#define IOCODE_CONTROL		0x00220020UL	// �`���[�i�R���g���[��
#define IOCODE_SETTSSIZE	0x00220038UL	// TS�X�g���[���T�C�Y�ݒ�
#define IOCODE_ABORTPIPE	0x00220044UL	// TS�X�g���[�����~
#define IOCODE_STREAM		0x0022004BUL	// �X�g���[�����N�G�X�g


/////////////////////////////////////////////////////////////////////////////
// ��Friio�p�@TS�`���[�i HAL�C���^�t�F�[�X�����N���X
/////////////////////////////////////////////////////////////////////////////

HINSTANCE CBonWhiteTuner::m_hModule = NULL;

const BONGUID CBonWhiteTuner::GetDeviceType(void)
{
	// �f�o�C�X�̃^�C�v��Ԃ�
	return ::BON_NAME_TO_GUID(TEXT("IHalTsTuner"));
}

const DWORD CBonWhiteTuner::GetDeviceName(LPTSTR lpszName)
{
	// �f�o�C�X����Ԃ�
	if(lpszName)::_tcscpy(lpszName, DEVICENAME);

	return ::_tcslen(DEVICENAME);
}

const DWORD CBonWhiteTuner::GetTotalDeviceNum(void)
{
	// ��Friio�̑�����Ԃ�
	return (m_BonEnumerator.EnumDevice(false))? m_BonEnumerator.GetTotalNum() : 0UL;
}

const DWORD CBonWhiteTuner::GetActiveDeviceNum(void)
{
	// �g�p���̔�Friio�̐���Ԃ�
	return (m_BonEnumerator.EnumDevice(false))? m_BonEnumerator.GetActiveNum() : 0UL;
}

const bool CBonWhiteTuner::OpenTuner(void)
{
	// ��U�N���[�Y
	CloseTuner();
	
	try{
		// �h���C�o��
		if(!m_BonEnumerator.EnumDevice(false))throw __LINE__;
	
		// ���g�p�̃`���[�i���擾
		LPCTSTR lpszDriverPath = m_BonEnumerator.GetAvailableDriverPath();
		if(!lpszDriverPath)throw __LINE__;

		// �h���C�o�I�[�v��
		if((m_hFriioDriver = ::CreateFile(lpszDriverPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL)) == INVALID_HANDLE_VALUE)throw __LINE__;

		// �h���C�o���b�N
		if(!m_BonEnumerator.LockDevice(lpszDriverPath))throw __LINE__;
		
		// �������V�[�P���X���M
		if(!SendFixedRequest(IDR_INISEQ_WHITE))throw __LINE__;
		
		// �f�t�H���g�`�����l���ݒ�
		if(!SetChannel(0UL, 0UL))throw __LINE__;
		
		// �C�x���g���Z�b�g
		if(!m_TsRecvEvent.ResetEvent())throw __LINE__;
		
		// �o�b�t�@�m��
		if(!AllocateBuffer(ASYNCBUFFNUM + REQRESERVNUM))throw __LINE__;
		
		// �X���b�h�N��
		if(!m_IoReqThread.StartThread(this, &CBonWhiteTuner::IoReqThread))throw __LINE__;
		if(!m_IoRecvThread.StartThread(this, &CBonWhiteTuner::IoRecvThread))throw __LINE__;
		}
	catch(...){
		// �G���[����
		CloseTuner();
		return false;
		}
	
	return true;
}

void CBonWhiteTuner::CloseTuner(void)
{
	// �X���b�h��~
	m_IoReqThread.EndThread();
	m_IoRecvThread.EndThread();

	// �C�x���g�Z�b�g(�ҋ@�X���b�h�J��)
	m_TsRecvEvent.SetEvent();

	// �o�b�t�@�J��
	ReleaseBuffer();
	
	if(m_hFriioDriver != INVALID_HANDLE_VALUE){
		// �I���V�[�P���X���M
		SendFixedRequest(IDR_ENDSEQ_WHITE);

		// �h���C�o�N���[�Y
		::CloseHandle(m_hFriioDriver);
		m_hFriioDriver = INVALID_HANDLE_VALUE;
		}

	// �h���C�o���b�N�J��
	m_BonEnumerator.ReleaseDevice();
	
	// �`�����l��������
	m_dwCurChannel = 0UL;
}

const DWORD CBonWhiteTuner::EnumTuningSpace(const DWORD dwSpace, LPTSTR lpszSpace)
{
	static const TCHAR aszTuningSpace[][16] = 
	{
		TEXT("�n�f�W")
	};

	// �g�p�\�ȃ`���[�j���O��Ԃ�Ԃ�
	if(dwSpace >= (sizeof(aszTuningSpace) / sizeof(aszTuningSpace[0])))return 0UL;

	// �`���[�j���O��Ԗ����R�s�[
	if(lpszSpace)::_tcscpy(lpszSpace, aszTuningSpace[dwSpace]);

	// �`���[�j���O��Ԗ�����Ԃ�
	return ::_tcslen(aszTuningSpace[dwSpace]);
}

const DWORD CBonWhiteTuner::EnumChannelName(const DWORD dwSpace, const DWORD dwChannel, LPTSTR lpszChannel)
{
	// �g�p�\�ȃ`�����l����Ԃ�
	static const TCHAR aszChannelName[][16] = 
	{
		TEXT("UHF 13ch"), TEXT("UHF 14ch"), TEXT("UHF 15ch"), TEXT("UHF 16ch"), TEXT("UHF 17ch"), TEXT("UHF 18ch"), TEXT("UHF 19ch"),
		TEXT("UHF 20ch"), TEXT("UHF 21ch"), TEXT("UHF 22ch"), TEXT("UHF 23ch"), TEXT("UHF 24ch"), TEXT("UHF 25ch"), TEXT("UHF 26ch"), TEXT("UHF 27ch"), TEXT("UHF 28ch"), TEXT("UHF 29ch"),
		TEXT("UHF 30ch"), TEXT("UHF 31ch"), TEXT("UHF 32ch"), TEXT("UHF 33ch"), TEXT("UHF 34ch"), TEXT("UHF 35ch"), TEXT("UHF 36ch"), TEXT("UHF 37ch"), TEXT("UHF 38ch"), TEXT("UHF 39ch"), 
		TEXT("UHF 40ch"), TEXT("UHF 41ch"), TEXT("UHF 42ch"), TEXT("UHF 43ch"), TEXT("UHF 44ch"), TEXT("UHF 45ch"), TEXT("UHF 46ch"), TEXT("UHF 47ch"), TEXT("UHF 48ch"), TEXT("UHF 49ch"), 
		TEXT("UHF 50ch"), TEXT("UHF 51ch"), TEXT("UHF 52ch"), TEXT("UHF 53ch"), TEXT("UHF 54ch"), TEXT("UHF 55ch"), TEXT("UHF 56ch"), TEXT("UHF 57ch"), TEXT("UHF 58ch"), TEXT("UHF 59ch"), 
		TEXT("UHF 60ch"), TEXT("UHF 61ch"), TEXT("UHF 62ch")
	};

	// �g�p�\�ȃ`�����l����Ԃ�
	if(!EnumTuningSpace(dwSpace, NULL))return 0UL;
	if(dwChannel >= (sizeof(aszChannelName) / sizeof(aszChannelName[0])))return 0UL;

	// �`�����l�������R�s�[
	if(lpszChannel)::_tcscpy(lpszChannel, aszChannelName[dwChannel]);

	// �`�����l��������Ԃ�
	return ::_tcslen(aszChannelName[dwSpace]);
}

const DWORD CBonWhiteTuner::GetCurSpace(void)
{
	// ���݂̃`���[�j���O��Ԃ�Ԃ�
	return 0UL;
}

const DWORD CBonWhiteTuner::GetCurChannel(void)
{
	// ���݂̃`�����l����Ԃ�
	return m_dwCurChannel;
}

const bool CBonWhiteTuner::SetChannel(const DWORD dwSpace, const DWORD dwChannel)
{
	// �`���[�j���O�V�[�P���X�ϕ���
	static const BYTE abyPllConfA[] = {0x81U};
	static const BYTE abyPllConfB[] = {0x40U, 0x03U, 0x00U, 0x30U, 0xFEU, 0x00U, 0x05U, 0x00U, 0x0AU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x26U, 0x00U, 0x00U, 0x00U, 0x05U, 0x00U, 0x00U, 0x00U, 0xC0U, 0x00U, 0x00U, 0xB2U, 0x08U};
	static const BYTE abyPllConfC[] = {0x40U, 0x03U, 0x00U, 0x30U, 0xFEU, 0x00U, 0x05U, 0x00U, 0x0AU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x26U, 0x00U, 0x00U, 0x00U, 0x05U, 0x00U, 0x00U, 0x00U, 0xC0U, 0x00U, 0x00U, 0x9AU, 0x50U};
	static const BYTE abyPllConfD[] = {0xC0U, 0x02U, 0x00U, 0x30U, 0xB0U, 0x00U, 0x01U, 0x00U, 0x0AU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x26U, 0x00U, 0x00U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U, 0x00U};
	static const BYTE abyPllConfE[] = {0xC0U, 0x02U, 0x00U, 0x30U, 0x80U, 0x00U, 0x01U, 0x00U, 0x0AU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x26U, 0x00U, 0x00U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U, 0x00U};
	static const BYTE abyPllConfF[] = {0x81U, 0x00U, 0xBCU, 0x00U, 0x00U};

	// �`���[�j���O�͈̓`�F�b�N
	if(!EnumChannelName(dwSpace, dwChannel, NULL))return false;

	// �`�����l���f�[�^�v�Z
	const WORD wChCode = 0x0E7F + 0x002A * (WORD)dwChannel;

	BYTE TxdData[256];

	// 1��ڃ��N�G�X�g���M
	if(SendIoRequest(IOCODE_ABORTPIPE, abyPllConfA, sizeof(abyPllConfA), m_RxdBuff, 0UL) == -1)return false;
	if(SendIoRequest(IOCODE_ABORTPIPE, abyPllConfA, sizeof(abyPllConfA), m_RxdBuff, 0UL) == -1)return false;

	// 2��ڃ��N�G�X�g�ݒ�
	::CopyMemory(TxdData, abyPllConfB, sizeof(abyPllConfB));
	TxdData[39] = (BYTE)(wChCode >> 8);
	TxdData[40] = (BYTE)(wChCode & 0xFF);

	// 2��ڃ��N�G�X�g���M
	if(SendIoRequest(IOCODE_CONTROL, TxdData, sizeof(abyPllConfB), m_RxdBuff, sizeof(abyPllConfB)) == -1)return false;

	// 3��ڃ��N�G�X�g�ݒ�
	CopyMemory(TxdData, abyPllConfC, sizeof(abyPllConfC));
	TxdData[39] = (BYTE)(wChCode >> 8);
	TxdData[40] = (BYTE)(wChCode & 0xFF);

	// 3��ڃ��N�G�X�g���M
	if(SendIoRequest(IOCODE_CONTROL, TxdData, sizeof(abyPllConfC), m_RxdBuff, sizeof(abyPllConfC)) == -1)return false;

	// 4��ڃ��N�G�X�g���M
	if(SendIoRequest(IOCODE_CONTROL, abyPllConfD, sizeof(abyPllConfD), m_RxdBuff, sizeof(abyPllConfD)) == -1)return false;

	// 5��ڃ��N�G�X�g���M
	if(SendIoRequest(IOCODE_CONTROL, abyPllConfE, sizeof(abyPllConfE), m_RxdBuff, sizeof(abyPllConfE)) == -1)return false;

	// 6��ڃ��N�G�X�g���M
	if(SendIoRequest(IOCODE_SETTSSIZE, abyPllConfF, sizeof(abyPllConfF), m_RxdBuff, sizeof(abyPllConfF)) == -1)return false;

	// �`�����l�����X�V
	m_dwCurChannel = dwChannel;

	// �X�g���[���p�[�W
	PurgeStream();

	return true;
}

const bool CBonWhiteTuner::SetLnbPower(const bool bEnable)
{
	// ��ɔ�Ή�
	return false;
}

const float CBonWhiteTuner::GetSignalLevel(void)
{
	// �M�����x���v��
	static const BYTE abyGetLevReq[] = {0xC0U, 0x02U, 0x00U, 0x30U, 0x89U, 0x00U, 0x25U, 0x00U, 0x0AU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x26U, 0x00U, 0x00U, 0x00U, 0x25U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x31U, 0x3AU, 0x4BU, 0x03U, 0x00U, 0x34U, 0x08U, 0x78U, 0x00U, 0x00U, 0x40U, 0x7FU, 0xDCU, 0xACU, 0x0FU, 0x00U, 0x00U, 0x00U, 0x01U, 0xE3U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x02U, 0x00U, 0x02U, 0x00U, 0x02U, 0x00U, 0x00U, 0xF4U};

	// ��M�o�b�t�@�N���A
	::ZeroMemory(m_RxdBuff, sizeof(abyGetLevReq));

	// �M�����x���v��
	if(SendIoRequest(IOCODE_CONTROL, (BYTE *)abyGetLevReq, sizeof(abyGetLevReq), m_RxdBuff, sizeof(abyGetLevReq)) == -1)return 0.0f;

	// �M�����x���v�Z
	const DWORD dwHex = ((DWORD)m_RxdBuff[40] << 16) | ((DWORD)m_RxdBuff[41] << 8) | (DWORD)m_RxdBuff[42];
	if(!dwHex)return 0.0f;

	return (float)(68.4344993f - log10((double)dwHex) * 10.0f + pow(log10((double)dwHex) - 5.045f, (double)4.0f) / 4.09f);
}

const bool CBonWhiteTuner::GetStream(BYTE **ppStream, DWORD *pdwSize, DWORD *pdwRemain)
{
	CBlockLock AutoLock(&m_AsyncReqLock);

	// ��M�L���[���珈���σf�[�^�����o��
	if(m_hFriioDriver == INVALID_HANDLE_VALUE){
		// �`���[�i���I�[�v������Ă��Ȃ�
		return false;
		}
	else if(m_AsyncRecvQue.empty()){
		// ���o���\�ȃf�[�^���Ȃ�
		if(ppStream)*ppStream = NULL;
		if(pdwSize)*pdwSize = 0UL;
		if(pdwRemain)*pdwRemain = 0UL;
		}
	else{	
		// �f�[�^�����o��
		if(ppStream)*ppStream = m_AsyncRecvQue.front()->RxdBuff;
		if(pdwSize)*pdwSize = m_AsyncRecvQue.front()->dwRxdSize;

		m_AsyncRecvQue.pop();
		
		if(pdwRemain)*pdwRemain = m_AsyncRecvQue.size();
		}

	return true;
}

void CBonWhiteTuner::PurgeStream(void)
{
	CBlockLock AutoLock(&m_AsyncReqLock);

	// ��M�L���[�N���A
	while(!m_AsyncRecvQue.empty())m_AsyncRecvQue.pop();
}

const DWORD CBonWhiteTuner::WaitStream(const DWORD dwTimeOut)
{
	// �X�g���[�������E�F�C�g
	return m_TsRecvEvent.WaitEvent(dwTimeOut);
}

const DWORD CBonWhiteTuner::GetAvailableNum(void)
{
	// ���o���\�X�g���[������Ԃ�
	return m_AsyncRecvQue.size();
}

CBonWhiteTuner::CBonWhiteTuner(IBonObject *pOwner)
	: CBonObject(pOwner)
	, m_hFriioDriver(NULL)
	, m_dwCurChannel(0UL)
{

}

CBonWhiteTuner::~CBonWhiteTuner(void)
{
	CloseTuner();
}

void CBonWhiteTuner::SetModuleHandle(const HINSTANCE hModule)
{
	// ���W���[���n���h����o�^
	m_hModule = hModule;
}

const int CBonWhiteTuner::SendIoRequest(const DWORD dwIoCode, const BYTE *pTxdData, const DWORD dwTxdSize, BYTE *pRxdData, const DWORD dwRxdSize)
{
	// �I�[�v���`�F�b�N
	if(m_hFriioDriver == INVALID_HANDLE_VALUE)return -1;

	// ���M�p�f�[�^�ݒ�
	OVERLAPPED OverLapped;
	::ZeroMemory(&OverLapped, sizeof(OverLapped));
	if(!(OverLapped.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL)))return -1;

	// ���M�f�[�^�R�s�[
	::CopyMemory(m_TxdBuff, pTxdData, dwTxdSize);

	// �f�[�^���M
	DWORD dwReturned = 0UL;
	const BOOL bReturn = ::DeviceIoControl(m_hFriioDriver, dwIoCode, m_TxdBuff, dwTxdSize, pRxdData, dwRxdSize, &dwReturned, &OverLapped);
	
	// �C�x���g�폜
	::CloseHandle(OverLapped.hEvent);

	// �G���[����A��M�f�[�^�擾
	if(!bReturn){
		if(::GetLastError() != ERROR_IO_PENDING)return -1;
		if(!::GetOverlappedResult(m_hFriioDriver, &OverLapped, &dwReturned, TRUE))return -1;
		}

	return (int)dwReturned;
}

const bool CBonWhiteTuner::SendFixedRequest(const UINT nID)
{
	// ���\�[�X�n���h���擾
	HRSRC hRsrc = ::FindResource(m_hModule, MAKEINTRESOURCE(nID), TEXT("BINARY"));
	if(!hRsrc)return false;

	// ���\�[�X���[�h
	HGLOBAL hGlobal = ::LoadResource(m_hModule, hRsrc);
	if(!hGlobal)return false;

	// �������|�C���^�擾
	BYTE *pFixedBuff = (BYTE *)::LockResource(hGlobal);
	if(!pFixedBuff)return false;

	// ���\�[�X�T�C�Y���擾
	const DWORD dwIoReqSize = ::SizeofResource(m_hModule, hRsrc);

	// ���\�[�X�f�[�^�𑗐M
	DWORD dwXmited = 0UL;
	FIXED_REQ_HEAD *pFixedReqHead;

	while(dwXmited < dwIoReqSize){
		// �R���g���[�����擾
		pFixedReqHead = (FIXED_REQ_HEAD *)pFixedBuff;
		pFixedBuff += sizeof(FIXED_REQ_HEAD);

		// �f�[�^���M
		if(SendIoRequest(pFixedReqHead->dwIoCode, pFixedBuff, pFixedReqHead->wTxdSize, m_RxdBuff, pFixedReqHead->wRxdSize) == -1)return false;
		pFixedBuff += pFixedReqHead->wTxdSize;
		
		// ���M�σT�C�Y�X�V
		dwXmited += (sizeof(FIXED_REQ_HEAD) + pFixedReqHead->wTxdSize);
		}

	return true;
}

const bool CBonWhiteTuner::AllocateBuffer(const DWORD dwBufferNum)
{
	// �o�b�t�@���m�ۂ���
	m_AsyncReqPool.resize(dwBufferNum);
	if(m_AsyncReqPool.size() < dwBufferNum)return false;

	// �o�b�t�@�v�[���ʒu������
	m_itAsyncReq = m_AsyncReqPool.begin();

	return true;
}

void CBonWhiteTuner::ReleaseBuffer(void)
{
	// ���N�G�X�g�L���[�J��
	while(!m_AsyncReqQue.empty())m_AsyncReqQue.pop();

	// ��M�L���[�J��
	while(!m_AsyncRecvQue.empty())m_AsyncRecvQue.pop();

	// �o�b�t�@���J������
	m_AsyncReqPool.clear();
}

void CBonWhiteTuner::IoReqThread(CSmartThread<CBonWhiteTuner> *pThread, bool &bKillSignal, PVOID pParam)
{
	// �h���C�o�ɃX�g���[�����N�G�X�g�𔭍s����
	while(!bKillSignal){
		// ���N�G�X�g�����҂����K�薢���Ȃ�ǉ�����
		if(m_AsyncReqQue.size() < REQRESERVNUM){

			// �h���C�o��TS�f�[�^���N�G�X�g�𔭍s����
			if(!PushIoRequest())break;

			continue;
			}

		// ���N�G�X�g�����҂����t���̏ꍇ�̓E�F�C�g
		::Sleep(REQPOLLINGWAIT);
		}
}

void CBonWhiteTuner::IoRecvThread(CSmartThread<CBonWhiteTuner> *pThread, bool &bKillSignal, PVOID pParam)
{
	// �����σ��N�G�X�g���|�[�����O���ă��N�G�X�g������������
	while(!bKillSignal){
		// �����σf�[�^������΃��N�G�X�g����������
		if(m_AsyncReqQue.size() && (m_AsyncRecvQue.size() < ASYNCBUFFNUM)){

			// ���N�G�X�g����������
			if(!PopIoRequest())break;

			continue;
			}

		// �����σL���[���t���̏ꍇ�̓E�F�C�g
		::Sleep(REQPOLLINGWAIT);
		}
}

const bool CBonWhiteTuner::PushIoRequest(void)
{
	// �X�g���[�����N�G�X�g
	static const BYTE abyStreamReq[]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	// ���N�G�X�g�L���[�Ƀ��N�G�X�g��ǉ�����
	if(m_hFriioDriver == INVALID_HANDLE_VALUE)return false;

	// ���N�G�X�g�ݒ�
	m_itAsyncReq->dwRxdSize = 0UL;
	::CopyMemory(m_itAsyncReq->TxdBuff, abyStreamReq, sizeof(abyStreamReq));
	::ZeroMemory(&m_itAsyncReq->OverLapped, sizeof(OVERLAPPED));
	if(!(m_itAsyncReq->OverLapped.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL)))return false;
	
	// ���N�G�X�g���s
	if(!::DeviceIoControl(m_hFriioDriver, IOCODE_STREAM, m_itAsyncReq->TxdBuff, sizeof(abyStreamReq), m_itAsyncReq->RxdBuff, TSRECVSIZE, &m_itAsyncReq->dwRxdSize, &m_itAsyncReq->OverLapped)){
		if(::GetLastError() != ERROR_IO_PENDING){
			::CloseHandle(m_itAsyncReq->OverLapped.hEvent);
			return false;
			}
		}

	// �L���[�ɓ���
	m_AsyncReqLock.Lock();
	m_AsyncReqQue.push(&(*m_itAsyncReq));
	if(++m_itAsyncReq == m_AsyncReqPool.end())m_itAsyncReq = m_AsyncReqPool.begin();
	m_AsyncReqLock.Unlock();
	
	return true;
}

const bool CBonWhiteTuner::PopIoRequest(void)
{
	// ���N�G�X�g�L���[���珈���σf�[�^�����o���A��M�L���[�ɒǉ�����
	if(m_hFriioDriver == INVALID_HANDLE_VALUE)return false;

	// ���N�G�X�g�������ʎ擾
	m_AsyncReqLock.Lock();
	ASYNC_REQ_DATA *pFrontReq = m_AsyncReqQue.front();
	m_AsyncReqLock.Unlock();
	
	if(!::GetOverlappedResult(m_hFriioDriver, &pFrontReq->OverLapped, &pFrontReq->dwRxdSize, FALSE)){
		if(::GetLastError() == ERROR_IO_INCOMPLETE){
			// �����������A�E�F�C�g
			::Sleep(REQPOLLINGWAIT);
			return true;
			}
		else{
			// �G���[����
			::CloseHandle(pFrontReq->OverLapped.hEvent);		
			return false;
			}
		}

	// �C�x���g�J��
	::CloseHandle(pFrontReq->OverLapped.hEvent);

	// �L�[������o���A�ǉ�
	m_AsyncReqLock.Lock();
	m_AsyncReqQue.pop();
	m_AsyncRecvQue.push(pFrontReq);
	m_AsyncReqLock.Unlock();

	// �C�x���g�Z�b�g
	m_TsRecvEvent.SetEvent();

	return true;
}
