// TsTcpServer.cpp: TCP�� TS�T�[�o�N���X
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "TsTcpServer.h"


/////////////////////////////////////////////////////////////////////////////
// �t�@�C�����[�J���萔�ݒ�
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// TCP�� TS�T�[�o�N���X
/////////////////////////////////////////////////////////////////////////////

const bool CTsTcpServer::OpenServer(const WORD wServerPort)
{
	m_pServerSocket->Listen(wServerPort);

	return true;
}

void CTsTcpServer::CloseServer(void)
{

}

CTsTcpServer::CTsTcpServer(IBonObject *pOwner)
	: CBonObject(pOwner)
	, m_pServerSocket(::BON_SAFE_CREATE<ISmartSocket *>(TEXT("CSmartSocket")))
{
	::BON_ASSERT(m_pServerSocket != NULL);
}

CTsTcpServer::~CTsTcpServer(void)
{
	BON_SAFE_RELEASE(m_pServerSocket);
}

void CTsTcpServer::AcceptThread(CSmartThread<CTsTcpServer> *pThread, bool &bKillSignal, PVOID pParam)
{
	ISmartSocket *pClientSocket;

	while(!bKillSignal){
		pClientSocket = m_pServerSocket->Accept();

		

		}
}
