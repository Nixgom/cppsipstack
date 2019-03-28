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

#include "Log.h"
#include "EchoSipServer.h"
#include "EchoSipServerSetup.h"
#include "SipStackDefine.h"
#include <string.h>
#include <sys/stat.h>
#include "MemoryDebug.h"

CEchoSipServerSetup gclsSetup;

/**
 * @ingroup EchoSipServer
 * @brief ������
 */
CEchoSipServerSetup::CEchoSipServerSetup() : m_iUdpPort(5060), m_iUdpThreadCount(10)
	, m_iTcpPort(5060), m_iTcpThreadCount(10), m_iTcpCallBackThreadCount(0), m_iTcpRecvTimeout(SIP_TCP_RECV_TIMEOUT)
	, m_iTlsPort(0), m_iTlsAcceptTimeout(SIP_TLS_ACCEPT_TIMEOUT)
	, m_iLogLevel(0), m_iLogMaxSize(20000000)
	, m_iFileSize(0)
{
}

/**
 * @ingroup EchoSipServer
 * @brief �Ҹ���
 */
CEchoSipServerSetup::~CEchoSipServerSetup()
{
}

/**
 * @ingroup EchoSipServer
 * @brief ���� ������ �о ��� ������ �����Ѵ�.
 * @param pszFileName ���� ���� full path
 * @returns �����ϸ� true �� �����ϰ� �����ϸ� false �� �����Ѵ�.
 */
bool CEchoSipServerSetup::Read( const char * pszFileName )
{
	CXmlElement clsXml, * pclsElement;

	if( clsXml.ParseFile( pszFileName ) == false ) return false;

	// SIP ����
	pclsElement = clsXml.SelectElement( "Sip" );
	if( pclsElement == NULL ) return false;

	pclsElement->SelectElementData( "LocalIp", m_strLocalIp );
	pclsElement->SelectElementData( "UdpPort", m_iUdpPort );
	pclsElement->SelectElementData( "UdpThreadCount", m_iUdpThreadCount );
	pclsElement->SelectElementData( "TcpPort", m_iTcpPort );
	pclsElement->SelectElementData( "TcpThreadCount", m_iTcpThreadCount );
	pclsElement->SelectElementData( "TcpCallBackThreadCount", m_iTcpCallBackThreadCount );
	pclsElement->SelectElementData( "TcpRecvTimeout", m_iTcpRecvTimeout );
	pclsElement->SelectElementData( "TlsPort", m_iTlsPort );
	pclsElement->SelectElementData( "TlsAcceptTimeout", m_iTlsAcceptTimeout );
	pclsElement->SelectElementData( "CertFile", m_strCertFile );
	pclsElement->SelectElementData( "CaCertFile", m_strCaCertFile );

	// �α�
	pclsElement = clsXml.SelectElement( "Log" );
	if( pclsElement == NULL ) return false;

	pclsElement->SelectElementData( "Folder", m_strLogFolder );

	Read( clsXml );

	m_strFileName = pszFileName;
	SetFileSizeTime();

	return true;
}

/**
 * @ingroup EchoSipServer
 * @brief ������ ���� ������ �д´�.
 * @returns �����ϸ� true �� �����ϰ� �����ϸ� false �� �����Ѵ�.
 */
bool CEchoSipServerSetup::Read( )
{
	if( m_strFileName.length() == 0 ) return false;

	CXmlElement clsXml;

	if( clsXml.ParseFile( m_strFileName.c_str() ) == false ) return false;

	Read( clsXml );
	SetFileSizeTime();

	return true;
}

/**
 * @brief ���� ������ ���� �߿��� �ǽð����� ���� ������ �׸��� �ٽ� �����Ѵ�.
 * @param clsXml ���� ������ ������ ������ ����
 * @returns �����ϸ� true �� �����ϰ� �����ϸ� false �� �����Ѵ�.
 */
bool CEchoSipServerSetup::Read( CXmlElement & clsXml )
{
	CXmlElement * pclsElement;

	// �α�
	pclsElement = clsXml.SelectElement( "Log" );
	if( pclsElement ) 
	{
		m_iLogLevel = 0;

		CXmlElement * pclsClient = pclsElement->SelectElement( "Level" );
		if( pclsClient )
		{
			bool bTemp;

			pclsClient->SelectAttribute( "Debug", bTemp );
			if( bTemp ) m_iLogLevel |= LOG_DEBUG;

			pclsClient->SelectAttribute( "Info", bTemp );
			if( bTemp ) m_iLogLevel |= LOG_INFO;

			pclsClient->SelectAttribute( "Network", bTemp );
			if( bTemp ) m_iLogLevel |= LOG_NETWORK;

			pclsClient->SelectAttribute( "Sql", bTemp );
			if( bTemp ) m_iLogLevel |= LOG_SQL;
		}

		pclsElement->SelectElementData( "MaxSize", m_iLogMaxSize );

		CLog::SetLevel( m_iLogLevel );
		CLog::SetMaxLogSize( m_iLogMaxSize );
	}

	return true;
}

/**
 * @ingroup EchoSipServer
 * @brief ���������� �����Ǿ����� Ȯ���Ѵ�.
 * @returns ���������� �����Ǿ����� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CEchoSipServerSetup::IsChange()
{
	struct stat	clsStat;

	if( stat( m_strFileName.c_str(), &clsStat ) == 0 )
	{
		if( m_iFileSize != clsStat.st_size || m_iFileTime != clsStat.st_mtime ) return true;
	}

	return false;
}

/**
 * @ingroup EchoSipServer
 * @brief ���������� ���� �ð��� �����Ѵ�.
 */
void CEchoSipServerSetup::SetFileSizeTime( )
{
	struct stat	clsStat;

	if( stat( m_strFileName.c_str(), &clsStat ) == 0 )
	{
		m_iFileSize = clsStat.st_size;
		m_iFileTime = clsStat.st_mtime;
	}
}
