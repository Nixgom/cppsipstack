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

#ifndef _ECHO_SIP_SERVER_SETUP_H_
#define _ECHO_SIP_SERVER_SETUP_H_

#include "XmlElement.h"
#include "StringMap.h"
#include <string>
#include <list>

/**
 * @ingroup EchoSipServer
 * @brief EchoSipServer ���� ������ ������ �����ϴ� Ŭ����
 */
class CEchoSipServerSetup
{
public:
	CEchoSipServerSetup();
	~CEchoSipServerSetup();

	/** SIP ����� ���� UDP ��Ʈ ��ȣ */
	int					m_iUdpPort;

	/** SIP ����� ���� ���� IP �ּ� */
	std::string	m_strLocalIp;

	/** SIP ����� ���� UDP ���� ������ ���� */
	int					m_iUdpThreadCount;

	/** SIP ����� ���� TCP ��Ʈ ��ȣ */
	int					m_iTcpPort;

	/** SIP ����� ���� TCP ���� ������ ���� */
	int					m_iTcpThreadCount;

	/** SIP �޽��� ���� callback ó���� ���� TCP ������ ����. 
			�� ������ 0 �̸� TCP ���� �����忡�� callback �� ȣ���ϰ� 0 ���� ũ�� tcp callback �����忡�� callback �� ȣ���Ѵ�. */
	int					m_iTcpCallBackThreadCount;

	/** SIP ����� ���� TCP ���� �ִ� ��� �ð� ( �ʴ��� ) */
	int					m_iTcpRecvTimeout;

	/** SIP ����� ���� TLS ��Ʈ ��ȣ */
	int					m_iTlsPort;

	/** SIP ����� ���� TLS handshake timeout �ð� (�ʴ���) */
	int					m_iTlsAcceptTimeout;

	/** TLS ���������� ���� ���� ������ + Ű�� ������ PEM ���� full path */ 
	std::string	m_strCertFile;

	/** TLS �������� ������ Ŭ���̾�Ʈ ������ ���� ���� ��� ������ PEM ���� */
	std::string m_strCaCertFile;

	// ================================================================
	// �α� ���

	/** �α� ���� */
	std::string	m_strLogFolder;

	/** �α� ���� */
	int					m_iLogLevel;

	/** �α� ������ �ִ� ũ�� */
	int					m_iLogMaxSize;

	bool Read( const char * pszFileName );
	bool Read( );
	
	bool IsChange();

private:
	bool Read( CXmlElement & clsXml );
	void SetFileSizeTime( );

	std::string	m_strFileName;	// ���� ���� �̸�
	time_t			m_iFileTime;		// ���� ���� ���� �ð�
	int					m_iFileSize;		// ���� ���� ũ��
};

extern CEchoSipServerSetup gclsSetup;

#endif
