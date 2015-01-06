// PtDevice.cpp: PT�f�o�C�X�N���X
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "PtDevice.h"
#include <TChar.h>
#include <MmSystem.h>


#pragma comment(lib, "WinMm.lib")


/////////////////////////////////////////////////////////////////////////////
// PT�}�l�[�W���N���X
/////////////////////////////////////////////////////////////////////////////

CPtManager::CPtManager(void)
	: m_pPtBus(NULL)
	, m_pPtLibrary(new Library)
{
	// �^�C�}������
	::timeBeginPeriod(1UL);

	::BON_ASSERT(m_pPtLibrary != NULL);
	::BON_ASSERT(EnumerateBoards());
}

CPtManager::~CPtManager(void)
{
	::BON_ASSERT(ReleaseInstance());

	BON_SAFE_DELETE(m_pPtLibrary);

	// �^�C�}�J��
	::timeEndPeriod(1UL);
}

const DWORD CPtManager::GetTotalTunerNum(const Device::ISDB dwTunerType)
{
	// ��������Ă���PT1�̃`���[�i����Ԃ�
	return m_Boards.size() * CPtBoard::TUNER_NUM;
}

const DWORD CPtManager::GetActiveTunerNum(const Device::ISDB dwTunerType)
{
	// �g�p����Ă���PT1�̃`���[�i����Ԃ�
	DWORD dwActiveNum = 0UL;

	for(DWORD dwIndex = 0UL ; dwIndex < m_Boards.size() ; dwIndex++){
		dwActiveNum += m_Boards[dwIndex]->GetActiveTunerNum(dwTunerType);
		}

	return dwActiveNum;
}

CPtTuner * CPtManager::OpenTuner(const Device::ISDB dwTunerType)
{
	// ���g�p�̃`���[�i���������ĕԂ�
	for(DWORD dwBoardID = 0UL ; dwBoardID < m_Boards.size() ; dwBoardID++){
		CPtTuner *pTuner = m_Boards[dwBoardID]->OpenTuner(dwTunerType);
		if(pTuner)return pTuner;
		}

	return NULL;
}

const bool CPtManager::EnumerateBoards(void)
{
	// PT1�{�[�h��񋓂���
	try{
		// Bus�C���X�^���X�t�@�N�g���[�擾
		Bus::NewBusFunction pfnBusFactory = m_pPtLibrary->Function();
		if(!pfnBusFactory)throw ::BON_EXPECTION(TEXT("SDK_EARTHSOFT_PT1.dll�����[�h�ł��܂���"));

		// Bus�C���X�^���X����
		if(pfnBusFactory(&m_pPtBus) != STATUS_OK)throw ::BON_EXPECTION(TEXT("Bus�C���X�^���X�������s"));
		
		// PT1�{�[�h��
		Bus::DeviceInfo DeviceInfo[8];
		DWORD dwDeviceInfoNum = sizeof(DeviceInfo) / sizeof(*DeviceInfo);
		
		if(m_pPtBus->Scan(DeviceInfo, &dwDeviceInfoNum) != STATUS_OK)throw ::BON_EXPECTION(TEXT("PT�{�[�h�񋓎��s"));

		// PT1�f�o�C�X�C���X�^���X����
		for(DWORD dwBoardID = 0UL ; dwBoardID < dwDeviceInfoNum ; dwBoardID++){
			// �S�Ă�PT1�{�[�h�̃f�o�C�X�C���X�^���X�𐶐�����
			Device *pNewDevice = NULL;
			
			if(m_pPtBus->NewDevice(&DeviceInfo[dwBoardID], &pNewDevice) != STATUS_OK)throw ::BON_EXPECTION(TEXT("PT�f�o�C�X�C���X�^���X�������s"));
			
			// ���X�g�ɒǉ�
			m_Boards.push_back(new CPtBoard(dwBoardID, pNewDevice, &DeviceInfo[dwBoardID]));
			}
		}
	catch(CBonException &Exception){
		// �G���[����
		Exception.Notify();
		ReleaseInstance();
		return false;
		}
	
	return true;
}

const bool CPtManager::ReleaseInstance(void)
{
	// PT�{�[�h�̃C���X�^���X���J������
	for(DWORD dwBoardID = 0UL ; dwBoardID < m_Boards.size() ; dwBoardID++){
		delete m_Boards[dwBoardID];
		}
		
	m_Boards.clear();

	// �o�X�C���X�^���X�J��
	if(m_pPtBus){
		m_pPtBus->Delete();
		m_pPtBus = NULL;
		}

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// PT�{�[�h�N���X
/////////////////////////////////////////////////////////////////////////////

CPtBoard::CPtBoard(const DWORD dwBoardID, Device *pDevice, const Bus::DeviceInfo *pDeviceInfo)
	: m_dwBoardID(dwBoardID)
	, m_pDevice(pDevice)
	, m_DeviceInfo(*pDeviceInfo)
{
	::BON_ASSERT(m_pDevice != NULL);

	// �`���[�i�C���X�^���X����
	for(DWORD dwTunerID = 0UL ; dwTunerID < TUNER_NUM ; dwTunerID++){
		m_TunerList.push_back(new CPtTuner(*this, dwTunerID, Device::ISDB_S));
		m_TunerList.push_back(new CPtTuner(*this, dwTunerID, Device::ISDB_T));
		}
}

CPtBoard::~CPtBoard(void)
{
	// �`���[�i�C���X�^���X�J��
	for(DWORD dwIndex = 0UL ; dwIndex < m_TunerList.size() ; dwIndex++){
		delete m_TunerList[dwIndex];
		}

	// �f�o�C�X�C���X�^���X�J��
	m_pDevice->Delete();
}

CPtTuner * CPtBoard::OpenTuner(const Device::ISDB dwTunerType)
{
	CBlockLock AutoLock(&m_TunerLock);
	CPtTuner *pOpenTuner = NULL;

	// �I�[�v������`���[�i����������(�A���e�i�[�qS1/T2�D��)
	DWORD dwTunerID = m_TunerList.size();
	
	while(dwTunerID--){
		if(!m_TunerList[dwTunerID]->m_bIsOpen && (m_TunerList[dwTunerID]->m_dwTunerType == dwTunerType)){
			// ���g�p�`���[�i����ނ���v
			pOpenTuner = m_TunerList[dwTunerID];
			}
		}

	if(!pOpenTuner)return NULL;
	
	if(!GetOpenTunerNum()){
		// �S�`���[�i�����g�p�̏ꍇ�̓{�[�h���I�[�v��
		if(!OpenBoard())return NULL;
		}
		
	// �I�[�v���t���O�Z�b�g
	pOpenTuner->m_bIsOpen = true;

	return pOpenTuner;
}

void CPtBoard::CloseTuner(CPtTuner *pTuner)
{
	CBlockLock AutoLock(&m_TunerLock);

	if(!IsValidTuner(pTuner))return;
	if(!pTuner->m_bIsOpen)return;

	// �X�g���[����~�A�I�[�v���t���O�N���A
	pTuner->StopStream();
	pTuner->m_bIsOpen = false;

	if(!GetOpenTunerNum()){
		// �S�`���[�i�����g�p�̏ꍇ�̓{�[�h���N���[�Y
		CloseBoard();
		}
}

const DWORD CPtBoard::GetActiveTunerNum(const Device::ISDB dwTunerType)
{
	// �{�[�h�O���[�o�����b�NMutex���݊m�F	���u/�o�X�ԍ�/�X���b�g�ԍ�/�t�@���N�V����/PT1/�v���Z�XID/�v
	TCHAR szMutexName[1024];
	::_stprintf(szMutexName, TEXT("/%lu/%lu/%lu/PT1/"), m_DeviceInfo.Bus, m_DeviceInfo.Slot, m_DeviceInfo.Function);

	if(!CSmartMutex::IsExist(szMutexName)){
		// �{�[�h�����g�p�̏ꍇ
		return 0UL;
		}
	else if(!GetOpenTunerNum()){
		// ���v���Z�X���{�[�h���g�p���Ă���ꍇ
		return TUNER_NUM;
		}
	else{
		// ���v���Z�X���{�[�h���g�p���Ă���ꍇ
		DWORD dwActiveNum = 0UL;

		for(DWORD dwIndex = 0UL ; dwIndex < m_TunerList.size() ; dwIndex++){
			if(m_TunerList[dwIndex]->m_dwTunerType == dwTunerType){
				if(m_TunerList[dwIndex]->m_bIsOpen)dwActiveNum++;
				}
			}
		
		// �g�p���̃`���[�i����Ԃ�
		return dwActiveNum;
		}
}

const bool CPtBoard::StartStream(CPtTuner *pTuner)
{
	if(!IsValidTuner(pTuner))return false;
	if(pTuner->m_bIsStream)return true;

	try{
		// �X�g���[���]���J�n
		if(m_pDevice->SetStreamEnable(pTuner->m_dwTunerID, pTuner->m_dwTunerType, true) != STATUS_OK)::BON_EXPECTION(TEXT("PT�`���[�i�X�g���[���]���J�n���s"));

		if(!GetStreamTunerNum()){
			// �X�g���[�~���O�X���b�h�J�n
			if(!m_StreamingThread.StartThread(this, &CPtBoard::StreamingThread, NULL, true))throw ::BON_EXPECTION(TEXT("�X�g���[�~���O�X���b�h�N�����s"));
			}
		}
	catch(CBonException &Exception){
		// �G���[����
		Exception.Notify();
		return false;
		}

	// �X�g���[����M���t���O�Z�b�g
	pTuner->m_bIsStream = true;

	return true;
}

void CPtBoard::StopStream(CPtTuner *pTuner)
{
	if(!IsValidTuner(pTuner))return;
	if(!pTuner->m_bIsStream)return;

	// �X�g���[����M���t���O�N���A
	pTuner->m_bIsStream = false;

	if(!GetStreamTunerNum()){
		// �X�g���[�~���O�X���b�h��~
		m_StreamingThread.EndThread();
		}

	// �X�g���[���]���I��
	::BON_ASSERT(m_pDevice->SetStreamEnable(pTuner->m_dwTunerID, pTuner->m_dwTunerType, false) == STATUS_OK, TEXT("PT�`���[�i�X�g���[���]����~���s"));
}

const bool CPtBoard::SetLnbPower(CPtTuner *pTuner, const bool bEnable)
{
	if(!IsValidTuner(pTuner))return false;
	if(pTuner->m_dwTunerType != Device::ISDB_S)return false;

	// �X�V�OLNB�d����Ԏ擾
	const bool bLastState = (GetLnbOnTunerNum())? true : false;
	
	// LNB�d����ԍX�V
	pTuner->m_bIsLnbOn = bEnable;

	// �X�V��LNB�d����Ԏ擾
	const bool bNewState = (GetLnbOnTunerNum())? true : false;
	if(bLastState == bNewState)return true;			

	// LNB�d����Ԑݒ�
	::BON_ASSERT(m_pDevice->SetLnbPower((bNewState)? Device::LNB_POWER_15V : Device::LNB_POWER_OFF) == STATUS_OK, TEXT("PT�`���[�iLNB�d�����䎸�s"));
	
	return true;
}

const bool CPtBoard::IsValidTuner(const CPtTuner *pTuner) const
{
	// �L���ȃ`���[�i�C���X�^���X�̗L����Ԃ�
	for(DWORD dwIndex = 0UL ; dwIndex < m_TunerList.size() ; dwIndex++){
		if(m_TunerList[dwIndex] == pTuner)return true;
		}

	return false;
}

const DWORD CPtBoard::GetOpenTunerNum(void) const
{
	// �I�[�v���ς݃`���[�i����Ԃ�
	DWORD dwOpenNum = 0UL;

	for(DWORD dwIndex = 0UL ; dwIndex < m_TunerList.size() ; dwIndex++){
		if(m_TunerList[dwIndex]->m_bIsOpen)dwOpenNum++;
		}

	return dwOpenNum;
}

const DWORD CPtBoard::GetLnbOnTunerNum(void) const
{
	// LNB�d��ON�̃`���[�i����Ԃ�
	DWORD dwLnbOnNum = 0UL;

	for(DWORD dwIndex = 0UL ; dwIndex < m_TunerList.size() ; dwIndex++){
		if((m_TunerList[dwIndex]->m_dwTunerType == Device::ISDB_S) && m_TunerList[dwIndex]->m_bIsLnbOn)dwLnbOnNum++;
		}

	return dwLnbOnNum;
}

const DWORD CPtBoard::GetStreamTunerNum(void) const
{
	// �X�g���[����M���̃`���[�i����Ԃ�
	DWORD dwOpenNum = 0UL;

	for(DWORD dwIndex = 0UL ; dwIndex < m_TunerList.size() ; dwIndex++){
		if(m_TunerList[dwIndex]->m_bIsStream)dwOpenNum++;
		}

	return dwOpenNum;
}

const bool CPtBoard::OpenBoard(void)
{
	// �{�[�h���I�[�v������
	try{
		// �f�o�C�X�I�[�v��
		if(m_pDevice->Open() != STATUS_OK)throw ::BON_EXPECTION(TEXT("PT�f�o�C�X�I�[�v�����s"));
		
		// �`���[�i�[�u�I��/�C�l�[�u���v
		if(m_pDevice->SetTunerPowerReset(Device::TUNER_POWER_ON_RESET_ENABLE) != STATUS_OK)throw ::BON_EXPECTION(TEXT("PT�`���[�i���Z�b�g�ڍs���s"));

		// 20ms�E�F�C�g
		::Sleep(20UL);

		// �`���[�i�[�u�I��/�f�B�Z�[�u���v
		if(m_pDevice->SetTunerPowerReset(Device::TUNER_POWER_ON_RESET_DISABLE) != STATUS_OK)throw ::BON_EXPECTION(TEXT("PT�`���[�i���Z�b�g�������s"));

		// 1ms�E�F�C�g
		::Sleep(1UL);
		
		for(DWORD dwTunerID = 0UL ; dwTunerID < TUNER_NUM ; dwTunerID++){
			// �`���[�i���W���[��������
			if(m_pDevice->InitTuner(dwTunerID) != STATUS_OK)throw ::BON_EXPECTION(TEXT("PT�`���[�i���������s"));

			// �`���[�i�X���[�v����
			for(DWORD dwTunerType = 0UL ; dwTunerType < Device::ISDB_COUNT ; dwTunerType++){
				if(m_pDevice->SetTunerSleep(dwTunerID, (Device::ISDB)dwTunerType, false) != STATUS_OK)throw ::BON_EXPECTION(TEXT("PT�`���[�i�ȓd�͉������s"));
				}
			}
		
		// �{�[�h�O���[�o�����b�NMutex�쐬	���u/�o�X�ԍ�/�X���b�g�ԍ�/�t�@���N�V����/PT1/�v
		TCHAR szMutexName[1024];
		::_stprintf(szMutexName, TEXT("/%lu/%lu/%lu/PT1/"), m_DeviceInfo.Bus, m_DeviceInfo.Slot, m_DeviceInfo.Function);
		if(!m_BoardLock.Create(szMutexName))throw ::BON_EXPECTION(TEXT("PT�{�[�h�O���[�o�����b�NMutex�쐬���s"));
		}
	catch(CBonException &Exception){
		// �G���[����
		Exception.Notify();
		CloseBoard();		
		return false;
		}

	return true;
}

void CPtBoard::CloseBoard(void)
{
	// �{�[�h���N���[�Y����
	try{
		// �`���[�i�X���[�v�ڍs
		for(DWORD dwTunerID = 0UL ; dwTunerID < TUNER_NUM ; dwTunerID++){
			for(DWORD dwTunerType = 0UL ; dwTunerType < Device::ISDB_COUNT ; dwTunerType++){
				if(m_pDevice->SetTunerSleep(dwTunerID, (Device::ISDB)dwTunerType, false) != STATUS_OK)throw ::BON_EXPECTION(TEXT("PT�`���[�i�ȓd�͈ڍs���s"));
				}
			}	

		// �{�[�h���N���[�Y����
		if(m_pDevice->Close() != STATUS_OK)throw ::BON_EXPECTION(TEXT("PT�f�o�C�X�N���[�Y���s"));
		}
	catch(CBonException &Exception){
		// �G���[����
		Exception.Notify();
		}

	// �{�[�h���b�NMutex�J��
	m_BoardLock.Close();
}

void CPtBoard::StreamingThread(CSmartThread<CPtBoard> *pThread, bool &bKillSignal, PVOID pParam)
{
	::BON_TRACE(TEXT("CPtBoard::StreamingThread() Start\n"));

	struct PT_DMABUF_SET
	{
		DWORD *pdwPacket;			// �o�b�t�@�|�C���^
		DWORD *pdwEndMark;			// �]�������}�[�N�̃|�C���^
		PT_DMABUF_SET *pNextSet;	// ���̃o�b�t�@�Z�b�g�ւ̃|�C���^
	};

	// DMA�o�b�t�@�Z�b�g
	PT_DMABUF_SET aDmaBufSet[DMABUF_SIZE * DMABUF_NUM * DMACHK_UNIT];
	PT_DMABUF_SET *pDmaBufSet = aDmaBufSet;

	// �f�B�X�p�b�`�o�b�t�@
	DWORD dwWaitTime = 0UL;
	DWORD dwDispatchTerm = 0UL;
	CMediaData DispatchBuffer(DMABLK_SIZE);

	try{
		// DMA�o�b�t�@�T�C�Y�ݒ�
		const static Device::BufferInfo BufInfo = {DMABUF_SIZE, DMABUF_NUM, DMABUF_SIZE};
		if(m_pDevice->SetBufferInfo(&BufInfo) != STATUS_OK)throw ::BON_EXPECTION(TEXT("PT�f�o�C�XDMA�o�b�t�@�m�ێ��s"));

		// DMA�o�b�t�@�|�C���^�擾(�󃊃X�g���`������)
		for(DWORD dwBuffIndex = 0UL ; dwBuffIndex < DMABUF_NUM ; dwBuffIndex++){
			// �o�b�t�@�|�C���^�擾		
			DWORD *pdwPointer = NULL;
			if(m_pDevice->GetBufferPtr(dwBuffIndex, (PVOID*)&pdwPointer) != STATUS_OK)throw ::BON_EXPECTION(TEXT("PT�f�o�C�XDMA�o�b�t�@�|�C���^�擾���s"));
		
			for(DWORD dwBlockIndex = 0UL ; dwBlockIndex < (DMABUF_SIZE * DMACHK_UNIT) ; dwBlockIndex++){
				// �o�b�t�@�|�C���^�Z�b�g
				pDmaBufSet->pdwPacket = &pdwPointer[0]; 
			
				// �]�������}�[�N�|�C���^�Z�b�g
				pDmaBufSet->pdwEndMark = &pdwPointer[DMABLK_SIZE / DMACHK_UNIT / 4 - 1];
				pdwPointer = &pDmaBufSet->pdwEndMark[1];
		
				// ���̃Z�b�g�ւ̃|�C���^���Z�b�g
				pDmaBufSet->pNextSet = &pDmaBufSet[1];
				pDmaBufSet++;
				}
			}

		// �ŏI�v�f�̎��Z�b�g�|�C���^�␳
		aDmaBufSet[sizeof(aDmaBufSet) / sizeof(*aDmaBufSet) - 1].pNextSet = aDmaBufSet;

		// ������DMA�]������~�����ꍇ�͂��������蒼��(goto�͎g�������Ȃ����Ƃ肠������ނȂ�)
		StartDmaTransfer:

		pDmaBufSet = aDmaBufSet;

		// �]�������}�[�N���Z�b�g
		for(DWORD dwIndex = 0UL ; dwIndex < (sizeof(aDmaBufSet) / sizeof(*aDmaBufSet)) ; dwIndex++){
			*aDmaBufSet[dwIndex].pdwEndMark = 0UL;
			}

		// �f�B�X�p�b�`�o�b�t�@���Z�b�g
		for(DWORD dwIndex = 0UL ; dwIndex < m_TunerList.size() ; dwIndex++){
			m_aDispatchBuff[dwIndex].dwPacketPos = 0UL;
			}
		
		dwDispatchTerm = 0UL;
		DispatchBuffer.ClearSize();

		// DMA�]����~
		if(m_pDevice->SetTransferEnable(false) != STATUS_OK)throw ::BON_EXPECTION(TEXT("PT�f�o�C�XDMA�]����~���s"));
	
		// �]���J�E���^���Z�b�g
		if(m_pDevice->ResetTransferCounter() != STATUS_OK)throw ::BON_EXPECTION(TEXT("PT�f�o�C�XDMA�]���J�E���^���Z�b�g���s"));

		// �]���J�E���^�C���N�������g
		for(DWORD dwIndex = 0UL ; dwIndex < (DMABUF_SIZE * DMABUF_NUM) ; dwIndex++){
			if(m_pDevice->IncrementTransferCounter() != STATUS_OK)throw ::BON_EXPECTION(TEXT("PT�f�o�C�XDMA�]���J�E���^�C���N�������g���s"));
			}

		// DMA�]���J�n
		if(m_pDevice->SetTransferEnable(true) != STATUS_OK)throw ::BON_EXPECTION(TEXT("PT�f�o�C�XDMA�]���J�n���s"));

		// DMA�]���������[�v
		while(!bKillSignal){
		
			for(DWORD dwUnit = 0UL ; !bKillSignal && (dwUnit < DMACHK_UNIT) ; dwUnit++){

				// �]�������E�F�C�g
				while(!bKillSignal){

					//	�^�C���A�E�g�J�E���^���Z�b�g
					dwWaitTime = 0UL;

					// �u���b�N�]�������`�F�b�N
					if(*pDmaBufSet->pdwEndMark){

						// �f�B�X�p�b�`�f�[�^�R�s�[(DMA�o�b�t�@��CPU�L���b�V������Ȃ�����)
						DispatchBuffer.AddData((const BYTE *)pDmaBufSet->pdwPacket, DMABLK_SIZE / DMACHK_UNIT);

						// �}�C�N���p�P�b�g�f�B�X�p�b�`
						if(++dwDispatchTerm >= GetStreamTunerNum()){
							DispatchStream((const DWORD *)DispatchBuffer.GetData(), DispatchBuffer.GetSize() / 4UL);
							DispatchBuffer.ClearSize();
							dwDispatchTerm = 0UL;
							}
					
						// �]�������}�[�J�[���Z�b�g
						*pDmaBufSet->pdwEndMark = 0UL;
	
						// DMA�o�b�t�@�Z�b�g�X�V
						pDmaBufSet = pDmaBufSet->pNextSet;
					
						break;
						}
					else{
						if(++dwWaitTime >= (DMACHK_TIMEOUT / DMACHK_WAIT)){
							// DMA�]���^�C���A�E�g���̕��A����
							::BON_TRACE(TEXT("DMA�]���^�C���A�E�g\n"));
							goto StartDmaTransfer;
							}
						else{						
							// �E�F�C�g
							::Sleep(DMACHK_WAIT);
							}				
						}
					}
				}

			// �]���J�E���^�C���N�������g
			if(m_pDevice->IncrementTransferCounter() != STATUS_OK)throw ::BON_EXPECTION(TEXT("PT�f�o�C�XDMA�]���J�E���^�C���N�������g���s"));
			}

		// DMA�]����~
		if(m_pDevice->SetTransferEnable(false) != STATUS_OK)throw ::BON_EXPECTION(TEXT("PT�f�o�C�XDMA�]����~���s"));

		// DMA�o�b�t�@�J��
		if(m_pDevice->SetBufferInfo(NULL) != STATUS_OK)throw ::BON_EXPECTION(TEXT("PT�f�o�C�XDMA�o�b�t�@�J�����s"));
		}
	catch(CBonException &Exception){
		// �G���[����
		Exception.Notify();
		return;
		}

	::BON_TRACE(TEXT("CPtBoard::StreamingThread() End\n"));
}

void CPtBoard::DispatchStream(const DWORD *pdwData, const DWORD dwNum)
{
	#pragma pack(1)
	struct MICRO_PACKET
	{
		BYTE abyData[3];	// �f�[�^	�������� 2 �� 1 �� 0 �ƂȂ�
		BYTE Error : 1;		// �G���[�t���O
		BYTE Store : 1;		// TS�p�P�b�g�J�n�t���O
		BYTE Count : 3;		// �X�g���[���J�E���^
		BYTE Tuner : 3;		// �X�g���[���C���f�b�N�X
	};
	#pragma pack()

	const MICRO_PACKET *pPacket = (const MICRO_PACKET *)pdwData;

	// �f�B�X�p�b�`�o�b�t�@���Z�b�g
	for(DWORD dwIndex = 0UL ; dwIndex < m_TunerList.size() ; dwIndex++){
		m_aDispatchBuff[dwIndex].pDataPtr = m_aDispatchBuff[dwIndex].abyDataBuff;
		}

	// �}�C�N���p�P�b�g�W�J�A�f�B�X�p�b�`
	for(DWORD dwPos = 0UL ; dwPos < dwNum ; dwPos++, pPacket++){
		if((pPacket->Tuner >= 1) && (pPacket->Tuner <= 4)){
			DISPATCH_BUFF &DispatchBuff = m_aDispatchBuff[pPacket->Tuner - 1];

			for(DWORD dwData = 0UL ; dwData < 3UL ; dwData++){
				if(!DispatchBuff.dwPacketPos){
					// TS�p�P�b�g�҂���
					if(pPacket->Store && (pPacket->abyData[2 - dwData] == 0x47)){
						// TS�p�P�b�g�擪
						*(DispatchBuff.pDataPtr++) = pPacket->abyData[2 - dwData];
						DispatchBuff.dwPacketPos = 1UL;
						}
					}
				else if(DispatchBuff.dwPacketPos++ < 188UL){
					// TS�p�P�b�g�X�g�A��
					*(DispatchBuff.pDataPtr++) = pPacket->abyData[2 - dwData];
					
					if(DispatchBuff.dwPacketPos == 188UL){
						// TS�p�P�b�g�X�g�A����
						DispatchBuff.dwPacketPos = 0UL;
						}
					}
				}
			}		
		}

	// �e�`���[�i�ɏo��
	for(DWORD dwIndex = 0UL ; dwIndex < m_TunerList.size() ; dwIndex++){
		if(m_TunerList[dwIndex]->m_bIsStream){
			m_TunerList[dwIndex]->OnTsStream(m_aDispatchBuff[dwIndex].abyDataBuff, m_aDispatchBuff[dwIndex].pDataPtr - m_aDispatchBuff[dwIndex].abyDataBuff);
			}
		}
}


/////////////////////////////////////////////////////////////////////////////
// PT�`���[�i�N���X
/////////////////////////////////////////////////////////////////////////////

CPtTuner::CPtTuner(CPtBoard &Board, const DWORD dwTunerID, const Device::ISDB dwTunerType)
	: m_Board(Board)
	, m_dwTunerID(dwTunerID)
	, m_dwTunerType(dwTunerType)
	, m_bIsOpen(false)
	, m_bIsStream(false)
	, m_bIsLnbOn(false)
	, m_pTsReceiver(NULL)
{
	// �������Ȃ�
}

CPtTuner::~CPtTuner()
{
	// �`���[�i���N���[�Y
	CloseTuner();
}

void CPtTuner::CloseTuner(void)
{
	// LNB�d��OFF
	SetLnbPower(false);

	// �`���[�i���N���[�Y
	m_Board.CloseTuner(this);
}

const bool CPtTuner::StartStream(ITsReceiver *pTsReceiver)
{
	// TS���V�[�o�C���X�^���X�o�^
	{
		CBlockLock AutoLock(&m_Lock);
		m_pTsReceiver = pTsReceiver;
	}

	// �X�g���[����M�X�^�[�g
	return m_Board.StartStream(this);
}

void CPtTuner::StopStream(void)
{
	// TS���V�[�o�C���X�^���X�o�^����
	{
		CBlockLock AutoLock(&m_Lock);
		m_pTsReceiver = NULL;
	}

	// �X�g���[����M�I��
	m_Board.StopStream(this);
}

const bool CPtTuner::SetChannel(const DWORD dwChannel, const DWORD dwOffset)
{
	try{
		// ��M���g���ݒ�
		if(m_Board.m_pDevice->SetFrequency(m_dwTunerID, m_dwTunerType, dwChannel, dwOffset) != STATUS_OK)throw ::BON_EXPECTION(TEXT("PT��M���g���ݒ莸�s"));
		}
	catch(CBonException &Exception){
		// �G���[����
		Exception.Notify();
		return false;
		}
	
	return true;
}

const bool CPtTuner::SetTsID(const WORD wTsID)
{
	DWORD dwCurTsID;

	try{
		// �`���[�i�̎�ނ��`�F�b�N
		if(m_dwTunerType != Device::ISDB_S)throw ::BON_EXPECTION(TEXT("PT��MTS ID�ݒ��Ή�"));
	
		// ��MTS ID�ݒ�
		if(m_Board.m_pDevice->SetIdS(m_dwTunerID, wTsID) != STATUS_OK)throw ::BON_EXPECTION(TEXT("PT��MTS ID�ݒ莸�s"));
		
		// ���f�E�F�C�g
		const DWORD dwStartTime = ::GetTickCount();
		
		do{
			::Sleep(1UL);

			// TS ID�擾
			if(m_Board.m_pDevice->GetIdS(m_dwTunerID, &dwCurTsID) != STATUS_OK)throw ::BON_EXPECTION(TEXT("PT��MTS ID�擾���s"));
			
			// TS ID�ݒ�^�C���A�E�g����
			if((::GetTickCount() - dwStartTime) >= 500UL){
				// TS ID�^�C���A�E�g
				::BON_EXPECTION(TEXT("PT��MTS ID�ݒ�^�C���A�E�g")).Notify(false);
				return false;
				}
			}
		while((WORD)dwCurTsID != wTsID);
		}
	catch(CBonException &Exception){
		// �G���[����
		Exception.Notify();
		return false;
		}
	
	return true;
}

const bool CPtTuner::SetLnbPower(const bool bEnable)
{
	// LNB�d������
	return m_Board.SetLnbPower(this, bEnable);
}

const float CPtTuner::GetSignalLevel(void)
{
	DWORD dwCnr100 = 0UL, dwCurAgc = 0UL, dwMaxAgc = 0UL;
	
	try{
		// CNR���擾
		if(m_Board.m_pDevice->GetCnAgc(m_dwTunerID, m_dwTunerType, &dwCnr100, &dwCurAgc, &dwMaxAgc) != STATUS_OK)throw ::BON_EXPECTION(TEXT("PT�`���[�iCNR�擾���s"));
		}
	catch(CBonException &Exception){
		// �G���[����
		Exception.Notify();
		return 0.0f;
		}
		
	// dB�ɕϊ����ĕԂ�
	return (float)dwCnr100 / 100.0f;
}

void CPtTuner::OnTsStream(const BYTE *pData, const DWORD dwSize)
{
	CBlockLock AutoLock(&m_Lock);

	// TS���V�[�o�ɓ]��
	if(m_pTsReceiver){
		m_pTsReceiver->OnTsStream(this, pData, dwSize);
		}
}
