//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

#pragma once

#include "jnc_ct_Value.h"

namespace jnc {
namespace ct {

//..............................................................................

class Attribute:
	public ModuleItem,
	public ModuleItemDecl,
	public ModuleItemInitializer
{
	friend class AttributeBlock;

protected:
	Value m_value;

public:
	Value
	getValue()
	{
		return m_value;
	}

protected:
	virtual
	bool
	calcLayout();
};

//..............................................................................

class AttributeBlock:
	public ModuleItem,
	public ModuleItemDecl
{
	friend class AttributeMgr;

protected:
	ModuleItem* m_parentItem;

	sl::List<Attribute> m_attributeList;
	sl::Array<Attribute*> m_attributeArray;
	sl::StringHashTable<Attribute*> m_attributeMap;

public:
	AttributeBlock()
	{
		m_parentItem = NULL;
	}

	ModuleItem*
	getParentItem()
	{
		return m_parentItem;
	}

	sl::Array<Attribute*>
	getAttributeArray()
	{
		return m_attributeArray;
	}

	Attribute*
	findAttribute(const sl::StringRef& name)
	{
		sl::StringHashTableIterator<Attribute*> it = m_attributeMap.find(name);
		return it ? it->m_value : NULL;
	}

	Attribute*
	createAttribute(
		const sl::StringRef& name,
		sl::BoxList<Token>* initializer = NULL
		);
};

//..............................................................................

} // namespace ct
} // namespace jnc
