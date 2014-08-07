/*
ERRORģ��, ����TSD(Thread-specific Data)����, 
��ʵ�� �Զ���� errNo, errMsg �ڶ��̻߳����еİ�ȫ����.
*/


#ifndef __ESF_SYS_ERROR_H__
#define __ESF_SYS_ERROR_H__

#include "esf_sys_comm.h"

#pragma pack(1)

typedef struct tagERROR_INFO
{
    int err_no_;
    char err_msg_[ESF_MAX_ERROR_MSG_LEN];
}ESF_ERROR_INFO;

#pragma pack()

class ESFError
{
protected:
    
    ESFError ();
public:
    static int init ();    //��������ʱ����, ���ڴ���key, ȫ��ֻ����һ��
    static void fini ();    //�����˳�ʱ����, ����ɾ��key, ȫ��ֻ����һ��
    static int init_t();    //�߳�����ʱ����, ���ڴ����߳�ר���ڴ��,���߳̾������
    static void fini_t();   //�߳��˳�ʱ����, ����ɾ���߳�ר���ڴ��,���߳̾������
    static ESFError *instance ();

    operator int (void) const;
    int operator= (int);
    operator char* (void);

    static void clear_error();
    static void set_error(int iErrno, const char* sErrMsg, ...);
    static void set_error_sys(int iErrno, const char* sErrMsg);

private:
    static ESFError *instance_;
#ifdef WIN32
    static DWORD err_key_;
#else
    static pthread_key_t err_key_;
#endif

    static void free_err(void * ptr);
};


#define ESF_ERRNO (*(ESFError::instance ()))
#define ESF_ERRMSG (*(ESFError::instance ())) 

#define ESF_ERROR_INIT  ESFError::init
#define ESF_ERROR_FINI  ESFError::fini

#define ESF_ERROR_INIT_T  ESFError::init_t
#define ESF_ERROR_FINI_T  ESFError::fini_t

#define CLEAR_ERROR  ESFError::clear_error
#define SET_ERROR  ESFError::set_error
#define SET_ERROR_SYS  ESFError::set_error_sys


//���ô���ֵ, ����������
#define ERROR_RETURN_SYS_ERROR(errNo, errMsg) \
            ESFError::set_error_sys(errNo, errMsg); \
            return errNo;

#define ERRNO_RETURN(errNo); \
    		ESF_ERRNO = (int)errNo; \
    		return errNo;

#define ERROR_RETURN(errNo, errMsg); \
    		ESFError::set_error(errNo, errMsg); \
    		return errNo;

#define ERROR_RETURN_1(errNo, errMsg, param1) \
            ESFError::set_error(errNo, errMsg, param1); \
            return errNo;

#define ERROR_RETURN_2(errNo, errMsg, param1, param2) \
            ESFError::set_error(errNo, errMsg, param1, param2); \
            return errNo;

#define ERROR_RETURN_3(errNo, errMsg, param1, param2, param3) \
            ESFError::set_error(errNo, errMsg, param1, param2, param3); \
            return errNo;

#define ERROR_RETURN_NULL(errNo, errMsg); \
    		ESFError::set_error(errNo, errMsg); \
    		return NULL;

#define ERROR_RETURN_NULL_1(errNo, errMsg, param1) \
            ESFError::set_error(errNo, errMsg, param1); \
            return NULL;

#define ERROR_RETURN_NULL_2(errNo, errMsg, param1, param2) \
            ESFError::set_error(errNo, errMsg, param1, param2); \
            return NULL;

#define ERROR_RETURN_NULL_3(errNo, errMsg, param1, param2, param3) \
            ESFError::set_error(errNo, errMsg, param1, param2, param3); \
            return NULL;


#endif

