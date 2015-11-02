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

extern std::unique_ptr< std::map<uint64_t, uint64_t> > main_func(long long value);

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
					_stop_read_thread = true;
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
			catch (std::exception & e) 
			{ 
				std::cout << e.what() << std::endl; 
				throw;
			};
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

	void pushQueue(t1 & elem)
	{
		//std::unique_lock<std::mutex> lg(_mu);
		q.push(elem);
		//lg.unlock();
		_var.notify_one();
	}

	volatile bool isNotEmpty()
	{
		//std::lock_guard<std::mutex> lg(_emp_mu);
		volatile bool result = !(q.empty());
		return result;
	}

public:
	_reader_thread()
		: _th(&_reader_thread::operator(), this)
	{
		_stop_read_thread = false;
	}

	std::unique_ptr<t1> popQueue(void)
	{
		std::unique_lock<std::mutex> lg(_mu);
		_var.wait(lg);

		auto _elem  = std::make_unique<t1>(sizeof(t1));
		if (!q.empty())
		{
			* _elem = q.front();
			q.pop();
		}
		lg.unlock();
		return std::move(_elem);
	}
	
	volatile bool isFinished()
	{
		return _finished;
	}

	void join()
	{
		_th.join();
	}
};

template <class t1>
void _process_thread(_reader_thread<t1> & th1, std::promise< std::map < t1, std::map<t1, t1> > > & p)
{
	std::map < t1, std::map<t1, t1> > _th_map;

	while (! th1.isFinished())
	{
		auto	_val = th1.popQueue();
		if (_val != nullptr)
		{
			auto		_map = main_func((long long) * _val);
			_th_map[* _val] = *_map;
		}
		
		#ifdef THREAD_WORK_PRINT
		{
			std::lock_guard<std::mutex> lock(cout_mut);
			std::cout << "Thread id " << std::this_thread::get_id() << " is working" << std::endl;
			//std::cout << "Readed value: " << * _val << std::endl;
			//for (auto __val : *_map)
			//{
			//	std::cout << "Prime number: " << __val.first << "\t\tPower: " << __val.second << std::endl;
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

int main(int argc, char * argv[])
{
#ifdef MAKE_INPUT_FILE
	stdout_thread();
#else
	_reader_thread<uint64_t> th1;

	std::map < uint64_t, std::map<uint64_t, uint64_t> > full_map;
	std::list<std::promise<std::map < uint64_t, std::map<uint64_t, uint64_t> > > * > promises;
	std::list<std::thread *> threads;

	for (int i = 0; i < thread_count; ++i)
	{
		auto elem_promise = new std::promise< std::map < uint64_t, std::map<uint64_t, uint64_t> > >();
		auto _th1 = new std::thread(_process_thread<uint64_t>, std::ref(th1), std::ref(* elem_promise));
		threads.push_back(_th1);
		promises.push_back(elem_promise);
	}
	
	for (auto & thread : threads)
	{
		thread->join();
	}
	th1.join();

	std::cout << "Threads processing is ended. Now is merging and saving" << std::endl;

	for (auto & promiseElem : promises)
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

	getchar();
	return 0;
}

