#pragma once

#define GOOGLE_GLOG_DLL_DECL				 // be used by static glog lib
//#define GLOG_NO_ABBREVIATED_SEVERITIES	// it will cause re-definition because of <windows.h>

#include "glog/include/logging.h"

#ifdef _DEBUG
#pragma comment(lib,"glog/lib/Debug/libglog_static.lib") 
#else
#pragma comment(lib,"glog/lib/Release/libglog_static.lib")
#endif

class CGLog
{
public:
	enum 
	{
		CGLog_INFO		= 0,
		CGLog_WARNING,
		CGLog_ERROR, 
		CGLog_FATAL
	};

	inline static CGLog* get_glog()
	{
		return const_cast<CGLog*>(google_log);
	}

	// 初始化日志
	inline void init_log(const std::string& log_dir)
	{
		google::InitGoogleLogging("MainTrade");
		set_default_setting(log_dir);
	}

	// 关闭日志
	inline void release_log()
	{
		google::ShutdownGoogleLogging();
	}

	// 打印日志
	void print_log(char* out_msg, int severity);

private:
	void set_default_setting(const std::string& log_dir);

private:
	class GcGlog
	{
	public:
		~GcGlog()
		{
			if (google_log != nullptr)
			{
				delete google_log;
				google_log = nullptr;
			}
		}
	};

	static const CGLog* google_log;
	static const GcGlog gc_log;
};

// str: 日志信息; svt: 严重程度
#define GLOG(str, svt) CGLog::get_glog()->print_log(str, svt);