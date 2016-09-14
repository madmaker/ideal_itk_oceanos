#ifndef MISC_HXX_
#define MISC_HXX_

#include <string>
#include <tc/tc.h>
/**
 *
 */
template<class T> class auto_itk_mem_free {
public:
	auto_itk_mem_free(T * t = NULL) :
			m_t(t) {
	}
	virtual ~ auto_itk_mem_free() {
		printf("-Destructor\n");
		if (m_t) {
			MEM_free(m_t);
			m_t = NULL;
		}
	}
	operator T *() {
		return m_t;
	}
	operator T **() {
		return &m_t;
	}
private:
	T * m_t;
};

/**
 *
 */
bool starts_with(const std::string & param, const std::string & what);
void appendToResult(char*, char*, char*, int);
char* getId(char*);
int cmpfunc(const void *, const void *);
bool isvalueinarray(int , int *, int );

#endif /* MISC_HXX_ */
