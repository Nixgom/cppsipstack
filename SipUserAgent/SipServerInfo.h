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

#ifndef _SIP_REGISTER_INFO_H_
#define _SIP_REGISTER_INFO_H_

#include "SipMessage.h"

class CSipServerInfo
{
public:
	CSipServerInfo(void);
	~CSipServerInfo(void);

	void ClearLogin();

	std::string		m_strIp;
	int						m_iPort;

	std::string		m_strDomain;
	std::string		m_strUserId;
	std::string		m_strPassWord;

	int						m_iLoginTimeout;

	bool					m_bLogin;
	time_t				m_iLoginTime;
	time_t				m_iSendTime;
	CSipCallId		m_clsCallId;
	int						m_iSeqNo;

	CSipMessage * GetRegisterMessage( const CSipMessage * pclsResponse );
	bool AddAuth( CSipMessage * pclsRequest, const CSipMessage * pclsResponse );
};

typedef std::list< CSipServerInfo > SIP_SERVER_INFO_LIST;

#endif