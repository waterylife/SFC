#ifndef __ESF_SYS_OBJECT_H__
#define __ESF_SYS_OBJECT_H__

#include "esf_sys_comm.h"
#include "esf_sys_debug_log.h"
#include "esf_sys_error.h"

namespace esf { namespace sys {

class ESFObject
{
public:
    enum ESF_OBJECT_ERROR
    {
        ESF_OBJECT_ERROR_BASE = -1,   
        ESF_OBJECT_ERROR_LOG_INIT_FAIL = ESF_OBJECT_ERROR_BASE -1,   //初始化日志模块失败
        ESF_OBJECT_ERROR_ERR_INIT_FAIL = ESF_OBJECT_ERROR_BASE -2,   //初始化Error模块失败
    };
public:

    ESFObject() 
    {
    }
    virtual ~ESFObject()
    {
        fini();
    }
    virtual int init()     
    {
        if (DEBUG_INIT() != 0)
        {
            return ESF_OBJECT_ERROR_LOG_INIT_FAIL;
        }
        if (ESF_ERROR_INIT() != 0)
        {
            return ESF_OBJECT_ERROR_ERR_INIT_FAIL;
        }
        return 0;
    }
    virtual void fini()     
    {
        ESF_ERROR_FINI();
        DEBUG_FINI();
    }
};

}}

#endif //
