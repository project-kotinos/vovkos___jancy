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

namespace jnc {
namespace ct {

//..............................................................................

enum TokenKind
{
	// common tokens

	TokenKind_Eof = 0,
	TokenKind_Error = -1,
	TokenKind_Identifier = 256,
	TokenKind_Integer,
	TokenKind_Fp,
	TokenKind_Literal,
	TokenKind_DoxyComment1,
	TokenKind_DoxyComment2,
	TokenKind_DoxyComment3,
	TokenKind_DoxyComment4,

	// special literals

	TokenKind_BinLiteral,
	TokenKind_FmtLiteral,
	TokenKind_FmtSpecifier,

	// special declarations

	TokenKind_Import,
	TokenKind_Namespace,
	TokenKind_Extension,
	TokenKind_DynamicLib,
	TokenKind_Using,
	TokenKind_Friend,
	TokenKind_Public,
	TokenKind_Protected,
	TokenKind_Alignment,
	TokenKind_SetAs,

	// storage specifiers

	TokenKind_Typedef,
	TokenKind_Alias,
	TokenKind_Static,
	TokenKind_ThreadLocal,
	TokenKind_Abstract,
	TokenKind_Virtual,
	TokenKind_Override,
	TokenKind_Mutable,
	TokenKind_Disposable,

	// type modifiers

	TokenKind_Unsigned,
	TokenKind_BigEndian,
	TokenKind_Const,
	TokenKind_ReadOnly,
	TokenKind_CMut,
	TokenKind_Volatile,
	TokenKind_Weak,
	TokenKind_Thin,
	TokenKind_Safe,
	TokenKind_Unsafe,
	TokenKind_ErrorCode,
	TokenKind_Cdecl,
	TokenKind_Stdcall,
	TokenKind_Thiscall,
	TokenKind_Jnccall,
	TokenKind_Array,
	TokenKind_Function,
	TokenKind_Property,
	TokenKind_Bindable,
	TokenKind_AutoGet,
	TokenKind_Indexed,
	TokenKind_Multicast,
	TokenKind_Event,
	TokenKind_Reactor,
	TokenKind_Async,

	// type specifiers

	TokenKind_Void,
	TokenKind_Variant,
	TokenKind_Bool,
	TokenKind_Int,
	TokenKind_IntPtr,
	TokenKind_Char,
	TokenKind_Short,
	TokenKind_Long,
	TokenKind_Float,
	TokenKind_Double,

	// named type specifiers

	TokenKind_Enum,
	TokenKind_Struct,
	TokenKind_Union,
	TokenKind_Class,
	TokenKind_Opaque,
	TokenKind_Exposed,
	TokenKind_BitFlag,
	TokenKind_AnyData,

	// special member methods

	TokenKind_Get,
	TokenKind_Set,
	TokenKind_PreConstruct,
	TokenKind_Construct,
	TokenKind_Destruct,
	TokenKind_Operator,
	TokenKind_Postfix,

	// statements

	TokenKind_If,
	TokenKind_Else,
	TokenKind_For,
	TokenKind_While,
	TokenKind_Do,
	TokenKind_Break,
	TokenKind_Continue,
	TokenKind_Return,
	TokenKind_Switch,
	TokenKind_ReSwitch,
	TokenKind_Case,
	TokenKind_Default,
	TokenKind_Once,
	TokenKind_OnEvent,
	TokenKind_Try,
	TokenKind_Throw,
	TokenKind_Catch,
	TokenKind_Finally,
	TokenKind_NestedScope,
	TokenKind_Assert,
	TokenKind_Await,

	// pre-defined values

	TokenKind_BaseType,
	TokenKind_This,
	TokenKind_True,
	TokenKind_False,
	TokenKind_Null,

	// keyword operators

	TokenKind_New,
	TokenKind_SizeOf,
	TokenKind_CountOf,
	TokenKind_OffsetOf,
	TokenKind_TypeOf,
	TokenKind_BindingOf,
	TokenKind_Dynamic,

	// symbol tokens

	TokenKind_Inc,
	TokenKind_Dec,
	TokenKind_Ptr,
	TokenKind_Imply,
	TokenKind_Shl,
	TokenKind_Shr,
	TokenKind_LogAnd,
	TokenKind_LogOr,
	TokenKind_Eq,
	TokenKind_Ne,
	TokenKind_Le,
	TokenKind_Ge,
	TokenKind_RefAssign,
	TokenKind_AddAssign,
	TokenKind_SubAssign,
	TokenKind_MulAssign,
	TokenKind_DivAssign,
	TokenKind_ModAssign,
	TokenKind_ShlAssign,
	TokenKind_ShrAssign,
	TokenKind_AndAssign,
	TokenKind_XorAssign,
	TokenKind_OrAssign,
	TokenKind_AtAssign,
	TokenKind_Ellipsis,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum TokenChannelMask
{
	TokenChannelMask_Main        = lex::TokenChannelMask_Main, // 0x01,
	TokenChannelMask_DoxyComment = 0x02,
	TokenChannelMask_All         = -1,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

AXL_LEX_BEGIN_TOKEN_NAME_MAP(TokenName)

	// common tokens

	AXL_LEX_TOKEN_NAME(TokenKind_Eof,          "eof")
	AXL_LEX_TOKEN_NAME(TokenKind_Error,        "error")
	AXL_LEX_TOKEN_NAME(TokenKind_Identifier,   "identifier")
	AXL_LEX_TOKEN_NAME(TokenKind_Integer,      "integer-constant")
	AXL_LEX_TOKEN_NAME(TokenKind_Fp,           "floating-point-constant")

	AXL_LEX_TOKEN_NAME(TokenKind_DoxyComment1, "///")
	AXL_LEX_TOKEN_NAME(TokenKind_DoxyComment2, "//!")
	AXL_LEX_TOKEN_NAME(TokenKind_DoxyComment3, "/**")
	AXL_LEX_TOKEN_NAME(TokenKind_DoxyComment4, "/*!")

	// literal tokens

	AXL_LEX_TOKEN_NAME(TokenKind_Literal,      "string-literal")
	AXL_LEX_TOKEN_NAME(TokenKind_BinLiteral,   "bin-literal")
	AXL_LEX_TOKEN_NAME(TokenKind_FmtLiteral,   "fmt-literal")
	AXL_LEX_TOKEN_NAME(TokenKind_FmtSpecifier, "fmt-specifier")

	// special declarations

	AXL_LEX_TOKEN_NAME(TokenKind_Import,       "import")
	AXL_LEX_TOKEN_NAME(TokenKind_Namespace,    "namespace")
	AXL_LEX_TOKEN_NAME(TokenKind_Extension,    "extension")
	AXL_LEX_TOKEN_NAME(TokenKind_DynamicLib,   "dynamiclib")
	AXL_LEX_TOKEN_NAME(TokenKind_Using,        "using")
	AXL_LEX_TOKEN_NAME(TokenKind_Friend,       "friend")
	AXL_LEX_TOKEN_NAME(TokenKind_Public,       "public")
	AXL_LEX_TOKEN_NAME(TokenKind_Protected,    "protected")
	AXL_LEX_TOKEN_NAME(TokenKind_Alignment,    "alignment")
	AXL_LEX_TOKEN_NAME(TokenKind_SetAs,        "setas")

	// storage specifiers

	AXL_LEX_TOKEN_NAME(TokenKind_Typedef,      "typedef")
	AXL_LEX_TOKEN_NAME(TokenKind_Alias,        "alias")
	AXL_LEX_TOKEN_NAME(TokenKind_Static,       "static")
	AXL_LEX_TOKEN_NAME(TokenKind_ThreadLocal,  "threadlocal")
	AXL_LEX_TOKEN_NAME(TokenKind_Abstract,     "abstract")
	AXL_LEX_TOKEN_NAME(TokenKind_Virtual,      "virtual")
	AXL_LEX_TOKEN_NAME(TokenKind_Override,     "override")
	AXL_LEX_TOKEN_NAME(TokenKind_Mutable,      "mutable")
	AXL_LEX_TOKEN_NAME(TokenKind_Disposable,   "disposable")

	// type modifiers

	AXL_LEX_TOKEN_NAME(TokenKind_Unsigned,     "unsigned")
	AXL_LEX_TOKEN_NAME(TokenKind_BigEndian,    "bigendian")
	AXL_LEX_TOKEN_NAME(TokenKind_Const,        "const")
	AXL_LEX_TOKEN_NAME(TokenKind_ReadOnly,     "readonly")
	AXL_LEX_TOKEN_NAME(TokenKind_CMut,         "cmut")
	AXL_LEX_TOKEN_NAME(TokenKind_Volatile,     "volatile")
	AXL_LEX_TOKEN_NAME(TokenKind_Weak,         "weak")
	AXL_LEX_TOKEN_NAME(TokenKind_Thin,         "thin")
	AXL_LEX_TOKEN_NAME(TokenKind_Safe,         "safe")
	AXL_LEX_TOKEN_NAME(TokenKind_Unsafe,       "unsafe")
	AXL_LEX_TOKEN_NAME(TokenKind_ErrorCode,    "errorcode")
	AXL_LEX_TOKEN_NAME(TokenKind_Cdecl,        "cdecl")
	AXL_LEX_TOKEN_NAME(TokenKind_Stdcall,      "stdcall")
	AXL_LEX_TOKEN_NAME(TokenKind_Thiscall,     "thiscall")
	AXL_LEX_TOKEN_NAME(TokenKind_Jnccall,      "jnccall")
	AXL_LEX_TOKEN_NAME(TokenKind_Array,        "array")
	AXL_LEX_TOKEN_NAME(TokenKind_Function,     "function")
	AXL_LEX_TOKEN_NAME(TokenKind_Property,     "property")
	AXL_LEX_TOKEN_NAME(TokenKind_Bindable,     "bindable")
	AXL_LEX_TOKEN_NAME(TokenKind_AutoGet,      "autoget")
	AXL_LEX_TOKEN_NAME(TokenKind_Indexed,      "indexed")
	AXL_LEX_TOKEN_NAME(TokenKind_Multicast,    "multicast")
	AXL_LEX_TOKEN_NAME(TokenKind_Event,        "event")
	AXL_LEX_TOKEN_NAME(TokenKind_Reactor,      "reactor")
	AXL_LEX_TOKEN_NAME(TokenKind_Async,        "async")

	// type specifiers

	AXL_LEX_TOKEN_NAME(TokenKind_AnyData,      "anydata")
	AXL_LEX_TOKEN_NAME(TokenKind_Void,         "void")
	AXL_LEX_TOKEN_NAME(TokenKind_Variant,      "variant")
	AXL_LEX_TOKEN_NAME(TokenKind_Bool,         "bool")
	AXL_LEX_TOKEN_NAME(TokenKind_Int,          "int")
	AXL_LEX_TOKEN_NAME(TokenKind_IntPtr,       "intptr")
	AXL_LEX_TOKEN_NAME(TokenKind_Char,         "char")
	AXL_LEX_TOKEN_NAME(TokenKind_Short,        "short")
	AXL_LEX_TOKEN_NAME(TokenKind_Long,         "long")
	AXL_LEX_TOKEN_NAME(TokenKind_Float,        "float")
	AXL_LEX_TOKEN_NAME(TokenKind_Double,       "double")

	// named type specifiers

	AXL_LEX_TOKEN_NAME(TokenKind_Enum,         "enum")
	AXL_LEX_TOKEN_NAME(TokenKind_Struct,       "struct")
	AXL_LEX_TOKEN_NAME(TokenKind_Union,        "union")
	AXL_LEX_TOKEN_NAME(TokenKind_Class,        "class")
	AXL_LEX_TOKEN_NAME(TokenKind_Opaque,       "opaque")
	AXL_LEX_TOKEN_NAME(TokenKind_Exposed,      "exposed")
	AXL_LEX_TOKEN_NAME(TokenKind_BitFlag,      "bitflag")

	// special members

	AXL_LEX_TOKEN_NAME(TokenKind_Get,          "get")
	AXL_LEX_TOKEN_NAME(TokenKind_Set,          "set")
	AXL_LEX_TOKEN_NAME(TokenKind_PreConstruct, "preconstruct")
	AXL_LEX_TOKEN_NAME(TokenKind_Construct,    "construct")
	AXL_LEX_TOKEN_NAME(TokenKind_Destruct,     "destruct")
	AXL_LEX_TOKEN_NAME(TokenKind_Operator,     "operator")
	AXL_LEX_TOKEN_NAME(TokenKind_Postfix,      "postfix")

	// statements

	AXL_LEX_TOKEN_NAME(TokenKind_If,           "if")
	AXL_LEX_TOKEN_NAME(TokenKind_Else,         "else")
	AXL_LEX_TOKEN_NAME(TokenKind_For,          "for")
	AXL_LEX_TOKEN_NAME(TokenKind_While,        "while")
	AXL_LEX_TOKEN_NAME(TokenKind_Do,           "do")
	AXL_LEX_TOKEN_NAME(TokenKind_Break,        "break")
	AXL_LEX_TOKEN_NAME(TokenKind_Continue,     "continue")
	AXL_LEX_TOKEN_NAME(TokenKind_Return,       "return")
	AXL_LEX_TOKEN_NAME(TokenKind_Switch,       "switch")
	AXL_LEX_TOKEN_NAME(TokenKind_ReSwitch,     "reswitch")
	AXL_LEX_TOKEN_NAME(TokenKind_Case,         "case")
	AXL_LEX_TOKEN_NAME(TokenKind_Default,      "default")
	AXL_LEX_TOKEN_NAME(TokenKind_Once,         "once")
	AXL_LEX_TOKEN_NAME(TokenKind_OnEvent,      "onevent")
	AXL_LEX_TOKEN_NAME(TokenKind_Try,          "try")
	AXL_LEX_TOKEN_NAME(TokenKind_Throw,        "throw")
	AXL_LEX_TOKEN_NAME(TokenKind_Catch,        "catch")
	AXL_LEX_TOKEN_NAME(TokenKind_Finally,      "finally")
	AXL_LEX_TOKEN_NAME(TokenKind_NestedScope,  "nestedscope")
	AXL_LEX_TOKEN_NAME(TokenKind_Assert,       "assert")
	AXL_LEX_TOKEN_NAME(TokenKind_Await,        "await")

	// pre-defined values

	AXL_LEX_TOKEN_NAME(TokenKind_BaseType,     "basetype")
	AXL_LEX_TOKEN_NAME(TokenKind_This,         "this")
	AXL_LEX_TOKEN_NAME(TokenKind_True,         "true")
	AXL_LEX_TOKEN_NAME(TokenKind_False,        "false")
	AXL_LEX_TOKEN_NAME(TokenKind_Null,         "null")

	// keyword operators

	AXL_LEX_TOKEN_NAME(TokenKind_New,          "new")
	AXL_LEX_TOKEN_NAME(TokenKind_SizeOf,       "sizeof")
	AXL_LEX_TOKEN_NAME(TokenKind_CountOf,      "countof")
	AXL_LEX_TOKEN_NAME(TokenKind_OffsetOf,     "offsetof")
	AXL_LEX_TOKEN_NAME(TokenKind_TypeOf,       "typeof")
	AXL_LEX_TOKEN_NAME(TokenKind_BindingOf,    "bindingof")
	AXL_LEX_TOKEN_NAME(TokenKind_Dynamic,      "dynamic")

	// symbol tokens

	AXL_LEX_TOKEN_NAME(TokenKind_Inc,          "++")
	AXL_LEX_TOKEN_NAME(TokenKind_Dec,          "--")
	AXL_LEX_TOKEN_NAME(TokenKind_Ptr,          "->")
	AXL_LEX_TOKEN_NAME(TokenKind_Imply,        "=>")
	AXL_LEX_TOKEN_NAME(TokenKind_Shl,          "<<")
	AXL_LEX_TOKEN_NAME(TokenKind_Shr,          ">>")
	AXL_LEX_TOKEN_NAME(TokenKind_LogAnd,       "&&")
	AXL_LEX_TOKEN_NAME(TokenKind_LogOr,        "||")
	AXL_LEX_TOKEN_NAME(TokenKind_Eq,           "==")
	AXL_LEX_TOKEN_NAME(TokenKind_Ne,           "!=")
	AXL_LEX_TOKEN_NAME(TokenKind_Le,           "<=")
	AXL_LEX_TOKEN_NAME(TokenKind_Ge,           ">=")
	AXL_LEX_TOKEN_NAME(TokenKind_RefAssign,    ":=")
	AXL_LEX_TOKEN_NAME(TokenKind_AddAssign,    "+=")
	AXL_LEX_TOKEN_NAME(TokenKind_SubAssign,    "-=")
	AXL_LEX_TOKEN_NAME(TokenKind_MulAssign,    "*=")
	AXL_LEX_TOKEN_NAME(TokenKind_DivAssign,    "/=")
	AXL_LEX_TOKEN_NAME(TokenKind_ModAssign,    "%=")
	AXL_LEX_TOKEN_NAME(TokenKind_ShlAssign,    "<<=")
	AXL_LEX_TOKEN_NAME(TokenKind_ShrAssign,    ">>=")
	AXL_LEX_TOKEN_NAME(TokenKind_AndAssign,    "&=")
	AXL_LEX_TOKEN_NAME(TokenKind_XorAssign,    "^=")
	AXL_LEX_TOKEN_NAME(TokenKind_OrAssign,     "|=")
	AXL_LEX_TOKEN_NAME(TokenKind_AtAssign,     "@=")
	AXL_LEX_TOKEN_NAME(TokenKind_Ellipsis,     "...")

AXL_LEX_END_TOKEN_NAME_MAP();

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class TokenData: public lex::StdTokenData
{
public:
	sl::Array<char> m_binData;
};

typedef lex::RagelToken<TokenKind, TokenName, TokenData> Token;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Lexer: public lex::RagelLexer<Lexer, Token>
{
	friend class lex::RagelLexer<Lexer, Token>;

protected:
	Token* m_fmtLiteralToken;
	Token* m_mlLiteralToken;
	int m_mlBinLiteralTokenRadix;
	sl::Array<intptr_t> m_parenthesesLevelStack;
	sl::String m_dir;

public:
	Lexer();

protected:
	Token*
	createKeywordTokenEx(
		int tokenKind,
		int param
		);

	Token*
	createStringToken(
		int tokenKind,
		size_t left = 0,
		size_t right = 0,
		bool useEscapeEncoding = false
		);

	Token*
	createSourceFileToken();

	Token*
	createSourceDirToken();

	Token*
	createBinLiteralToken(int radix);

	Token*
	createCharToken(
		int tokenKind,
		size_t left,
		size_t right,
		bool useEscapeEncoding = false
		);

	Token*
	createIntegerToken(
		int radix = 10,
		size_t left = 0
		);

	Token*
	createFpToken();

	Token*
	createConstIntegerToken(int value);

	// multi-line literals

	Token*
	preCreateMlLiteralToken(int radix = 0);

	Token*
	createMlLiteralToken();

	// formatting literals

	Token*
	preCreateFmtLiteralToken();

	Token*
	createFmtLiteralToken(
		int tokenKind,
		int param = 0
		);

	void
	createFmtSimpleIdentifierTokens();

	void
	createFmtLastErrorDescriptionTokens();

	void
	createFmtIndexTokens();

	Token*
	createFmtSpecifierToken();

	Token*
	createDoxyCommentToken(TokenKind tokenKind);

	void
	onLeftParentheses();

	bool
	onRightParentheses();

	bool
	onSemicolon();

	void
	terminateFmtSpecifier();

	void
	terminateFmtLiteral();

	// implemented in *.rl

	void
	init();

	void
	exec();
};

//..............................................................................

} // namespace ct
} // namespace jnc
