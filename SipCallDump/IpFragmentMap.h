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

#ifndef _IP_FRAGMENT_MAP_H_
#define _IP_FRAGMENT_MAP_H_

#include "PacketHeader.h"
#include "SipMutex.h"
#include <string>
#include <list>
#include <map>

/**
 * @ingroup SipCallDump
 * @brief IP Header �� ������ ��Ŷ�� �����ϴ� Ŭ����
 */
class CIpPacket
{
public:
	CIpPacket();
	~CIpPacket();

	u_char		* m_pszPacket;
	int				m_iPacketLen;
};

/**
 * @ingroup SipCallDump
 * @brief �ϳ��� IP Fragment �� ��Ŷ�� �����ϴ� Ŭ����
 */
class CIpFragmentData
{
public:
	CIpFragmentData();
	~CIpFragmentData();

	bool			m_bMoreFragment;
	uint16_t	m_sFragmentOffset;
	u_char	* m_pszIpBody;
	int				m_iIpBodyLen;
};

typedef std::list< CIpFragmentData * > IP_FRAGMENT_DATA_LIST;

/**
 * @ingroup SipCallDump
 * @brief ID �� ��ġ�ϴ� IP Fragment �� ��Ŷ�� �����ϴ� Ŭ����
 */
class CIpFragmentInfo
{
public:
	CIpFragmentInfo();

	bool Insert( Ip4Header * psttIp4Header, char * pszIpBody, int iIpBodyLen, bool & bEnd );
	bool GetPacket( CIpPacket * pclsPacket );

	IP_FRAGMENT_DATA_LIST m_clsList;
	time_t		m_iTime;
	bool			m_bRecvFragmentEnd;
	Ip4Header m_sttIpHeader;
};

typedef std::map< std::string, CIpFragmentInfo * > IP_FRAGMENT_MAP;

/**
 * @ingroup SipCallDump
 * @brief IP Fragment �� ��Ŷ�� IP �� ID �� �������� �����ϴ� �ڷᱸ��
 */
class CIpFragmentMap
{
public:
	CIpFragmentMap();
	~CIpFragmentMap();

	bool Insert( Ip4Header * psttIp4Header, char * pszIpBody, int iIpBodyLen, bool & bEnd );
	bool Delete( Ip4Header * psttIp4Header, CIpPacket * pclsPacket );

	void DeleteTimeout();

private:
	void GetKey( Ip4Header * psttIp4Header, std::string & strKey );

	IP_FRAGMENT_MAP m_clsMap;
	CSipMutex m_clsMutex;
};

extern CIpFragmentMap gclsIpFragmentMap;

#endif
