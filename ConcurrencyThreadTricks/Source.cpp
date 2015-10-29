#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <string>
#include <tuple>  
#include <map>
#include <queue>
#include <vector>
#include <future>
#include <list>
#include <random>
#include "settingsHeader.h"

extern std::shared_ptr<std::map<uint64_t, uint64_t>> main_func(long long value);

template <class t1>
class _reader_thread
{
	volatile bool _stop_read_thread;
	volatile bool _finished;
	std::thread _th;
	std::mutex _mu;
	std::mutex _emp_mu;
	std::condition_variable _var;
	std::queue<t1> q;				//shared

	void operator()()
	{
	#ifdef INPUT_WITH_IFSTREAM
		std::ifstream in(inputFileName);
		while (!_stop_read_thread)
		{
			t1 tmp;
			try
			{
				if (in.eof())
				{ 
					_stop_thread = true; 
				}
				in >> tmp;
				if (in.fail())
				{
					in.clear();
					char tmp;
					in >> tmp;
					std::cout << "Not a num. Skipping.." << std::endl;
				}
				pushQueue(tmp);
			}
			catch (std::exception &) {};
		}

		while (isNotEmpty())
		{
			_var.notify_one();
		}

		_finished = true;
		_var.notify_all();
	#else 
		#ifdef INPUT_WITH_STDIN
		for (auto i = 0; i < stdinMaxCount; ++i)
		{
			t1 tmp;
			try
			{
				std::cin >> tmp;
				if (std::cin.fail())
				{
					std::cin.clear();
					char tmp;
					std::cin >> tmp;
					std::cout << "Not a num. Skipping.." << std::endl;
				}
				pushQueue(tmp);
			}
			catch (std::exception &) {};
		}
		
		while (isNotEmpty())
		{
			_var.notify_one();
		}
		_finished = true;
		_var.notify_all();

		#endif
	#endif
	}

public:
	_reader_thread()
		: _th(&_reader_thread::operator(), this)
	{
		_stop_read_thread = false;
	}

	void pushQueue(t1 elem)
	{
		std::unique_lock<std::mutex> lg(_mu);
		q.push(elem);
		lg.unlock();
		_var.notify_one();
	}

	t1 popQueue(void)
	{
		std::unique_lock<std::mutex> lg(_mu);
		_var.wait(lg);

		t1 _elem;
		if (!q.empty())
		{
			_elem = q.front();
			q.pop();
		}
		lg.unlock();
		return _elem;
	}
	
	volatile bool isNotEmpty()
	{
		//std::lock_guard<std::mutex> lg(_emp_mu);
		volatile bool result = !(q.empty());
		return result;
	}

	volatile bool isFinished()
	{
		return _finished;
	}
};

void _process_thread(_reader_thread<uint64_t> & th1, std::promise< std::map < uint64_t, std::map<uint64_t, uint64_t> > > & p)
{
	std::map < uint64_t, std::map<uint64_t, uint64_t> > _th_map;

	while (! th1.isFinished())
	{
		uint64_t	_val = th1.popQueue();
		auto		_map = main_func(_val);
		_th_map[_val] = *_map;

		#ifdef THREAD_WORK_PRINT
		{
			std::lock_guard<std::mutex> lock(cout_mut);
			std::cout << "Thread id " << std::this_thread::get_id() << " is working" << std::endl;
			//std::cout << "Readed value: " << _val << std::endl;
			//for (auto _val : *_map)
			//{
			//	std::cout << "Prime number: " << _val.first << "\t\tPower: " << _val.second << std::endl;
			//}
			//std::cout << std::endl;
		}
		#endif
	}

	p.set_value(std::move(_th_map));
}

#ifdef MAKE_INPUT_FILE
void stdout_thread()
{
	uint64_t seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine gen(seed);
	std::uniform_int_distribution<uint64_t> dist(0, (UINT64_MAX >> 32));
	std::ofstream out(inputFileName, std::ofstream::out | std::ofstream::app);

	for (uint64_t i = 0; i < numCount; ++i)
	{
		out << dist(gen) << std::endl;
	}
	printf("End\n");
}
#endif

int main()
{
#ifdef MAKE_INPUT_FILE
	stdout_thread();
#else
	
	int thread_count = 6;

	_reader_thread<uint64_t> th1;

	std::map < uint64_t, std::map<uint64_t, uint64_t> > full_map;
	std::list<std::promise<std::map < uint64_t, std::map<uint64_t, uint64_t> > > * > promises_list;
	std::list<std::future<std::map < uint64_t, std::map<uint64_t, uint64_t> > > > futures_list;
	std::list<std::thread *> threads;

	for (int i = 0; i < thread_count; ++i)
	{
		auto elem_promise = new std::promise< std::map < uint64_t, std::map<uint64_t, uint64_t> > >();
		auto _th1 = new std::thread(_process_thread, std::ref(th1), std::ref(* elem_promise));
		threads.push_back(_th1);
		promises_list.push_back(elem_promise);
	}
	
	for (auto & thread : threads)
	{
		thread->join();
	}

	std::cout << "Threads processing is ended. Now is merging and saving" << std::endl;

	for (auto & promiseElem : promises_list)
	{
		auto tmpMap = promiseElem->get_future().get();
		
		for (auto & map : tmpMap)
		{
			full_map[map.first] = map.second;
		}
	}
	
	#ifdef OUTPUT_WITH_OFSTREAM
	std::ofstream out(outputFileName, std::ofstream::trunc);
	for (auto & _val : full_map)
	{
		out << "Readed value: " << _val.first << std::endl;
		for (auto & __val : _val.second)
		{
			out << "Prime number: " << __val.first << "\t\tPower: " << __val.second << std::endl;
		}
		out << std::endl << std::endl;
	}
	#else 
	#ifdef OUTPUT_WITH_STDOUT
	for (auto & _val : full_map)
	{
		std::cout << "Readed value: " << _val.first << std::endl;
		for (auto & __val : _val.second)
		{
			std::cout << "Prime number: " << __val.first << "\t\tPower: " << __val.second << std::endl;
		}
		std::cout << std::endl << std::endl;
	}

	#endif
	#endif

	std::cout << "All operations completed" << std::endl;
#endif

//	system("pause");
	return 0;
}

