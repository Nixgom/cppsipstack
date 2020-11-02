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

#include "SipParserDefine.h"
#include "SipParameterList.h"
#include "SipUtility.h"
#include "MemoryDebug.h"

CSipParameterList::CSipParameterList()
{
}

CSipParameterList::~CSipParameterList()
{
	ClearParam();
}

/**
 * @ingroup SipParser
 * @brief SIP Header �� , �� ���еǾ N �� ����Ǵ� SIP Header �� parameter list �� �Ľ��Ѵ�.
 * @param pszText		parameter ����Ʈ ���ڿ�
 * @param iTextLen	parameter ����Ʈ ���ڿ��� ����
 * @returns �����ϸ� �Ľ��� ���ڿ��� ���̸� �����ϰ� �׷��� ������ -1 �� �����Ѵ�.
 */
int CSipParameterList::HeaderListParamParse( const char * pszText, int iTextLen )
{
	int iCurPos = 0, iPos;

	while( iCurPos < iTextLen )
	{
		if( pszText[iCurPos] == ' ' || pszText[iCurPos] == '\t' || pszText[iCurPos] == ';' )
		{
			++iCurPos;
			continue;
		}
		else if( pszText[iCurPos] == ',' )
		{
			break;
		}

		iPos = ParamParse( pszText + iCurPos, iTextLen - iCurPos );
		if( iPos == -1 ) return -1;
		iCurPos += iPos;
	}

	return iCurPos;
}

/**
 * @ingroup SipParser
 * @brief parameter ����Ʈ ���ڿ��� �Ľ��Ͽ��� parameter ����Ʈ ��ü�� �����Ѵ�.
 * @param pszText		parameter ����Ʈ ���ڿ�
 * @param iTextLen	parameter ����Ʈ ���ڿ��� ����
 * @returns �����ϸ� �Ľ��� ���ڿ��� ���̸� �����ϰ� �׷��� ������ -1 �� �����Ѵ�.
 */
int CSipParameterList::ParamParse( const char * pszText, int iTextLen )
{
	CSipParameter * pclsParam = new CSipParameter();
	if( pclsParam == NULL ) return -1;

	int iPos = pclsParam->Parse( pszText, iTextLen );
	if( iPos == -1 ) 
	{
		delete pclsParam;
		return -1;
	}

	m_clsParamList.push_back( pclsParam );

	return iPos;
}

/**
 * @ingroup SipParser
 * @brief parameter ����Ʈ ��ü�� parameter ����Ʈ ���ڿ��� �����Ѵ�.
 * @param pszText		parameter ����Ʈ ���ڿ��� ������ ����
 * @param iTextSize parameter ����Ʈ ���ڿ��� ũ��
 * @param cBeginSep ���� ���� ����
 * @param cSep			parameter ���� ���� ����
 * @returns parameter ����Ʈ ���ڿ��� ���̸� �����Ѵ�.
 */
int CSipParameterList::ParamToString( char * pszText, int iTextSize, char cBeginSep, char cSep )
{
	SIP_PARAMETER_LIST::iterator	itList;
	int iLen = 0, iPos;

	for( itList = m_clsParamList.begin(); itList != m_clsParamList.end(); ++itList )
	{
		if( iLen >= iTextSize ) return -1;

		if( itList == m_clsParamList.begin() )
		{
			pszText[iLen++] = cBeginSep;
		}
		else
		{
			pszText[iLen++] = cSep;
		}

		iPos = (*itList)->ToString( pszText + iLen, iTextSize - iLen );
		if( iPos == -1 ) return -1;
		iLen += iPos;
	}

	return iLen;
}

/**
 * @ingroup SipParser
 * @brief parameter list ���� �Էµ� �̸��� ���� �����Ѵ�.
 * @param pszName		parameter �̸�
 * @param pszValue	parameter ��
 * @returns �����ϸ� true �� �����ϰ� �����ϸ� false �� �����Ѵ�.
 */
bool CSipParameterList::InsertParam( const char * pszName, const char * pszValue )
{
	if( pszName == NULL ) return false;

	CSipParameter * pclsParam = new CSipParameter();
	if( pclsParam == NULL ) return false;

	pclsParam->m_strName = pszName;
	if( pszValue ) pclsParam->m_strValue = pszValue;

	m_clsParamList.push_back( pclsParam );

	return true;
}

/**
 * @ingroup SipParser
 * @brief parameter list ���� �Էµ� �̸��� ���� ���� �����Ѵ�.
 * @param pszName		parameter �̸�
 * @param pszValue	parameter ��
 * @returns parameter �̸��� �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CSipParameterList::UpdateParam( const char * pszName, const char * pszValue )
{
	SIP_PARAMETER_LIST::iterator	itList;

	for( itList = m_clsParamList.begin(); itList != m_clsParamList.end(); ++itList )
	{
		if( !strcasecmp( (*itList)->m_strName.c_str(), pszName ) )
		{
			(*itList)->m_strValue = pszValue;
			return true;
		}
	}

	return false;
}

/**
 * @ingroup SipParser
 * @brief parameter list ���� �Էµ� �̸��� �˻��Ѵ�.
 * @param pszName		parameter �̸�
 * @param strValue	parameter ���� ������ ����
 * @returns parameter �̸��� �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CSipParameterList::SelectParam( const char * pszName, std::string & strValue )
{
	SIP_PARAMETER_LIST::iterator	itList;

	for( itList = m_clsParamList.begin(); itList != m_clsParamList.end(); ++itList )
	{
		if( !strcasecmp( (*itList)->m_strName.c_str(), pszName ) )
		{
			strValue = (*itList)->m_strValue;
			return true;
		}
	}

	return false;
}

/**
 * @ingroup SipParser
 * @brief parameter list ���� �Էµ� �̸��� �˻��Ѵ�.
 * @param pszName parameter �̸�
 * @returns parameter �̸��� �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CSipParameterList::SelectParam( const char * pszName )
{
	SIP_PARAMETER_LIST::iterator	itList;

	for( itList = m_clsParamList.begin(); itList != m_clsParamList.end(); ++itList )
	{
		if( !strcasecmp( (*itList)->m_strName.c_str(), pszName ) )
		{
			return true;
		}
	}

	return false;
}

/**
 * @ingroup SipParser
 * @brief parameter list ���� �Էµ� �̸��� �˻��Ѵ�.
 * @param pszName parameter �̸�
 * @returns parameter �̸��� �����ϸ� �ش� ���� �����ϰ� �׷��� ������ NULL �� �����Ѵ�.
 */
const char * CSipParameterList::SelectParamValue( const char * pszName )
{
	SIP_PARAMETER_LIST::iterator	itList;

	for( itList = m_clsParamList.begin(); itList != m_clsParamList.end(); ++itList )
	{
		if( !strcasecmp( (*itList)->m_strName.c_str(), pszName ) )
		{
			return (*itList)->m_strValue.c_str();
		}
	}

	return NULL;
}

/**
 * @ingroup SipParser
 * @brief parameter list �� �����Ѵ�.
 */
void CSipParameterList::ClearParam()
{
	ClearList<SIP_PARAMETER_LIST>( m_clsParamList );
}

CSipParameterList & CSipParameterList::operator=( CSipParameterList & clsOld )
{
	SIP_PARAMETER_LIST::iterator	itList;

	for( itList = clsOld.m_clsParamList.begin(); itList != clsOld.m_clsParamList.end(); ++itList )
	{
		CSipParameter * pclsParam = new CSipParameter();
		if( pclsParam )
		{
			*pclsParam = **itList;
			m_clsParamList.push_back( pclsParam );
		}
	}

	return *this;
}