#pragma once

#include <vector>
#include <iostream>

#include "EventBase.h"

class slotbase
{
public:
	virtual void Execute(Event ev) = 0;
};

template<typename T>
class slotimpl : public slotbase
{
public:
	using member_function = void (T::*)(Event);

	slotimpl(T* pObj, member_function pMemberFunc)
	{
		m_pObj = pObj;
		m_pMemberFunc = pMemberFunc;
	}

	inline void Execute(Event ev) override
	{
		(m_pObj->*m_pMemberFunc)(ev);
	}
	
private:
	T* m_pObj;
	member_function m_pMemberFunc;
};

class CallBack
{
public:
	template<typename T>
	CallBack(T* pObj, void (T::*pMemberFunc)(Event))
	{
		m_pSlotbase = new slotimpl<T>(pObj, pMemberFunc);
	}
	~CallBack()
	{
		delete m_pSlotbase;
	}

	inline void Execute(Event ev)
	{
		m_pSlotbase->Execute(ev);
	}

private:
	slotbase* m_pSlotbase;
};