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

#include "SipDeleteQueue.h"
#include "MemoryDebug.h"

CSipDeleteQueue gclsSipDeleteQueue;
extern time_t giTime;

/**
 * @ingroup SipStack
 * @brief ������
 */
CSipDeleteQueue::CSipDeleteQueue() : m_iQueueSize(0)
{
}

/**
 * @ingroup SipStack
 * @brief �Ҹ���
 */
CSipDeleteQueue::~CSipDeleteQueue()
{
}

/**
 * @ingroup SipStack
 * @brief ���� �ð� �Ŀ� ������ SIP �޽����� �����Ѵ�.
 * @param pclsMessage ����� SIP �޽���
 */
void CSipDeleteQueue::Insert( CSipMessage * pclsMessage )
{
	if( pclsMessage->m_iUseCount == 0 )
	{
		if( InsertCache( pclsMessage, true ) == false )
		{
			delete pclsMessage;
		}

		return;
	}

	CSipDeleteInfo clsInfo;

	clsInfo.m_pclsMessage = pclsMessage;
	clsInfo.m_iDeleteTime = giTime + SIP_DELETE_TIME;

	m_clsMutex.acquire();
	m_clsDeleteQueue.push_back( clsInfo );
	m_clsMutex.release();
}

/**
 * @ingroup SipStack
 * @brief ��� �ð��� �ʰ��� SIP �޽����� �����Ѵ�.
 */
void CSipDeleteQueue::DeleteTimeout( )
{
	time_t	iTime = giTime;

	m_clsMutex.acquire();
	while( m_clsDeleteQueue.empty() == false )
	{
		CSipDeleteInfo & clsInfo = m_clsDeleteQueue.front();

		if( clsInfo.m_iDeleteTime > iTime ) break;
		if( clsInfo.m_pclsMessage->m_iUseCount > 0 ) break;

		if( InsertCache( clsInfo.m_pclsMessage, false ) == false )
		{
			delete clsInfo.m_pclsMessage;
		}

		m_clsDeleteQueue.pop_front();
	}
	m_clsMutex.release();
}

/**
 * @ingroup SipStack
 * @brief ��� SIP �޽����� ��� �����Ѵ�.
 */
void CSipDeleteQueue::DeleteAll( )
{
	SIP_DELETE_QUEUE::iterator	it;

	m_clsMutex.acquire();
	while( m_clsDeleteQueue.empty() == false )
	{
		CSipDeleteInfo & clsInfo = m_clsDeleteQueue.front();
		delete clsInfo.m_pclsMessage;
		m_clsDeleteQueue.pop_front();
	}

	while( m_clsCacheQueue.empty() == false )
	{
		delete m_clsCacheQueue.front();
		m_clsCacheQueue.pop_front();
	}
	m_clsMutex.release();
}

/**
 * @ingroup SipStack
 * @brief		�ڷᱸ���� ����� SIP �޽����� ������ �����Ѵ�.
 * @returns �ڷᱸ���� ����� SIP �޽����� ������ �����Ѵ�.
 */
int CSipDeleteQueue::GetSize()
{
	int iSize;

	m_clsMutex.acquire();
	iSize = (int)m_clsDeleteQueue.size();
	m_clsMutex.release();

	return iSize;
}

int CSipDeleteQueue::GetCacheSize()
{
	int iSize;

	m_clsMutex.acquire();
	iSize = (int)m_clsCacheQueue.size();
	m_clsMutex.release();

	return iSize;
}

/**
 * @ingroup SipStack2
 * @brief CSipMessage �� ť���ϴ� �ִ� ũ�⸦ �����Ѵ�.
 * @param iQueueSize CSipMessage �� ť���ϴ� �ִ� ũ��
 */
void CSipDeleteQueue::SetQueueSize( int iQueueSize )
{
	m_iQueueSize = iQueueSize;
}

CSipMessage * CSipDeleteQueue::Get( )
{
	CSipMessage * pclsMessage = NULL;

	if( m_iQueueSize > 0 )
	{
		m_clsMutex.acquire();
		if( m_clsCacheQueue.empty() == false )
		{
			pclsMessage = m_clsCacheQueue.front();
			m_clsCacheQueue.pop_front();
		}
		m_clsMutex.release();
	}

	if( pclsMessage == NULL )
	{
		pclsMessage = new CSipMessage();
	}

	return pclsMessage;
}

bool CSipDeleteQueue::InsertCache( CSipMessage * pclsMessage, bool bUseMutex )
{
	bool bRes = false;

	if( m_iQueueSize > 0 )
	{
		pclsMessage->Clear();

		if( bUseMutex ) m_clsMutex.acquire();
		if( m_iQueueSize > (int)m_clsCacheQueue.size() )
		{
			m_clsCacheQueue.push_back( pclsMessage );
			bRes = true;
		}
		if( bUseMutex ) m_clsMutex.release();
	}

	return bRes;
}
