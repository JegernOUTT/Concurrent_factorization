//#define THREAD_WORK_PRINT
#define INPUT_WITH_IFSTREAM
//#define	INPUT_WITH_STDIN
#define OUTPUT_WITH_OFSTREAM
//#define	OUTPUT_WITH_STDOUT
//#define MAKE_INPUT_FILE	//Just with IFSTREAM

#ifdef THREAD_WORK_PRINT
std::mutex cout_mut;
#endif

const int thread_count = 4;
const std::string inputFileName = "in.txt";
const std::string outputFileName = "out.txt";
const uint64_t numCount = 2000000ul;
const uint64_t stdinMaxCount = 5ul;

#ifdef INPUT_WITH_IFSTREAM
	#undef INPUT_WITH_STDIN
#endif

#ifdef INPUT_WITH_STDIN
	#undef INPUT_WITH_IFSTREAM
#endif

#ifdef OUTPUT_WITH_OFSTREAM
	#undef OUTPUT_WITH_STDOUT
#endif

#ifdef OUTPUT_WITH_STDOUT
	#undef OUTPUT_WITH_OFSTREAM
#endif

