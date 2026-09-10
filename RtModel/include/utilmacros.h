//////////////////////////////////////////////////////////////////////
// UtilMacros.h: standard utility macros
//
// Copyright (C) 1999-2001
// $Id: UtilMacros.h,v 1.1 2007-10-29 01:43:52 Derek Lane Exp $
//////////////////////////////////////////////////////////////////////

#if !defined(UTILMACROS_H)
#define UTILMACROS_H

//////////////////////////////////////////////////////////////////////
// Debug macros
//
// MFC-free replacements for ASSERT / TRACE.  Call sites use assert()
// directly; RTM_TRACE mirrors the old TRACE -- printf-style, and
// compiled out entirely in release builds.
//////////////////////////////////////////////////////////////////////

#include <cassert>
#include <cstdio>

#ifndef _DEBUG
#define RTM_TRACE(...) ((void) 0)
#elif defined(_WIN32)
// routed to the debugger output window, as TRACE was
#define RTM_TRACE(...)									\
{														\
	char _rtmTraceBuf[1024];							\
	_snprintf_s(_rtmTraceBuf, sizeof(_rtmTraceBuf),		\
		_TRUNCATE, __VA_ARGS__);						\
	::OutputDebugStringA(_rtmTraceBuf);					\
}
#else
#define RTM_TRACE(...) \
	std::fprintf(stderr, __VA_ARGS__)
#endif

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

#define DECLARE_ATTRIBUTE_EVT(NAME, TYPE, EVT) \
private:									\
	TYPE m_##NAME;							\
public:										\
	const TYPE& Get##NAME() const			\
	{ return m_##NAME; }					\
	void Set##NAME(const TYPE& value)		\
	{ m_##NAME = value; EVT.Fire(this); }


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


#define BeginNamespace(NAME) namespace NAME {

#define EndNamespace(NAME) }



#define CHECK_HRESULT(call) \
{							\
	HRESULT hr = call;		\
	if (FAILED(hr))			\
	{						\
		return hr;			\
	}						\
}

// check COM result
#define CR(Cmd) \
{						\
	HRESULT hr = (Cmd); \
	if (FAILED(hr))		\
	{					\
		RTM_TRACE("COM Command Failed -- Error %x\n", hr); \
		return hr;		\
	}					\
}

#define CHECK_CONDITION(expr) \
if (!(expr))				\
{							\
	return E_FAIL;			\
}


// prevent re-definition of these
#ifndef LOG_MACROS_DEFINED
#define LOG_MACROS_DEFINED

#define USES_FMT

#define LOG_HRESULT(call) \
{							\
	HRESULT hr = call;		\
	assert(SUCCEEDED(hr));	\
}

// log file utilities
#define BeginLogSection(section_name) { \
	RTM_TRACE("<log_section name=\"%s\">", section_name);

#define EndLogSection() \
	RTM_TRACE("</log_section>\n"); }

#define Log(message) RTM_TRACE("%s", message)

#endif


#endif
