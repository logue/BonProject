// BonBlackTuner.cpp: ��Friio�`���[�i�[�N���X
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Resource.h"
#include "BonSDK.h"
#include "BonBlackTuner.h"
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
#define IOCODE_STREAM		0x0022004BUL	// �X�g���[�����N�G�X�g


/////////////////////////////////////////////////////////////////////////////
// ��Friio�p�@TS�`���[�i HAL�C���^�t�F�[�X�����N���X
/////////////////////////////////////////////////////////////////////////////

HINSTANCE CBonBlackTuner::m_hModule = NULL;

const BONGUID CBonBlackTuner::GetDeviceType(void)
{
	// �f�o�C�X�̃^�C�v��Ԃ�
	return ::BON_NAME_TO_GUID(TEXT("IHalTsTuner"));
}

const DWORD CBonBlackTuner::GetDeviceName(LPTSTR lpszName)
{
	// �f�o�C�X����Ԃ�
	if(lpszName)::_tcscpy(lpszName, DEVICENAME);

	return ::_tcslen(DEVICENAME);
}

const DWORD CBonBlackTuner::GetTotalDeviceNum(void)
{
	// ��Friio�̑�����Ԃ�
	return (m_BonEnumerator.EnumDevice(true))? m_BonEnumerator.GetTotalNum() : 0UL;
}

const DWORD CBonBlackTuner::GetActiveDeviceNum(void)
{
	// �g�p���̍�Friio�̐���Ԃ�
	return (m_BonEnumerator.EnumDevice(true))? m_BonEnumerator.GetActiveNum() : 0UL;
}

const bool CBonBlackTuner::OpenTuner(void)
{
	// ��U�N���[�Y
	CloseTuner();
	
	try{
		// �h���C�o��
		if(!m_BonEnumerator.EnumDevice(true))throw __LINE__;
	
		// ���g�p�̃`���[�i���擾
		LPCTSTR lpszDriverPath = m_BonEnumerator.GetAvailableDriverPath();
		if(!lpszDriverPath)throw __LINE__;

		// �h���C�o�I�[�v��
		if((m_hFriioDriver = ::CreateFile(lpszDriverPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL)) == INVALID_HANDLE_VALUE)throw __LINE__;

		// �h���C�o���b�N
		if(!m_BonEnumerator.LockDevice(lpszDriverPath))throw __LINE__;
		
		// �������V�[�P���X���M
		if(!SendFixedRequest(IDR_INISEQ_BLACK))throw __LINE__;
		
		// �f�t�H���g�`�����l���ݒ�
		if(!SetChannel(0UL, 0UL))throw __LINE__;
		
		// �C�x���g���Z�b�g
		if(!m_TsRecvEvent.ResetEvent())throw __LINE__;
		
		// �o�b�t�@�m��
		if(!AllocateBuffer(ASYNCBUFFNUM + REQRESERVNUM))throw __LINE__;
		
		// �X���b�h�N��
		if(!m_IoReqThread.StartThread(this, &CBonBlackTuner::IoReqThread))throw __LINE__;
		if(!m_IoRecvThread.StartThread(this, &CBonBlackTuner::IoRecvThread))throw __LINE__;
		}
	catch(...){
		// �G���[����
		CloseTuner();
		return false;
		}
	
	return true;
}

void CBonBlackTuner::CloseTuner(void)
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
		SendFixedRequest(IDR_ENDSEQ_BLACK);

		// �h���C�o�N���[�Y
		::CloseHandle(m_hFriioDriver);
		m_hFriioDriver = INVALID_HANDLE_VALUE;
		}

	// �h���C�o���b�N�J��
	m_BonEnumerator.ReleaseDevice();
	
	// �`�����l��������
	m_dwCurSpace = 0UL;
	m_dwCurChannel = 0UL;
}

const DWORD CBonBlackTuner::EnumTuningSpace(const DWORD dwSpace, LPTSTR lpszSpace)
{
	static const TCHAR aszTuningSpace[][16] = 
	{
		TEXT("BS"	),
		TEXT("110CS")
	};

	// �g�p�\�ȃ`���[�j���O��Ԃ�Ԃ�
	if(dwSpace >= (sizeof(aszTuningSpace) / sizeof(aszTuningSpace[0])))return 0UL;

	// �`���[�j���O��Ԗ����R�s�[
	if(lpszSpace)::_tcscpy(lpszSpace, aszTuningSpace[dwSpace]);

	// �`���[�j���O��Ԗ�����Ԃ�
	return ::_tcslen(aszTuningSpace[dwSpace]);
}

const DWORD CBonBlackTuner::EnumChannelName(const DWORD dwSpace, const DWORD dwChannel, LPTSTR lpszChannel)
{
	// �g�p�\�ȃ`�����l����Ԃ�
	static const TCHAR aszChannelNameBS[][32] = 
	{
		TEXT("BS1/TS0 BS����"			),
		TEXT("BS1/TS1 BS-i"				),
		TEXT("BS3/TS0 WOWOW"			),
		TEXT("BS3/TS1 BS�W���p��"		),
		TEXT("BS9/TS0 BS11"				),
		TEXT("BS9/TS1 Star Channel HV"	),
		TEXT("BS9/TS2 TwellV"			),
		TEXT("BS13/TS0 BS���e��"		),
		TEXT("BS13/TS1 BS�t�W"			),
		TEXT("BS15/TS1 NHK BS1/2"		),
		TEXT("BS15/TS2 NHK BS-hi"		)
	};

	static const TCHAR aszChannelNameCS[][32] = 
	{
		TEXT("ND2 110CS #1"  ),
		TEXT("ND4 110CS #2"  ),
		TEXT("ND6 110CS #3"  ),
		TEXT("ND8 110CS #4"  ),
		TEXT("ND10 110CS #5" ),
		TEXT("ND12 110CS #6" ),
		TEXT("ND14 110CS #7" ),
		TEXT("ND16 110CS #8" ),
		TEXT("ND18 110CS #9" ),
		TEXT("ND20 110CS #10"),
		TEXT("ND22 110CS #11"),
		TEXT("ND24 110CS #12")
	};

	switch(dwSpace){
		case 0UL :
			// BS
			if(dwChannel >= (sizeof(aszChannelNameBS) / sizeof(aszChannelNameBS[0])))return 0UL;

			// �`�����l�������R�s�[
			if(lpszChannel)::_tcscpy(lpszChannel, aszChannelNameBS[dwChannel]);

			// �`�����l��������Ԃ�
			return ::_tcslen(aszChannelNameBS[dwSpace]);
		
		case 1UL :
			// 110CS
			if(dwChannel >= (sizeof(aszChannelNameCS) / sizeof(aszChannelNameCS[0])))return 0UL;

			// �`�����l�������R�s�[
			if(lpszChannel)::_tcscpy(lpszChannel, aszChannelNameCS[dwChannel]);

			// �`�����l��������Ԃ�
			return ::_tcslen(aszChannelNameCS[dwSpace]);
		
		default  :
			// ��Ή��̃`���[�j���O���
			return 0UL;
		}
}

const DWORD CBonBlackTuner::GetCurSpace(void)
{
	// ���݂̃`���[�j���O��Ԃ�Ԃ�
	return m_dwCurSpace;
}

const DWORD CBonBlackTuner::GetCurChannel(void)
{
	// ���݂̃`�����l����Ԃ�
	return m_dwCurChannel;
}

const bool CBonBlackTuner::SetChannel(const DWORD dwSpace, const DWORD dwChannel)
{
	// �`���[�j���O�V�[�P���X�ϕ���
	static const BYTE abyPllConfA[] = {0x08U, 0x08U, 0x08U, 0x08U, 0x09U, 0x09U, 0x09U, 0x0AU, 0x0AU, 0x0AU, 0x0AU, 0x0CU, 0x0CU, 0x0DU, 0x0DU, 0x0DU, 0x0EU, 0x0EU, 0x0EU, 0x0FU, 0x0FU, 0x0FU, 0x10U};	// 1/17��� +40
	static const BYTE abyPllConfB[] = {0x32U, 0x32U, 0x80U, 0x80U, 0x64U, 0x64U, 0x64U, 0x00U, 0x00U, 0x4CU, 0x4CU, 0x9AU, 0xEAU, 0x3AU, 0x8AU, 0xDAU, 0x2AU, 0x7AU, 0xCAU, 0x1AU, 0x6AU, 0xBAU, 0x0AU};	// 2/18��� +40
	static const BYTE abyPllConfC[] = {0x40U, 0x40U, 0x40U, 0x40U, 0x40U, 0x40U, 0x40U, 0x40U, 0x40U, 0x40U, 0x40U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U};	// 75���	+02
	static const BYTE abyPllConfD[] = {0x10U, 0x11U, 0x30U, 0x31U, 0x90U, 0x91U, 0x92U, 0xD0U, 0xD1U, 0xF1U, 0xF2U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U};	// 76���	+02

	// �`���[�j���O�͈̓`�F�b�N
	if(!EnumChannelName(dwSpace, dwChannel, NULL))return false;

	// ���\�[�X�n���h���擾
	HRSRC hRsrc = ::FindResource(m_hModule, MAKEINTRESOURCE(IDR_CHSEQ_BLACK), TEXT("BINARY"));
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
	const DWORD dwChIndex = dwSpace * 11UL + dwChannel;
	DWORD dwXmitNum = 0UL;
	DWORD dwXmited = 0UL;
	FIXED_REQ_HEAD *pFixedReqHead;

	while(dwXmited < dwIoReqSize){
		// �R���g���[�����擾
		pFixedReqHead = (FIXED_REQ_HEAD *)pFixedBuff;
		pFixedBuff += sizeof(FIXED_REQ_HEAD);

		// ���M�f�[�^����
		::CopyMemory(m_RxdBuff, pFixedBuff, pFixedReqHead->wTxdSize);

		switch(dwXmitNum++){
			case 0UL  :	m_RxdBuff[40] = abyPllConfA[dwChIndex];	break;
			case 1UL  :	m_RxdBuff[40] = abyPllConfB[dwChIndex];	break;
			case 16UL :	m_RxdBuff[40] = abyPllConfA[dwChIndex];	break;
			case 17UL :	m_RxdBuff[40] = abyPllConfB[dwChIndex];	break;
			case 74UL :	m_RxdBuff[ 2] = abyPllConfC[dwChIndex];	break;
			case 75UL :	m_RxdBuff[ 2] = abyPllConfD[dwChIndex];	break;
			}

		// �f�[�^���M
		if(SendIoRequest(pFixedReqHead->dwIoCode, m_RxdBuff, pFixedReqHead->wTxdSize, m_RxdBuff, pFixedReqHead->wRxdSize) == -1)return false;
		pFixedBuff += pFixedReqHead->wTxdSize;
		
		// ���M�σT�C�Y�X�V
		dwXmited += (sizeof(FIXED_REQ_HEAD) + pFixedReqHead->wTxdSize);
		}

	// �`�����l�����X�V
	m_dwCurSpace = dwSpace;
	m_dwCurChannel = dwChannel;

	// �X�g���[���p�[�W
	PurgeStream();

	return true;
}

const bool CBonBlackTuner::SetLnbPower(const bool bEnable)
{
	// ��ɔ�Ή�(�ߓd���ی�@�\�̂Ȃ�Friio�ł͏đ��̉\�������邽��)
	return false;
}

const float CBonBlackTuner::GetSignalLevel(void)
{
	// �M�����x���v��
	static const BYTE abyGetLevReq[] = {0xC0, 0x02 ,0x00, 0x32, 0xBA, 0x00, 0x04, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	// ���`�⊮�ŋߎ�����
	static const float afLevelTable[] =
	{
		24.07f,	// 00	00	0		24.07dB
		24.07f,	// 10	00	4096	24.07dB
		18.61f,	// 20	00	8192	18.61dB
		15.21f,	// 30	00	12288	15.21dB
		12.50f,	// 40	00	16384	12.50dB
		10.19f,	// 50	00	20480	10.19dB
		8.140f,	// 60	00	24576	8.140dB
		6.270f,	// 70	00	28672	6.270dB
		4.550f,	// 80	00	32768	4.550dB
		3.730f,	// 88	00	34816	3.730dB
		3.630f,	// 88	FF	35071	3.630dB
		2.940f,	// 90	00	36864	2.940dB
		1.420f,	// A0	00	40960	1.420dB
		0.000f	// B0	00	45056	-0.01dB
	};

	// ��M�o�b�t�@�N���A
	::ZeroMemory(m_RxdBuff, sizeof(abyGetLevReq));

	// �M�����x���v��
	if(SendIoRequest(IOCODE_CONTROL, (BYTE *)abyGetLevReq, sizeof(abyGetLevReq), m_RxdBuff, sizeof(abyGetLevReq)) == -1)return 0.0f;

	// �M�����x���v�Z
	if(m_RxdBuff[40] <= 0x10U){
		// �ő�N���b�v
		return 24.07f;
		}
	else if(m_RxdBuff[40] >= 0xB0U){
		// �ŏ��N���b�v
		return 0.0f;
		}
	else{
		// ���`�⊮
		const float fMixRate = (float)(((WORD)(m_RxdBuff[40] & 0x0FU) << 8) | (WORD)m_RxdBuff[41]) / 4095.0f;
		return afLevelTable[m_RxdBuff[40] >> 4] * (1.0f - fMixRate) + afLevelTable[(m_RxdBuff[40] >> 4) + 0x01U] * fMixRate;
		}
}

const bool CBonBlackTuner::GetStream(BYTE **ppStream, DWORD *pdwSize, DWORD *pdwRemain)
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

void CBonBlackTuner::PurgeStream(void)
{
	CBlockLock AutoLock(&m_AsyncReqLock);

	// ��M�L���[�N���A
	while(!m_AsyncRecvQue.empty())m_AsyncRecvQue.pop();
}

const DWORD CBonBlackTuner::WaitStream(const DWORD dwTimeOut)
{
	// �X�g���[�������E�F�C�g
	return m_TsRecvEvent.WaitEvent(dwTimeOut);
}

const DWORD CBonBlackTuner::GetAvailableNum(void)
{
	// ���o���\�X�g���[������Ԃ�
	return m_AsyncRecvQue.size();
}

CBonBlackTuner::CBonBlackTuner(IBonObject *pOwner)
	: CBonObject(pOwner)
	, m_hFriioDriver(NULL)
	, m_dwCurSpace(0UL)
	, m_dwCurChannel(0UL)
{

}

CBonBlackTuner::~CBonBlackTuner(void)
{
	CloseTuner();
}

void CBonBlackTuner::SetModuleHandle(const HINSTANCE hModule)
{
	// ���W���[���n���h����o�^
	m_hModule = hModule;
}

const int CBonBlackTuner::SendIoRequest(const DWORD dwIoCode, const BYTE *pTxdData, const DWORD dwTxdSize, BYTE *pRxdData, const DWORD dwRxdSize)
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

const bool CBonBlackTuner::SendFixedRequest(const UINT nID)
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

const bool CBonBlackTuner::AllocateBuffer(const DWORD dwBufferNum)
{
	// �o�b�t�@���m�ۂ���
	m_AsyncReqPool.resize(dwBufferNum);
	if(m_AsyncReqPool.size() < dwBufferNum)return false;

	// �o�b�t�@�v�[���ʒu������
	m_itAsyncReq = m_AsyncReqPool.begin();

	return true;
}

void CBonBlackTuner::ReleaseBuffer(void)
{
	// ���N�G�X�g�L���[�J��
	while(!m_AsyncReqQue.empty())m_AsyncReqQue.pop();

	// ��M�L���[�J��
	while(!m_AsyncRecvQue.empty())m_AsyncRecvQue.pop();

	// �o�b�t�@���J������
	m_AsyncReqPool.clear();
}

void CBonBlackTuner::IoReqThread(CSmartThread<CBonBlackTuner> *pThread, bool &bKillSignal, PVOID pParam)
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

void CBonBlackTuner::IoRecvThread(CSmartThread<CBonBlackTuner> *pThread, bool &bKillSignal, PVOID pParam)
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

const bool CBonBlackTuner::PushIoRequest(void)
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

const bool CBonBlackTuner::PopIoRequest(void)
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
