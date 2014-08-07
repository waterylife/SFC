#ifndef __SFC_AUTO_PTR__
#define __SFC_AUTO_PTR__

#include <iostream>

using namespace std;

template <class T>
class sfc_auto_ptr 
{
private:
	T* m_ptr;

public:
	//构造函数
	explicit sfc_auto_ptr(T* t = NULL) : m_ptr(t) {}
	sfc_auto_ptr(const sfc_auto_ptr& sat) : m_ptr(sat.release()) {}
	template<class T1>
	sfc_auto_ptr(const sfc_auto_ptr<T1>& sat) : m_ptr(sat.release()) {}
	
	//析构函数
	~sfc_auto_ptr()
	{
		delete m_ptr;
		m_ptr = NULL;
	}

	//赋值操作
	sfc_auto_ptr& operator=(sfc_auto_ptr& sap)
	{
		if(m_ptr != sap.get()) {
			delete m_ptr;
		}
		m_ptr = sap.release();
		return *this;
	}
	template<class T1>
	sfc_auto_ptr& operator=(sfc_auto_ptr<T1>& sap)
	{
		if(m_ptr != (T*)(sap.get())) {
			delete m_ptr;
			m_ptr = (T*)(sap.release());
		}
		return *this;
	}

	T& operator*() const
	{
		return *m_ptr;
	} 

	T* operator->() const
	{
		return m_ptr;
	}


	T* get() const
	{
		return m_ptr;
	}

	T* release()
	{
		T* tmp = m_ptr;
		m_ptr = NULL;
		return tmp;
	}

	void reset(const T* t)
	{
		if(m_ptr != t) {
			delete m_ptr;
			m_ptr =t;
		}
	}

};

#endif