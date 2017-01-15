#include "maintrade.h"
#include <QtWidgets/QApplication>
#include "MainEngine.h"

#include <vector>
#include <algorithm>
#include <iostream>

#include "TechIndicator.h"
#include "GlogWrapper.h"

// 全局主引擎，后期可改造成单例
MainEngine* me = nullptr;


int main(int argc, char *argv[])
{
	// 初始化日志
	CGLog::get_glog()->init_log("./log/");

	//GLOG("Hello, world_info", CGLog::CGLog_INFO);
	//GLOG("Hello, world_warning", CGLog::CGLog_WARNING);
	//GLOG("Hello, world_error", CGLog::CGLog_ERROR);
	/* GLOG("Hello, world_fatal", CGLog::CGLog_FATAL);	// 有问题？ */

	// 初始化talib
	TechIndicator::initialize();
	
	// 开启主进程
	QApplication app(argc, argv);
	me = new MainEngine();

	MainTrade win;

	// 开启QT事件循环
	win.show();
	app.exec();

	if (me)
	{
		delete me;
		me = nullptr;
	}

	// 关闭talib
	TechIndicator::taShutdown();
	// 关闭日志
	CGLog::get_glog()->release_log();

	return 0;
}
