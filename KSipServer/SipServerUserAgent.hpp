/* 
 * Copyright (C) 2012 Yee Young Han <websearch@naver.com> (http://blog.naver.com/websearch)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

/**
 * @ingroup KSipServer
 * @brief SIP REGISTER ���� �޽��� ���� �̺�Ʈ �ڵ鷯
 * @param pclsInfo	SIP REGISTER ���� �޽����� ������ IP-PBX ���� ���� ��ü
 * @param iStatus		SIP REGISTER ���� �ڵ�
 */
void CSipServer::EventRegister( CSipServerInfo * pclsInfo, int iStatus )
{

}

/**
 * @ingroup KSipServer
 * @brief SIP ��ȭ ��û ���ſ� ���� ���� Ȯ�� �̺�Ʈ �ڵ鷯
 * @param pclsMessage	SIP INVITE ��û �޽���
 * @return ������ �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CSipServer::EventIncomingCallAuth( CSipMessage * pclsMessage )
{
	if( gclsUserMap.Select( pclsMessage->m_clsFrom.m_clsUri.m_strUser.c_str() ) == false )
	{
		SIP_CREDENTIAL_LIST::iterator	itCL = pclsMessage->m_clsAuthorizationList.begin();

		if( itCL == pclsMessage->m_clsAuthorizationList.end() )
		{
			return SendUnAuthorizedResponse( pclsMessage );
		}

		ECheckAuthResult eRes = CheckAuthorization( &(*itCL), pclsMessage->m_strSipMethod.c_str() );
		switch( eRes )
		{
		case E_AUTH_NONCE_NOT_FOUND:
			SendUnAuthorizedResponse( pclsMessage );
			return false;
		case E_AUTH_ERROR:
			SendResponse( pclsMessage, SIP_FORBIDDEN );
			return false;
		default:
			break;
		}
	}

	return true;
}

/**
 * @ingroup KSipServer
 * @brief SIP ��ȭ ��û ���� �̺�Ʈ �ڵ鷯
 * @param	pszCallId	SIP Call-ID
 * @param pszFrom		SIP From ����� ���̵�
 * @param pszTo			SIP To ����� ���̵�
 * @param pclsRtp		RTP ���� ���� ��ü
 */
void CSipServer::EventIncomingCall( const char * pszCallId, const char * pszFrom, const char * pszTo, CSipCallRtp * pclsRtp )
{
	CXmlUser	clsXmlUser;

	if( SelectUser( pszTo, clsXmlUser ) == false )
	{
		gclsUserAgent.StopCall( pszCallId );
		return;
	}

	if( clsXmlUser.m_bDnd )
	{
		gclsUserAgent.StopCall( pszCallId );
		return;
	}

	if( clsXmlUser.m_strCallForward.empty() == false )
	{
		CSipMessage * pclsInvite = gclsUserAgent.DeleteIncomingCall( pszCallId );
		if( pclsInvite )
		{
			CSipMessage * pclsResponse = pclsInvite->CreateResponseWithToTag( SIP_MOVED_TEMPORARILY );
			if( pclsResponse )
			{
				CSipFrom clsContact;

				clsContact.m_clsUri.m_strProtocol = "sip";
				clsContact.m_clsUri.m_strUser = clsXmlUser.m_strCallForward;
				clsContact.m_clsUri.m_strHost = gclsSetup.m_strLocalIp;
				clsContact.m_clsUri.m_iPort = gclsSetup.m_iUdpPort;

				pclsResponse->m_clsContactList.push_back( clsContact );

				gclsSipStack.SendSipMessage( pclsResponse );
				return;
			}
		}

		gclsUserAgent.StopCall( pszCallId );
		return;
	}

	CUserInfo	clsUserInfo;

	if( gclsUserMap.Select( pszTo, clsUserInfo ) == false )
	{
		gclsUserAgent.StopCall( pszCallId );
		return;
	}

	std::string	strCallId;
	CSipCallRoute	clsRoute;

	clsRoute.m_strDestIp = clsUserInfo.m_strIp;
	clsRoute.m_iDestPort = clsUserInfo.m_iPort;

	if( gclsUserAgent.StartCall( pszFrom, pszTo, pclsRtp, &clsRoute, strCallId ) == false )
	{
		gclsUserAgent.StopCall( pszCallId );
		return;
	}

	gclsCallMap.Insert( pszCallId, strCallId.c_str() );
}

/**
 * @ingroup KSipServer
 * @brief SIP Ring / Session Progress ���� �̺�Ʈ �ڵ鷯
 * @param	pszCallId		SIP Call-ID
 * @param iSipStatus	SIP ���� �ڵ�
 * @param pclsRtp			RTP ���� ���� ��ü
 */
void CSipServer::EventCallRing( const char * pszCallId, int iSipStatus, CSipCallRtp * pclsRtp )
{
	std::string	strCallId;

	if( gclsCallMap.Select( pszCallId, strCallId ) == false ) return;

	gclsUserAgent.RingCall( strCallId.c_str(), iSipStatus, pclsRtp );
}

/**
 * @ingroup KSipServer
 * @brief SIP ��ȭ ���� �̺�Ʈ �ڵ鷯
 * @param	pszCallId	SIP Call-ID
 * @param pclsRtp		RTP ���� ���� ��ü
 */
void CSipServer::EventCallStart( const char * pszCallId, CSipCallRtp * pclsRtp )
{
	std::string	strCallId;

	if( gclsCallMap.Select( pszCallId, strCallId ) == false ) return;

	gclsUserAgent.AcceptCall( strCallId.c_str(), pclsRtp );
}

/**
 * @ingroup KSipServer
 * @brief SIP ��ȭ ���� �̺�Ʈ �ڵ鷯
 * @param	pszCallId		SIP Call-ID
 * @param iSipStatus	SIP ���� �ڵ�. INVITE �� ���� ���� �������� ��ȭ�� ����� ���, INVITE �� ���� �ڵ带 �����Ѵ�.
 */
void CSipServer::EventCallEnd( const char * pszCallId, int iSipStatus )
{
	std::string	strCallId;

	if( gclsCallMap.Select( pszCallId, strCallId ) == false ) return;

	gclsUserAgent.StopCall( strCallId.c_str() );
	gclsCallMap.Delete( pszCallId );
}

/**
 * @ingroup KSipServer
 * @brief SIP ReINVITE ���� �̺�Ʈ �ڵ鷯
 * @param	pszCallId	SIP Call-ID
 * @param pclsRtp		RTP ���� ���� ��ü
 */
void CSipServer::EventReInvite( const char * pszCallId, CSipCallRtp * pclsRtp )
{

}

/**
 * @ingroup KSipServer
 * @brief 1�� ������ ����Ǵ� Ÿ�̸� �̺�Ʈ �ڵ鷯
 */
void CSipServer::EventTimer( )
{
	
}