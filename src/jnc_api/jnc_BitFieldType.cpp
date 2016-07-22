#include "pch.h"
#include "jnc_BitFieldType.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_DynamicExtensionLibHost.h"
#	include "jnc_ExtensionLib.h"
#elif defined (_JNC_CORE)
#	include "jnc_rt_Runtime.h"
#	include "jnc_ct_Module.h"
#endif

//.............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_Type*
jnc_BitFieldType_getBaseType (jnc_BitFieldType* type)
{
	return type->getBaseType ();
}

JNC_EXTERN_C
size_t
jnc_BitFieldType_getBitOffset (jnc_BitFieldType* type)
{
	return type->getBitOffset ();
}

JNC_EXTERN_C
size_t
jnc_BitFieldType_getBitCount (jnc_BitFieldType* type)
{
	return type->getBitCount ();
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//.............................................................................
