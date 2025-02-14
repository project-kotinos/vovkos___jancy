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

#include "pch.h"
#include "jnc_ct_BinOp_Logic.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

Type*
BinOp_LogAnd::getResultType(
	const Value& opValue1,
	const Value& opValue2
	)
{
	return m_module->m_typeMgr.getPrimitiveType(TypeKind_Bool);
}

//..............................................................................

Type*
BinOp_LogOr::getResultType(
	const Value& opValue1,
	const Value& opValue2
	)
{
	return m_module->m_typeMgr.getPrimitiveType(TypeKind_Bool);
}

//..............................................................................

} // namespace ct
} // namespace jnc
