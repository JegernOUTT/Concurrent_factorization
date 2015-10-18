#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include <tuple>  
#include <map>
#include <queue>
#include <vector>
#include <future>
#include <list>

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
		while (!_stop_thread)
		{
			t1 tmp;
			try
			{
				std::cin >> tmp;
				pushQueue(tmp);
			}
			catch (std::exception &) {};
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
		_var.wait(lg, [&]() {return !q.empty();});
		auto _elem = q.front();
		q.pop();
		lg.unlock();
		return _elem;
	}
};

void _process_thread(_reader_thread<uint64_t> & th1, std::promise< std::map < uint64_t, std::map<uint64_t, uint64_t> > > && p)
{
	std::map < uint64_t, std::map<uint64_t, uint64_t> > _th_map;
	std::mutex mu;
	while (true)
	{/*здесь составление std::map < uint64_t, std::map<uint64_t, uint64_t> > full_map;*/
		uint64_t	_val = th1.popQueue();
		auto		_map = main_func(_val);
		_th_map[_val] = *_map;

		{
			std::lock_guard<std::mutex> lock(mu);
			std::cout << "From thread id: " << std::this_thread::get_id() << std::endl;
			std::cout << "Readed value: " << _val << std::endl;
			for (auto _val : *_map)
			{
				std::cout << "Prime number: " << _val.first << "\t\tPower: " << _val.second << std::endl;
			}
			std::cout << std::endl;
		}
	}

	p.set_value(std::move(_th_map));
}

int main()
{
	int thread_count = 4;

	_reader_thread<uint64_t> th1;

	std::list<std::promise<std::map < uint64_t, std::map<uint64_t, uint64_t> > > > promises_list;
	std::list<std::future<std::map < uint64_t, std::map<uint64_t, uint64_t> > > > futures_list;
	//std::list<std::thread> threads;

	for (int i = 0; i < thread_count; ++i)
	{
		auto elem_promise = new std::promise< std::map < uint64_t, std::map<uint64_t, uint64_t> > >;
		auto elem_future = elem_promise->get_future();
		promises_list.push_back(* elem_promise);
		futures_list.push_back(elem_future);

		//auto elem_thread = new std::thread(&_process_thread, std::ref(th1), std::move(* elem_promise));
		//threads.push_back(* elem_thread);
	}
	
	//for (auto & thread : threads)
	//{
	//	thread.join();
	//}


	/*
	Здесь объединение std::map < uint64_t, std::map<uint64_t, uint64_t> > full_map;
	Вывод на экран
	*/

	getchar();
	return 0;
}