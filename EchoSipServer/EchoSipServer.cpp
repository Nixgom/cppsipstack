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

#include "EchoSipServer.h"
#include "SipServer.h"
#include "EchoSipServerSetup.h"
#include "Log.h"
#include "ServerUtility.h"
#include "Directory.h"
#include "EchoSipServerVersion.h"
#include "SipUserAgentVersion.h"
#include "ServerService.h"
#include "MemoryDebug.h"

bool gbFork = true;

/**
 * @ingroup EchoSipServer
 * @brief ��ȭ ���Ž� �ش� ��ȭ�� ������ �ִ� ����
 * @returns ���� �����ϸ� 0 �� �����ϰ� ������ �߻��ϸ� -1 �� �����Ѵ�.
 */
int ServiceMain( )
{
#ifdef WIN32
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF );
#endif

	if( gclsSetup.Read( GetConfigFileName() ) == false && gclsSetup.Read( CONFIG_FILENAME ) == false )
	{
		printf( "config filename(%s) read error\n", GetConfigFileName() );
		return -1;
	}

	CLog::SetDirectory( gclsSetup.m_strLogFolder.c_str() );
	CLog::Print( LOG_SYSTEM, "EchoSipServer is started ( version-%s %s %s )", ECHO_SIP_SERVER_VERSION, __DATE__, __TIME__ );

	CSipStackSetup clsSetup;

	if( gclsSetup.m_strLocalIp.empty() )
	{
		// N���� IP�ּҸ� ����ϴ� ȣ��Ʈ������ SIP �������ݷ� ����� IP�ּҸ� ���� �Է��� �ּ���.
		// Vmware ���� ����ϴ� ��� N���� IP�ּҰ� ȣ��Ʈ�� �����մϴ�.
		GetLocalIp( clsSetup.m_strLocalIp );
		gclsSetup.m_strLocalIp = clsSetup.m_strLocalIp;
	}
	else
	{
		clsSetup.m_strLocalIp = gclsSetup.m_strLocalIp;
	}

	clsSetup.m_iLocalUdpPort = gclsSetup.m_iUdpPort;
	clsSetup.m_iUdpThreadCount = gclsSetup.m_iUdpThreadCount;

	clsSetup.m_iLocalTcpPort = gclsSetup.m_iTcpPort;
	clsSetup.m_iTcpThreadCount = gclsSetup.m_iTcpThreadCount;
	clsSetup.m_iTcpCallBackThreadCount = gclsSetup.m_iTcpCallBackThreadCount;

	clsSetup.m_iLocalTlsPort = gclsSetup.m_iTlsPort;
	clsSetup.m_iTlsAcceptTimeout = gclsSetup.m_iTlsAcceptTimeout;
	clsSetup.m_strCertFile = gclsSetup.m_strCertFile;
	clsSetup.m_strCaCertFile = gclsSetup.m_strCaCertFile;
	
	clsSetup.m_strUserAgent = "EchoSipServer_";
	clsSetup.m_strUserAgent.append( ECHO_SIP_SERVER_VERSION );

	Fork( gbFork );
	SetCoreDumpEnable();
	ServerSignal();

	if( gclsSipServer.Start( clsSetup ) == false )
	{
		CLog::Print( LOG_ERROR, "SipServer start error" );
		return -1;
	}

	int iSecond = 0;

	while( gbStop == false )
	{
		sleep(1);
		++iSecond;

		if( iSecond == 3600 )
		{
			iSecond = 0;
		}

		if( gclsSetup.IsChange() )
		{
			gclsSetup.Read();
		}
	}

	gclsUserAgent.Stop();
	gclsUserAgent.Final();

	CLog::Print( LOG_SYSTEM, "EchoSipServer is terminated" );
	CLog::Release();

	return 0;
}

/**
 * @ingroup EchoSipServer
 * @brief ��ȭ ���Ž� �ش� ��ȭ�� ������ �ִ� ����
 * @param argc 
 * @param argv 
 * @returns ���� �����ϸ� 0 �� �����ϰ� ������ �߻��ϸ� -1 �� �����Ѵ�.
 */
int main( int argc, char * argv[] )
{
	CServerService clsService;

	clsService.m_strName = SERVICE_NAME;
	clsService.m_strDisplayName = SERVICE_DISPLAY_NAME;
	clsService.m_strDescription = SERVICE_DESCRIPTION_STRING;
	clsService.m_strConfigFileName = CONFIG_FILENAME;
	clsService.m_strVersion = ECHO_SIP_SERVER_VERSION;
	clsService.SetBuildDate( __DATE__, __TIME__ );

	if( argc == 3 && !strcmp( argv[2], "-n" ) )
	{
		gbFork = false;
	}

	ServerMain( argc, argv, clsService, ServiceMain );

	return 0;
}
