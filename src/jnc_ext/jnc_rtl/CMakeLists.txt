#...............................................................................
#
#  This file is part of the Jancy toolkit.
#
#  Jancy is distributed under the MIT license.
#  For details see accompanying license.txt file,
#  the public copy of which is also available at:
#  http://tibbo.com/downloads/archive/jancy/license.txt
#
#...............................................................................

#
# lib folder
#

set(
	LIB_H_LIST
	jnc_rtl_DynamicLib.h
	jnc_rtl_DynamicLayout.h
	jnc_rtl_Multicast.h
	jnc_rtl_Promise.h
	jnc_rtl_Reactor.h
	jnc_rtl_Regex.h
	)

set(
	LIB_CPP_LIST
	jnc_rtl_CoreLib.cpp
	jnc_rtl_DynamicLib.cpp
	jnc_rtl_DynamicLayout.cpp
	jnc_rtl_Multicast.cpp
	jnc_rtl_Promise.cpp
	jnc_rtl_Reactor.cpp
	jnc_rtl_Reactor.cpp
	jnc_rtl_Regex.cpp
	)

source_group(
	lib
	FILES
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# jnc folder
#

set(
	JNC_LIST
	jnc/jnc_globals.jnc
	jnc/jnc_GcTriggers.jnc
	jnc/jnc_GcStats.jnc
	jnc/jnc_DynamicLib.jnc
	jnc/jnc_Promise.jnc
	jnc/jnc_Regex.jnc
	jnc/jnc_Scheduler.jnc
	jnc/jnc_StdFunctions.jnc
	jnc/jnc_StdTypes.jnc
	jnc/jnc_StdTypedefs.jnc
	)

source_group(
	jnc
	FILES
	${JNC_LIST}
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# pch folder
#

set(PCH_H     pch.h)
set(PCH_CPP   pch.cpp)
set(PCH_O_CPP pch_o.cpp)

source_group(
	pch
	FILES
	${PCH_H}
	${PCH_CPP}
	)

#...............................................................................
#
# jnc_rtl library
#

include_directories(
	${LLVM_INC_DIR}
	${AXL_INC_DIR}
	${JANCY_INC_DIR}
	${JANCY_SRC_DIR}/jnc_ct/include
	${JANCY_SRC_DIR}/jnc_rt
	${GEN_DIR}
	)

add_library(
	jnc_rtl
	${PCH_H}
	${PCH_CPP}
	${LIB_H_LIST}
	${LIB_CPP_LIST}
	${JNC_LIST}
	)

axl_set_pch(
	jnc_rtl
	${PCH_H}
	${PCH_CPP}
	)

set_target_properties(
	jnc_rtl
	PROPERTIES
	FOLDER extensions
	)

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

# object library to be re-used as sources for the jancy shared/dylib

if(UNIX)
	add_library(
		jnc_rtl_o OBJECT
		${PCH_H}
		${PCH_O_CPP}
		${LIB_H_LIST}
		${LIB_CPP_LIST}
		)

	target_compile_definitions(
		jnc_rtl_o
		PUBLIC
		-D_JNC_OBJECT_LIB=1
		)

	axl_set_pch(
		jnc_rtl_o
		${PCH_H}
		${PCH_O_CPP}
		)

	set_target_properties(
		jnc_rtl_o
		PROPERTIES
		FOLDER extensions
		)
endif()

#. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

install(
	TARGETS jnc_rtl
	DESTINATION ${JANCY_INSTALL_LIB_SUBDIR}
	)

#...............................................................................
