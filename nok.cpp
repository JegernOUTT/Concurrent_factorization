#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include <map>
#include <regex>
#include <queue>
#include <future>
#include <list>
#include <memory>
#include <random>

using std::regex;
using std::regex_replace;
using std::string;
using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::map;
using std::queue;
using std::list;
using std::unique_ptr;
using std::shared_ptr;
using std::unique_lock;
using std::thread;
using std::mutex;
using std::condition_variable;
using std::promise;
using std::lock_guard;

typedef map<uint64_t, uint64_t> map_uu;

extern unique_ptr< map_uu > main_func(int64_t value);
mutex cerr_mutex;

template <class t1 = uint64_t>
class reader_thread
{
public:
	reader_thread(uint64_t maxCount) :
		stdinMaxCount(maxCount),
		stop_read_thread(false),
		finished(false),
		r_digit("[^\\d]"),
		th(&reader_thread::operator(), this)
	{
	}

	reader_thread(const reader_thread&) = delete;

	reader_thread& operator=(const reader_thread&) = delete;

	~reader_thread() = default;

	volatile t1 popQueue()
	{
		unique_lock<mutex> lg(mu);
		cond_var.wait(lg);

		if (!q.empty())
		{
			auto u_ptr(q.front());
			q.pop();
			lg.unlock();
			return u_ptr;
		}
		else
		{
			lg.unlock();
			return t1();
		}
	}

	volatile bool isFinished() const
	{
		return finished;
	}

	void join()
	{
		th.join();
	}

	volatile uint64_t getSize() const
	{
		return static_cast<uint64_t>(q.size());
	}
private:
	uint64_t stdinMaxCount;
	volatile bool stop_read_thread;
	volatile bool finished;
	mutex mu;
	regex r_digit;
	condition_variable cond_var;
	queue<t1> q;
	thread th;

	void operator()()
	{
		for (uint64_t i = 0; i < stdinMaxCount; ++i)
		{
			string input;
			cin >> input;

			auto out_reg_str =
				regex_replace(input, r_digit, "");

			if (out_reg_str == "")
			{
				cerr << "Not a number. Skipping" << endl;
			}
			else
			{
				t1 t = stoll(out_reg_str);
				pushQueue(t);
			}
		}

		while (isNotEmpty())
		{
			cond_var.notify_one();
		}

		finished = true;
		cond_var.notify_all();
	}

	void pushQueue(t1 & elem)
	{
		q.push(elem);
		cond_var.notify_one();
	}

	volatile bool isNotEmpty() const
	{
		volatile bool result = !(q.empty());
		return result;
	}
};

template <class t1 = uint64_t>
void process_thread(shared_ptr< reader_thread<t1> > th1, map < t1, unique_ptr< map<t1, t1> > > & _th_map)
{
	shared_ptr<reader_thread<> > lp = th1;
	while (!lp->isFinished())
	{
		auto	val = lp->popQueue();
		if (val != t1())
		{
			auto		_map = main_func(static_cast<int64_t>(val));
			_th_map.insert(std::pair<t1, unique_ptr< map<t1, t1> > >(val, std::move(_map)));
		}

		{
			//lock_guard<mutex> lock(cerr_mutex);
			//cerr << "Thread id " << std::this_thread::get_id() << " is working" << endl;
		}
	}
}

uint32_t findCoresNumber()
{
	auto coresNumber = std::thread::hardware_concurrency();

	if (coresNumber != 0)
	{
		return coresNumber;
	}
	else
	{
		return 4u;
	}
}

bool findExtraThreadStateOption(int argc, char ** argv)
{
	if (argc >= 3)
	{
		for (int i = 0; i < argc; i++)
		{
			string input(argv[i]);
			if (input == "--extra" || input == "-e")
			{
				return true;
			}
		}
	}

	return false;
}

volatile void startStatusThread(bool needToStart, const shared_ptr< reader_thread<> >& _thread)
{
	if (needToStart)
	{
		thread status_thread([] (const shared_ptr< reader_thread<> > _thread)
		{
			shared_ptr< reader_thread<> > main_thread = _thread;

			while (true)
			{
				std::this_thread::sleep_for(std::chrono::seconds(5));
				cerr << "Numbers in queue: " << main_thread->getSize() << endl;
				if (main_thread->getSize() == 0)
				{
					break;
				}
			}
		}, _thread);

		status_thread.detach();
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

int main(int argc, char ** argv)
{
	uint64_t numCount = 0ul;
	regex r_digit("[^\\d]");

	if (argc >= 2)
	{
		string input(argv[1]);

		if (input == "-h" || input == "--help")
		{
			cout << "1st param: [REQUIRED] read number count" << endl;
			cout << "2nd param: [OPTIONAL] extra thread for process information ('-e' or '--extra')" << endl;
			return 0;
		}

		auto out_reg_str =
			regex_replace(input, r_digit, "");
		if (out_reg_str != "")
		{
			numCount = stoll(out_reg_str);
		}
		else
		{
			cerr << "Format error. Input valid read number count" << endl;
			return -1;
		}
	}
	else
	{
		cerr << "Input read number count" << endl;
		return -1;
	}

	const uint32_t coresNumber = findCoresNumber();
	cerr << "Processing thread count: " << coresNumber << endl;

	shared_ptr< reader_thread<> > th1(new reader_thread<>(numCount));
	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	startStatusThread(findExtraThreadStateOption(argc, argv), th1);

	map <uint64_t, map_uu> full_map;
	list<thread *> thread_list;
	list<map <uint64_t, unique_ptr < map_uu > > *> split_maps_list;

	for (uint32_t i = 0; i < coresNumber; ++i)
	{
		auto * split_map = new map <uint64_t, unique_ptr < map_uu > >;
		thread_list.push_back(new thread(process_thread<>, th1, std::ref(*split_map)));
		split_maps_list.push_back(split_map);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	th1->join();
	for (auto & thread : thread_list)
	{
		thread->join();
	}

	cerr << "Threads processing is ended. Now is merging and saving" << endl;

	for (auto & split_map : split_maps_list)
	{
		for (auto & map : *split_map)
		{
			full_map[map.first] = *map.second;
		}
		delete split_map;
	}

	for (auto & _val : full_map)
	{
		cout << "Readed value: " << _val.first << endl;
		for (auto & __val : _val.second)
		{
			cout << "Prime number: " << __val.first << "\t\tPower: " << __val.second << endl;
		}
		cout << endl << endl;
	}

	return 0;
}
