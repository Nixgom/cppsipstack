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

#include "SipUserAgent.h"
#include "SipServerInfo.h"
#include "SipUtility.h"
#include "SipMd5.h"
#include "StringUtility.h"
#include "MemoryDebug.h"

/**
 * @ingroup SipUserAgent
 * @brief ������
 */
CSipServerInfo::CSipServerInfo() : m_iPort(5060), m_iLoginTimeout(3600)
	, m_eTransport(E_SIP_UDP), m_iNatTimeout(0)
	, m_iNextSendTime(0), m_iSeqNo(0), m_bAuth(false), m_bDelete(false)
{
	ClearLogin();
}

/**
 * @ingroup SipUserAgent
 * @brief �Ҹ���
 */
CSipServerInfo::~CSipServerInfo()
{
}

/**
 * @ingroup SipUserAgent
 * @brief ������ IP-PBX �������� �˻��Ѵ�.
 * @param clsInfo IP-PBX ����
 * @returns ������ IP-PBX �����̸� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CSipServerInfo::Equal( CSipServerInfo & clsInfo )
{
	if( !strcmp( clsInfo.m_strIp.c_str(), m_strIp.c_str() ) &&
			!strcmp( clsInfo.m_strUserId.c_str(), m_strUserId.c_str() ) &&
			clsInfo.m_iPort == m_iPort &&
			clsInfo.m_eTransport == m_eTransport )
	{
		return true;
	}

	return false;
}

bool CSipServerInfo::Equal( const char * pszIp, int iPort, ESipTransport eTransport )
{
	if( !strcmp( pszIp, m_strIp.c_str() ) &&
			iPort == m_iPort &&
			eTransport == m_eTransport )
	{
		return true;
	}

	return false;
}

/**
 * @ingroup SipUserAgent
 * @brief IP-PBX ������ �����Ѵ�.
 * @param clsInfo IP-PBX ����
 */
void CSipServerInfo::Update( CSipServerInfo & clsInfo )
{
	m_strDomain = clsInfo.m_strDomain;
	m_strPassWord = clsInfo.m_strPassWord;
	m_iLoginTimeout = clsInfo.m_iLoginTimeout;
}

/**
 * @ingroup SipUserAgent
 * @brief �α��ε� ������ �ʱ�ȭ��Ų��.
 */
void CSipServerInfo::ClearLogin()
{
	m_bLogin = false;
	m_iLoginTime = 0;
	m_iSendTime = 0;
	m_iResponseTime = 0;
	m_clsCallId.Clear();
	m_clsChallenge.Clear();
	m_iChallengeStatusCode = 0;
	m_iNonceCount = 1;
}

/**
 * @ingroup SipUserAgent
 * @brief SIP REGISTER �޽����� �����Ѵ�.
 * @param pclsSipStack SIP stack
 * @param pclsResponse SIP REGISTER ��û�� ���� ���� �޽���
 * @returns �����ϸ� REGISTER �޽����� �����ϰ� �����ϸ� NULL �� �����Ѵ�.
 */
CSipMessage * CSipServerInfo::CreateRegister( CSipStack * pclsSipStack, CSipMessage * pclsResponse )
{
	CSipMessage * pclsRequest = new CSipMessage();
	if( pclsRequest == NULL ) return NULL;

	// REGISTER sip:127.0.0.1 SIP/2.0
	pclsRequest->m_strSipMethod = SIP_METHOD_REGISTER;
	pclsRequest->m_clsReqUri.Set( SIP_PROTOCOL, NULL, m_strDomain.c_str(), m_iPort );

	// To
	pclsRequest->m_clsTo.m_clsUri.Set( SIP_PROTOCOL, m_strUserId.c_str(), m_strDomain.c_str(), m_iPort );

	// From
	pclsRequest->m_clsFrom = pclsRequest->m_clsTo;
	pclsRequest->m_clsFrom.InsertTag();

	// Expires: 300
	pclsRequest->m_iExpires = m_iLoginTimeout;
	
	// CSeq: 1 REGISTER
	++m_iSeqNo;
	if( m_iSeqNo >= 2000000000 ) m_iSeqNo = 1;
	pclsRequest->m_clsCSeq.m_iDigit = m_iSeqNo;
	pclsRequest->m_clsCSeq.m_strMethod = SIP_METHOD_REGISTER;

	// Route
	pclsRequest->AddRoute( m_strIp.c_str(), m_iPort, m_eTransport );

	// Call-Id
	if( m_clsCallId.Empty() )
	{
		pclsRequest->m_clsCallId.Make( pclsSipStack->m_clsSetup.m_strLocalIp.c_str() );
		m_clsCallId = pclsRequest->m_clsCallId;
	}
	else
	{
		pclsRequest->m_clsCallId = m_clsCallId;
	}

	m_bAuth = false;

	if( pclsResponse )
	{
		m_bAuth = AddAuth( pclsRequest, pclsResponse );

		/* �Ʒ��� �ҽ� �ڵ� ������ FreeSwitch �� REGISTER ���� �ʾƼ� �ּ� ó����
		std::string	strToTag;

		if( pclsResponse->m_clsTo.SelectParam( SIP_TAG, strToTag ) )
		{
			pclsRequest->m_clsTo.InsertParam( SIP_TAG, strToTag.c_str() );
		}
		*/
	}
	else if( m_clsChallenge.m_strAlgorithm.empty() == false )
	{
		++m_iNonceCount;
		m_bAuth = AddAuth( pclsRequest, &m_clsChallenge, m_iChallengeStatusCode, m_iNonceCount );
	}

	pclsRequest->m_eTransport = m_eTransport;

	return pclsRequest;
}

/**
 * @ingroup SipUserAgent
 * @brief SIP REGISTER ��û�� ���� ���� �޽������� Chanllenge �� �����Ѵ�.
 * @param pclsResponse SIP REGISTER ��û�� ���� ���� �޽���
 * @returns �����ϸ� true �� �����ϰ� �����ϸ� false �� �����Ѵ�.
 */
bool CSipServerInfo::SetChallenge( CSipMessage * pclsResponse )
{
	SIP_CHALLENGE_LIST::const_iterator itAT;

	if( pclsResponse->m_iStatusCode == SIP_PROXY_AUTHENTICATION_REQUIRED )
	{
		if( pclsResponse->m_clsProxyAuthenticateList.size() == 0 ) return false;
		itAT = pclsResponse->m_clsProxyAuthenticateList.begin();
	}
	else
	{
		if( pclsResponse->m_clsWwwAuthenticateList.size() == 0 ) return false;
		itAT = pclsResponse->m_clsWwwAuthenticateList.begin();
	}

	if( (*itAT)->m_strQop.empty() ) return false;
	if( strncmp( (*itAT)->m_strQop.c_str(), "auth", 4 ) ) return false;
	
	m_clsChallenge = *(*itAT);
	m_iChallengeStatusCode = pclsResponse->m_iStatusCode;

	return true;
}

/**
 * @ingroup SipUserAgent
 * @brief SIP ��û �޽����� ���� ������ �߰��Ѵ�.
 * @param pclsRequest		SIP ��û �޽���
 * @param pclsResponse	SIP ���� �޽���
 * @returns �����ϸ� true �� �����ϰ� �����ϸ� false �� �����Ѵ�.
 */
bool CSipServerInfo::AddAuth( CSipMessage * pclsRequest, CSipMessage * pclsResponse )
{
	SIP_CHALLENGE_LIST::const_iterator itAT;

	if( pclsResponse->m_iStatusCode == SIP_PROXY_AUTHENTICATION_REQUIRED )
	{
		if( pclsResponse->m_clsProxyAuthenticateList.size() == 0 ) return false;
		itAT = pclsResponse->m_clsProxyAuthenticateList.begin();
	}
	else
	{
		if( pclsResponse->m_clsWwwAuthenticateList.size() == 0 ) return false;
		itAT = pclsResponse->m_clsWwwAuthenticateList.begin();
	}

	return AddAuth( pclsRequest, *itAT, pclsResponse->m_iStatusCode, 1 );
}

/**
 * @ingroup SipUserAgent
 * @brief SIP ��û �޽����� ���� ������ �߰��Ѵ�.
 * @param pclsRequest		SIP ��û �޽���
 * @param pclsChallenge SIP ���� �޽����� ����� Challenge ���� ���� ��ü
 * @param iStatusCode		SIP ���� �޽����� Status Code
 * @param iNonceCount		nonce count
 * @returns �����ϸ� true �� �����ϰ� �����ϸ� false �� �����Ѵ�.
 */
bool CSipServerInfo::AddAuth( CSipMessage * pclsRequest, const CSipChallenge * pclsChallenge, int iStatusCode, int iNonceCount )
{
	CSipCredential * pclsCredential = new CSipCredential();
	if( pclsCredential == NULL ) return false;

	pclsCredential->m_strType = pclsChallenge->m_strType;

	if( m_strAuthId.empty() )
	{
		pclsCredential->m_strUserName = m_strUserId;
	}
	else
	{
		pclsCredential->m_strUserName = m_strAuthId;
	}

	pclsCredential->m_strRealm = pclsChallenge->m_strRealm;
	pclsCredential->m_strNonce = pclsChallenge->m_strNonce;
	pclsCredential->m_strAlgorithm = pclsChallenge->m_strAlgorithm;
	pclsCredential->m_strOpaque = pclsChallenge->m_strOpaque;

	pclsCredential->m_strUri = "sip:";
	pclsCredential->m_strUri.append( m_strDomain );

	char	szA1[1024], szA2[1024], szMd5[33], szResponse[1024];
	const char * pszQop = pclsChallenge->m_strQop.c_str();

	if( pclsChallenge->m_strQop.empty() == false && !strncmp( pszQop, "auth", 4 ) )
	{
		STRING_LIST clsQopList;

		// qop="auth,auth-int" �� �ԷµǾ��� ���� ������ �׸��� �����Ѵ�.
		if( strstr( pszQop, "," ) )
		{
			SplitString( pszQop, clsQopList, ',' );
			
			STRING_LIST::iterator itSL;

			for( itSL = clsQopList.begin(); itSL != clsQopList.end(); ++itSL )
			{
				pclsCredential->m_strQop = *itSL;
			}
		}
		else
		{
			pclsCredential->m_strQop = pclsChallenge->m_strQop;
		}

		char szNonceCount[9];

		snprintf( szNonceCount, sizeof(szNonceCount), "%08d", iNonceCount );
		pclsCredential->m_strNonceCount = szNonceCount;
		pclsCredential->m_strCnonce = "1";

		snprintf( szA1, sizeof(szA1), "%s:%s:%s", pclsCredential->m_strUserName.c_str(), pclsCredential->m_strRealm.c_str(), m_strPassWord.c_str() );
		SipMd5String( szA1, szMd5 );
		snprintf( szA1, sizeof(szA1), "%s", szMd5 );
		
		// http://qnimate.com/understanding-http-authentication-in-depth/
		if( !strcmp( pclsCredential->m_strQop.c_str(), "auth-int" ) )
		{
			SipMd5String( pclsRequest->m_strBody.c_str(), szMd5 );
			snprintf( szA2, sizeof(szA2), "%s:%s:%s", pclsRequest->m_strSipMethod.c_str(), pclsCredential->m_strUri.c_str(), szMd5 );
		}
		else
		{
			snprintf( szA2, sizeof(szA2), "%s:%s", pclsRequest->m_strSipMethod.c_str(), pclsCredential->m_strUri.c_str() );
		}

		SipMd5String( szA2, szMd5 );
		snprintf( szA2, sizeof(szA2), "%s", szMd5 );
		
		snprintf( szResponse, sizeof(szResponse), "%s:%s:%s:%s:%s:%s", szA1, pclsCredential->m_strNonce.c_str(), pclsCredential->m_strNonceCount.c_str()
			, pclsCredential->m_strCnonce.c_str(), pclsCredential->m_strQop.c_str(), szA2 );
		SipMd5String( szResponse, szMd5 );
		snprintf( szResponse, sizeof(szResponse), "%s", szMd5 );

		pclsCredential->m_strResponse = szMd5;
	}
	else
	{
		snprintf( szA1, sizeof(szA1), "%s:%s:%s", pclsCredential->m_strUserName.c_str(), pclsCredential->m_strRealm.c_str(), m_strPassWord.c_str() );
		SipMd5String( szA1, szMd5 );
		snprintf( szA1, sizeof(szA1), "%s", szMd5 );
		
		snprintf( szA2, sizeof(szA2), "%s:%s", pclsRequest->m_strSipMethod.c_str(), pclsCredential->m_strUri.c_str() );
		SipMd5String( szA2, szMd5 );
		snprintf( szA2, sizeof(szA2), "%s", szMd5 );
		
		snprintf( szResponse, sizeof(szResponse), "%s:%s:%s", szA1, pclsCredential->m_strNonce.c_str(), szA2 );
		SipMd5String( szResponse, szMd5 );

		pclsCredential->m_strResponse = szMd5;
	}

	if( iStatusCode == SIP_PROXY_AUTHENTICATION_REQUIRED )
	{
		pclsRequest->m_clsProxyAuthorizationList.push_front( pclsCredential );
	}
	else
	{
		pclsRequest->m_clsAuthorizationList.push_front( pclsCredential );
	}

	return true;
}
