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
 * @brief SIP �޽����� ���Ե� ���� ������ ��ȿ���� �˻��Ѵ�.
 * @param pclsMessage SIP �޽���
 * @returns SIP �޽����� ���Ե� ���� ������ ��ȿ�ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CSipServer::CheckAuthrization( CSipMessage * pclsMessage )
{
	SIP_CREDENTIAL_LIST::iterator	itCL = pclsMessage->m_clsAuthorizationList.begin();

	if( itCL == pclsMessage->m_clsAuthorizationList.end() )
	{
		SendUnAuthorizedResponse( pclsMessage );
		return false;
	}

	CXmlUser clsXmlUser;

	ECheckAuthResult eRes = CheckAuthorization( &(*itCL), pclsMessage->m_strSipMethod.c_str(), clsXmlUser );
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

	gclsUserMap.Insert( pclsMessage, NULL, &clsXmlUser );

	return true;
}

/**
 * @ingroup KSipServer
 * @brief SIP REGISTER ���� �޽��� ���� �̺�Ʈ �ڵ鷯
 * @param pclsInfo	SIP REGISTER ���� �޽����� ������ IP-PBX ���� ���� ��ü
 * @param iStatus		SIP REGISTER ���� �ڵ�
 */
void CSipServer::EventRegister( CSipServerInfo * pclsInfo, int iStatus )
{
	gclsSipServerMap.Set( pclsInfo, iStatus );
}

/**
 * @ingroup KSipServer
 * @brief SIP ��ȭ ��û ���ſ� ���� ���� Ȯ�� �̺�Ʈ �ڵ鷯
 * @param pclsMessage	SIP INVITE ��û �޽���
 * @return ������ �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CSipServer::EventIncomingRequestAuth( CSipMessage * pclsMessage )
{
	std::string	strIp;
	int		iPort;
	CUserInfo clsUserInfo;

	if( pclsMessage->GetTopViaIpPort( strIp, iPort ) == false )
	{
		CLog::Print( LOG_ERROR, "EventIncomingRequestAuth - GetTopViaIpPort error" );
		SendResponse( pclsMessage, SIP_BAD_REQUEST );
		return false;
	}

	// IP-PBX ���� ������ SIP ��û �޽����� ���� ������� ó���Ѵ�.
	if( gclsSipServerMap.Select( strIp.c_str(), pclsMessage->m_clsTo.m_clsUri.m_strUser.c_str() ) )
	{
		CLog::Print( LOG_DEBUG, "EventIncomingRequestAuth ip(%s) user(%s) IP-PBX => allowed", strIp.c_str(), pclsMessage->m_clsTo.m_clsUri.m_strUser.c_str() );
		return true;
	}

	if( gclsUserMap.Select( pclsMessage->m_clsFrom.m_clsUri.m_strUser.c_str(), clsUserInfo ) == false )
	{
		std::string strDestToId;

		// IP-PBX ���� ������ BYE �޽����� ���������� ó���ϱ� ���� ���
		if( pclsMessage->IsMethod( SIP_METHOD_BYE ) )
		{
			std::string	strCallId;

			pclsMessage->GetCallId( strCallId );

			if( gclsCallMap.Select( strCallId.c_str() ) )
			{
				CLog::Print( LOG_DEBUG, "EventIncomingRequestAuth BYE CallId(%s) is found", strCallId.c_str() );
				return true;
			}
		}
		else if( gclsSipServerMap.SelectIncomingRoute( strIp.c_str(), pclsMessage->m_clsTo.m_clsUri.m_strUser.c_str(), strDestToId ) )
		{
			CLog::Print( LOG_DEBUG, "EventIncomingRequestAuth ip(%s) user(%s) IP-PBX dest_user(%s)"
				, strIp.c_str(), pclsMessage->m_clsTo.m_clsUri.m_strUser.c_str(), strDestToId.c_str() );
			return true;
		}

		if( CheckAuthrization( pclsMessage ) == false )
		{
			CLog::Print( LOG_DEBUG, "EventIncomingRequestAuth no UserInfo and CheckAuthrization() return false" );
			return false;
		}

		if( gclsUserMap.Select( pclsMessage->m_clsFrom.m_clsUri.m_strUser.c_str(), clsUserInfo ) == false )
		{
			CLog::Print( LOG_DEBUG, "EventIncomingRequestAuth no UserInfo and CheckAuthrization() return true and no UserInfo" );
			return false;
		}
	}

	if( strcmp( clsUserInfo.m_strIp.c_str(), strIp.c_str() ) || clsUserInfo.m_iPort != iPort )
	{
		CLog::Print( LOG_DEBUG, "EventIncomingRequestAuth current(%s:%d) != recv(%s:%d)", clsUserInfo.m_strIp.c_str(), clsUserInfo.m_iPort, strIp.c_str(), iPort );

		if( CheckAuthrization( pclsMessage ) == false )
		{
			CLog::Print( LOG_DEBUG, "EventIncomingRequestAuth CheckAuthrization() return false" );
			return false;
		}

		gclsUserMap.SetIpPort( pclsMessage->m_clsFrom.m_clsUri.m_strUser.c_str(), strIp.c_str(), iPort );
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
	CUserInfo	clsUserInfo;
	bool			bRoutePrefix = false;
	std::string	strTo;

	CLog::Print( LOG_DEBUG, "EventIncomingCall(%s,%s,%s)", pszCallId, pszFrom, pszTo );

	if( strlen( pszTo ) == 0 )
	{
		CLog::Print( LOG_DEBUG, "EventIncomingCall to(%s) is not defined", pszTo );
		return StopCall( pszCallId, SIP_DECLINE );
	}

	if( SelectUser( pszTo, clsXmlUser ) == false )
	{
		CXmlSipServer clsXmlSipServer;

		// �α��� ��� ����ڰ� �ƴϸ� ������ IP-PBX �� �����ϴ��� �˻��Ѵ�.
		if( gclsSipServerMap.SelectRoutePrefix( pszTo, clsXmlSipServer, strTo ) )
		{
			clsXmlUser.m_strId = clsXmlSipServer.m_strUserId;
			clsXmlUser.m_strPassWord = clsXmlSipServer.m_strPassWord;

			clsUserInfo.m_strIp = clsXmlSipServer.m_strIp;
			clsUserInfo.m_iPort = clsXmlSipServer.m_iPort;
			clsUserInfo.m_eTransport = E_SIP_UDP;

			pszFrom = clsXmlUser.m_strId.c_str();
			pszTo = strTo.c_str();

			bRoutePrefix = true;
			CLog::Print( LOG_DEBUG, "EventIncomingCall routePrefix IP-PBX(%s:%d)", clsUserInfo.m_strIp.c_str(), clsUserInfo.m_iPort );
		}
		else if( gclsSipServerMap.SelectIncomingRoute( NULL, pszTo, strTo ) )
		{
			if( SelectUser( strTo.c_str(), clsXmlUser ) == false )
			{
				CLog::Print( LOG_DEBUG, "EventIncomingCall to(%s) is not found - dest to(%s)", pszTo, strTo.c_str() );
				return StopCall( pszCallId, SIP_NOT_FOUND );
			}
		}
		else if( gclsSetup.IsCallPickupId( pszTo ) )
		{
			return PickUp( pszCallId, pszFrom, pszTo, pclsRtp );
		}
		else
		{
			CLog::Print( LOG_DEBUG, "EventIncomingCall to(%s) is not found in XML or DB", pszTo );
			return StopCall( pszCallId, SIP_NOT_FOUND );
		}
	}

	if( clsXmlUser.IsDnd() )
	{
		// ����ڰ� DND �� �����Ǿ� ������ ��ȭ ��û�� �����Ѵ�.
		CLog::Print( LOG_DEBUG, "EventIncomingCall to(%s) is DND", pszTo );
		return StopCall( pszCallId, SIP_DECLINE );
	}

	if( clsXmlUser.IsCallForward() )
	{
		CLog::Print( LOG_DEBUG, "EventIncomingCall to(%s) is CallForward(%s)", pszTo, clsXmlUser.m_strCallForward.c_str() );

		// ����ڰ� ������ȯ �����Ǿ� ������ ������ȯ ó���Ѵ�.
		CSipMessage * pclsInvite = gclsUserAgent.DeleteIncomingCall( pszCallId );
		if( pclsInvite )
		{
			CSipMessage * pclsResponse = pclsInvite->CreateResponseWithToTag( SIP_MOVED_TEMPORARILY );
			if( pclsResponse )
			{
				CSipFrom clsContact;

				clsContact.m_clsUri.m_strProtocol = SIP_PROTOCOL;
				clsContact.m_clsUri.m_strUser = clsXmlUser.m_strCallForward;
				clsContact.m_clsUri.m_strHost = gclsSetup.m_strLocalIp;
				clsContact.m_clsUri.m_iPort = gclsSetup.m_iUdpPort;

				pclsResponse->m_clsContactList.push_back( clsContact );

				gclsUserAgent.m_clsSipStack.SendSipMessage( pclsResponse );
				return;
			}
		}
		else
		{
			CLog::Print( LOG_ERROR, "EventIncomingCall(%s) INVITE it not found", pszCallId );
		}

		return StopCall( pszCallId, SIP_MOVED_TEMPORARILY );
	}

	if( bRoutePrefix == false )
	{
		if( gclsUserMap.Select( pszTo, clsUserInfo ) == false )
		{
			CLog::Print( LOG_DEBUG, "EventIncomingCall(%s) to(%s) is not found", pszCallId, pszTo );
			return StopCall( pszCallId, SIP_NOT_FOUND );
		}
	}

	int iStartPort = -1;
	std::string	strCallId;
	CSipCallRoute	clsRoute;

	if( gclsSetup.m_bUseRtpRelay )
	{
		iStartPort = gclsRtpMap.CreatePort( SOCKET_COUNT_PER_MEDIA * pclsRtp->GetMediaCount() );
		if( iStartPort == -1 )
		{
			return StopCall( pszCallId, SIP_INTERNAL_SERVER_ERROR );
		}

		pclsRtp->SetIpPort( gclsSetup.m_strLocalIp.c_str(), iStartPort, SOCKET_COUNT_PER_MEDIA );
	}

	clsUserInfo.GetCallRoute( clsRoute );
	clsRoute.m_b100rel = gclsUserAgent.Is100rel( pszCallId );

	CSipMessage * pclsInvite;

	if( gclsUserAgent.CreateCall( pszFrom, pszTo, pclsRtp, &clsRoute, strCallId, &pclsInvite ) == false )
	{
		CLog::Print( LOG_ERROR, "EventIncomingCall(%s) CreateCall errr", pszCallId );
		return StopCall( pszCallId, SIP_INTERNAL_SERVER_ERROR );
	}

	gclsCallMap.Insert( pszCallId, strCallId.c_str(), iStartPort );

	if( gclsUserAgent.StartCall( strCallId.c_str(), pclsInvite ) == false )
	{
		gclsCallMap.Delete( pszCallId );

		CLog::Print( LOG_ERROR, "EventIncomingCall(%s) StartCall error", pszCallId );
		return StopCall( pszCallId, SIP_INTERNAL_SERVER_ERROR );
	}
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
	CCallInfo clsCallInfo;

	CLog::Print( LOG_DEBUG, "EventCallRing(%s,%d)", pszCallId, iSipStatus );

	if( gclsCallMap.Select( pszCallId, clsCallInfo ) )
	{
		if( pclsRtp && clsCallInfo.m_iPeerRtpPort > 0 )
		{
			pclsRtp->m_iPort = clsCallInfo.m_iPeerRtpPort;
			pclsRtp->m_strIp = gclsSetup.m_strLocalIp;
		}

		int iRSeq = gclsUserAgent.GetRSeq( pszCallId );
		if( iRSeq != -1 )
		{
			gclsUserAgent.SetRSeq( clsCallInfo.m_strPeerCallId.c_str(), iRSeq );
		}

		gclsUserAgent.RingCall( clsCallInfo.m_strPeerCallId.c_str(), iSipStatus, pclsRtp );
	}
	else if( gclsTransCallMap.Select( pszCallId, clsCallInfo ) )
	{
		gclsUserAgent.SendNotify( clsCallInfo.m_strPeerCallId.c_str(), iSipStatus );
	}
}

/**
 * @ingroup KSipServer
 * @brief SIP ��ȭ ���� �̺�Ʈ �ڵ鷯
 * @param	pszCallId	SIP Call-ID
 * @param pclsRtp		RTP ���� ���� ��ü
 */
void CSipServer::EventCallStart( const char * pszCallId, CSipCallRtp * pclsRtp )
{
	CCallInfo clsCallInfo;

	CLog::Print( LOG_DEBUG, "EventCallStart(%s)", pszCallId );

	if( gclsCallMap.Select( pszCallId, clsCallInfo ) )
	{
		if( pclsRtp && clsCallInfo.m_iPeerRtpPort > 0 )
		{
			pclsRtp->SetIpPort( gclsSetup.m_strLocalIp.c_str(), clsCallInfo.m_iPeerRtpPort, SOCKET_COUNT_PER_MEDIA );
		}

		if( gclsUserAgent.IsConnected( clsCallInfo.m_strPeerCallId.c_str() ) )
		{
			gclsUserAgent.SendReInvite( clsCallInfo.m_strPeerCallId.c_str(), pclsRtp );
		}
		else
		{
			gclsUserAgent.AcceptCall( clsCallInfo.m_strPeerCallId.c_str(), pclsRtp );
		}
	}
	else if( gclsTransCallMap.Select( pszCallId, clsCallInfo ) )
	{
		gclsUserAgent.SendNotify( clsCallInfo.m_strPeerCallId.c_str(), SIP_OK );

		std::string	strReferToCallId;
		int iStartPort = -1;

		if( gclsCallMap.Select( clsCallInfo.m_strPeerCallId.c_str(), strReferToCallId ) )
		{
			gclsUserAgent.StopCall( clsCallInfo.m_strPeerCallId.c_str() );
			gclsCallMap.Delete( clsCallInfo.m_strPeerCallId.c_str() );
		}

		if( pclsRtp && clsCallInfo.m_iPeerRtpPort > 0 )
		{
			iStartPort = clsCallInfo.m_iPeerRtpPort - 2;

			pclsRtp->SetIpPort( gclsSetup.m_strLocalIp.c_str(), clsCallInfo.m_iPeerRtpPort, SOCKET_COUNT_PER_MEDIA );
		}

		gclsUserAgent.SendReInvite( strReferToCallId.c_str(), pclsRtp );

		gclsCallMap.Insert( strReferToCallId.c_str(), pszCallId, iStartPort );
		gclsTransCallMap.Delete( pszCallId, false );
	}
	else
	{
		gclsUserAgent.StopCall( pszCallId );
	}
}

/**
 * @ingroup KSipServer
 * @brief SIP ��ȭ ���� �̺�Ʈ �ڵ鷯
 * @param	pszCallId		SIP Call-ID
 * @param iSipStatus	SIP ���� �ڵ�. INVITE �� ���� ���� �������� ��ȭ�� ����� ���, INVITE �� ���� �ڵ带 �����Ѵ�.
 */
void CSipServer::EventCallEnd( const char * pszCallId, int iSipStatus )
{
	CCallInfo clsCallInfo;

	CLog::Print( LOG_DEBUG, "EventCallEnd(%s:%d)", pszCallId, iSipStatus );

	if( gclsCallMap.Select( pszCallId, clsCallInfo ) )
	{
		if( clsCallInfo.m_bRecv )
		{
			SaveCdr( pszCallId, iSipStatus );
		}
		else
		{
			SaveCdr( clsCallInfo.m_strPeerCallId.c_str(), iSipStatus );
		}

		gclsUserAgent.StopCall( clsCallInfo.m_strPeerCallId.c_str() );
		gclsCallMap.Delete( pszCallId );
	}
	else
	{
		std::string	strCallId;

		if( gclsTransCallMap.Select( pszCallId, strCallId ) )
		{
			gclsUserAgent.SendNotify( strCallId.c_str(), iSipStatus );
			gclsTransCallMap.Delete( pszCallId );
		}
	}
}

/**
 * @ingroup KSipServer
 * @brief SIP ReINVITE ���� �̺�Ʈ �ڵ鷯
 * @param	pszCallId	SIP Call-ID
 * @param pclsRemoteRtp		���� RTP ���� ���� ��ü
 * @param pclsLocalRtp		�� RTP ���� ���� ��ü
 */
void CSipServer::EventReInvite( const char * pszCallId, CSipCallRtp * pclsRemoteRtp, CSipCallRtp * pclsLocalRtp )
{
	CCallInfo	clsCallInfo;

	CLog::Print( LOG_DEBUG, "EventReInvite(%s)", pszCallId );

	if( gclsCallMap.Select( pszCallId, clsCallInfo ) )
	{
		if( pclsRemoteRtp && clsCallInfo.m_iPeerRtpPort > 0 )
		{
			pclsRemoteRtp->SetIpPort( gclsSetup.m_strLocalIp.c_str(), clsCallInfo.m_iPeerRtpPort, SOCKET_COUNT_PER_MEDIA );
		}

		gclsUserAgent.SendReInvite( clsCallInfo.m_strPeerCallId.c_str(), pclsRemoteRtp );
	}
}

/**
 * @ingroup KSipServer
 * @brief SIP PRACK ���� �̺�Ʈ �ڵ鷯
 * @param	pszCallId	SIP Call-ID
 * @param pclsRtp		RTP ���� ���� ��ü
 */
void CSipServer::EventPrack( const char * pszCallId, CSipCallRtp * pclsRtp )
{
	CCallInfo	clsCallInfo;

	CLog::Print( LOG_DEBUG, "EventPrack(%s)", pszCallId );

	if( gclsCallMap.Select( pszCallId, clsCallInfo ) )
	{
		if( pclsRtp && clsCallInfo.m_iPeerRtpPort > 0 )
		{
			pclsRtp->SetIpPort( gclsSetup.m_strLocalIp.c_str(), clsCallInfo.m_iPeerRtpPort, SOCKET_COUNT_PER_MEDIA );
		}

		gclsUserAgent.SendPrack( clsCallInfo.m_strPeerCallId.c_str(), pclsRtp );
	}
}

/**
 * @ingroup KSipServer
 * @brief Screened / Unscreened Transfer ��û ���� �̺�Ʈ �ڵ鷯
 * @param pszCallId					SIP Call-ID
 * @param pszReferToCallId	��ȭ�� ���޵� SIP Call-ID
 * @param bScreenedTransfer Screened Transfer �̸� true �� �Էµǰ� Unscreened Transfer �̸� false �� �Էµȴ�.
 * @returns ��û�� �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CSipServer::EventTransfer( const char * pszCallId, const char * pszReferToCallId, bool bScreenedTransfer )
{
	CCallInfo		clsCallInfo, clsReferToCallInfo;
	CSipCallRtp clsRtp;

	CLog::Print( LOG_DEBUG, "EventTransfer(%s,%s,%s)", pszCallId, pszReferToCallId, ( bScreenedTransfer ? "screened" : "unscreened" ) );

	if( gclsCallMap.Select( pszCallId, clsCallInfo ) == false ) return false;
	if( gclsCallMap.Select( pszReferToCallId, clsReferToCallInfo ) == false ) return false;

	gclsCallMap.Delete( pszCallId );
	gclsCallMap.Delete( pszReferToCallId, false );
	
	if( gclsUserAgent.GetRemoteCallRtp( clsCallInfo.m_strPeerCallId.c_str(), &clsRtp ) == false ) return false;
	clsRtp.SetDirection( E_RTP_SEND_RECV );

	if( gclsSetup.m_bUseRtpRelay )
	{
		if( bScreenedTransfer )
		{
			clsRtp.SetIpPort( gclsSetup.m_strLocalIp.c_str(), clsReferToCallInfo.m_iPeerRtpPort, SOCKET_COUNT_PER_MEDIA );
		}
		else
		{
			clsRtp.SetIpPort( gclsSetup.m_strLocalIp.c_str(), clsReferToCallInfo.m_iPeerRtpPort + 2, SOCKET_COUNT_PER_MEDIA );
		}
	}

	if( bScreenedTransfer )
	{
		CSipCallRtp clsReferToRtp;

		if( gclsUserAgent.GetRemoteCallRtp( clsReferToCallInfo.m_strPeerCallId.c_str(), &clsReferToRtp ) == false ) return false;
		clsReferToRtp.SetDirection( E_RTP_SEND_RECV );

		if( gclsSetup.m_bUseRtpRelay )
		{
			clsReferToRtp.SetIpPort( gclsSetup.m_strLocalIp.c_str(), clsReferToCallInfo.m_iPeerRtpPort + 2, SOCKET_COUNT_PER_MEDIA );
		}

		gclsCallMap.Insert( clsCallInfo.m_strPeerCallId.c_str(), clsReferToCallInfo.m_strPeerCallId.c_str(), clsReferToCallInfo.m_iPeerRtpPort );
		gclsUserAgent.SendReInvite( clsCallInfo.m_strPeerCallId.c_str(), &clsReferToRtp );
		gclsUserAgent.SendReInvite( clsReferToCallInfo.m_strPeerCallId.c_str(), &clsRtp );
	}

	gclsUserAgent.StopCall( pszCallId );
	gclsUserAgent.StopCall( pszReferToCallId, SIP_REQUEST_TERMINATED );

	if( bScreenedTransfer == false )
	{
		std::string	strNewCallId, strFromId, strToId;
		CUserInfo	clsUserInfo;
		CSipCallRoute	clsRoute;

		gclsUserAgent.GetToId( clsCallInfo.m_strPeerCallId.c_str(), strFromId );
		gclsUserAgent.GetToId( clsReferToCallInfo.m_strPeerCallId.c_str(), strToId );

		if( gclsUserMap.Select( strToId.c_str(), clsUserInfo ) )
		{
			clsUserInfo.GetCallRoute( clsRoute );

			gclsUserAgent.StopCall( clsReferToCallInfo.m_strPeerCallId.c_str() );
			gclsUserAgent.StartCall( strFromId.c_str(), strToId.c_str(), &clsRtp, &clsRoute, strNewCallId );

			gclsCallMap.Insert( strNewCallId.c_str(), clsCallInfo.m_strPeerCallId.c_str(), clsReferToCallInfo.m_iPeerRtpPort );
		}
		else
		{
			gclsUserAgent.StopCall( clsCallInfo.m_strPeerCallId.c_str() );
			gclsUserAgent.StopCall( clsReferToCallInfo.m_strPeerCallId.c_str() );
		}
	}

	if( gclsSetup.m_bUseRtpRelay )
	{
		gclsRtpMap.ReSetIpPort( clsReferToCallInfo.m_iPeerRtpPort );
	}

	return true;
}

/**
 * @ingroup KSipServer
 * @brief Blind Transfer ��û ���� �̺�Ʈ �ڵ鷯
 * @param pszCallId			SIP Call-ID
 * @param pszReferToId	��ȭ�� ���޵� ����� ���̵�
 * @returns ��û�� �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CSipServer::EventBlindTransfer( const char * pszCallId, const char * pszReferToId )
{
	std::string	strCallId, strInviteCallId, strToId;
	CSipCallRtp clsRtp;
	CUserInfo		clsUserInfo;
	CSipCallRoute	clsRoute;
	int iStartPort = -1;

	CLog::Print( LOG_DEBUG, "EventBlindTransfer(%s,%s)", pszCallId, pszReferToId );

	if( gclsCallMap.Select( pszCallId, strCallId ) == false ) return false;
	if( gclsUserAgent.GetToId( strCallId.c_str(), strToId ) == false ) return false;
	if( gclsUserMap.Select( pszReferToId, clsUserInfo ) == false ) return false;

	if( gclsUserAgent.GetRemoteCallRtp( strCallId.c_str(), &clsRtp ) == false ) return false;
	clsRtp.SetDirection( E_RTP_SEND_RECV );

	if( gclsSetup.m_bUseRtpRelay )
	{
		iStartPort = gclsRtpMap.CreatePort( SOCKET_COUNT_PER_MEDIA * clsRtp.GetMediaCount() );
		if( iStartPort == -1 ) return false;

		clsRtp.SetIpPort( gclsSetup.m_strLocalIp.c_str(), iStartPort, SOCKET_COUNT_PER_MEDIA );
	}

	clsUserInfo.GetCallRoute( clsRoute );

	if( gclsUserAgent.StartCall( strToId.c_str(), pszReferToId, &clsRtp, &clsRoute, strInviteCallId ) == false )
	{
		return false;
	}

	gclsTransCallMap.Insert( pszCallId, strInviteCallId.c_str(), iStartPort );

	return true;
}

/**
 * @ingroup KSipServer
 * @brief SIP MESSAGE �޽��� ���� �̺�Ʈ �ڵ鷯
 * @param pszFrom			�߽��� ���̵�
 * @param pszTo				������ ���̵�
 * @param pclsMessage ���ŵ� SIP �޽���
 * @returns 
 */
bool CSipServer::EventMessage( const char * pszFrom, const char * pszTo, CSipMessage * pclsMessage )
{
	CUserInfo		clsUserInfo;
	CSipCallRoute	clsRoute;

	if( gclsUserMap.Select( pszTo, clsUserInfo ) == false ) return false;

	clsUserInfo.GetCallRoute( clsRoute );

	return gclsUserAgent.SendSms( pszFrom, pszTo, pclsMessage->m_strBody.c_str(), &clsRoute );
}