
//#include <vld.h> // for memory leak checking

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>

#include "readonly_clau_parser.h"

#include <ctime>


int main(void)
{
	std::string fileName;

	clau_parser::Node global;

	std::cin >> fileName;

	char* buffer = nullptr;
	clau_parser::Node* node_arr = nullptr;
	std::vector<clau_parser::MemoryPool> pool;

	auto a = std::chrono::steady_clock::now();
	
	if (clau_parser::LoadData::LoadDataFromFile(fileName, &global, &buffer, node_arr, &pool, 0, 0)) {
		//
	}
	
	auto c = std::chrono::steady_clock::now();

		//clau_parser::LoadData::Saveclau_parserDB(global, buffer, "output.eu4");
		
		if (buffer) {
			delete[] buffer;
		}
		if (node_arr) {
			free(node_arr);
		}
		if (pool.empty() == false) {
			for (auto& x : pool) {
				x.Clear();
			}
		}

		auto b = std::chrono::steady_clock::now();

		auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(c - a);
		auto dur2 = std::chrono::duration_cast<std::chrono::milliseconds>(b - c);
		std::cout << dur.count() << "milli seconds" << "\n";
		std::cout << dur2.count() << "milli seconds" << "\n";

	return 0;
}
