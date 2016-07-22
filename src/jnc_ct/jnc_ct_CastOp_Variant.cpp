#include "pch.h"
#include "jnc_ct_CastOp_Variant.h"
#include "jnc_Variant.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//.............................................................................

bool
Cast_Variant::constCast (
	const Value& opValue,
	Type* type,
	void* dst
	)
{
	ASSERT (type->getTypeKind () == TypeKind_Variant);

	Variant* variant = (Variant*) dst;
	memset (variant, 0, sizeof (Variant));

	Type* opType = opValue.getType ();
	if (opType->getSize () <= sizeof (DataPtr))
	{
		memcpy (variant, opValue.getConstData (), opType->getSize ());
	}
	else // store it as reference
	{
		const void* p = m_module->m_constMgr.saveValue (opValue).getConstData ();

		variant->m_dataPtr.m_p = (void*) p;
		variant->m_dataPtr.m_validator = m_module->m_constMgr.createConstDataPtrValidator (p, opType);
		
		opType = opType->getDataPtrType (
			TypeKind_DataRef, 
			DataPtrTypeKind_Normal, 
			PtrTypeFlag_Const | PtrTypeFlag_Safe
			);
	}

	variant->m_type = opType;
	return true;
}

bool
Cast_Variant::llvmCast (
	const Value& rawOpValue,
	Type* type,
	Value* resultValue
	)
{
	ASSERT (type->getTypeKind () == TypeKind_Variant);

	bool result;
		
	Value opValue;

	Type* opType = rawOpValue.getType ();
	if ((opType->getTypeKindFlags () & TypeKindFlag_DataPtr) &&
		((DataPtrType*) opType)->getPtrTypeKind () == DataPtrTypeKind_Lean)
	{
		opType = ((DataPtrType*) opType)->getTargetType ()->getDataPtrType (
			opType->getTypeKind (), 
			DataPtrTypeKind_Normal, 
			opType->getFlags ()
			);

		m_module->m_operatorMgr.castOperator (rawOpValue, opType, &opValue);
	}
	else if (opType->getSize () <= sizeof (DataPtr))
	{
		opValue = rawOpValue;
	}
	else // store it as reference
	{		
		result = m_module->m_operatorMgr.gcHeapAllocate (opType, &opValue);
		if (!result)
			return false;

		Value ptrValue;
		m_module->m_llvmIrBuilder.createExtractValue (opValue, 0, NULL, &ptrValue);
		m_module->m_llvmIrBuilder.createBitCast (ptrValue, opType->getDataPtrType_c (), &ptrValue);
		m_module->m_llvmIrBuilder.createStore (rawOpValue, ptrValue);

		opType = opType->getDataPtrType (
			TypeKind_DataRef, 
			DataPtrTypeKind_Normal, 
			PtrTypeFlag_Const | PtrTypeFlag_Safe
			);
	}
	
	Value opTypeValue (&opType, m_module->m_typeMgr.getStdType (StdType_BytePtr));
	Value variantValue;
	Value castValue;

	m_module->m_llvmIrBuilder.createAlloca (type, "tmpVariant", NULL, &variantValue);
	m_module->m_llvmIrBuilder.createBitCast (variantValue, opType->getDataPtrType_c (), &castValue);
	m_module->m_llvmIrBuilder.createStore (opValue, castValue);
	m_module->m_llvmIrBuilder.createLoad (variantValue, NULL, &variantValue);
	m_module->m_llvmIrBuilder.createInsertValue (variantValue, opTypeValue, VariantField_Type, type, resultValue);
	return true;
}

//.............................................................................

bool
Cast_FromVariant::constCast (
	const Value& opValue,
	Type* type,
	void* dst
	)
{
	ASSERT (opValue.getType ()->getTypeKind () == TypeKind_Variant);
	Variant* variant = (Variant*) opValue.getConstData ();
	
	Value tmpValue; 
	if (!variant->m_type)
	{
		memset (dst, 0, type->getSize ());
		return true;
	}

	if (variant->m_type->getSize () > sizeof (DataPtr))
	{
		err::setFormatStringError ("invalid variant type '%s'", variant->m_type->getTypeString ().cc ());
		return false;
	}

	tmpValue.createConst (variant, variant->m_type);

	bool result = m_module->m_operatorMgr.castOperator (&tmpValue, type);
	if (!result)
		return false;

	ASSERT (tmpValue.getValueKind () == ValueKind_Const);
	memcpy (dst, tmpValue.getConstData (), type->getSize ());
	return true;
}

bool
Cast_FromVariant::llvmCast (
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	ASSERT (opValue.getType ()->getTypeKind () == TypeKind_Variant);

	Value typeValue (&type, m_module->m_typeMgr.getStdType (StdType_BytePtr));
	Value tmpValue;
	
	m_module->m_llvmIrBuilder.createAlloca (type, "tmpValue", type->getDataPtrType_c (), &tmpValue);

	Function* function = m_module->m_functionMgr.getStdFunction (StdFunc_DynamicCastVariant);
	bool result = m_module->m_operatorMgr.callOperator (function, opValue, typeValue, tmpValue);
	if (!result)
		return false;

	m_module->m_llvmIrBuilder.createLoad (tmpValue, type, resultValue);
	return true;
}

//.............................................................................

} // namespace ct
} // namespace jnc
