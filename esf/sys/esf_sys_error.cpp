#include "esf_sys_error.h"

ESFError* ESFError::instance_ = 0;

#ifdef WIN32
DWORD ESFError::err_key_ = 0xffffffff;
#else
pthread_key_t ESFError::err_key_ = 0xffffffff;
#endif

ESFError::ESFError()
{
}

ESFError* ESFError::instance()
{
    if (instance_ == NULL)
    {
        instance_ = new  ESFError;
    }
    return instance_;
}

int ESFError::init ()
{
    if (instance() == NULL)
    {
        return -1;
    }
#ifdef WIN32
    err_key_ = TlsAlloc ();
#else
     pthread_key_create(&err_key_, free_err);
#endif
    return init_t();    //主线程也调用一次
}

void ESFError::fini ()
{
    if(instance_ != NULL)
    {
        fini_t();       //主线程也调用一次
        delete instance_;
        instance_ = NULL;
    }
}

int ESFError::init_t()
{
    ESF_ERROR_INFO* err_info = new ESF_ERROR_INFO;
    if (err_info == NULL)
    {
        return -1;
    }
    memset(err_info, 0, sizeof(ESF_ERROR_INFO));
#ifdef WIN32
    TlsSetValue(err_key_, err_info);
#else
    pthread_setspecific(err_key_, err_info);
#endif

    return 0;
}

void ESFError::fini_t()
{
    ESF_ERROR_INFO* err_info = NULL;

#ifdef WIN32
    err_info = (ESF_ERROR_INFO*)TlsGetValue(err_key_);
    TlsSetValue(err_key_, NULL);
#else
    err_info = (ESF_ERROR_INFO*)pthread_getspecific(err_key_);
    pthread_setspecific(err_key_, NULL);
#endif
    free_err(err_info);
}

void ESFError::free_err(void* ptr)
{
    ESF_ERROR_INFO* err_info = (ESF_ERROR_INFO*)ptr;
    if (err_info != NULL)
    {
        delete err_info;
    }
}

ESFError::operator int (void) const
{
    ESF_ERROR_INFO* err_info = NULL;

#ifdef WIN32
    err_info = (ESF_ERROR_INFO*)TlsGetValue(err_key_);
#else
    err_info = (ESF_ERROR_INFO*)pthread_getspecific(err_key_);
#endif
    return err_info->err_no_;
}

int ESFError::operator= (int x)
{
    ESF_ERROR_INFO* err_info = NULL;

#ifdef WIN32
    err_info = (ESF_ERROR_INFO*)TlsGetValue(err_key_);
#else
    err_info = (ESF_ERROR_INFO*)pthread_getspecific(err_key_);
#endif
    err_info->err_no_ = x;
    return x;
}

ESFError::operator char* (void)
{
    ESF_ERROR_INFO* err_info = NULL;

#ifdef WIN32
    err_info = (ESF_ERROR_INFO*)TlsGetValue(err_key_);
#else
    err_info = (ESF_ERROR_INFO*)pthread_getspecific(err_key_);
#endif
    return err_info->err_msg_;
}


void ESFError::clear_error()
{
    ESF_ERROR_INFO* err_info = NULL;

#ifdef WIN32
    err_info = (ESF_ERROR_INFO*)TlsGetValue(err_key_);
#else
    err_info = (ESF_ERROR_INFO*)pthread_getspecific(err_key_);
#endif
    err_info->err_no_ = 0;
    err_info->err_msg_[0] = 0;
}

void ESFError::set_error(int iErrno, const char* sErrMsg, ...)
{          
    ESF_ERROR_INFO* err_info = NULL;

#ifdef WIN32
    err_info = (ESF_ERROR_INFO*)TlsGetValue(err_key_);
#else
    err_info = (ESF_ERROR_INFO*)pthread_getspecific(err_key_);
#endif

    va_list ap;
    va_start(ap, sErrMsg);
    vsnprintf(err_info->err_msg_, ESF_MAX_ERROR_MSG_LEN, sErrMsg, ap);
    va_end(ap);
    err_info->err_msg_[ESF_MAX_ERROR_MSG_LEN -1] = 0;

    err_info->err_no_ = iErrno;
}

void ESFError::set_error_sys(int iErrno, const char* sErrMsg)
{          
    ESF_ERROR_INFO* err_info = NULL;

#ifdef WIN32
    err_info = (ESF_ERROR_INFO*)TlsGetValue(err_key_);
#else
    err_info = (ESF_ERROR_INFO*)pthread_getspecific(err_key_);
#endif

    snprintf(err_info->err_msg_, ESF_MAX_ERROR_MSG_LEN, "%s.[%d:%s]", sErrMsg, errno, strerror(errno));
    err_info->err_msg_[ESF_MAX_ERROR_MSG_LEN -1] = 0;

    err_info->err_no_ = iErrno;
}




