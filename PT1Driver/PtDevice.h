// PtDevice.h: PT�f�o�C�X�N���X
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


#include "EARTH_PT.h"
#include "OS_Library.h"
#include <Vector>
#include <Queue>


using namespace EARTH;
using namespace OS;
using namespace PT;
using namespace std;


/////////////////////////////////////////////////////////////////////////////
// PT�}�l�[�W���N���X
/////////////////////////////////////////////////////////////////////////////

class CPtBoard;
class CPtTuner;

class CPtManager
{
public:
	CPtManager(void);
	virtual ~CPtManager(void);

	const DWORD GetTotalTunerNum(const Device::ISDB dwTunerType);
	const DWORD GetActiveTunerNum(const Device::ISDB dwTunerType);

	CPtTuner * OpenTuner(const Device::ISDB dwTunerType);

protected:
	const bool EnumerateBoards(void);
	const bool ReleaseInstance(void);
	
	Bus *m_pPtBus;
	vector<CPtBoard *> m_Boards;

	Library *m_pPtLibrary;
};


/////////////////////////////////////////////////////////////////////////////
// PT�{�[�h�N���X
/////////////////////////////////////////////////////////////////////////////

class CPtBoard
{
friend CPtTuner;

public:
	enum{TUNER_1ST = 0UL, TUNER_2ND = 1UL, TUNER_NUM = 2UL};

	enum{
		DMABUF_SIZE = 6UL,									// DMA�u���b�N��
		DMABUF_NUM  = 1UL,									// DMA�o�b�t�@��
		DMABLK_SIZE = 4096UL * Device::BUFFER_PAGE_COUNT,	// DMA�u���b�N�T�C�Y
		DMACHK_UNIT = 1UL,									// DMA�]�������`�F�b�N������
		DMACHK_WAIT = 5UL,									// DMA�]�������`�F�b�N�E�F�C�g
		DMACHK_TIMEOUT = 1000UL								// DMA�]�������^�C���A�E�g
		};

	CPtBoard(const DWORD dwBoardID, Device *pDevice, const Bus::DeviceInfo *pDeviceInfo);
	virtual ~CPtBoard(void);

	CPtTuner * OpenTuner(const Device::ISDB dwTunerType);
	void CloseTuner(CPtTuner *pTuner);

	const DWORD GetActiveTunerNum(const Device::ISDB dwTunerType);

protected:
	const bool StartStream(CPtTuner *pTuner);
	void StopStream(CPtTuner *pTuner);

	const bool SetLnbPower(CPtTuner *pTuner, const bool bEnable);

	const bool IsValidTuner(const CPtTuner *pTuner) const;
	const DWORD GetOpenTunerNum(void) const;
	const DWORD GetStreamTunerNum(void) const;
	const DWORD GetLnbOnTunerNum(void) const;

	struct DISPATCH_BUFF
	{
		DWORD dwPacketPos;
		BYTE *pDataPtr;
		BYTE abyDataBuff[DMABLK_SIZE * Device::ISDB_COUNT * TUNER_NUM];
	};
	
	const DWORD m_dwBoardID;
	Device *m_pDevice;
	Bus::DeviceInfo m_DeviceInfo;
	
	vector<CPtTuner *> m_TunerList;
	DISPATCH_BUFF m_aDispatchBuff[TUNER_NUM * Device::ISDB_COUNT];

	CSmartMutex m_BoardLock;
	CSmartLock m_TunerLock;

private:
	const bool OpenBoard(void);
	void CloseBoard(void);

	void StreamingThread(CSmartThread<CPtBoard> *pThread, bool &bKillSignal, PVOID pParam);
	void DispatchStream(const DWORD *pdwData, const DWORD dwNum);

	CSmartThread<CPtBoard> m_StreamingThread;
};


/////////////////////////////////////////////////////////////////////////////
// PT�`���[�i�N���X
/////////////////////////////////////////////////////////////////////////////

class CPtTuner
{
friend CPtBoard;

public:
	class ITsReceiver :	public IBonObject
	{
	public:
		virtual void OnTsStream(CPtTuner *pTuner, const BYTE *pData, const DWORD dwSize) = 0;
	};

	CPtTuner(CPtBoard &Board, const DWORD dwTunerID, const Device::ISDB dwTunerType);
	virtual ~CPtTuner();

	void CloseTuner(void);

	const bool StartStream(ITsReceiver *pTsReceiver);
	void StopStream(void);

	const bool SetChannel(const DWORD dwChannel, const DWORD dwOffset = 0UL);
	const bool SetTsID(const WORD wTsID);
	const bool SetLnbPower(const bool bEnable);
	const float GetSignalLevel(void);

protected:
	void OnTsStream(const BYTE *pData, const DWORD dwSize);

	CPtBoard &m_Board;

	const DWORD m_dwTunerID;
	const Device::ISDB m_dwTunerType;

	bool m_bIsOpen;
	bool m_bIsStream;
	bool m_bIsLnbOn;
	
	ITsReceiver *m_pTsReceiver;
	CSmartLock m_Lock;
};
