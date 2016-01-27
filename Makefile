all: build_numGen build_nok
build_numGen: 
	g++ -std=c++11 -Wall -O2 -static-libstdc++ -o numGen numGen.cpp
build_nok:
	g++ -std=c++11 -Wall -O2 -static-libstdc++ -pthread -o thread_nok nok.cpp factorization.cpp
