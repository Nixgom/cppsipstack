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

#include "SipPlatformDefine.h"
#include "Log.h"
#include "SipCallMap.h"
#include "RtpMap.h"
#include "IpFragmentMap.h"
#include "MemoryDebug.h"

/**
 * @ingroup SipCallDump
 * @brief UDP ��Ŷ�� �м��Ѵ�.
 * @param psttPcap			pcap_t ������
 * @param psttHeader		pcap_pkthdr ������
 * @param pszData				��Ŷ
 * @param psttIp4Header IPv4 ���
 * @param iIpPos				IP ��� ��ġ
 * @param iIpHeaderLen	IP ��� ����
 */
void PacketDumpUdp( pcap_t * psttPcap, struct pcap_pkthdr * psttHeader, const u_char * pszData, Ip4Header * psttIp4Header, int iIpPos, int iIpHeaderLen )
{
	UdpHeader * psttUdpHeader = (UdpHeader *)( pszData + iIpPos + iIpHeaderLen );
	char * pszUdpBody = (char *)( pszData + iIpPos + iIpHeaderLen + 8 );
	int iUdpBodyLen = psttHeader->caplen - ( iIpPos + iIpHeaderLen + 8 );
	if( iUdpBodyLen <= MIN_UDP_BODY_SIZE ) return;

	if( IsRtpPacket( pszUdpBody, iUdpBodyLen ) )
	{
		gclsRtpMap.Insert( psttHeader, pszData, psttIp4Header, psttUdpHeader );
	}
	else
	{
		bool bEnd;

		// fragment ó��
		if( gclsIpFragmentMap.Insert( psttIp4Header, (char *)pszData + iIpPos + iIpHeaderLen, psttHeader->caplen - ( iIpPos + iIpHeaderLen ), bEnd ) )
		{
			if( bEnd == false ) return;

			CIpPacket clsPacket;

			if( gclsIpFragmentMap.Delete( psttIp4Header, &clsPacket ) == false ) 
			{
				CLog::Print( LOG_ERROR, "%s gclsIpFragmentMap.Delete() error", __FUNCTION__ );
				return;
			}

			pszUdpBody = (char *)( clsPacket.m_pszPacket + 20 + 8 );
			iUdpBodyLen = clsPacket.m_iPacketLen - ( 20 + 8 );

			if( IsSipPacket( pszUdpBody, iUdpBodyLen ) )
			{
				u_char * pszNewData = (u_char *)malloc( iIpPos + clsPacket.m_iPacketLen );
				if( pszNewData == NULL )
				{
					CLog::Print( LOG_ERROR, "%s new data malloc error", __FUNCTION__ );
				}
				else
				{
					memcpy( pszNewData, pszData, iIpPos );
					memcpy( pszNewData + iIpPos, clsPacket.m_pszPacket, clsPacket.m_iPacketLen );
					psttHeader->caplen = iIpPos + clsPacket.m_iPacketLen;
					psttHeader->len = psttHeader->caplen;
					gclsCallMap.Insert( psttPcap, psttHeader, pszNewData, pszUdpBody, iUdpBodyLen );
					free( pszNewData );
				}
			}
		}
		else if( IsSipPacket( pszUdpBody, iUdpBodyLen ) )
		{
			gclsCallMap.Insert( psttPcap, psttHeader, pszData, pszUdpBody, iUdpBodyLen );
		}
	}
}
