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
#include "jnc_ct_Property.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

Property::Property():
	NamedTypeBlock(this)
{
	m_itemKind = ModuleItemKind_Property;
	m_namespaceKind = NamespaceKind_Property;
	m_propertyKind = PropertyKind_Normal;
	m_type = NULL;

	m_getter = NULL;
	m_setter = NULL;
	m_binder = NULL;

	m_autoGetValue = NULL;
	m_onChanged = NULL;

	m_parentType = NULL;
	m_parentClassVtableIndex = -1;

	m_extensionNamespace = NULL;
	m_vtableVariable = NULL;
}

bool
Property::compile()
{
	bool result;

	if (m_flags & PropertyFlag_AutoGet)
	{
		result = compileAutoGetter();
		if (!result)
			return false;
	}

	if (m_flags & PropertyFlag_AutoSet)
	{
		result = compileAutoSetter();
		if (!result)
			return false;
	}

	if (m_type->getFlags() & PropertyTypeFlag_Bindable)
	{
		result = compileBinder();
		if (!result)
			return false;
	}

	return true;
}

bool
Property::create(PropertyType* type)
{
	bool result;

	StorageKind storageKind = m_storageKind == StorageKind_Abstract ? StorageKind_Virtual : m_storageKind;

	uint_t getterFlags = 0;
	uint_t setterFlags = 0;

	if (m_flags & ModuleItemFlag_User)
	{
		if (!(m_flags & PropertyFlag_AutoGet))
			getterFlags |= ModuleItemFlag_User;

		if (!(m_flags & PropertyFlag_AutoSet))
			setterFlags |= ModuleItemFlag_User;
	}

	if (type->getFlags() & PropertyTypeFlag_Bindable)
	{
		result = createOnChanged();
		if (!result)
			return false;
	}

	FunctionType* getterType = type->getGetterType();

	if (m_flags & PropertyFlag_AutoGet)
	{
		result = createAutoGetValue(getterType->getReturnType());
		if (!result)
			return false;
	}
	else
	{
		Function* getter = m_module->m_functionMgr.createFunction(FunctionKind_Getter, getterType);
		getter->m_storageKind = storageKind;
		getter->m_flags |= getterFlags;

		if (m_parentType)
			getter->m_thisArgTypeFlags = PtrTypeFlag_Const;

		result = addMethod(getter);
		if (!result)
			return false;
	}

	size_t setterTypeOverloadCount = type->getSetterType()->getOverloadCount();
	for (size_t i = 0; i < setterTypeOverloadCount; i++)
	{
		FunctionType* setterType = type->getSetterType()->getOverload(i);

		Function* setter = m_module->m_functionMgr.createFunction(FunctionKind_Setter, setterType);
		setter->m_storageKind = storageKind;
		setter->m_flags |= setterFlags;

		result = addMethod(setter);
		if (!result)
			return false;
	}

	m_type = m_parentType ? m_parentType->getMemberPropertyType(type) : type;

	if (m_flags & (PropertyFlag_AutoGet | PropertyFlag_AutoSet))
		m_module->markForCompile(this);

	return true;
}

bool
Property::setOnChanged(
	ModuleItem* item,
	bool isForced
	)
{
	if (m_onChanged && !isForced)
	{
		err::setFormatStringError(
			"'%s' already has 'bindable %s'",
			getQualifiedName().sz(),
			m_onChanged->getDecl()->getQualifiedName().sz()
			);

		return false;
	}

	m_onChanged = item;
	m_flags |= PropertyFlag_Bindable;

	if (item->getItemKind() == ModuleItemKind_Alias)
		return true; // will be fixed up later

	Type* type = getModuleItemType(item);
	if (!type)
	{
		err::setFormatStringError("invalid bindable item");
		return false;
	}

	FunctionType* binderType = (FunctionType*)m_module->m_typeMgr.getStdType(StdType_Binder);

	Function* binder = m_module->m_functionMgr.createFunction(FunctionKind_Binder, binderType);
	binder->m_storageKind = m_storageKind == StorageKind_Abstract ? StorageKind_Virtual : m_storageKind;

	if (m_parentType)
		binder->m_thisArgTypeFlags = PtrTypeFlag_Const;

	m_module->markForCompile(this);

	return addMethod(binder);
}

bool
Property::createOnChanged()
{
	sl::String name = "m_onChanged";

	Type* type = m_module->m_typeMgr.getStdType(StdType_SimpleMulticast);

	if (m_parentType)
	{
		StructField* field = createField(name, type);
		return
			field != NULL &&
			setOnChanged(field);
	}
	else
	{
		Variable* variable = m_module->m_variableMgr.createVariable(
			StorageKind_Static,
			name,
			createQualifiedName(name),
			type
			);

		return
			variable != NULL &&
			addItem(variable) &&
			setOnChanged(variable);
	}
}

bool
Property::setAutoGetValue(
	ModuleItem* item,
	bool isForced
	)
{
	if (m_autoGetValue && !isForced)
	{
		err::setFormatStringError(
			"'%s' already has 'autoget %s'",
			getQualifiedName().sz(),
			m_autoGetValue->getDecl()->getQualifiedName().sz()
			);

		return false;
	}

	m_autoGetValue = item;
	m_flags |= PropertyFlag_AutoGet;

	if (item->getItemKind() == ModuleItemKind_Alias)
		return true; // will be fixed up later

	Type* type = getModuleItemType(item);
	if (!type)
	{
		err::setFormatStringError("invalid autoget item");
		return false;
	}

	FunctionType* getterType = m_module->m_typeMgr.getFunctionType(type, NULL, 0, 0);

	if (m_getter)
	{
		if (m_getter->getType()->getReturnType()->cmp(type) != 0)
		{
			err::setFormatStringError("'autoget %s' does not match property declaration", type->getTypeString().sz());
			return false;
		}

		return true;
	}

	Function* getter = m_module->m_functionMgr.createFunction(FunctionKind_Getter, getterType);
	getter->m_storageKind = m_storageKind == StorageKind_Abstract ? StorageKind_Virtual : m_storageKind;

	if (m_parentType)
		getter->m_thisArgTypeFlags = PtrTypeFlag_Const;

	m_module->markForCompile(this);

	return addMethod(getter);
}

bool
Property::createAutoGetValue(Type* type)
{
	sl::String name = "m_value";

	if (m_parentType)
	{
		StructField* field = createField(name, type);
		return
			field != NULL &&
			setAutoGetValue(field);
	}
	else
	{
		Variable* variable = m_module->m_variableMgr.createVariable(
			StorageKind_Static,
			name,
			createQualifiedName(name),
			type
			);

		return
			variable != NULL &&
			addItem(variable) &&
			setAutoGetValue(variable);
	}
}

StructField*
Property::createFieldImpl(
	const sl::StringRef& name,
	Type* type,
	size_t bitCount,
	uint_t ptrTypeFlags,
	sl::BoxList<Token>* constructor,
	sl::BoxList<Token>* initializer
	)
{
	ASSERT(m_parentType);

	if (!(m_parentType->getTypeKindFlags() & TypeKindFlag_Derivable))
	{
		err::setFormatStringError("'%s' cannot have field members", m_parentType->getTypeString().sz());
		return NULL;
	}

	DerivableType* parentType = (DerivableType*)m_parentType;

	bool result;

	// don't add field to parent namespace

	StructField* field = parentType->createField(sl::String(), type, bitCount, ptrTypeFlags, constructor, initializer);
	if (!field)
		return NULL;

	// re-parent

	field->m_parentNamespace = this;
	field->m_name = name;

	if (!name.isEmpty())
	{
		result = addItem(field);
		if (!result)
			return NULL;
	}

	return field;
}

bool
Property::addMethod(Function* function)
{
	bool result;

	StorageKind storageKind = function->getStorageKind();
	FunctionKind functionKind = function->getFunctionKind();
	uint_t functionKindFlags = getFunctionKindFlags(functionKind);
	uint_t thisArgTypeFlags = function->m_thisArgTypeFlags;
	bool hasArgs = !function->getType()->getArgArray().isEmpty();

	if (m_parentType)
	{
		switch (storageKind)
		{
		case StorageKind_Static:
			if (thisArgTypeFlags)
			{
				err::setFormatStringError("static method cannot be '%s'", getPtrTypeFlagString(thisArgTypeFlags).sz());
				return false;
			}

			break;

		case StorageKind_Undefined:
			function->m_storageKind = StorageKind_Member;
			// and fall through

		case StorageKind_Member:
			if (functionKind == FunctionKind_Getter)
				function->m_thisArgTypeFlags |= PtrTypeFlag_Const;

			function->convertToMemberMethod(m_parentType);
			break;

		case StorageKind_Abstract:
		case StorageKind_Virtual:
		case StorageKind_Override:
			if (functionKind == FunctionKind_Getter)
				function->m_thisArgTypeFlags |= PtrTypeFlag_Const;

			if (m_parentType->getTypeKind() != TypeKind_Class)
			{
				err::setFormatStringError("virtual method cannot be added to '%s'", m_parentType->getTypeString().sz());
				return false;
			}

			if (m_parentType->getFlags() & ModuleItemFlag_Sealed)
			{
				err::setFormatStringError("'%s' is completed, cannot add virtual methods to it", m_parentType->getTypeString().sz());
				return false;
			}

			if (!function->isAccessor())
				((ClassType*)m_parentType)->m_virtualMethodArray.append(function); // otherwise we are already on VirtualPropertyArray

			function->convertToMemberMethod(m_parentType);
			break;

		default:
			err::setFormatStringError("invalid storage specifier '%s' for method member", getStorageKindString(storageKind));
			return false;
		}
	}
	else
	{
		switch (storageKind)
		{
		case StorageKind_Undefined:
			function->m_storageKind = StorageKind_Static;
			// and fall through

		case StorageKind_Static:
			break;

		default:
			err::setFormatStringError("invalid storage specifier '%s' for static property member", getStorageKindString(storageKind));
			return false;
		}

		if (thisArgTypeFlags)
		{
			err::setFormatStringError("global property methods cannot be '%s'", getPtrTypeFlagString(thisArgTypeFlags).sz());
			return false;
		}

	}

	function->m_parentNamespace = this;
	function->m_property = this;
	function->m_extensionNamespace = m_extensionNamespace;

	Function** target = NULL;

	switch (functionKind)
	{
	case FunctionKind_Constructor:
		if (hasArgs)
		{
			err::setFormatStringError("property constructor cannot have arguments");
			return false;
		}

		if (storageKind != StorageKind_Static)
		{
			target = &m_constructor;
			break;
		}

		functionKind = FunctionKind_StaticConstructor;
		function->m_functionKind = FunctionKind_StaticConstructor;

		// and fall through

	case FunctionKind_StaticConstructor:
		target = &m_staticConstructor;
		break;

	case FunctionKind_Destructor:
		if (storageKind != StorageKind_Static)
		{
			target = &m_destructor;
			break;
		}

		functionKind = FunctionKind_StaticDestructor;
		function->m_functionKind = FunctionKind_StaticDestructor;

		// and fall through

	case FunctionKind_StaticDestructor:
		target = &m_staticDestructor;
		break;

	case FunctionKind_Getter:
		result = m_verifier.checkGetter(function->getType());
		if (!result)
			return false;

		target = &m_getter;
		break;

	case FunctionKind_Setter:
		if (m_flags & PropertyFlag_Const)
		{
			err::setFormatStringError("const property '%s' cannot have setters", getQualifiedName().sz());
			return false;
		}

		result = m_verifier.checkSetter(function->getType());
		if (!result)
			return false;

		target = &m_setter;
		break;

	case FunctionKind_Binder:
		target = &m_binder;
		break;

	case FunctionKind_Normal:
		return addFunction(function) != -1;

	default:
		err::setFormatStringError(
			"invalid %s in '%s'",
			getFunctionKindString(functionKind),
			getQualifiedName().sz()
			);
		return false;
	}

	function->m_qualifiedName = createQualifiedName(getFunctionKindString(functionKind));
	if (!*target)
	{
		*target = function;
	}
	else
	{
		result = (*target)->addOverload(function) != -1;
		if (!result)
			return false;
	}

	return true;
}

bool
Property::addProperty(Property* prop)
{
	ASSERT(prop->isNamed());
	bool result = addItem(prop);
	if (!result)
		return false;

	prop->m_parentNamespace = this;

	if (!m_parentType)
		return true;

	StorageKind storageKind = prop->getStorageKind();
	switch (storageKind)
	{
	case StorageKind_Static:
		break;

	case StorageKind_Undefined:
		prop->m_storageKind = StorageKind_Member;
		// and fall through

	case StorageKind_Member:
		prop->m_parentType = m_parentType;
		break;

	case StorageKind_Abstract:
	case StorageKind_Virtual:
	case StorageKind_Override:
		if (m_parentType->getTypeKind() != TypeKind_Class)
		{
			err::setFormatStringError(
				"'%s' property cannot be part of '%s'",
				getStorageKindString(storageKind),
				m_parentType->getTypeString().sz()
				);
			return false;
		}

		((ClassType*)m_parentType)->m_virtualPropertyArray.append(prop);
		prop->m_parentType = m_parentType;
		break;

	default:
		err::setFormatStringError("invalid storage specifier '%s' for property member", getStorageKindString(storageKind));
		return false;
	}

	return true;
}

bool
Property::calcLayout()
{
	bool result;

	ASSERT(m_storageKind && m_vtable.isEmpty());

	if (m_autoGetValue && m_autoGetValue->getItemKind() == ModuleItemKind_Alias)
	{
		Alias* alias = (Alias*)m_autoGetValue;
		result = alias->ensureLayout() && setAutoGetValue(alias->getTargetItem(), true);
		if (!result)
			return false;

		ASSERT(m_autoGetValue->getItemKind() != ModuleItemKind_Alias);
	}

	if (m_onChanged && m_onChanged->getItemKind() == ModuleItemKind_Alias)
	{
		Alias* alias = (Alias*)m_onChanged;
		result = alias->ensureLayout() && setOnChanged(alias->getTargetItem(), true);
		if (!result)
			return false;

		ASSERT(m_onChanged->getItemKind() != ModuleItemKind_Alias);
	}

	if (!m_getter)
	{
		err::setFormatStringError("incomplete property: no 'get' method or 'autoget' field");
		return false;
	}

	if (!m_type)
		createType();

	size_t setterCount = m_setter ? m_setter->getOverloadCount() : 0;

	m_vtable.reserve(2 + setterCount);

	if (m_binder)
		m_vtable.append(m_binder);

	m_vtable.append(m_getter);

	for (size_t i = 0; i < setterCount; i++)
		m_vtable.append(m_setter->getOverload(i));

	createVtableVariable();
	return true;
}

PropertyType*
Property::createType()
{
	ASSERT(!m_type);

	uint_t typeFlags = 0;
	if (m_onChanged)
		typeFlags |= PropertyTypeFlag_Bindable;

	m_type = m_setter ?
		m_module->m_typeMgr.getPropertyType(
			m_getter->getType(),
			*m_setter->getTypeOverload(),
			typeFlags
			) :
		m_module->m_typeMgr.getPropertyType(
			m_getter->getType(),
			NULL,
			typeFlags
			);

	return m_type;
}

void
Property::createVtableVariable()
{
	char buffer[256];
	sl::Array<llvm::Constant*> llvmVtable(ref::BufKind_Stack, buffer, sizeof(buffer));

	size_t count = m_vtable.getCount();
	llvmVtable.setCount(count);

	for (size_t i = 0; i < count; i++)
	{
		Function* function = m_vtable[i];

		if (function->getStorageKind() == StorageKind_Abstract)
			function = function->getType()->getAbstractFunction();

		llvmVtable[i] = function->getLlvmFunction();
	}

	StructType* vtableStructType = m_type->getVtableStructType();

	llvm::Constant* llvmVtableConst = llvm::ConstantStruct::get(
		(llvm::StructType*)vtableStructType->getLlvmType(),
		llvm::ArrayRef<llvm::Constant*> (llvmVtable, count)
		);

	m_vtableVariable = m_module->m_variableMgr.createSimpleStaticVariable(
		sl::String(),
		getQualifiedName() + ".m_vtable",
		vtableStructType,
		Value(llvmVtableConst, vtableStructType)
		);
}

bool
Property::compileAutoGetter()
{
	ASSERT(m_getter);

	bool result;

	m_module->m_functionMgr.internalPrologue(m_getter);

	Value autoGetValue;
	result =
		m_module->m_operatorMgr.getPropertyAutoGetValue(getAutoAccessorPropertyValue(), &autoGetValue) &&
		m_module->m_controlFlowMgr.ret(autoGetValue);

	if (!result)
		return false;

	m_module->m_functionMgr.internalEpilogue();
	return true;
}

bool
Property::compileAutoSetter()
{
	ASSERT(m_setter && !m_setter->isOverloaded());
	ASSERT(m_type->getFlags() & PropertyTypeFlag_Bindable);

	bool result;

	Value srcValue;

	if (isMember())
	{
		Value argValueArray[2];
		m_module->m_functionMgr.internalPrologue(m_setter, argValueArray, 2);
		srcValue = argValueArray[1];
	}
	else
	{
		m_module->m_functionMgr.internalPrologue(m_setter, &srcValue, 1);
	}

	BasicBlock* assignBlock = m_module->m_controlFlowMgr.createBlock("assign_block");
	BasicBlock* returnBlock = m_module->m_controlFlowMgr.createBlock("return_block");

	Value autoGetValue;
	Value cmpValue;

	result =
		m_module->m_operatorMgr.getPropertyAutoGetValue(getAutoAccessorPropertyValue(), &autoGetValue) &&
		m_module->m_operatorMgr.binaryOperator(BinOpKind_Ne, autoGetValue, srcValue, &cmpValue) &&
		m_module->m_controlFlowMgr.conditionalJump(cmpValue, assignBlock, returnBlock) &&
		m_module->m_operatorMgr.storeDataRef(autoGetValue, srcValue) &&
		m_module->m_functionMgr.fireOnChanged();

	if (!result)
		return false;

	m_module->m_controlFlowMgr.follow(returnBlock);
	m_module->m_functionMgr.internalEpilogue();
	return true;
}

bool
Property::compileBinder()
{
	ASSERT(m_binder);

	bool result;

	m_module->m_functionMgr.internalPrologue(m_binder);

	Value onChangedValue;
	result =
		m_module->m_operatorMgr.getPropertyOnChanged(getAutoAccessorPropertyValue(), &onChangedValue) &&
		m_module->m_controlFlowMgr.ret(onChangedValue);

	if (!result)
		return false;

	m_module->m_functionMgr.internalEpilogue();
	return true;
}

Value
Property::getAutoAccessorPropertyValue()
{
	if (!isMember())
		return this;

	Value thisValue = m_module->m_functionMgr.getThisValue();
	Value propertyValue = this;
	Closure* closure = propertyValue.createClosure();
	closure->insertThisArgValue(thisValue);
	return propertyValue;
}

bool
Property::generateDocumentation(
	const sl::StringRef& outputDir,
	sl::String* itemXml,
	sl::String* indexXml
	)
{
	dox::Block* doxyBlock = m_module->m_doxyHost.getItemBlock(this);

	itemXml->format("<memberdef kind='property' id='%s'", doxyBlock->getRefId ().sz());

	if (m_accessKind != AccessKind_Public)
		itemXml->appendFormat(" prot='%s'", getAccessKindString(m_accessKind));

	if (m_storageKind == StorageKind_Static)
		itemXml->append(" static='yes'");

	if (isVirtual())
		itemXml->appendFormat(" virt='%s'", getStorageKindString(m_storageKind));

	itemXml->appendFormat(">\n<name>%s</name>\n", m_name.sz());
	itemXml->append(m_type->getDoxyTypeString());

	sl::String modifierString; // type modifiers are already encoded in <type>

	if (m_flags & PropertyFlag_AutoGet)
		modifierString.append(" autoget");

	if (m_flags & PropertyFlag_AutoSet)
		modifierString.append(" autoset");

	if (!modifierString.isEmpty())
		itemXml->appendFormat("<modifiers>%s</modifiers>\n", modifierString.getTrimmedString().sz());

	itemXml->append(doxyBlock->getImportString());
	itemXml->append(doxyBlock->getDescriptionString());
	itemXml->append(getDoxyLocationString());
	itemXml->append("</memberdef>\n");

	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
