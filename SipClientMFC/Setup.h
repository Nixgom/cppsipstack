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

#pragma once

#include <string>
#include <map>

#define SETUP_FILENAME	"SipClient.ini"

#define ST_SIP_SERVER_IP		"sip_server_ip"
#define ST_SIP_SERVER_PORT	"sip_server_port"
#define ST_SIP_DOMAIN				"sip_domain"
#define ST_USER_ID					"user_id"
#define ST_PASSWORD					"password"
#define ST_USERAGENT				"user_agent"
#define ST_PROTOCOL					"protocol"
#define ST_LOCAL_IP					"local_ip"

typedef std::map< std::string, std::string > SETUP_MAP;

/**
 * @ingroup SipClientMFC
 * @brief 설정 파일 저장/읽기 클래스
 */
class CSetup
{
public:
	CSetup();
	~CSetup();

	bool Get();
	bool Put();
	bool IsSet();

	std::string m_strSipServerIp;
	int         m_iSipServerPort;
	std::string m_strSipDomain;
	std::string m_strUserId;
	std::string m_strPassWord;
	std::string m_strUserAgent;
	std::string m_strProtocol;
	std::string m_strLocalIp;

private:
	bool GetFile();
	bool PutFile();

	int GetInt( const char * pszName, int iIndex, int iDefaultValue );
	int GetInt( const char * pszName, int iDefaultValue );
	bool GetString( const char * pszName, std::string & strValue );

	bool PutInt( const char * pszName, int iIndex, int iValue );
	bool PutInt( const char * pszName, int iValue );
	bool PutString( const char * pszName, const char * pszValue );

	SETUP_MAP	m_clsMap;
};

extern CSetup gclsSetup;
