namespace sfc {
template<class T>
class sfc_shared_ptr
{
public:
	sfc_shared_ptr() : m_ptr(NULL), m_count(NULL) {} 

	sfc_shared_ptr(T* p) : m_ptr(p)
	{
		m_count = new (unsigned int)(1);
	}

	sfc_shared_ptr(const sfc_shared_ptr& sp)
	{
		if(NULL != sp.get()) {
			this->m_ptr = sp.get();
			this->m_count = sp.get_count();
			this->m_count ++;
		}
		else {
			m_ptr = NULL;
			m_count = NULL;
		}	
	}

	~sfc_shared_ptr()
	{
		if(NULL != m_count) {
			if(1 == *m_count) {
				delete m_ptr;
				delete m_count;
			}
			else {
				(*m_count)--;
			}
		}
	}

	sfc_shared_ptr& operator=(sfc_shared_ptr& sp)
	{
		if(NULL != sp.get() && this->get() != sp.get()) {
			this->release();
			m_ptr = sp.get();
			m_count = sp.get_count();
			(*m_count) ++;
		}
		return *this;
	}

	void release()
	{
		if(NULL != m_count) {
			if(1 == *m_count) {
				delete m_ptr; m_ptr = NULL;
				delete m_count; m_count = NULL;
			}
			else {
				(*m_count) --;
			}
		}
	}

	T* get()
	{
		return m_ptr;
	}

	unsigned int use_count()
	{
		if(NULL != m_count) return *m_count;
		return 0;
	}

	unsigned int* get_count()
	{
		return m_count;
	}

	T& operator*()
	{
		if(NULL != m_ptr) return *m_ptr;
	}

	T* operator->()
	{
		return m_ptr;
	}

private:
	T* m_ptr;
	unsigned int* m_count;
};

}