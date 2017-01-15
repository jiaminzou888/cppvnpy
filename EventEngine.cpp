#include "EventEngine.h"

#include <thread>

void EventEngine::run()
{
	while (__active_thread.load())
	{
		// 获取任务
		Event evt;
		if (__queue.wait_and_pop(evt, -1))
		{
			process(evt);
		}
	}
}

void EventEngine::process(Event event)
{
	std::lock_guard<std::mutex> lock(handlers_mutex);

	if (__handlers.find(event.type_) != __handlers.end())
	{
		std::list<std::shared_ptr<CallBack>>& handlerList = __handlers[event.type_];
		std::for_each(handlerList.cbegin(), handlerList.cend(), [&event](const std::shared_ptr<CallBack>& func_ptr)
		{
			func_ptr->Execute(event);
		});
	}
}

void EventEngine::onTimer()
{
	putEvent(Event(EVENT_TIMER));
}

void EventEngine::ee_begin()
{
	// 启动定时器线程
	__timer = new QTimer(this);
	__timer->setSingleShot(false);
	connect(__timer, SIGNAL(timeout()), this, SLOT(onTimer()), Qt::DirectConnection);
	__timer->start(1000);

	// 打开队列开关
	__active_put.store(true);

	// 打开事件引擎线程
	__active_thread.store(true);
	start();
}

void EventEngine::ee_stop()
{
	// 关闭定时器线程
	__timer->stop();

	// 关闭队列开关
	__active_put.store(false);
	while (!__queue.empty())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	// 关闭事件引擎线程
	__active_thread.store(false);
	__queue.notify_all(); // 通知等待在空队列上的线程

	this->quit();
	this->wait();
}

void EventEngine::removeEvent(std::string type)
{
	// 因为无法区分特定 obj + obj_ordinary_function, 故Event关系按type整体移除
	std::lock_guard<std::mutex> lock(handlers_mutex);

	// 判断event_type是否存在，存在则删除事件关联
	auto handle_list = __handlers.find(type);
	if (handle_list != __handlers.end())
	{
		__handlers.erase(handle_list);
	}
}

void EventEngine::putEvent(Event event)
{
	if (__active_put.load())
	{
		__queue.push_back(event);
	}
}