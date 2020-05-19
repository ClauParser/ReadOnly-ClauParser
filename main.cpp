
//#include <vld.h> // for memory leak checking

#include <iostream>

#include "readonly_clau_parser.h"

#include <ctime>


int main(void)
{
	std::string fileName;

	wiz::Node global;

	std::cin >> fileName;

	char* buffer = nullptr;
	std::vector<wiz::MemoryPool> pool;
	int a = clock();
	if (wiz::LoadData::LoadDataFromFile(fileName, &global, &buffer, &pool, 0, 0)) {
		
	}
	
	int c = clock();

		//wiz::LoadData::SaveWizDB(global, buffer, "output.eu4");
		if (buffer) {
			delete[] buffer;
		}
		if (pool.empty() == false) {
			for (auto& x : pool) {
				x.Clear();
			}
		}
int b = clock();
	std::cout << c - a << "ms" << "\n";
	std::cout << b - c << "ms" << "\n";


	return 0;
}
