#include "esf_sys_debug_log.h"


ESFDebugLog* ESFDebugLog::instance_ = NULL;

ESFDebugLog::ESFDebugLog()
{
    log_level_ = LOG_TRACE;
    log_type_ = LOG_TYPE_NORMAL;

    file_open_ = false;
    file_ = stdout;

    pre_time_ = time(NULL);

    max_file_size_ = DEFAULT_MAX_FILE_SIZE;
    max_file_no_ = DEFAULT_MAX_FILE_NO;

    cur_file_size_ = 0;
    cur_file_no_ = 1;
}

ESFDebugLog::~ESFDebugLog()
{
    close_file();
}

void ESFDebugLog::close_file()
{
    if (file_open_)
    {
        fclose(file_);
        file_ = stdout;
        file_open_ = false;
    }
}

void ESFDebugLog::flush_file(void)
{
    if (file_open_)
    {
        fflush(file_);
    }
}

ESFDebugLog* ESFDebugLog::instance()
{
    if (instance_ == NULL)
    {
        instance_ = new  ESFDebugLog;
    }
    return instance_;
}

int ESFDebugLog::init()
{
    if (instance() == NULL)
    {
        return -1;
    }
    return 0;
}

void ESFDebugLog::fini()
{
    if (instance_ != NULL)
    {
        delete instance_;
        instance_ = NULL;
    }
}

int ESFDebugLog::open(int log_level, int log_type, char* path, 
                    char* name_prefix, int max_file_size, int max_file_no)
{
    log_level_ = log_level;
    log_type_ = log_type;
    if (path == NULL || name_prefix == NULL)
    {
        //表示使用stdout
        close_file();
        name_prefix_ = "";
        path_ = "";
    }
    else
    {
        name_prefix_ = name_prefix;
        path_ = path;
        if (path_[path_.length()-1] == DIR_SEPERATOR)
        {
            path_[path_.length()-1] = 0;
        }
    }

    max_file_size_ = max_file_size;
    if (max_file_size_ > DEFAULT_MAX_FILE_SIZE) { max_file_size_ = DEFAULT_MAX_FILE_SIZE; };

    max_file_no_ = max_file_no;

    init_cur_file_no();
    
    return open_file();
}

int ESFDebugLog::init_cur_file_no()
{
    cur_file_no_ = 1;
    if (name_prefix_.length() == 0) { return 0; /*直接使用stdout*/}
    if (log_type_ == LOG_TYPE_NORMAL) { return 0; /*普通模式不需要编号*/}

    //查找当前最大编号
    char base_file[MAX_LOG_NAME_LEN];
    build_file_name(base_file);

    int file_no = 1;
    time_t min_modify_time = 0;

    int cur_time = time(NULL);
    
    for (int i = 1; ; i++)
    {
        char dest_file[MAX_LOG_NAME_LEN];
        snprintf(dest_file, MAX_LOG_NAME_LEN, "%s.%d", base_file, i);
        dest_file[MAX_LOG_NAME_LEN -1] = 0;

        cur_time = time(NULL);
#ifdef _SHOWINFO_
        ESFDebugLog::instance()->log_i(LOG_DEBUG, &cur_time, "stat file:%s \n", dest_file);
#endif
        struct stat sb;
        if (stat (dest_file, &sb) != 0)
        {
            //文件不存在, 使用该编号
            file_no = i;
            cur_time = time(NULL);
#ifdef _SHOWINFO_
            ESFDebugLog::instance()->log_i(LOG_DEBUG, &cur_time, "dest_file not exist:%s \n", dest_file);
#endif      
		break;  
        }
        if (LOG_TYPE_CYCLE == log_type_)
        {
            if ( i >= max_file_no_)
            {
                break;
            }
            //循环日志, 需要按最后修改时候进行排序
            if (min_modify_time == 0 || sb.st_mtime < min_modify_time)
            {
                file_no = i;
                min_modify_time = sb.st_mtime;
            }
        }
    }

    cur_file_no_ = file_no;
    cur_time = time(NULL);
#ifdef _SHOWINFO_
    ESFDebugLog::instance()->log_i(LOG_NORMAL, &cur_time, "cur_file_no:%d \n", cur_file_no_);
#endif
    return 0;
}


int ESFDebugLog::open(int log_level, int log_type, string path, string name_prefix, 
            int max_file_size, int max_file_no)
{
    if (path.length() <= 0 || name_prefix.length() <=0)
    {
        return open(log_level, log_type, NULL, NULL, max_file_size, max_file_no);
    }
    return open(log_level, log_type, (char*)path.c_str(), (char*)name_prefix.c_str(), max_file_size, max_file_no);
}

int ESFDebugLog::build_file_name(char* pathname, string* filename)
{
    char time_str[50];
    GetPackTimeFormat(pre_time_, time_str);
    switch (log_type_)
    {    
    case LOG_TYPE_DAILY:
        {
            time_str[8] = '.'; //只需要到天数
            time_str[9] = 0;
            break;
        }
    case LOG_TYPE_HOURLY:
        {
            time_str[10] = '.'; //只需要到小时
            time_str[11] = 0;
            break;
        }
    default:
        {
            time_str[0] = 0; //不需要记录时间
            break;
        }
    }
    
    string file_name = name_prefix_ + "." + time_str+ "log";
    if (filename != NULL)
    {
        *filename = file_name;
    }
    if (pathname != NULL)
    {
        snprintf(pathname, MAX_LOG_NAME_LEN, "%s%c%s", 
                    path_.c_str(), DIR_SEPERATOR, file_name.c_str());
        pathname[MAX_LOG_NAME_LEN-1]=0;
    }
    return 0;
}

int ESFDebugLog::open_file()
{
    if (file_open_) { return 0; }
    if (name_prefix_.length() == 0) { return 0; /*直接使用stdout*/}

    char pathname[MAX_LOG_NAME_LEN];
    build_file_name(pathname, &filename_);

    if ((file_ = fopen(pathname, "a+")) == NULL)
    {
        file_ = stdout;
        file_open_ = false;
        name_prefix_ = "";
        int cur_time = time(NULL);
        ESFDebugLog::instance()->log_i(LOG_FATAL, &cur_time, "log fopen fail:%s\n", pathname);
	return -1;
    }

    file_open_ = true;
    
    struct stat sb;
    if (stat (pathname, &sb) < 0)
    {
        cur_file_size_ = 0; 
    }
    else
    {
        cur_file_size_ = (int) sb.st_size;
    }
    
    return 0;
}

#ifdef WIN32
struct tm* localtime_r(time_t* t, struct tm* stm)
{
    memcpy(stm, localtime(t), sizeof(struct tm));
    return stm;
}
#endif

int ESFDebugLog::force_rename(const char* src_pathname, const char* dest_pathname)
{
    remove(dest_pathname);
    if (rename(src_pathname, dest_pathname) != 0) { return -1; }
    return 0;
}

int ESFDebugLog::shift_file(int cur_time)
{
    if (name_prefix_.length() == 0)
    {
        //未定义文件名前缀, 表示使用stdout, 不需要进行文件切换
        return 0;
    }
    if (log_type_ == LOG_TYPE_NORMAL)
    {
        //普通日志, 不需要滚动
        return 0;
    }

    bool need_shift = false;
    
    struct tm cur_tm ;
    struct tm pre_tm ;
    localtime_r((time_t*)&cur_time, &cur_tm);
    localtime_r((time_t*)&pre_time_, &pre_tm);

    int new_file_no = 0;
    
    //先进行时间检查
    switch (log_type_)
    {    
    case LOG_TYPE_DAILY:
        {
            if (cur_tm.tm_yday != pre_tm.tm_yday)
            {
                //天数不等, 说明日期已经切换
                need_shift = true;
                new_file_no = 1;
            }
            break;
        }
    case LOG_TYPE_HOURLY:
        {
            if ((cur_tm.tm_yday != pre_tm.tm_yday) || (cur_tm.tm_hour != pre_tm.tm_hour))
            {
                //天数或小时不等, 说明小时已经切换
                need_shift = true;
                new_file_no = 1;
            }
            break;
        }
    default:
        break;
        
    }

    //再进行文件SIZE检查
    if (need_shift == false)
    {
        if (cur_file_size_ >= max_file_size_)
        {
            need_shift = true;
            new_file_no = cur_file_no_ + 1;
            if ((log_type_ == LOG_TYPE_CYCLE) && (new_file_no > max_file_no_))
            {
                //滚动日志, 需要检查最大文件编号
                new_file_no = 1;
            }
        }
    }

    if (need_shift == false)
    {
        return 0; //不需要进行文件切换
    }

    //move原文件
    close_file();

    char src_file[MAX_LOG_NAME_LEN];
    snprintf(src_file, MAX_LOG_NAME_LEN, "%s%c%s", 
                path_.c_str(), DIR_SEPERATOR, filename_.c_str());
    src_file[MAX_LOG_NAME_LEN -1] = 0;
    
    char dest_file[MAX_LOG_NAME_LEN];
    snprintf(dest_file, MAX_LOG_NAME_LEN, "%s.%d", src_file, cur_file_no_);
    dest_file[MAX_LOG_NAME_LEN -1] = 0;

    if (force_rename(src_file, dest_file) < 0 )
    {
        ESFDebugLog::instance()->log_i(LOG_FATAL, &cur_time, "log rename fail (%d): %s ==> %s \n", errno, filename_.c_str(), dest_file);
        return -1;
    }

    //打开新文件
    pre_time_ = cur_time;
    cur_file_no_ = new_file_no;
    return open_file();
}

void ESFDebugLog::log_level(int level)
{
#ifndef WIN32
    MutexGuard g(p_mutex_);
#endif
    log_level_ = level;
}
int  ESFDebugLog::log_level(void)
{
#ifndef WIN32
    MutexGuard g(p_mutex_);
#endif
	return log_level_;
}
void ESFDebugLog::log_p(int log_level, const char* FMT, ...)
{
#ifndef WIN32
    MutexGuard g(p_mutex_);
#endif
    if (log_level < log_level_)   {  return ;  }
    int cur_time = time(NULL);
    shift_file(cur_time);

    va_list ap;
    va_start(ap, FMT);
    log_i_v(log_level, &cur_time, FMT, ap);
    va_end(ap);
}
void ESFDebugLog::log_p_no_time(int log_level, const char* FMT, ...)
{
#ifndef WIN32
    MutexGuard g(p_mutex_);
#endif
    if (log_level < log_level_)   {  return ;  }
    int cur_time = time(NULL);
    shift_file(cur_time);
    
    va_list ap;
    va_start(ap, FMT);
    log_i_v(log_level, NULL, FMT, ap);
    va_end(ap);    
}
void ESFDebugLog::log_buf_p(int log_level, const char *buf, int iLen)
{
#ifndef WIN32
	MutexGuard g(p_mutex_);
#endif
	if (log_level < log_level_)   {  return ;  }
	int cur_time = time(NULL);
	shift_file(cur_time);
	log_buf_i(log_level, &cur_time, buf, iLen);
}

void  ESFDebugLog::log_bin_p(int log_level, const char *buf, int iLen)
{
#ifndef WIN32
	MutexGuard g(p_mutex_);
#endif
	if (log_level < log_level_)   {  return ;  }
	int cur_time = time(NULL);
	shift_file(cur_time);

	if(!buf || iLen <=0)
	{
		return ;
	}	
//////////
	int ret = fwrite(buf, iLen, 1, file_);
	if(ret == 1)
	{
		cur_file_size_ += iLen;
		flush_file();
	}
}


char* ESFDebugLog::GetPackTimeFormat(int iTime, char* szDateTime)
{
    struct tm stTm ;
    localtime_r((time_t*)&iTime, &stTm);
    
    sprintf(szDateTime, "%04d%02d%02d%02d%02d%02d",
        stTm.tm_year+1900, stTm.tm_mon+1, stTm.tm_mday,
        stTm.tm_hour, stTm.tm_min, stTm.tm_sec );
    
    return szDateTime;
};

char* ESFDebugLog::GetPackCurTimeFormat(char* szDateTime)
{
    return GetPackTimeFormat((int)time(NULL), szDateTime);
};

char* ESFDebugLog::GetTimeFormat(int iTime, char* szDateTime)
{
    struct tm stTm ;
    localtime_r((time_t*)&iTime, &stTm);
    
    sprintf(szDateTime, "%04d-%02d-%02d %02d:%02d:%02d",
        stTm.tm_year+1900, stTm.tm_mon+1, stTm.tm_mday,
        stTm.tm_hour, stTm.tm_min, stTm.tm_sec );
    
    return szDateTime;
};

char* ESFDebugLog::GetCurTimeFormat(char* szDateTime)
{
    return GetTimeFormat((int)time(NULL), szDateTime);
};

void ESFDebugLog::log_i(int log_level, int* pt, const char * FMT,...)
{
    if (log_level < log_level_)   {  return; }
        
    //先打印时间
    int ret = 0;
    if (pt != NULL)
    {
        char time_str[50];
        GetTimeFormat(*pt, time_str);
        ret = fprintf(file_, "[%s] ", time_str);
        if (ret > 0)
        {
            cur_file_size_ += ret;
        }            
    }
    //再打印日志
    va_list ap;
    va_start(ap, FMT);
    ret = vfprintf(file_, FMT, ap);
    va_end(ap);
    if (ret > 0)
    {
        cur_file_size_ += ret;
    }    
///???????????
	flush_file();
};

void ESFDebugLog::log_i_v(int log_level, int* pt, const char * FMT, va_list ap)
{
    if (log_level < log_level_)   {  return; }
    
    //先打印时间
    int ret = 0;
    if (pt != NULL)
    {
        char time_str[50];
        GetTimeFormat(*pt, time_str);
        ret = fprintf(file_, "[%s] ", time_str);
        if (ret > 0)
        {
            cur_file_size_ += ret;
        }            
    }
    //再打印日志
    ret = vfprintf(file_, FMT, ap);
    if (ret > 0)
    {
        cur_file_size_ += ret;
    }    
///???????????
	flush_file();
};

void ESFDebugLog::log_buf_i(int log_level, int* pt, const char *buf, int iLen)
{

    if (log_level < log_level_)   {  return; }

	//先打印时间
	int ret = 0;
	if (pt != NULL)
	{
		char time_str[50];
		GetTimeFormat(*pt, time_str);
		ret = fprintf(file_, "[%s] \n", time_str);
		if (ret > 0)
		{
		    cur_file_size_ += ret;
		}            
	}
	//再打印日志

	if(!buf || iLen <=0)
	{
		return ;
	}
	char szLine[128] = {0};;
	char strTemp[16] = {0};
	
	for(int i = 0; i < iLen; i++ )
	{
		if(i ==0)
		{
			sprintf(szLine, "%04d>    ", 0);
		}
		else if( !(i%16))
		{
			ret = fprintf(file_, "%s\n", szLine);
			if(ret > 0)
			{
				cur_file_size_ += ret;
			}
			sprintf(szLine, "%04d>    ", i/16);
		}
		sprintf(strTemp, "%02X ", (unsigned char)buf[i]);
		strcat(szLine, strTemp);
	}
	ret = fprintf(file_, "%s\n", szLine);
	if(ret > 0)
	{
		cur_file_size_ += ret;
	}
///???????????
	flush_file();
}


