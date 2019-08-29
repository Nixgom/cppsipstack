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
#include "TcpMap.h"
#include "IpFragmentMap.h"
#include "SipCallDumpSetup.h"
#include "MemoryDebug.h"

/**
 * @ingroup SipCallDump
 * @brief TCP ��Ŷ�� �м��Ѵ�.
 * @param psttPcap			pcap_t ������
 * @param psttHeader		pcap_pkthdr ������
 * @param pszData				��Ŷ
 * @param psttIp4Header IPv4 ���
 * @param iIpPos				IP ��� ��ġ
 * @param iIpHeaderLen	IP ��� ����
 */
void PacketDumpTcp( pcap_t * psttPcap, struct pcap_pkthdr * psttHeader, const u_char * pszData, Ip4Header * psttIp4Header, int iIpPos, int iIpHeaderLen )
{
	TcpHeader * psttTcpHeader = (TcpHeader *)( pszData + iIpPos + iIpHeaderLen );

	if( ( psttTcpHeader->flags & TCP_SYN ) && !( psttTcpHeader->flags & TCP_ACK ) )
	{
		return;
	}
	else if( ( psttTcpHeader->flags & TCP_FIN ) || ( psttTcpHeader->flags & TCP_RST ) )
	{
		return;
	}

	uint16_t sSrcPort = ntohs( psttTcpHeader->sport );
	uint16_t sDstPort = ntohs( psttTcpHeader->dport );

	// ���� ���Ͽ� ������ SIP ��Ʈ ��ȣ�� �ƴϸ� �����Ѵ�.
	if( gclsSetup.IsTcpSipPort( sSrcPort ) == false && gclsSetup.IsTcpSipPort( sDstPort ) == false ) return;

	int iTcpHeaderLen = GetTcpHeaderLength( psttTcpHeader );
	int iTcpBodyPos = iIpPos + iIpHeaderLen + iTcpHeaderLen;
	char * pszTcpBody = (char *)( pszData + iTcpBodyPos );
	int iTcpBodyLen = psttHeader->caplen - iTcpBodyPos;

	// TCP body len �� 0 �̸� �����Ѵ�.
	if( iTcpBodyLen <= 0 ) return;

	// IP fragment �� TCP packet �� ���� ���������� �����Ѵ�.

	if( IsSipPacket( pszTcpBody, iTcpBodyLen ) )
	{
		gclsTcpMap.Insert( psttHeader, pszData, psttIp4Header, psttTcpHeader, iTcpBodyPos, iTcpBodyLen );
	}
	else
	{
		gclsTcpMap.Update( psttHeader, pszData, psttIp4Header, psttTcpHeader, iTcpBodyPos, iTcpBodyLen );
	}
}
