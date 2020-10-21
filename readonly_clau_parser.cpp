
#define _CRT_SECURE_NO_WARNINGS

#include "readonly_clau_parser.h"


namespace clau_parser {
	Node* MemoryPool::Get() {
		if (size > 0 && count < size) {
			count++;
			return arr + count - 1;
		}
		else {
			// in real depth?  <= 10 ?
			// static Node[10] and list<Node*> ?
			count++; // for number check.
			else_list.push_back(new Node());
			return else_list.back();
		}
	}
	MemoryPool::~MemoryPool() {
		//
	}
	
	void MemoryPool::Clear() { // maybe just one called....
		if (arr) {
			//free(arr);
			arr = nullptr;
		}
		for (Node* x : else_list) {
			delete x;
		}
		else_list.clear();
	}

	const InFileReserver::BomInfo InFileReserver::bomInfo[1] = {
			{ 3, { (char)0xEF, (char)0xBB, (char)0xBF } }
	};
}
