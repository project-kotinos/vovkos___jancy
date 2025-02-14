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

#define _JNC_VARIABLE_H

#include "jnc_Type.h"

/**

\defgroup variable Variable
	\ingroup module-subsystem
	\import{jnc_Variable.h}

\addtogroup variable
@{

\struct jnc_Variable
	\verbatim

	Opaque structure used as a handle to Jancy variable.

	Use functions from the `Variable` group to access and manage the contents of this structure.

	\endverbatim

*/

//..............................................................................

enum jnc_VariableFlag
{
	jnc_VariableFlag_Arg = 0x010000,
};

typedef enum jnc_VariableFlag jnc_VariableFlag;

//..............................................................................

JNC_EXTERN_C
bool_t
jnc_Variable_hasInitializer(jnc_Variable* variable);

JNC_EXTERN_C
const char*
jnc_Variable_getInitializerString_v(jnc_Variable* variable);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_Variable: jnc_ModuleItem
{
	bool
	hasInitializer()
	{
		return jnc_Variable_hasInitializer(this) != 0;
	}

	const char*
	getInitializerString_v()
	{
		return jnc_Variable_getInitializerString_v(this);
	}
};

#endif // _JNC_CORE

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//..............................................................................

typedef enum jnc_VariableFlag VariableFlag;

const VariableFlag VariableFlag_Arg = jnc_VariableFlag_Arg;

//..............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
