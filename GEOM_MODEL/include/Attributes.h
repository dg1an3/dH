#pragma once

#define DECLARE_ATTRIBUTE(NAME, TYPE) \
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


#define DECLARE_ATTRIBUTE_PTR_GI(NAME, TYPE) \
private:									\
	TYPE *m_p##NAME;						\
public:										\
	TYPE *Get##NAME()						\
	{ return m_p##NAME; }					\
	const TYPE *Get##NAME() const			\
	{ return m_p##NAME; }					\
	void Set##NAME(TYPE *pValue);
