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
#include "jnc_ct_StructType.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_ArrayType.h"

namespace jnc {
namespace ct {

//..............................................................................

StructField::StructField()
{
	m_itemKind = ModuleItemKind_StructField;
	m_type = NULL;
	m_ptrTypeFlags = 0;
	m_bitFieldBaseType = NULL;
	m_bitCount = 0;
	m_offset = 0;
	m_llvmIndex = -1;
	m_prevDynamicFieldIndex = -1;
}

bool
StructField::generateDocumentation(
	const sl::StringRef& outputDir,
	sl::String* itemXml,
	sl::String* indexXml
	)
{
	dox::Block* doxyBlock = m_module->m_doxyHost.getItemBlock(this);

	bool isMulticast = isClassType(m_type, ClassTypeKind_Multicast);
	const char* kind = isMulticast ? "event" : "variable";

	itemXml->format("<memberdef kind='%s' id='%s'", kind, doxyBlock->getRefId ().sz());

	if (m_accessKind != AccessKind_Public)
		itemXml->appendFormat(" prot='%s'", getAccessKindString(m_accessKind));

	if (m_storageKind == StorageKind_Static)
		itemXml->append(" static='yes'");
	else if (m_storageKind == StorageKind_Tls)
		itemXml->append(" tls='yes'");

	if (m_ptrTypeFlags & PtrTypeFlag_Const)
		itemXml->append(" const='yes'");
	else if (m_ptrTypeFlags & PtrTypeFlag_ReadOnly)
		itemXml->append(" readonly='yes'");

	itemXml->appendFormat(">\n<name>%s</name>\n", m_name.sz());
	itemXml->append(m_type->getDoxyTypeString());

	sl::String ptrTypeFlagString = getPtrTypeFlagString(m_ptrTypeFlags & ~PtrTypeFlag_DualEvent);
	if (!ptrTypeFlagString.isEmpty())
		itemXml->appendFormat("<modifiers>%s</modifiers>\n", ptrTypeFlagString.sz());

	if (!m_initializer.isEmpty())
		itemXml->appendFormat("<initializer>= %s</initializer>\n", getInitializerString().sz());

	itemXml->append(doxyBlock->getDescriptionString());
	itemXml->append(getDoxyLocationString());
	itemXml->append("</memberdef>\n");

	return true;
}

//..............................................................................

StructType::StructType()
{
	m_typeKind = TypeKind_Struct;
	m_structTypeKind = StructTypeKind_Normal;
	m_flags = TypeFlag_Pod | TypeFlag_StructRet;
	m_fieldAlignment = 8;
	m_fieldActualSize = 0;
	m_fieldAlignedSize = 0;
	m_lastBitFieldType = NULL;
	m_lastBitFieldOffset = 0;
}

void
StructType::prepareLlvmType()
{
	m_llvmType = llvm::StructType::create(*m_module->getLlvmContext(), getQualifiedName().sz());
}

StructField*
StructType::createFieldImpl(
	const sl::StringRef& name,
	Type* type,
	size_t bitCount,
	uint_t ptrTypeFlags,
	sl::BoxList<Token>* constructor,
	sl::BoxList<Token>* initializer
	)
{
	if (m_flags & ModuleItemFlag_Sealed)
	{
		err::setFormatStringError("'%s' is completed, cannot add fields to it", getTypeString().sz());
		return NULL;
	}

	StructField* field = m_module->m_typeMgr.createStructField(
		name,
		type,
		bitCount,
		ptrTypeFlags,
		constructor,
		initializer
		);

	field->m_parentNamespace = this;

	if (!field->m_constructor.isEmpty() ||
		!field->m_initializer.isEmpty())
	{
		m_initializedMemberFieldArray.append(field);
	}

	if (name.isEmpty())
	{
		m_unnamedFieldArray.append(field);
	}
	else if (name[0] != '!') // internal field
	{
		bool result = addItem(field);
		if (!result)
			return NULL;
	}

	m_memberFieldArray.append(field);
	return field;
}

bool
StructType::append(StructType* type)
{
	bool result;

	sl::Iterator<BaseTypeSlot> slot = type->m_baseTypeList.getHead();
	for (; slot; slot++)
	{
		result = addBaseType(slot->m_type) != NULL;
		if (!result)
			return false;
	}

	sl::Array<StructField*> fieldArray = type->getMemberFieldArray();
	size_t count = fieldArray.getCount();
	for (size_t i = 0; i < count; i++)
	{
		StructField* field = fieldArray[i];
		result = field->m_bitCount ?
			createField(field->m_name, field->m_bitFieldBaseType, field->m_bitCount, field->m_ptrTypeFlags) != NULL:
			createField(field->m_name, field->m_type, 0, field->m_ptrTypeFlags) != NULL;

		if (!result)
			return false;
	}

	return true;
}

bool
StructType::calcLayout()
{
	bool result;

	sl::Iterator<BaseTypeSlot> slotIt = m_baseTypeList.getHead();
	for (; slotIt; slotIt++)
	{
		BaseTypeSlot* slot = *slotIt;
		if (!(slot->m_type->getTypeKindFlags() & TypeKindFlag_Derivable) ||
			(slot->m_type->getFlags() & TypeFlag_Dynamic) ||
			slot->m_type->getTypeKind() == TypeKind_Class)
		{
			err::setFormatStringError("'%s' cannot be a base type of a struct", slot->m_type->getTypeString().sz());
			return false;
		}

		sl::StringHashTableIterator<BaseTypeSlot*> it = m_baseTypeMap.visit(slot->m_type->getSignature());
		if (it->m_value)
		{
			err::setFormatStringError(
				"'%s' is already a base type",
				slot->m_type->getTypeString().sz()
				);
			return false;
		}

		it->m_value = slot;

		result = slot->m_type->ensureLayout();
		if (!result)
			return false;

		if (slot->m_type->getFlags() & TypeFlag_GcRoot)
		{
			m_gcRootBaseTypeArray.append(slot);
			m_flags |= TypeFlag_GcRoot;
		}

		if (slot->m_type->getConstructor())
			m_baseTypeConstructArray.append(slot);

		result = layoutField(
			slot->m_type,
			&slot->m_offset,
			&slot->m_llvmIndex
			);

		if (!result)
			return false;
	}

	size_t count = m_memberFieldArray.getCount();
	for (size_t i = 0; i < count; i++)
	{
		StructField* field = m_memberFieldArray[i];
		result = layoutField(field);
		if (!result)
			return false;
	}

	if ((m_flags & TypeFlag_Dynamic) && m_dynamicFieldArray.isEmpty())
	{
		err::setFormatStringError("dynamic struct '%s' has no dynamic fields", getQualifiedName().sz());
		return false;
	}

	if (m_fieldAlignedSize > m_fieldActualSize)
		insertPadding(m_fieldAlignedSize - m_fieldActualSize);

	// scan members for gcroots and constructors (not for auxilary structs such as class iface)

	if (m_structTypeKind == StructTypeKind_Normal)
	{
		size_t count = m_memberFieldArray.getCount();
		for (size_t i = 0; i < count; i++)
		{
			StructField* field = m_memberFieldArray[i];
			Type* type = field->getType();

			uint_t fieldTypeFlags = type->getFlags();

			if (!(fieldTypeFlags & TypeFlag_Pod))
				m_flags &= ~TypeFlag_Pod;

			if (fieldTypeFlags & TypeFlag_GcRoot)
			{
				m_gcRootMemberFieldArray.append(field);
				m_flags |= TypeFlag_GcRoot;
			}

			if ((type->getTypeKindFlags() & TypeKindFlag_Derivable) && ((DerivableType*)type)->getConstructor())
				m_memberFieldConstructArray.append(field);
		}

		count = m_memberPropertyArray.getCount();
		for (size_t i = 0; i < count; i++)
		{
			Property* prop = m_memberPropertyArray[i];
			result = prop->ensureLayout();
			if (!result)
				return false;

			if (prop->getConstructor())
				m_memberPropertyConstructArray.append(prop);
		}
	}

	if (m_structTypeKind == StructTypeKind_Normal)
	{
		if (!m_staticConstructor && !m_initializedStaticFieldArray.isEmpty())
		{
			result = createDefaultMethod(FunctionKind_StaticConstructor, StorageKind_Static) != NULL;
			if (!result)
				return false;
		}

		if (!m_constructor &&
			(m_preconstructor ||
			!m_baseTypeConstructArray.isEmpty() ||
			!m_memberFieldConstructArray.isEmpty() ||
			!m_initializedMemberFieldArray.isEmpty() ||
			!m_memberPropertyConstructArray.isEmpty()))
		{
			result = createDefaultMethod(FunctionKind_Constructor) != NULL;
			if (!result)
				return false;
		}
	}
	else if (
		m_structTypeKind == StructTypeKind_IfaceStruct &&
		(((ClassType*)m_parentNamespace)->getFlags() & ClassTypeFlag_Opaque) &&
		!(m_module->getCompileFlags() & ModuleCompileFlag_IgnoreOpaqueClassTypeInfo)
		)
	{
		ClassType* classType = (ClassType*)m_parentNamespace;

		const OpaqueClassTypeInfo* typeInfo = m_module->m_extensionLibMgr.findOpaqueClassTypeInfo(classType->getQualifiedName());
		if (!typeInfo)
		{
			err::setFormatStringError("opaque class type info is missing for '%s'", classType->getTypeString().sz());
			return false;
		}

		if (typeInfo->m_size < m_fieldAlignedSize)
		{
			err::setFormatStringError(
				"invalid opaque class type size for '%s' (specified %d bytes; must be at least %d bytes)",
				getTypeString().sz(),
				typeInfo->m_size,
				m_fieldAlignedSize
				);

			return false;
		}

		if (typeInfo->m_size > m_fieldAlignedSize)
		{
			ArrayType* opaqueDataType = m_module->m_typeMgr.getArrayType(
				m_module->m_typeMgr.getPrimitiveType(TypeKind_Char),
				typeInfo->m_size - m_fieldAlignedSize
				);

			StructField* field = createField(opaqueDataType);
			result = layoutField(field);
			ASSERT(result);
		}

		if (typeInfo->m_markOpaqueGcRootsFunc)
		{
			classType->m_markOpaqueGcRootsFunc = typeInfo->m_markOpaqueGcRootsFunc;
			classType->m_flags |= TypeFlag_GcRoot;
		}

		if (typeInfo->m_isNonCreatable)
			classType->m_flags |= ClassTypeFlag_OpaqueNonCreatable;
	}

	if (m_flags & TypeFlag_Dynamic)
	{
		m_size = 0;
		m_flags |= TypeFlag_NoStack;

		m_module->m_typeMgr.getStdType(StdType_DynamicLayout); // ensure jnc.DynamicLayout is present
	}
	else
	{
		llvm::StructType* llvmStructType = (llvm::StructType*)getLlvmType();
		llvmStructType->setBody(
			llvm::ArrayRef<llvm::Type*> (m_llvmFieldTypeArray, m_llvmFieldTypeArray.getCount()),
			true
			);

		m_size = m_fieldAlignedSize;
		if (m_size > TypeSizeLimit_StackAllocSize)
			m_flags |= TypeFlag_NoStack;
	}

	return true;
}

bool
StructType::compile()
{
	bool result;

	if (m_staticConstructor && !(m_staticConstructor->getFlags() & ModuleItemFlag_User))
	{
		result = compileDefaultStaticConstructor();
		if (!result)
			return false;
	}

	if (m_constructor && !(m_constructor->getFlags() & ModuleItemFlag_User))
	{
		result = compileDefaultConstructor();
		if (!result)
			return false;
	}

	return true;
}

bool
StructType::layoutField(StructField* field)
{
	bool result;

	if (m_structTypeKind != StructTypeKind_IfaceStruct && field->m_type->getTypeKind() == TypeKind_Class)
	{
		err::setFormatStringError("class '%s' cannot be a struct member", field->m_type->getTypeString().sz());
		field->pushSrcPosError();
		return false;
	}

	if ((m_flags & TypeFlag_Dynamic) && field->m_type->getTypeKind() == TypeKind_Array)
	{
		result = ((ArrayType*)field->m_type)->ensureDynamicLayout(this, field);
		if (!result)
			return false;
	}
	else
	{
		result = field->m_type->ensureLayout();
		if (!result)
			return false;
	}

	result = field->m_bitCount ?
		layoutBitField(
			field->m_bitFieldBaseType,
			field->m_bitCount,
			&field->m_type,
			&field->m_offset,
			&field->m_llvmIndex
			) :
		layoutField(
			field->m_type,
			&field->m_offset,
			&field->m_llvmIndex
			);

	if (m_flags & TypeFlag_Dynamic)
	{
		field->m_prevDynamicFieldIndex = m_dynamicFieldArray.getCount() - 1;

		if (field->m_type->getFlags() & TypeFlag_Dynamic)
		{
			m_dynamicFieldArray.append(field);

			// reset sizes on each dynamic field

			m_fieldAlignedSize = 0;
			m_fieldActualSize = 0;
		}
	}
	else if (field->m_type->getFlags() & TypeFlag_Dynamic)
	{
		err::setFormatStringError("dynamic '%s' cannot be a struct member", field->m_type->getTypeString().sz());
		field->pushSrcPosError();
		return false;
	}

	return true;
}

bool
StructType::layoutField(
	llvm::Type* llvmType,
	size_t size,
	size_t alignment,
	size_t* offset_o,
	uint_t* llvmIndex
	)
{
	if (alignment > m_alignment)
		m_alignment = AXL_MIN(alignment, m_fieldAlignment);

	size_t offset = getFieldOffset(alignment);
	if (offset > m_fieldActualSize)
		insertPadding(offset - m_fieldActualSize);

	*offset_o = offset;

	if (!(m_flags & TypeFlag_Dynamic))
	{
		*llvmIndex = (uint_t)m_llvmFieldTypeArray.getCount();
		m_llvmFieldTypeArray.append(llvmType);
	}

	m_lastBitFieldType = NULL;
	m_lastBitFieldOffset = 0;

	setFieldActualSize(offset + size);
	return true;
}

bool
StructType::layoutBitField(
	Type* baseType,
	size_t bitCount,
	Type** type_o,
	size_t* offset_o,
	uint_t* llvmIndex
	)
{
	size_t baseBitCount = baseType->getSize() * 8;
	if (bitCount > baseBitCount)
	{
		err::setFormatStringError("type of bit field too small for number of bits");
		return false;
	}

	bool isMerged = m_lastBitFieldType && m_lastBitFieldType->getBaseType()->cmp(baseType) == 0;

	size_t bitOffset;
	if (baseType->getTypeKindFlags() & TypeKindFlag_BigEndian)
	{
		if (!isMerged)
		{
			bitOffset = baseBitCount - bitCount;
		}
		else
		{
			size_t lastBitOffset = m_lastBitFieldType->getBitOffset();
			isMerged = lastBitOffset >= bitCount;
			bitOffset = isMerged ? lastBitOffset - bitCount : baseBitCount - bitCount;
		}
	}
	else
	{
		if (!isMerged)
		{
			bitOffset = 0;
		}
		else
		{
			size_t lastBitOffset = m_lastBitFieldType->getBitOffset() + m_lastBitFieldType->getBitCount();
			isMerged = lastBitOffset + bitCount <= baseBitCount;
			bitOffset = isMerged ? lastBitOffset : 0;
		}
	}

	BitFieldType* type = m_module->m_typeMgr.getBitFieldType(baseType, bitOffset, bitCount);
	if (!type)
		return false;

	*type_o = type;
	m_lastBitFieldType = type;

	if (isMerged)
	{
		*offset_o = m_lastBitFieldOffset;
		*llvmIndex = (uint_t)m_llvmFieldTypeArray.getCount() - 1;
		return true;
	}

	size_t alignment = type->getAlignment();
	if (alignment > m_alignment)
		m_alignment = AXL_MIN(alignment, m_fieldAlignment);

	size_t offset = getFieldOffset(alignment);
	m_lastBitFieldOffset = offset;

	if (offset > m_fieldActualSize)
		insertPadding(offset - m_fieldActualSize);

	*offset_o = offset;

	if (!(m_flags & TypeFlag_Dynamic))
	{
		*llvmIndex = (uint_t)m_llvmFieldTypeArray.getCount();
		m_llvmFieldTypeArray.append(type->getLlvmType());
	}

	setFieldActualSize(offset + type->getSize());
	return true;
}

size_t
StructType::getFieldOffset(size_t alignment)
{
	size_t offset = m_fieldActualSize;

	if (alignment > m_fieldAlignment)
		alignment = m_fieldAlignment;

	size_t mod = offset % alignment;
	if (mod)
		offset += alignment - mod;

	return offset;
}

size_t
StructType::setFieldActualSize(size_t size)
{
	if (m_fieldActualSize >= size)
		return m_fieldAlignedSize;

	m_fieldActualSize = size;
	m_fieldAlignedSize = size;

	size_t mod = size % m_alignment;
	if (mod)
		m_fieldAlignedSize += m_alignment - mod;

	return m_fieldAlignedSize;
}

ArrayType*
StructType::insertPadding(size_t size)
{
	ArrayType* type = m_module->m_typeMgr.getArrayType(m_module->m_typeMgr.getPrimitiveType(TypeKind_Char), size);
	m_llvmFieldTypeArray.append(type->getLlvmType());
	return type;
}

void
StructType::prepareLlvmDiType()
{
	m_llvmDiType = m_module->m_llvmDiBuilder.createEmptyStructType(this);
	m_module->m_llvmDiBuilder.setStructTypeBody(this);
}

void
StructType::markGcRoots(
	const void* _p,
	rt::GcHeap* gcHeap
	)
{
	char* p = (char*)_p;

	size_t count = m_gcRootBaseTypeArray.getCount();
	for (size_t i = 0; i < count; i++)
	{
		BaseTypeSlot* slot = m_gcRootBaseTypeArray[i];
		slot->getType()->markGcRoots(p + slot->getOffset(), gcHeap);
	}

	count = m_gcRootMemberFieldArray.getCount();
	for (size_t i = 0; i < count; i++)
	{
		StructField* field = m_gcRootMemberFieldArray[i];
		field->getType()->markGcRoots(p + field->getOffset(), gcHeap);
	}
}

//..............................................................................

} // namespace ct
} // namespace jnc
