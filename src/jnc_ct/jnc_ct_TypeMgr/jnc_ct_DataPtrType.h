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

#include "jnc_ct_Type.h"

namespace jnc {
namespace ct {

//..............................................................................

class DataPtrType: public Type
{
	friend class TypeMgr;

protected:
	DataPtrTypeKind m_ptrTypeKind;
	Type* m_targetType;

public:
	DataPtrType();

	DataPtrTypeKind
	getPtrTypeKind()
	{
		return m_ptrTypeKind;
	}

	Type*
	getTargetType()
	{
		return m_targetType;
	}

	DataPtrType*
	getCheckedPtrType()
	{
		return !(m_flags & PtrTypeFlag_Safe) ?
			m_targetType->getDataPtrType(m_typeKind, m_ptrTypeKind, m_flags | PtrTypeFlag_Safe) :
			this;
	}

	DataPtrType*
	getUnCheckedPtrType()
	{
		return (m_flags & PtrTypeFlag_Safe) ?
			m_targetType->getDataPtrType(m_typeKind, m_ptrTypeKind, m_flags & ~PtrTypeFlag_Safe) :
			this;
	}

	DataPtrType*
	getUnConstPtrType()
	{
		return (m_flags & PtrTypeFlag_Const) ?
			m_targetType->getDataPtrType(m_typeKind, m_ptrTypeKind, m_flags & ~PtrTypeFlag_Const) :
			this;
	}

	static
	sl::String
	createSignature(
		Type* targetType,
		TypeKind typeKind,
		DataPtrTypeKind ptrTypeKind,
		uint_t flags
		);

	virtual
	void
	markGcRoots(
		const void* p,
		rt::GcHeap* gcHeap
		);

protected:
	virtual
	void
	prepareSignature()
	{
		m_signature = createSignature(m_targetType, m_typeKind, m_ptrTypeKind, m_flags);
	}

	virtual
	void
	prepareTypeString();

	virtual
	void
	prepareDoxyLinkedText();

	virtual
	void
	prepareLlvmType();

	virtual
	void
	prepareLlvmDiType();

	virtual
	Type*
	calcFoldedDualType(
		bool isAlien,
		bool isContainerConst
		);

	sl::String
	getPointerStringSuffix();
};

//..............................................................................

struct DataPtrTypeTuple: sl::ListLink
{
	DataPtrType* m_ptrTypeArray[2][3][3][2][2]; // ref x kind x const x volatile x safe
};

//..............................................................................

} // namespace ct
} // namespace jnc
