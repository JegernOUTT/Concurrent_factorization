#include <iostream>
#include <random>
#include <chrono>
#include <regex>
#include <string>

using std::cout;
using std::cerr;
using std::endl;
using std::regex;
using std::string;
using std::regex_replace;
using std::default_random_engine;
using std::uniform_int_distribution;

int main(int argc, char ** argv)
{
	uint64_t numCount = 0ul;
	regex r_digit("[^\\d]");

	if (argc == 2)
	{
		string input(argv[1]);

		if (input == "-h" || input == "--help")
		{
			cout << "1st param: generation number count" << endl;
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
			cerr << "Format error. Input valid generation count" << endl;
			return -1;
		}
	}
	else
	{
		cerr << "Input generation count" << endl;
		return -1;
	}

	uint64_t seed = std::chrono::system_clock::now().time_since_epoch().count();
	default_random_engine gen(seed);
	uniform_int_distribution<uint64_t> dist(0, (UINT64_MAX >> 32));

	for (uint64_t i = 0; i < numCount; ++i)
	{
		cout << dist(gen) << std::endl;
	}

	return 0;
}
