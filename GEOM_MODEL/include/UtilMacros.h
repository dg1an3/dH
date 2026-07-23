//////////////////////////////////////////////////////////////////////
// UtilMacros.h: standard utility macros
//
// Copyright (C) 1999-2001
// $Id$
//////////////////////////////////////////////////////////////////////

#if !defined(UTILMACROS_H)
#define UTILMACROS_H

//////////////////////////////////////////////////////////////////////
// Macros for attributes
//////////////////////////////////////////////////////////////////////

#define DECLARE_ATTRIBUTE_VAL(NAME, TYPE) \
private:									\
	TYPE m_##NAME;							\
public:										\
	TYPE Get##NAME() const			\
	{ return m_##NAME; }					\
	void Set##NAME(TYPE value)		\
	{ m_##NAME = value; }

#define DECLARE_ATTRIBUTE(NAME, TYPE) \
private:									\
	TYPE m_##NAME;							\
public:										\
	const TYPE& Get##NAME() const			\
	{ return m_##NAME; }					\
	void Set##NAME(const TYPE& value)		\
	{ m_##NAME = value; }

#define DeclareMember(NAME, TYPE) \
private:									\
	TYPE m_##NAME;							\
public:										\
	const TYPE& Get##NAME() const			\
	{ return m_##NAME; }					\
	void Set##NAME(const TYPE& value)		\
	{ m_##NAME = value; }

#define DECLARE_ATTRIBUTE_GI(NAME, TYPE) \
private:									\
	TYPE m_##NAME;							\
public:										\
	const TYPE& Get##NAME() const			\
	{ return m_##NAME; }					\
	void Set##NAME(const TYPE& value);

#define DeclareMemberGI(NAME, TYPE) \
private:									\
	TYPE m_##NAME;							\
public:										\
	const TYPE& Get##NAME() const			\
	{ return m_##NAME; }					\
	void Set##NAME(const TYPE& value);

#define DECLARE_ATTRIBUTE_PTR(NAME, TYPE) \
private:									\
	TYPE *m_p##NAME;						\
public:										\
	TYPE *Get##NAME()						\
	{ return m_p##NAME; }					\
	const TYPE *Get##NAME() const			\
	{ return m_p##NAME; }					\
	void Set##NAME(TYPE *pValue)			\
	{ m_p##NAME = pValue; }

#define DeclareMemberPtr(NAME, TYPE) \
private:									\
	TYPE *m_p##NAME;						\
public:										\
	TYPE *Get##NAME()						\
	{ return m_p##NAME; }					\
	const TYPE *Get##NAME() const			\
	{ return m_p##NAME; }					\
	void Set##NAME(TYPE *pValue)			\
	{ m_p##NAME = pValue; }

#define DECLARE_ATTRIBUTE_PTR_GI(NAME, TYPE) \
private:									\
	TYPE *m_p##NAME;						\
public:										\
	TYPE *Get##NAME()						\
	{ return m_p##NAME; }					\
	const TYPE *Get##NAME() const			\
	{ return m_p##NAME; }					\
	void Set##NAME(TYPE *pValue);

#define DeclareMemberPtrGI(NAME, TYPE) \
private:									\
	TYPE *m_p##NAME;						\
public:										\
	TYPE *Get##NAME()						\
	{ return m_p##NAME; }					\
	const TYPE *Get##NAME() const			\
	{ return m_p##NAME; }					\
	void Set##NAME(TYPE *pValue);

//////////////////////////////////////////////////////////////////////
// Macros for trace dumps
//////////////////////////////////////////////////////////////////////

#define CREATE_TAB(LENGTH) \
	CString TAB; \
	while (TAB.GetLength() < LENGTH * 2) \
		TAB += "\t\t"

#define DC_TAB(DUMP_CONTEXT) \
	DUMP_CONTEXT << TAB

#define PUSH_DUMP_DEPTH(DUMP_CONTEXT) \
	int OLD_DUMP_DEPTH = DUMP_CONTEXT.GetDepth(); \
	DUMP_CONTEXT.SetDepth(OLD_DUMP_DEPTH + 1)

#define POP_DUMP_DEPTH(DUMP_CONTEXT) \
	DUMP_CONTEXT.SetDepth(OLD_DUMP_DEPTH)


//////////////////////////////////////////////////////////////////////
// Macros for serialization
//////////////////////////////////////////////////////////////////////

#define SERIALIZE_VALUE(ar, value) \
	if (ar.IsLoading())				\
	{								\
		ar >> value;				\
	}								\
	else							\
	{								\
		ar << value;				\
	}

#define SERIALIZE_ARRAY(ar, array) \
	if (ar.IsLoading())				\
	{								\
		array.RemoveAll();			\
	}								\
	array.Serialize(ar);

#endif

#define CHECK_HRESULT(call) \
{							\
	HRESULT hr = call;		\
	if (FAILED(hr))			\
	{						\
		return hr;			\
	}						\
}

#define CHECK_CONDITION(expr) \
if (!(expr))				\
{							\
	return E_FAIL;			\
}

#define LOG_HRESULT(call) \
{							\
	HRESULT hr = call;		\
	ATLASSERT(SUCCEEDED(hr));	\
}

