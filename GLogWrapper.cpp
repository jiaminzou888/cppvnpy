#include "GLogWrapper.h"

const CGLog*		 CGLog::google_log = new CGLog();
const CGLog::GcGlog  CGLog::gc_log;

void CGLog::set_default_setting(const std::string& log_dir)
{
#ifdef _DEBUG
	//设置日志路径  INFO WARNING ERROR FATAL 
	std::string sub_dir = log_dir + "info/";
	google::SetLogDestination(google::INFO, sub_dir.c_str());	
	sub_dir = log_dir + "warning/";
	google::SetLogDestination(google::WARNING, sub_dir.c_str());
	sub_dir = log_dir + "error/";
	google::SetLogDestination(google::ERROR, sub_dir.c_str());
	sub_dir = log_dir + "fatal/";
	google::SetLogDestination(google::FATAL, sub_dir.c_str());

	FLAGS_logtostderr = false;									//是否将所有日志输出到 stderr，而非文件
	FLAGS_alsologtostderr = false;								//是否同时将日志输出到文件和stderr
	FLAGS_minloglevel = google::INFO;							//限制输出到 stderr 的部分信息，包括此错误级别和更高错误级别的日志信息 
	FLAGS_stderrthreshold = google::INFO;						//除了将日志输出到文件之外，还将此错误级别和更高错误级别的日志同时输出到 stderr，这个只能使用 -stderrthreshold=1 或代码中设置，而不能使用环境变量的形式。（这个参数可以替代上面两个参数）
	FLAGS_colorlogtostderr = false;								//将输出到 stderr 上的错误日志显示相应的颜色 
	FLAGS_max_log_size = 10;									//最大日志文件大小10M.
	//vmodule(string, default = "");　　						//分文件（不包括文件名后缀，支持通配符）设置自定义日志的可输出级别，如：GLOG_vmodule=server=2,client=3 表示文件名为server.* 的只输出小于 2 的日志，文件名为 client.* 的只输出小于 3 的日志。如果同时使用 GLOG_v 选项，将覆盖 GLOG_v 选项。
	//google::SetLogFilenameExtension("log_");
#else
	//设置日志路径  INFO WARNING ERROR FATAL 
	std::string sub_dir = log_dir + "info/";
	google::SetLogDestination(google::INFO, sub_dir.c_str());
	sub_dir = log_dir + "warning/";
	google::SetLogDestination(google::WARNING, sub_dir.c_str());
	sub_dir = log_dir + "error/";
	google::SetLogDestination(google::ERROR, sub_dir.c_str());
	sub_dir = log_dir + "fatal/";
	google::SetLogDestination(google::FATAL, sub_dir.c_str());

	FLAGS_logtostderr = false;
	FLAGS_alsologtostderr = false;
	FLAGS_minloglevel = google::INFO;
	FLAGS_stderrthreshold = google::ERROR;
	FLAGS_colorlogtostderr = false;
	FLAGS_max_log_size = 10;
	//vmodule(string, default = "")
#endif
}

void CGLog::print_log(char* out_msg, int severity)
{
	switch (severity)
	{
	case CGLog_INFO:
		LOG(INFO) << out_msg;
		break;
	case CGLog_WARNING:
		LOG(WARNING) << out_msg;
		break;
	case CGLog_ERROR:
		LOG(ERROR) << out_msg;
		break;
	case CGLog_FATAL:
		LOG(FATAL) << out_msg;
		break;
	default:
		LOG(INFO) << out_msg;
		break;
	}
}