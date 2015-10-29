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

extern std::shared_ptr<std::map<uint64_t, uint64_t>> main_func(long long value);


template <class t1>
class _reader_thread
{
	volatile bool _stop_thread;
	std::thread _th;
	std::mutex _mu;
	std::condition_variable _var;
	std::queue<t1> q;				//shared

	void operator()()
	{
		std::ifstream in("E:\\tmp.txt");
		while (!_stop_thread)
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
					std::cout << "Input for numbers only" << std::endl;
				}
				pushQueue(tmp);
			}
			catch (std::exception &) {};
			//std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		while (!q.empty())
		{
			_var.notify_one();
		}
	}

public:
	_reader_thread()
		: _th(&_reader_thread::operator(), this)
	{
		_stop_thread = false;
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
		auto _elem = q.front();
		q.pop();
		lg.unlock();
		return _elem;
	}
	
	volatile bool isAlive()
	{
		volatile bool result = q.size() > 0;
		return result;
	}
};

std::mutex mut;
void _process_thread(_reader_thread<uint64_t> & th1, std::promise< std::map < uint64_t, std::map<uint64_t, uint64_t> > > & p)
{
	std::map < uint64_t, std::map<uint64_t, uint64_t> > _th_map;

	while (th1.isAlive())
	{/*здесь составление std::map < uint64_t, std::map<uint64_t, uint64_t> > full_map;*/
		uint64_t	_val = th1.popQueue();
		auto		_map = main_func(_val);
		_th_map[_val] = *_map;

		{
			//std::lock_guard<std::mutex> lock(mut);
			//std::cout << "From thread id: " << std::this_thread::get_id() << std::endl;
			//std::cout << "Readed value: " << _val << std::endl;
			//for (auto _val : *_map)
			//{
			//	std::cout << "Prime number: " << _val.first << "\t\tPower: " << _val.second << std::endl;
			//}
			//std::cout << std::endl;
		}
	}

	p.set_value(std::move(_th_map));
}

void stdout_thread(std::string && name)
{
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine gen(seed);
	std::uniform_int_distribution<uint64_t> dist(0, (UINT64_MAX >> 4));
	std::ofstream out(name, std::ofstream::out | std::ofstream::app);

	for (uint64_t i = 0; i < 10000000ul; ++i)
	{
		out << dist(gen) << std::endl;
	}
	printf("End\n");
}


int main()
{
	//stdout_thread("E:\\tmp.txt");
	
	
	int thread_count = 4;

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

	std::cout << "threads end" << std::endl;

	for (auto & promiseElem : promises_list)
	{
		auto tmpMap = promiseElem->get_future().get();
		
		for (auto & map : tmpMap)
		{
			full_map[map.first] = map.second;
		}
	}
	
	std::ofstream out("E:\\out.txt");
	std::cout << std::endl << std::endl << std::endl << std::endl << std::endl << std::endl;
	for (auto & _val : full_map)
	{
		out << "Readed value: " << _val.first << std::endl;
		for (auto & __val : _val.second)
		{
			out << "Prime number: " << __val.first << "\t\tPower: " << __val.second << std::endl;
		}
		out << std::endl << std::endl;
	}
	std::cout << "End";
	getchar();

	
	return 0;
}

