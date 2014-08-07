
#ifndef __ESF_SYS_SO_H__
#define __ESF_SYS_SO_H__

#include <dlfcn.h>
#include <string>
//////////////////////////////////////////////////////////////////////////

namespace esf { namespace sys {
	
	class CSOFile
	{
	public:
		CSOFile() : _handle(NULL){}
		~CSOFile()
		{
			dlclose(_handle);
			_handle = NULL;
		}
		
		int open(const std::string& so_file_path)
		{
			if (_handle)
			{
				dlclose(_handle);
				_handle = NULL;
			}
			
			_handle = dlopen (so_file_path.c_str(), RTLD_LAZY);
			if(!_handle)
			printf("fail dlopen: %s error:%s\n", so_file_path.c_str(), dlerror());
			assert(_handle);
			
			_file_path = so_file_path;
			return 0;
			
		}
		void* get_func(const std::string& func_name)
		{
			dlerror();    /* Clear any existing error */
			void* ret = dlsym(_handle, func_name.c_str());
			
			char *error;
			if ((error = dlerror()) != NULL)
				return NULL;
			else
				return ret;
		}
		
	private:
		std::string _file_path;
		void* _handle;
	};
	
}}

//////////////////////////////////////////////////////////////////////////
#endif
///:~
