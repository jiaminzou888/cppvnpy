#pragma once

#include <qobject.h>
#include <qthread.h>
#include <qtimer.h>

#include "CallBack.h"
#include "cpp_basictool/CppQueue.hpp"


class EventEngine: public QThread
{
	Q_OBJECT
public:
	void ee_begin();
	void ee_stop();

	void putEvent(Event event);

public:
	template<typename T>
	void addEvent(std::string type, T* pObj, void (T::*pMemberFunc)(Event));
	void removeEvent(std::string type);

private:
	QTimer* __timer{ nullptr };
	ConcurrentQueue<Event> __queue;

	std::atomic<bool> __active_put{ false };
	std::atomic<bool> __active_thread{ false };
	
	std::mutex handlers_mutex;
	std::map < std::string, std::list<std::shared_ptr<CallBack>>> __handlers;
	
private slots:
	void onTimer();

private:
	void run()Q_DECL_OVERRIDE;
	void process(Event event);
};


template<typename T>
void EventEngine::addEvent(std::string type, T* pObj, void (T::*pMemberFunc)(Event))
{
	std::lock_guard<std::mutex> lock(handlers_mutex);

	// 判断event_type是否存在，不存在则初始化map
	if (__handlers.find(type) == __handlers.end())
	{
		std::list<std::shared_ptr<CallBack>> list = {};
		__handlers[type] = list;
	}

	// 判断 pObj + T::*pMemberFunc 是否已经包含在 event_type 的回调列表中，已存在则直接return
	// 但无法判断是否已包含 pObj + T::*pMemberFunc ， 需要程序员人工保证

	// push_back CallBack
	__handlers[type].push_back(std::make_shared<CallBack>(pObj, pMemberFunc));
}