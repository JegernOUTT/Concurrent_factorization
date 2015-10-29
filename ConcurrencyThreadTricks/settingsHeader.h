#define THREAD_WORK_PRINT
//#define INPUT_WITH_IFSTREAM
#define	INPUT_WITH_STDIN
//#define OUTPUT_WITH_OFSTREAM
#define	OUTPUT_WITH_STDOUT
//#define MAKE_INPUT_FILE	//Just with IFSTREAM

#ifdef THREAD_WORK_PRINT
std::mutex cout_mut;
#endif

#ifdef INPUT_WITH_IFSTREAM
	const std::string inputFileName = "in.txt";
#endif

#ifdef OUTPUT_WITH_OFSTREAM
	const std::string outputFileName = "out.txt";
#endif

#ifdef MAKE_INPUT_FILE
	const uint64_t numCount = 10000000ul;
#endif

#ifdef INPUT_WITH_STDIN
	const uint64_t stdinMaxCount = 5ul;
#endif