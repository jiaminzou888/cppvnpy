#pragma once

#include <atomic>
#include <functional>
#include <thread>

using function_type = void(*)(void*);
using id_type		= std::thread::id;

class CppThread
{
public:
	inline static void controll_function(void* data)
	{
		assert(data);
		CppThread* thread = (CppThread*)data;

		thread->_stop.store(false);
		thread->_exit.store(false);

		assert(thread->_function);
		thread->_function(thread);

		thread->_exit.store(true);
	}

	inline id_type get_id()
	{
		return _self.get_id();
	}

	inline bool create_thread(function_type funct)
	{
		// business function outside
		_function = funct;

		// movable construct transformation
		_self = std::move(std::thread(controll_function, this));

		return true;
	}

	inline void close_thread()
	{
		_stop.store(true);
		_self.join();
	}

	inline void	set_data(void* data)
	{
		_data.store(data);
	}

	inline void* get_data()
	{
		return _data.load();
	}

	inline bool is_stop()
	{
		return _stop.load();
	}

	inline void set_stop(bool flag)
	{
		_stop.store(flag);
	}

	inline void set_thread_index(size_t idx)
	{
		_index.store(idx);
	}
	
	inline size_t get_thread_index()
	{
		return _index.load();
	}

private:
	std::thread			_self;						// real thread object
	function_type		_function{ nullptr };		// real thread function
	std::atomic<void*>	_data{ nullptr };

	std::atomic<bool>	_stop{ true };
	std::atomic<bool>	_exit{ true };

	std::atomic<size_t>	_index{ 0 };	// several threads flag which call the same function
};