
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

#define ASSERT_HRESULT(call) \
{							\
	HRESULT hr = (call);	\
	ASSERT(SUCCEEDED(hr));	\
}

