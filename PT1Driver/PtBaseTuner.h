// PtBaseTuner.h: PT�`���[�i���N���X
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


#include "PtDevice.h"
#include "PtDriverIf.h"
#include <vector>
#include <queue>


/////////////////////////////////////////////////////////////////////////////
// PT1�`���[�i�v���p�e�B�N���X
/////////////////////////////////////////////////////////////////////////////

class CPtBaseTunerProperty	:	public CBonObject,
								public IConfigProperty,
								public IPtBaseTunerProperty
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CPtBaseTunerProperty)

// IConfigProperty
	virtual const bool LoadProperty(IConfigStorage *pStorage);
	virtual const bool SaveProperty(IConfigStorage *pStorage);
	virtual const bool CopyProperty(const IConfigProperty *pProperty);
	virtual IConfigTarget * GetConfigTarget(void);
	virtual const DWORD GetDialogClassName(LPTSTR lpszClassName);

// CPtBaseTunerProperty
	CPtBaseTunerProperty(IBonObject *pOwner);
	virtual ~CPtBaseTunerProperty(void);
	
protected:
	IConfigTarget *m_pConfigTarget;
};


/////////////////////////////////////////////////////////////////////////////
// PT1�`���[�i���N���X
/////////////////////////////////////////////////////////////////////////////

class CPtBaseTuner :	public CBonObject,
						public IHalDevice,
						public IHalTsTuner,
						public IConfigTarget,
						protected CPtTuner::ITsReceiver
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CPtBaseTuner)

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

// IConfigTarget
	virtual const bool SetProperty(const IConfigProperty *pProperty);
	virtual const bool GetProperty(IConfigProperty *pProperty);
	virtual const DWORD GetPropertyClassName(LPTSTR lpszClassName);

// CPtBaseTuner
	struct PT_CH_CONFIG
	{
		const TCHAR szChName[32];		// �`�����l����
		const DWORD dwChIndex;			// PT1�`�����l���C���f�b�N�X
		const WORD wTsID;				// TS ID
	};

	struct PT_CH_SPACE
	{
		const TCHAR szSpaceName[32];	// ��Ԗ�
		const PT_CH_CONFIG *pChConfig;	// �`�����l���ݒ�
		const DWORD dwChNum;			// �`�����l����
	};

	CPtBaseTuner(IBonObject *pOwner, const Device::ISDB dwTunerType = Device::ISDB_T);
	virtual ~CPtBaseTuner(void);

protected:
// CPtTuner::ITsReceiver
	virtual void OnTsStream(CPtTuner *pTuner, const BYTE *pData, const DWORD dwSize);

	static CPtManager *m_pPtManager;
	static DWORD m_dwInstanceCount;

	CPtBaseTunerProperty m_Property;

	CPtTuner *m_pTuner;
	
	const Device::ISDB m_dwTunerType;
	std::vector<const PT_CH_SPACE *> m_TuningSpace;

	DWORD m_dwCurSpace;
	DWORD m_dwCurChannel;
	
	CSmartLock m_FifoLock;
	CSmartEvent m_FifoEvent;

	std::vector<CMediaData> m_BlockPool;
	std::vector<CMediaData>::iterator m_itFreeBlock;
	std::queue<CMediaData *> m_FifoBuffer;
};
