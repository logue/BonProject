// BonWhiteTuner.h: ��Friio�`���[�i�[�N���X
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Vector>
#include <Queue>
#include "BonEnumerator.h"


/////////////////////////////////////////////////////////////////////////////
// ��Friio�p�@TS�`���[�i HAL�C���^�t�F�[�X�����N���X
/////////////////////////////////////////////////////////////////////////////

#define TSRECVSIZE				48128UL			// TS�f�[�^��M�T�C�Y

class CBonWhiteTuner :	public CBonObject,
						public IHalDevice,
						public IHalTsTuner
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CBonWhiteTuner)

// IHalDevice
	virtual const BONGUID GetDeviceType(void);
	virtual const DWORD GetDeviceName(LPTSTR lpszName);
	virtual const DWORD GetTotalDeviceNum(void);
	virtual const DWORD GetActiveDeviceNum(void);

// IHalTsTuner
	virtual const bool OpenTuner(void);
	virtual void CloseTuner(void);
	
	virtual const DWORD EnumTuningSpace(const DWORD dwSpace, LPTSTR lpszSpace);
	virtual const DWORD EnumChannelName(const DWORD dwSpace, const DWORD dwChannel, LPTSTR lpszChannel);
	
	virtual const DWORD GetCurSpace(void);
	virtual const DWORD GetCurChannel(void);

	virtual const bool SetChannel(const DWORD dwSpace, const DWORD dwChannel);
	virtual const bool SetLnbPower(const bool bEnable);
	virtual const float GetSignalLevel(void);

	virtual const bool GetStream(BYTE **ppStream, DWORD *pdwSize, DWORD *pdwRemain);

	virtual void PurgeStream(void);
	virtual const DWORD WaitStream(const DWORD dwTimeOut = 0UL);
	virtual const DWORD GetAvailableNum(void);

// CBonWhiteTuner
	CBonWhiteTuner(IBonObject *pOwner);
	virtual ~CBonWhiteTuner(void);
	
	static void SetModuleHandle(const HINSTANCE hModule);
	
protected:
	const int SendIoRequest(const DWORD dwIoCode, const BYTE *pTxdData, const DWORD dwTxdSize, BYTE *pRxdData, const DWORD dwRxdSize);
	const bool SendFixedRequest(const UINT nID);

	const bool AllocateBuffer(const DWORD dwBufferNum);
	void ReleaseBuffer(void);

	void IoReqThread(CSmartThread<CBonWhiteTuner> *pThread, bool &bKillSignal, PVOID pParam);
	void IoRecvThread(CSmartThread<CBonWhiteTuner> *pThread, bool &bKillSignal, PVOID pParam);

	const bool PushIoRequest(void);
	const bool PopIoRequest(void);

	// �Œ�I/O���N�G�X�g�w�b�_
	#pragma pack(1)
	struct FIXED_REQ_HEAD
	{
		DWORD dwIoCode;		// I/O�R���g���[���R�[�h
		WORD wTxdSize;		// ���M�f�[�^�T�C�Y
		WORD wRxdSize;		// ��M�f�[�^�T�C�Y
	};
	#pragma pack()

	// I/O���N�G�X�g�L���[�f�[�^
	struct ASYNC_REQ_DATA
	{
		OVERLAPPED OverLapped;
		DWORD dwRxdSize;
		BYTE TxdBuff[64];
		BYTE RxdBuff[TSRECVSIZE];
	};

	CBonEnumerator m_BonEnumerator;

	HANDLE m_hFriioDriver;
	BYTE m_TxdBuff[256];
	BYTE m_RxdBuff[256];

	std::vector<ASYNC_REQ_DATA> m_AsyncReqPool;
	std::vector<ASYNC_REQ_DATA>::iterator m_itAsyncReq;
	std::queue<ASYNC_REQ_DATA *> m_AsyncReqQue;
	std::queue<ASYNC_REQ_DATA *> m_AsyncRecvQue;
	
	CSmartThread<CBonWhiteTuner> m_IoReqThread;
	CSmartThread<CBonWhiteTuner> m_IoRecvThread;
	
	CSmartLock m_AsyncReqLock;
	CSmartEvent m_TsRecvEvent;
	
	DWORD m_dwCurChannel;
	
	static HINSTANCE m_hModule;
};
