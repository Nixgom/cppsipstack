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

#include "IpFragmentMap.h"
#include "TimeUtility.h"
#include "Log.h"
#include "MemoryDebug.h"

CIpFragmentMap gclsIpFragmentMap;

CIpPacket::CIpPacket() : m_pszPacket(NULL), m_iPacketLen(0)
{
}

CIpPacket::~CIpPacket()
{
	if( m_pszPacket )
	{
		free( m_pszPacket );
		m_pszPacket = NULL;
	}
}

CIpFragmentData::CIpFragmentData() : m_bMoreFragment(false), m_sFragmentOffset(0), m_pszIpBody(NULL), m_iIpBodyLen(0)
{
}

CIpFragmentData::~CIpFragmentData()
{
	if( m_pszIpBody )
	{
		free( m_pszIpBody );
		m_pszIpBody = NULL;
	}
}

CIpFragmentInfo::CIpFragmentInfo() : m_iTime(NULL), m_bRecvFragmentEnd(false)
{
}

/**
 * @ingroup SipCallDump
 * @brief fragment �� ��Ŷ�� �����Ѵ�.
 * @param psttIp4Header IPv4 ���
 * @param pszIpBody			IP body
 * @param iIpBodyLen		IP body len
 * @param bEnd					[out] ��� fragment ��Ŷ�� ���ŵǾ����� true �� ����ȴ�.
 * @returns �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CIpFragmentInfo::Insert( Ip4Header * psttIp4Header, char * pszIpBody, int iIpBodyLen, bool & bEnd )
{
	uint16_t sFlags = ntohs(psttIp4Header->flags_fo);

	CIpFragmentData * pclsData = new CIpFragmentData();
	if( pclsData == NULL )
	{
		CLog::Print( LOG_ERROR, "%s new error", __FUNCTION__ );
		return false;
	}

	pclsData->m_pszIpBody = (u_char *)malloc( iIpBodyLen );
	if( pclsData->m_pszIpBody == NULL )
	{
		CLog::Print( LOG_ERROR, "%s malloc error", __FUNCTION__ );
		delete pclsData;
		return false;
	}

	memcpy( pclsData->m_pszIpBody, pszIpBody, iIpBodyLen );
	pclsData->m_iIpBodyLen = iIpBodyLen;

	if( sFlags & 0x2000 )
	{
		pclsData->m_bMoreFragment = true;
	}
	else
	{
		m_bRecvFragmentEnd = true;
	}

	pclsData->m_sFragmentOffset = sFlags & 0x1FFF;

	if( pclsData->m_sFragmentOffset == 0 )
	{
		memcpy( &m_sttIpHeader, psttIp4Header, sizeof(m_sttIpHeader) );
	}

	if( m_clsList.empty() )
	{
		m_clsList.push_back( pclsData );
	}
	else
	{
		IP_FRAGMENT_DATA_LIST::iterator itList;
		bool bFound = false;

		for( itList = m_clsList.begin(); itList != m_clsList.end(); ++itList )
		{
			if( pclsData->m_sFragmentOffset == (*itList)->m_sFragmentOffset )
			{
				// ������ Fragment Offset �̸� �������� �ʴ´�.
				delete pclsData;
				bFound = true;
				break;
			}
			else if( pclsData->m_sFragmentOffset < (*itList)->m_sFragmentOffset )
			{
				m_clsList.insert( itList, pclsData );
				bFound = true;
				break;
			}
		}

		if( bFound == false )
		{
			m_clsList.push_back( pclsData );
		}
	}

	time( &m_iTime );

	if( m_bRecvFragmentEnd )
	{
		IP_FRAGMENT_DATA_LIST::iterator itList;
		int iWantOffset = 0;
		bool bError = false;

		for( itList = m_clsList.begin(); itList != m_clsList.end(); ++itList )
		{
			if( iWantOffset != (*itList)->m_sFragmentOffset )
			{
				bError = true;
				break;
			}

			iWantOffset += (*itList)->m_iIpBodyLen;
		}

		if( bError == false )
		{
			bEnd = true;
		}
	}

	return true;
}

/**
 * @ingroup SipCallDump
 * @brief fragment �� ��Ŷ���� ���ļ� �ϳ��� IP ��Ŷ���� �����Ѵ�.
 * @param pclsPacket [out] �ϳ��� IP ��Ŷ ���� ��ü
 * @returns �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CIpFragmentInfo::GetPacket( CIpPacket * pclsPacket )
{
	IP_FRAGMENT_DATA_LIST::iterator itList;
	int iIpBodyLen = 0;

	for( itList = m_clsList.begin(); itList != m_clsList.end(); ++itList )
	{
		iIpBodyLen += (*itList)->m_iIpBodyLen;
	}

	pclsPacket->m_pszPacket = (u_char *)malloc( sizeof(m_sttIpHeader) + iIpBodyLen );
	if( pclsPacket->m_pszPacket == NULL )
	{
		CLog::Print( LOG_ERROR, "%s malloc error", __FUNCTION__ );
		return false;
	}

	memcpy( pclsPacket->m_pszPacket, &m_sttIpHeader, sizeof(m_sttIpHeader) );
	pclsPacket->m_iPacketLen = sizeof(m_sttIpHeader);

	for( itList = m_clsList.begin(); itList != m_clsList.end(); ++itList )
	{
		memcpy( pclsPacket->m_pszPacket + pclsPacket->m_iPacketLen, (*itList)->m_pszIpBody, (*itList)->m_iIpBodyLen );
		pclsPacket->m_iPacketLen += (*itList)->m_iIpBodyLen;
	}

	Ip4Header * psttIpHeader = (Ip4Header *)pclsPacket->m_pszPacket;
	psttIpHeader->flags_fo = 0;
	psttIpHeader->tlen = htons( pclsPacket->m_iPacketLen );

	return true;
}

CIpFragmentMap::CIpFragmentMap()
{
}

CIpFragmentMap::~CIpFragmentMap()
{
}

/**
 * @ingroup SipCallDump
 * @brief �ڷᱸ���� fragment �� ��Ŷ�� �����Ѵ�.
 * @param psttIp4Header IPv4 ���
 * @param pszIpBody			IP body
 * @param iIpBodyLen		IP body len
 * @param bEnd					[out] ��� fragment ��Ŷ�� ���ŵǾ����� true �� ����ȴ�.
 * @returns �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CIpFragmentMap::Insert( Ip4Header * psttIp4Header, char * pszIpBody, int iIpBodyLen, bool & bEnd )
{
	if( psttIp4Header->flags_fo == 0 ) return false;

	std::string strKey;
	IP_FRAGMENT_MAP::iterator itMap;

	bEnd = false;
	GetKey( psttIp4Header, strKey );

	m_clsMutex.acquire();
	itMap = m_clsMap.find( strKey );
	if( itMap == m_clsMap.end() )
	{
		CIpFragmentInfo * pclsInfo = new CIpFragmentInfo();
		if( pclsInfo == NULL )
		{
			CLog::Print( LOG_ERROR, "%s new error", __FUNCTION__ );
		}
		else
		{
			pclsInfo->Insert( psttIp4Header, pszIpBody, iIpBodyLen, bEnd );
		}
	}
	else
	{
		itMap->second->Insert( psttIp4Header, pszIpBody, iIpBodyLen, bEnd );
	}
	m_clsMutex.release();
	
	return true;
}

/**
 * @ingroup SipCallDump
 * @brief �ڷᱸ������ fragment ��Ŷ ���� ������ �����Ѵ�.
 * @param psttIp4Header IPv4 ���
 * @param pclsPacket		[out] ������ fragment ��Ŷ���� ���ļ� �ϳ��� IP ��Ŷ���� ����� ��ü
 * @returns �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CIpFragmentMap::Delete( Ip4Header * psttIp4Header, CIpPacket * pclsPacket )
{
	bool bRes = false;
	std::string strKey;
	IP_FRAGMENT_MAP::iterator itMap;

	GetKey( psttIp4Header, strKey );

	m_clsMutex.acquire();
	itMap = m_clsMap.find( strKey );
	if( itMap != m_clsMap.end() )
	{
		if( pclsPacket )
		{
			bRes = itMap->second->GetPacket( pclsPacket );
		}
		else
		{
			bRes = true;
		}

		m_clsMap.erase( itMap );
	}
	m_clsMutex.release();

	return bRes;
}

/**
 * @ingroup SipCallDump
 * @brief 60�� �̻� ���� fragment ��Ŷ���� �����Ѵ�.
 */
void CIpFragmentMap::DeleteTimeout()
{
	IP_FRAGMENT_MAP::iterator itMap, itNext;
	time_t iTime;

	time( &iTime );
	iTime -= 60;

	m_clsMutex.acquire();
	for( itMap = m_clsMap.begin(); itMap != m_clsMap.end(); ++itMap )
	{
LOOP_START:
		if( iTime > itMap->second->m_iTime )
		{
			itNext = itMap;
			++itNext;
			delete itMap->second;
			m_clsMap.erase( itMap );

			if( itNext == m_clsMap.end() ) break;
			itMap = itNext;
			goto LOOP_START;
		}
	}
	m_clsMutex.release();
}

/**
 * @ingroup SipCallDump
 * @brief Ű�� �����Ѵ�.
 * @param psttIp4Header IPv4 ���
 * @param strKey				Ű ���� ����
 */
void CIpFragmentMap::GetKey( Ip4Header * psttIp4Header, std::string & strKey )
{
	char	szKey[22];

	snprintf( szKey, sizeof(szKey), "%X_%X_%X", psttIp4Header->saddr, psttIp4Header->daddr, psttIp4Header->identification );
	strKey = szKey;
}
