
#ifndef READONLY_CLAU_PARSER_H
#define READONLY_CLAU_PARSER_H

#include <iostream>
#include <vector>
#include <set>
#include <stack>
#include <string>

#include <string_view>

#include <list>

#include <fstream>

#include <algorithm>

#include <thread>

namespace wiz {
	template <typename T>
	inline T pos_1(const T x, const int base = 10)
	{
		if (x >= 0) { return x % base; }// x - ( x / 10 ) * 10; }
		else { return (x / base) * base - x; }
		// -( x - ( (x/10) * 10 ) )
	}


	template <typename T> /// T <- char, int, long, long long...
	std::string toStr(const T x) /// chk!!
	{
		const int base = 10;
		if (base < 2 || base > 16) { return "base is not valid"; }
		T i = x;

		const int INT_SIZE = sizeof(T) << 3; ///*8
		char* temp = new char[INT_SIZE + 1 + 1]; /// 1 NULL, 1 minus
		std::string tempString;
		int k;
		bool isMinus = (i < 0);
		temp[INT_SIZE + 1] = '\0'; //

		for (k = INT_SIZE; k >= 1; k--) {
			T val = pos_1<T>(i, base); /// 0 ~ base-1
									   /// number to ['0'~'9'] or ['A'~'F']
			if (val < 10) { temp[k] = val + '0'; }
			else { temp[k] = val - 10 + 'A'; }

			i /= base;

			if (0 == i) { // 
				k--;
				break;
			}
		}

		if (isMinus) {
			temp[k] = '-';
			tempString = std::string(temp + k);//
		}
		else {
			tempString = std::string(temp + k + 1); //
		}
		delete[] temp;

		return tempString;
	}


	class LoadDataOption
	{
	public:
		char LineComment = '#';	// # 
		char Left = '{', Right = '}';	// { }
		char Assignment = '=';	// = 
		char Removal = ' ';		// ',' 
	};

	inline bool isWhitespace(const char ch)
	{
		switch (ch)
		{
		case ' ':
		case '\t':
		case '\r':
		case '\n':
		case '\v':
		case '\f':
			return true;
			break;
		}
		return false;
	}


	class Utility {
	public:
		static long long GetIdx(long long x) {
			return (x >> 32) & 0x00000000FFFFFFFF;
		}
		static long long GetLength(long long x) {
			return (x & 0x00000000FFFFFFF8) >> 3;
		}
		static long long GetType(long long x) { //to enum or enum class?
			return (x & 6) >> 1;
		}

		static bool IsToken2(long long x) {
			return (x & 1);
		}
	};


	inline int Equal(const long long x, const long long y)
	{
		if (x == y) {
			return 0;
		}
		return -1;
	}
	class InFileReserver
	{
	private:
		class BomInfo
		{
		public:
			size_t bom_size;
			char seq[5];
		};

		const static size_t BOM_COUNT = 1;

		enum class BomType { UTF_8, ANSI };

		static const BomInfo bomInfo[1];

		static BomType ReadBom(FILE* file) {
			char btBom[5] = { 0, };
			size_t readSize = fread(btBom, sizeof(char), 5, file);


			if (0 == readSize) {
				clearerr(file);
				fseek(file, 0, SEEK_SET);

				return BomType::ANSI;
			}

			BomInfo stBom = { 0, };
			BomType type = ReadBom(btBom, readSize, stBom);

			if (type == BomType::ANSI) { // ansi
				clearerr(file);
				fseek(file, 0, SEEK_SET);
				return BomType::ANSI;
			}

			clearerr(file);
			fseek(file, stBom.bom_size * sizeof(char), SEEK_SET);
			return type;
		}

		static BomType ReadBom(const char* contents, size_t length, BomInfo& outInfo) {
			char btBom[5] = { 0, };
			size_t testLength = length < 5 ? length : 5;
			memcpy(btBom, contents, testLength);

			size_t i, j;
			for (i = 0; i < BOM_COUNT; ++i) {
				const BomInfo& bom = bomInfo[i];

				if (bom.bom_size > testLength) {
					continue;
				}

				bool matched = true;

				for (j = 0; j < bom.bom_size; ++j) {
					if (bom.seq[j] == btBom[j]) {
						continue;
					}

					matched = false;
					break;
				}

				if (!matched) {
					continue;
				}

				outInfo = bom;

				return (BomType)i;
			}

			return BomType::ANSI;
		}


		// todo - rename.
		static long long Get(long long position, long long length, char ch, const wiz::LoadDataOption& option) {
			long long x = (position << 32) + (length << 3) + 0;

			if (length != 1) {
				return x;
			}

			if (option.Left == ch) {
				x += 2; // 010
			}
			else if (option.Right == ch) {
				x += 4; // 100
			}
			else if (option.Assignment == ch) {
				x += 6;
			}

			return x;
		}

		static void PrintToken(const char* buffer, long long token) {
			std::cout << std::string(buffer + Utility::GetIdx(token), Utility::GetLength(token));
		}

		static void _Scanning(char* text, long long num, const long long length,
			long long*& token_arr, long long& _token_arr_size, const LoadDataOption& option) {

			long long token_arr_size = 0;

			{
				int state = 0;

				long long token_first = 0;
				long long token_last = -1;

				long long token_arr_count = 0;

				for (long long i = 0; i < length; i = i + 1) {

					const char ch = text[i];

					switch (ch) {
					case '\"':
						token_last = i - 1;
						if (token_last - token_first + 1 > 0) {
							token_arr[num + token_arr_count] = Get(token_first + num, token_last - token_first + 1, text[token_first], option);
							token_arr_count++;
						}

						token_first = i;
						token_last = i;

						token_first = i + 1;
						token_last = i + 1;

						{//
							token_arr[num + token_arr_count] = 1;
							token_arr[num + token_arr_count] += Get(i + num, 1, ch, option);
							token_arr_count++;
						}
						break;
					case '\\':
					{//
						token_arr[num + token_arr_count] = 1;
						token_arr[num + token_arr_count] += Get(i + num, 1, ch, option);
						token_arr_count++;
					}
					break;
					case '\n':
						token_last = i - 1;
						if (token_last - token_first + 1 > 0) {
							token_arr[num + token_arr_count] = Get(token_first + num, token_last - token_first + 1, text[token_first], option);
							token_arr_count++;
						}
						token_first = i + 1;
						token_last = i + 1;

						{//
							token_arr[num + token_arr_count] = 1;
							token_arr[num + token_arr_count] += Get(i + num, 1, ch, option);
							token_arr_count++;
						}
						break;
					case '\0':
						token_last = i - 1;
						if (token_last - token_first + 1 > 0) {
							token_arr[num + token_arr_count] = Get(token_first + num, token_last - token_first + 1, text[token_first], option);
							token_arr_count++;
						}
						token_first = i + 1;
						token_last = i + 1;

						{//
							token_arr[num + token_arr_count] = 1;
							token_arr[num + token_arr_count] += Get(i + num, 1, ch, option);
							token_arr_count++;
						}
						break;
					case '#':
						token_last = i - 1;
						if (token_last - token_first + 1 > 0) {
							token_arr[num + token_arr_count] = Get(token_first + num, token_last - token_first + 1, text[token_first], option);
							token_arr_count++;
						}
						token_first = i + 1;
						token_last = i + 1;

						{//
							token_arr[num + token_arr_count] = 1;
							token_arr[num + token_arr_count] += Get(i + num, 1, ch, option);
							token_arr_count++;
						}

						break;
					case ' ':
					case '\t':
					case '\r':
					case '\v':
					case '\f':
						token_last = i - 1;
						if (token_last - token_first + 1 > 0) {
							token_arr[num + token_arr_count] = Get(token_first + num, token_last - token_first + 1, text[token_first], option);
							token_arr_count++;
						}
						token_first = i + 1;
						token_last = i + 1;

						break;
					case '{':
						token_last = i - 1;
						if (token_last - token_first + 1 > 0) {
							token_arr[num + token_arr_count] = Get(token_first + num, token_last - token_first + 1, text[token_first], option);
							token_arr_count++;
						}

						token_first = i;
						token_last = i;

						token_arr[num + token_arr_count] = Get(token_first + num, token_last - token_first + 1, text[token_first], option);
						token_arr_count++;

						token_first = i + 1;
						token_last = i + 1;
						break;
					case '}':
						token_last = i - 1;
						if (token_last - token_first + 1 > 0) {
							token_arr[num + token_arr_count] = Get(token_first + num, token_last - token_first + 1, text[token_first], option);
							token_arr_count++;
						}
						token_first = i;
						token_last = i;

						token_arr[num + token_arr_count] = Get(token_first + num, token_last - token_first + 1, text[token_first], option);
						token_arr_count++;

						token_first = i + 1;
						token_last = i + 1;
						break;
					case '=':
						token_last = i - 1;
						if (token_last - token_first + 1 > 0) {
							token_arr[num + token_arr_count] = Get(token_first + num, token_last - token_first + 1, text[token_first], option);
							token_arr_count++;
						}
						token_first = i;
						token_last = i;

						token_arr[num + token_arr_count] = Get(token_first + num, token_last - token_first + 1, text[token_first], option);
						token_arr_count++;

						token_first = i + 1;
						token_last = i + 1;
						break;
					}

				}

				if (length - 1 - token_first + 1 > 0) {
					token_arr[num + token_arr_count] = Get(token_first + num, length - 1 - token_first + 1, text[token_first], option);
					token_arr_count++;
				}
				token_arr_size = token_arr_count;
			}

			{
				_token_arr_size = token_arr_size;
			}
		}



		static void ScanningNew(char* text, const long long length, const int thr_num,
			long long*& _token_arr, long long& _token_arr_size, const LoadDataOption& option)
		{
			std::vector<std::thread> thr(thr_num);
			std::vector<long long> start(thr_num);
			std::vector<long long> last(thr_num);

			{
				start[0] = 0;

				for (int i = 1; i < thr_num; ++i) {
					start[i] = length / thr_num * i;

					for (long long x = start[i]; x <= length; ++x) {
						if (isWhitespace(text[x]) || '\0' == text[x] ||
							option.Left == text[x] || option.Right == text[x] || option.Assignment == text[x]) {
							start[i] = x;
							break;
						}
					}
				}
				for (int i = 0; i < thr_num - 1; ++i) {
					last[i] = start[i + 1];
					for (long long x = last[i]; x <= length; ++x) {
						if (isWhitespace(text[x]) || '\0' == text[x] ||
							option.Left == text[x] || option.Right == text[x] || option.Assignment == text[x]) {
							last[i] = x;
							break;
						}
					}
				}
				last[thr_num - 1] = length + 1;
			}
			long long real_token_arr_count = 0;

			long long* tokens = new long long[length + 1];
			long long token_count = 0;

			std::vector<long long> token_arr_size(thr_num);

			for (int i = 0; i < thr_num; ++i) {
				thr[i] = std::thread(_Scanning, text + start[i], start[i], last[i] - start[i], std::ref(tokens), std::ref(token_arr_size[i]), option);
			}

			for (int i = 0; i < thr_num; ++i) {
				thr[i].join();
			}

			int state = 0;
			long long qouted_start;
			long long slush_start;

			for (long long t = 0; t < thr_num; ++t) {
				for (long long j = 0; j < token_arr_size[t]; ++j) {
					const long long i = start[t] + j;

					const long long len = Utility::GetLength(tokens[i]);
					const char ch = text[Utility::GetIdx(tokens[i])];
					const long long idx = Utility::GetIdx(tokens[i]);
					const bool isToken2 = Utility::IsToken2(tokens[i]);

					if (isToken2) {
						if (0 == state && '\"' == ch) {
							state = 1;
							qouted_start = i;
						}
						else if (0 == state && option.LineComment == ch) {
							state = 2;
						}
						else if (1 == state && '\\' == ch) {
							state = 3;
							slush_start = idx;
						}
						else if (1 == state && '\"' == ch) {
							state = 0;

							{
								long long idx = Utility::GetIdx(tokens[qouted_start]);
								long long len = Utility::GetLength(tokens[qouted_start]);

								len = Utility::GetIdx(tokens[i]) - idx + 1;

								tokens[real_token_arr_count] = Get(idx, len, text[idx], option);
								real_token_arr_count++;


							//	PrintToken(text, tokens[real_token_arr_count - 1]);
							//	std::cout << " ";
							}
						}
						else if (3 == state) {
							if (idx != slush_start + 1) {
								--j; // --i;
							}
							state = 1;
						}
						else if (2 == state && ('\n' == ch || '\0' == ch)) {
							state = 0;
						}
					}
					else if (0 == state) { // 
						tokens[real_token_arr_count] = tokens[i];
						real_token_arr_count++;

						//PrintToken(text, tokens[real_token_arr_count - 1]); std::cout << " ";
					}
				}
			}

			{
				if (0 != state) {
					std::cout << "[ERRROR] state [" << state << "] is not zero \n";
				}
			}


			{
				_token_arr = tokens;
				_token_arr_size = real_token_arr_count;
			}
		}


		static void Scanning(char* text, const long long length,
			long long*& _token_arr, long long& _token_arr_size, const LoadDataOption& option) {

			long long* token_arr = new long long[length + 1];
			long long token_arr_size = 0;

			{
				int state = 0;

				long long token_first = 0;
				long long token_last = -1;

				long long token_arr_count = 0;

				for (long long i = 0; i <= length; ++i) {
					const char ch = text[i];

					if (0 == state) {
						if (option.LineComment == ch) {
							token_last = i - 1;
							if (token_last - token_first + 1 > 0) {
								token_arr[token_arr_count] = Get(token_first, token_last - token_first + 1, text[token_first], option);
								token_arr_count++;
							}

							state = 3;
						}
						else if ('\"' == ch) {
							token_last = i - 1;
							if (token_last - token_first + 1 > 0) {
								token_arr[token_arr_count] = Get(token_first, token_last - token_first + 1, text[token_first], option);
								token_arr_count++;
							}

							token_first = i;
							token_last = i;
							state = 1;
						}
						else if (isWhitespace(ch) || '\0' == ch) {
							token_last = i - 1;
							if (token_last - token_first + 1 > 0) {
								token_arr[token_arr_count] = Get(token_first, token_last - token_first + 1, text[token_first], option);
								token_arr_count++;
							}
							token_first = i + 1;
							token_last = i + 1;
						}
						else if (option.Left == ch) {
							token_last = i - 1;
							if (token_last - token_first + 1 > 0) {
								token_arr[token_arr_count] = Get(token_first, token_last - token_first + 1, text[token_first], option);
								token_arr_count++;
							}

							token_first = i;
							token_last = i;

							token_arr[token_arr_count] = Get(token_first, token_last - token_first + 1, text[token_first], option);
							token_arr_count++;

							token_first = i + 1;
							token_last = i + 1;
						}
						else if (option.Right == ch) {
							token_last = i - 1;
							if (token_last - token_first + 1 > 0) {
								token_arr[token_arr_count] = Get(token_first, token_last - token_first + 1, text[token_first], option);
								token_arr_count++;
							}
							token_first = i;
							token_last = i;

							token_arr[token_arr_count] = Get(token_first, token_last - token_first + 1, text[token_first], option);
							token_arr_count++;

							token_first = i + 1;
							token_last = i + 1;

						}
						else if (option.Assignment == ch) {
							token_last = i - 1;
							if (token_last - token_first + 1 > 0) {
								token_arr[token_arr_count] = Get(token_first, token_last - token_first + 1, text[token_first], option);
								token_arr_count++;
							}
							token_first = i;
							token_last = i;

							token_arr[token_arr_count] = Get(token_first, token_last - token_first + 1, text[token_first], option);
							token_arr_count++;

							token_first = i + 1;
							token_last = i + 1;
						}
					}
					else if (1 == state) {
						if ('\\' == ch) {
							state = 2;
						}
						else if ('\"' == ch) {
							token_last = i;

							token_arr[token_arr_count] = Get(token_first, token_last - token_first + 1, text[token_first], option);
							token_arr_count++;

							token_first = i + 1;
							token_last = i + 1;

							state = 0;
						}
					}
					else if (2 == state) {
						state = 1;
					}
					else if (3 == state) {
						if ('\n' == ch || '\0' == ch) {
							state = 0;

							token_first = i + 1;
							token_last = i + 1;
						}
					}
				}

				token_arr_size = token_arr_count;

				if (0 != state) {
					std::cout << "[" << state << "] state is not zero.\n";
				}
			}

			{
				_token_arr = token_arr;
				_token_arr_size = token_arr_size;
			}
		}


		static std::pair<bool, int> Scan(FILE* inFile, const int num, const wiz::LoadDataOption& option, int thr_num,
			char*& _buffer, long long* _buffer_len, long long*& _token_arr, long long* _token_arr_len)
		{



			if (inFile == nullptr) {
				return { false, 0 };
			}

			long long* arr_count = nullptr; //
			long long arr_count_size = 0;

			std::string temp;
			char* buffer = nullptr;
			long long file_length;

			{
				fseek(inFile, 0, SEEK_END);
				unsigned long long length = ftell(inFile);
				fseek(inFile, 0, SEEK_SET);

				BomType x = ReadBom(inFile);

				//	wiz::Out << "length " << length << "\n";
				if (x == BomType::UTF_8) {
					length = length - 3;
				}

				file_length = length;
				buffer = new char[file_length + 1]; // 

				//int a = clock();
				// read data as a block:
				fread(buffer, sizeof(char), file_length, inFile);
				//int b = clock();
				//std::cout << b - a << " " << file_length <<"\n";

				buffer[file_length] = '\0';

				{
					//int a = clock();
					long long* token_arr;
					long long token_arr_size;

					if (thr_num == 1) {
						Scanning(buffer, file_length, token_arr, token_arr_size, option);
					}
					else {
						ScanningNew(buffer, file_length, thr_num, token_arr, token_arr_size, option);
					}

					std::stack<int> _stack;
					for (long long i = 0; i < token_arr_size; ++i) {
						if (_stack.empty() && Utility::GetType(token_arr[i]) == 2) { // right
							delete[] buffer;
							delete[] token_arr;
							
							return { false, 1 };
						}
						if (Utility::GetType(token_arr[i]) == 1) {
							_stack.push(1);
						}
						else if (Utility::GetType(token_arr[i]) == 2) {
							_stack.pop();
						}
					}

					if (!_stack.empty()) {
						delete[] buffer;
						delete[] token_arr;

						return { false, 2 };
					}

					//int b = clock();
				//	std::cout << b - a << "ms\n";
					_buffer = buffer;
					_token_arr = token_arr;
					*_token_arr_len = token_arr_size;
					*_buffer_len = file_length;
				}
			}

			return{ true, 1 };
		}

	private:
		FILE* pInFile;
	public:
		int Num;
	public:
		explicit InFileReserver(FILE* inFile)
		{
			pInFile = inFile;
			Num = 1;
		}
		//bool end()const { return pInFile->eof(); } //
	public:
		bool operator() (const wiz::LoadDataOption& option, int thr_num, char*& buffer, long long* buffer_len, long long*& token_arr, long long* token_arr_len)
		{
			bool x = Scan(pInFile, Num, option, thr_num, buffer, buffer_len, token_arr, token_arr_len).second > 0;

			//	std::cout << *token_arr_len << "\n";
			return x;
		}
	};

	
	class Node;

	class MemoryPool
	{
		public:
			Node* arr = nullptr;
			std::vector<Node*> else_list;
			long long count = 0;
			long long size = 0;
		public:
			MemoryPool() {
				else_list.reserve(10);
			}
		public:
			Node* Get();
		public:
			virtual ~MemoryPool(); //

			void Clear();
	};
	
	// Node`s parent is valid if Node`s First is Node.
	// Node`s last is valid if Node`s First is Node.
	class Node {
	public:
			
	public:
		long type = 2; // 1 itemtype, 2 usertype, -1 virtual node and usertype.
		long long name = 0;
		long long value = 0;
	private:
		Node* first = nullptr;
		Node* last = nullptr;
		Node* parent = nullptr;
		Node* child = nullptr;
		Node* next = nullptr;
	public:
		Node() {
			first = this;
			last = this;
		}
	public:
		Node* GetParent() {
			return this->first->parent;
		}
		Node* GetFirst() {
			return first;
		}
		Node* GetLast() {
			return this->first->last;
		}
		Node* GetChild() {
			return this->child;
		}
		Node* GetNext() {
			return this->next;
		}

		void SetLast(Node* x) { // Chk GetFirst()
			this->last = x;
		}
		void SetFirst(Node* x) {
			this->first = x;
		}
		void SetChild(Node* x) {
			this->child = x;
		}
		void SetParent(Node* x) { // Chk GetFirst()
			this->parent = x;
		}

		// usertype
		void AddItem(long long var, long long val, MemoryPool& pool) {
			if (type != 2 && type != -1) {
				return;
			}


			if (nullptr == this->child) {
				this->child = MakeNode(pool);
				this->child->first = this->child;
				this->child->last = this->child;
				this->child->parent = this;
				this->child->type = 1;
				this->child->name = var;
				this->child->value = val;
			}
			else {
				Node* temp = MakeNode(pool);

				this->child->first->last->next = temp;
				this->child->first->last = temp;
				temp->first = this->child->first;
				temp->type = 1;
				temp->name = var;
				temp->value = val;
			}
		}

		Node* AddUserTypeItem(MemoryPool& pool, long long var = 0) {
			if (type != 2 && type != -1) {
				return nullptr;
			}

			if (nullptr == this->child) {
				this->child = MakeNode(pool);
				this->child->first = this->child;
				this->child->last = this->child;
				this->child->parent = this;
				this->child->type = 2;
				this->child->name = var;
				
				return this->child;
			}
			else {
				Node* temp = MakeNode(pool);

				this->child->first->last->next = temp;
				this->child->first->last = temp;
				temp->first = this->child->first;
				temp->type = 2;
				temp->name = var;

				return temp;
			}
		}

		Node* AddVirtualNode(MemoryPool& pool) {
			Node* temp = AddUserTypeItem(pool);
			if (temp) {
				temp->type = -1;
			}

			return temp;
		}

		// itemtype
		
	public:
	
		static Node* MakeNode(MemoryPool& pool) {	
			return pool.Get(); // using memory pool.
		}

		void Save(std::ostream& stream, char* buffer) const {
			Save(stream, buffer, this);
		}
	private:
		void Save(std::ostream& stream, char* buffer, const Node* node) const {
			do {
				if (!node) {
					return;
				}

				if (Utility::GetLength(node->name) != 0) {
					stream << std::string_view(Utility::GetIdx(node->name) + buffer, Utility::GetLength(node->name)) << " = ";
				}
				if (node->type == -1) {
					stream << "# = { \n";
				}
				else if (node->type == 2 && node != this) {
					stream << " { \n";
				}
				else if (node->type == 1) {
					stream << std::string_view(Utility::GetIdx(node->value) + buffer, Utility::GetLength(node->value)) << " ";
				}

				if (node->type == -1 || node->type == 2) {
					Save(stream, buffer, node->child);
					if (node != this) {
						stream << " } \n ";
					}
				}
				node = node->next;
			} while (node != nullptr);
		}

	public:
		void Link(Node* x) {
			// this - usertype,  x - node..
			if (this->child) {
				this->child->last->next = x;
				this->child->last = x->first->last;
				x->parent = nullptr;
			}
			else {
				this->child = x;
				this->child->first = x;
				x->parent = this;
				x->last = x->first->last;
			}

			for (Node* iter = x; iter != nullptr; iter = iter->next) {
				iter->first = this->child;
			}
		}
	};

	

	// LoadDat
	class LoadData
	{
	private:
		static long long check_syntax_error1(long long str) {
			long long len = Utility::GetLength(str);
			long long type = Utility::GetType(str);

			if (1 == len && (type == 1 || type == 2 ||
				type == 3)) {
				throw "check syntax error 1 : " + str;
			}
			return str;
		}
		static int Merge(Node* next, Node* ut, Node** ut_next)
		{
			//check!!
			while (ut->GetChild()
				&& ut->GetChild()->type == -1)
			{
				ut = ut->GetChild();
			}

			while (true) {
				Node* _ut = ut;
				Node* _next = next;

				if (ut_next && _ut == *(ut_next)) { // check..
					*(ut_next) = next;
				}

				{
					Node* x = _ut->GetChild(); // x->first == x
					if (x) {
						if (x->type == -1) {
							Node* temp = x->GetNext();
							Node* last = x->GetLast();
							// delete x; //
							x = temp;

							if (x) {
								x->SetLast(last);
							}
						}
						if (x) {
							x->SetFirst(x);
							_next->Link(x);
							_ut->SetChild(nullptr);
						}
					}
				}


				ut = ut->GetParent();
				next = next->GetParent();


				if (next && ut) {
					//
				}
				else {
					// right_depth > left_depth
					if (!next && ut) {
						return -1;
					}
					else if (next && !ut) {
						return 1;
					}

					return 0;
				}
			}
		}

		
	private:
		static bool __LoadData(const char* buffer, const long long* token_arr, long long token_arr_len, Node* _global, const wiz::LoadDataOption* _option,
			int start_state, int last_state, Node** next, MemoryPool* _pool) // first, strVec.empty() must be true!!
		{
			{
				long long count_left = 0;
				long long count_right = 0;
				long long count_eq = 0;
				long long count_other = 0; // count_other

				for (long long x = 0; x < token_arr_len; ++x) {
					switch (Utility::GetType(token_arr[x]))
					{
					case 0:
						count_other++;
						break;
					case 1:
						count_left++;
						break;
					case 2:
						count_right++;
						break;
					case 3:
						count_eq++;
						break;
					}
				}

				// chk count_ - count_eq + count_left < 0 ? -
				// count_ = count_other..
				long long chkNum = count_other - count_eq + count_left;

				if (count_right > count_left) { // maybe has virtual node? - support some virutal node
					// cf) }}} {{{   right == left but, # of virtual node is 3.
					chkNum += count_right - count_left;
				}
				if (chkNum < 0) {
					_pool->arr = nullptr;
					_pool->count = 0;
					_pool->size = 0;
				}
				else {
					_pool->arr = new Node[1 + chkNum];
					_pool->count = 0;
					_pool->size = 1 + chkNum;
				}
			}

			MemoryPool& pool = *_pool;

			std::vector<long long> varVec;
			std::vector<long long> valVec;


			if (token_arr_len <= 0) {
				return false;
			}

			const wiz::LoadDataOption& option = *_option;

			int state = start_state;
			int braceNum = 0;
			std::vector< Node* > nestedUT(1);
			long long var = 0, val = 0;

			nestedUT.reserve(10);
			nestedUT[0] = _global;


			long long count = 0;
			const long long* x = token_arr;
			const long long* x_next = x;

			for (long long i = 0; i < token_arr_len; ++i) {
				x = x_next;
				{
					x_next = x + 1;
				}
				if (count > 0) {
					count--;
					continue;
				}
				long long len = Utility::GetLength(token_arr[i]);

				switch (state)
				{
				case 0:
				{
					// Left 1
					if (len == 1 && (-1 != Equal(1, Utility::GetType(token_arr[i])) || -1 != Equal(1, Utility::GetType(token_arr[i])))) {
						if (!varVec.empty()) {
							for (long long x = 0; x < varVec.size(); ++x) {
								nestedUT[braceNum]->AddItem(varVec[x], valVec[x], pool);
							}

							varVec.clear();
							valVec.clear();
						}

						Node* pTemp = nullptr;

						{
							pTemp = nestedUT[braceNum]->AddUserTypeItem(pool);
						}
						

						braceNum++;

						/// new nestedUT
						if (nestedUT.size() == braceNum) { /// changed 2014.01.23..
							nestedUT.push_back(nullptr);
						}

						/// initial new nestedUT.
						nestedUT[braceNum] = pTemp;
						///

						state = 0;
					}
					// Right 2
					else if (len == 1 && (-1 != Equal(2, Utility::GetType(token_arr[i])) || -1 != Equal(2, Utility::GetType(token_arr[i])))) {
						state = 0;

						if (!varVec.empty()) {
							for (long long x = 0; x < varVec.size(); ++x) {
								nestedUT[braceNum]->AddItem(varVec[x], valVec[x], pool);
							}

							varVec.clear();
							valVec.clear();
						}

						if (braceNum == 0) {
							Node ut;
							Node* temp = ut.AddVirtualNode(pool);

							temp->SetChild(nestedUT[0]->GetChild());
							temp->SetParent(nestedUT[0]);
							if (nestedUT[0]->GetChild()) {
								nestedUT[0]->GetChild()->SetParent(temp);
							}
							nestedUT[0]->SetChild(temp);

							braceNum++;
						}

						{
							if (braceNum < nestedUT.size()) {
								nestedUT[braceNum] = nullptr;
							}
							braceNum--;
						}
					}
					else {
						if (x < token_arr + token_arr_len - 1) {
							long long _len = Utility::GetLength(token_arr[i + 1]);
							// EQ 3
							if (_len == 1 && -1 != Equal(3, Utility::GetType(token_arr[i + 1]))) {
								var = token_arr[i];

								state = 1;

								{
									count = 1;
								}
							}
							else {
								// var1
								if (x <= token_arr + token_arr_len - 1) {

									val = token_arr[i];

									varVec.push_back(check_syntax_error1(var));
									valVec.push_back(check_syntax_error1(val));

									val = 0;

									state = 0;

								}
							}
						}
						else
						{
							// var1
							if (x <= token_arr + token_arr_len - 1)
							{
								val = token_arr[i];
								varVec.push_back(check_syntax_error1(var));
								valVec.push_back(check_syntax_error1(val));
								val = 0;

								state = 0;
							}
						}
					}
				}
				break;
				case 1:
				{
					// LEFT 1
					if (len == 1 && (-1 != Equal(1, Utility::GetType(token_arr[i])) || -1 != Equal(1, Utility::GetType(token_arr[i])))) {
						for (long long x = 0; x < varVec.size(); ++x) {
							nestedUT[braceNum]->AddItem(varVec[x], valVec[x], pool);
						}


						varVec.clear();
						valVec.clear();

						///
						{
							Node* pTemp;

							pTemp = nestedUT[braceNum]->AddUserTypeItem(pool, var);
							
							var = 0;
							braceNum++;

							/// new nestedUT
							if (nestedUT.size() == braceNum) {
								nestedUT.push_back(nullptr);
							}

							/// initial new nestedUT.
							nestedUT[braceNum] = pTemp;
						}
						///
						state = 0;
					}
					else {
						if (x <= token_arr + token_arr_len - 1) {
							val = token_arr[i];

							varVec.push_back(check_syntax_error1(var));
							valVec.push_back(check_syntax_error1(val));
							var = 0; val = 0;

							state = 0;
						}
					}
				}
				break;
				default:
					// syntax err!!
					throw "syntax error ";
					break;
				}
			}

			if (varVec.empty() == false) {
				for (long long x = 0; x < varVec.size(); ++x) {
					nestedUT[braceNum]->AddItem(varVec[x], valVec[x], pool);
				}

				varVec.clear();
				valVec.clear();
			}

			if (next) {
				*next = nestedUT[braceNum];
			}

			if (state != last_state) {
				throw std::string("error final state is not last_state!  : ") + toStr(state);
			}
			if (x > token_arr + token_arr_len) {
				throw std::string("error x > buffer + buffer_len: ");
			}

			return true;
		}

		static long long FindDivisionPlace(const char* buffer, const long long* token_arr, long long start, long long last, const wiz::LoadDataOption& option)
		{
			for (long long a = last; a >= start; --a) {
				long long len = Utility::GetLength(token_arr[a]);
				long long val = Utility::GetType(token_arr[a]);


				if (len == 1 && (-1 != Equal(2, val) || -1 != Equal(2, val))) { // right
					return a;
				}

				bool pass = false;
				if (len == 1 && (-1 != Equal(1, val) || -1 != Equal(1, val))) { // left
					return a;
				}
				else if (len == 1 && -1 != Equal(3, val)) { // assignment
					//
					pass = true;
				}

				if (a < last && pass == false) {
					long long len = Utility::GetLength(token_arr[a + 1]);
					long long val = Utility::GetType(token_arr[a + 1]);

					if (!(len == 1 && -1 != Equal(3, val))) // assignment
					{ // NOT
						return a;
					}
				}
			}
			return -1;
		}

		static bool _LoadData(InFileReserver& reserver, Node* global, const wiz::LoadDataOption option, char** _buffer, std::vector<wiz::MemoryPool>* _pool, const int lex_thr_num, const int parse_num) // first, strVec.empty() must be true!!
		{
			const int pivot_num = parse_num - 1;
			char* buffer = nullptr;
			long long* token_arr = nullptr;
			long long buffer_total_len;
			long long token_arr_len = 0;

			{
				bool success = reserver(option, lex_thr_num, buffer, &buffer_total_len, token_arr, &token_arr_len);

				if (!success) {
					return false;
				}
				if (token_arr_len <= 0) {
					return true;
				}
			}


			Node* before_next = nullptr;
			Node _global;

			bool first = true;
			long long sum = 0;

			{
				std::set<long long> _pivots;
				std::vector<long long> pivots;
				const long long num = token_arr_len; //

				if (pivot_num > 0) {
					std::vector<long long> pivot;
					pivots.reserve(pivot_num);
					pivot.reserve(pivot_num);

					for (int i = 0; i < pivot_num; ++i) {
						pivot.push_back(FindDivisionPlace(buffer, token_arr, (num / (pivot_num + 1)) * (i), (num / (pivot_num + 1)) * (i + 1) - 1, option));
					}

					for (int i = 0; i < pivot.size(); ++i) {
						if (pivot[i] != -1) {
							_pivots.insert(pivot[i]);
						}
					}

					for (auto& x : _pivots) {
						pivots.push_back(x);
					}
				}

				std::vector<Node*> next(pivots.size() + 1, nullptr);

				{
					std::vector<MemoryPool> pool(pivots.size() + 1);

					std::vector<Node> __global(pivots.size() + 1);

					std::vector<std::thread> thr(pivots.size() + 1);

					{
						long long idx = pivots.empty() ? num - 1 : pivots[0];
						long long _token_arr_len = idx - 0 + 1;


						thr[0] = std::thread(__LoadData, buffer, token_arr, _token_arr_len, &__global[0], &option, 0, 0, &next[0], &pool[0]);
					}

					for (int i = 1; i < pivots.size(); ++i) {
						long long _token_arr_len = pivots[i] - (pivots[i - 1] + 1) + 1;

						thr[i] = std::thread(__LoadData, buffer, token_arr + pivots[i - 1] + 1, _token_arr_len, &__global[i], &option, 0, 0, &next[i], &pool[i]);

					}

					if (pivots.size() >= 1) {
						long long _token_arr_len = num - 1 - (pivots.back() + 1) + 1;

						thr[pivots.size()] = std::thread(__LoadData, buffer, token_arr + pivots.back() + 1, _token_arr_len, &__global[pivots.size()],
							&option, 0, 0, &next[pivots.size()], &pool[pivots.size()]);
					}

					// wait
					for (int i = 0; i < thr.size(); ++i) {
						thr[i].join();
					}

					// Merge
					try { // chk empty global?
						if (__global[0].GetChild() && __global[0].GetChild()->type == -1) {
							std::cout << "not valid file1\n";
							throw 1;
						}
						if (next.back()->GetParent() && next.back()->GetParent()->GetParent() != nullptr) {
							std::cout << "not valid file2\n";
							throw 2;
						}
						
						/*
						SaveWizDB(__global[0], buffer, "0.txt");
						SaveWizDB(__global[1], buffer, "1.txt");
						SaveWizDB(__global[2], buffer, "2.txt");
						SaveWizDB(__global[3], buffer, "3.txt");
						SaveWizDB(__global[4], buffer, "4.txt");
						SaveWizDB(__global[5], buffer, "5.txt");
						SaveWizDB(__global[6], buffer, "6.txt");
						SaveWizDB(__global[7], buffer, "7.txt");
						*/

						for (int i = 1; i < pivots.size() + 1; ++i) {
							// linearly merge and error check...
							int err = Merge(next[i - 1], &__global[i], &next[i]);
							if (-1 == err) {
								std::cout << "not valid file4\n";
								throw 4;
							}
							else if (i == pivots.size() && 1 == err) {
								std::cout << "not valid file5\n";
								throw 5;
							}
						}

						_global = __global[0];
						*_pool = pool;
					}
					catch (...) {
						delete[] token_arr;
						delete[] buffer;
						for (auto& x : pool) {
							x.Clear();
						}
						buffer = nullptr;
						throw "in Merge, error";
					}

					before_next = next.back();
				}
			}

			delete[] token_arr;

			*_buffer = buffer;
			*global = _global;

			return true;
		}
	public:
		static bool LoadDataFromFile(const std::string& fileName, Node* global, char** _buffer, std::vector<wiz::MemoryPool>* pool, int lex_thr_num = 0, int parse_num = 0) /// global should be empty
		{
			if (lex_thr_num <= 0) {
				lex_thr_num = std::thread::hardware_concurrency();
			}
			if (lex_thr_num <= 0) {
				lex_thr_num = 1;
			}

			if (parse_num <= 0) {
				parse_num = std::thread::hardware_concurrency();
			}
			if (parse_num <= 0) {
				parse_num = 1;
			}

			bool success = true;
			FILE* inFile;
			fopen_s(&inFile, fileName.c_str(), "rb");

			if (nullptr == inFile)
			{
				return false;
			}

			Node globalTemp;

			try {

				InFileReserver ifReserver(inFile);
				wiz::LoadDataOption option;
				option.Assignment = ('=');
				option.Left = '{';
				option.Right = '}';
				option.LineComment = ('#');
				option.Removal = ' '; // ','

				ifReserver.Num = 1 << 19;
				//	strVec.reserve(ifReserver.Num);
				// cf) empty file..
				if (false == _LoadData(ifReserver, &globalTemp, option, _buffer, pool, lex_thr_num, parse_num))
				{
					fclose(inFile);
					return false; // return true?
				}

				fclose(inFile);
			}
			catch (const char* err) {
				std::cout << err << "\n"; fclose(inFile);
				for (auto& x : *pool) {
					x.Clear();
				}
				pool->clear();
				return false;
			}
			catch (const std::string & e) {
				std::cout << e << "\n"; fclose(inFile);
				for (auto& x : *pool) {
					x.Clear();
				}
				pool->clear();
				return false;
			}
			catch (std::exception e) {
				std::cout << e.what() << "\n"; fclose(inFile);
				for (auto& x : *pool) {
					x.Clear();
				}
				pool->clear();
				return false;
			}
			catch (...) {
				std::cout << "not expected error" << "\n"; fclose(inFile);
				for (auto& x : *pool) {
					x.Clear();
				}
				pool->clear();
				return false;
			}


			*global = globalTemp;
			return true;
		}
		static bool LoadWizDB(Node* global, const std::string& fileName, char** buffer, std::vector<wiz::MemoryPool>* pool, const int thr_num) {
			Node globalTemp;

			// Scan + Parse 
			if (false == LoadDataFromFile(fileName, &globalTemp, buffer, pool, thr_num, thr_num)) { return false; }
			//std::cout << "LoadData End" << "\n";

			*global = globalTemp;
			return true;
		}
		// SaveQuery
		static bool SaveWizDB(const Node& global, char* buffer, const std::string& fileName, const bool append = false) {
			std::ofstream outFile;
			if (fileName.empty()) { return false; }
			if (false == append) {
				outFile.open(fileName);
				if (outFile.fail()) { return false; }
			}
			else {
				outFile.open(fileName, std::ios::app);
				if (outFile.fail()) { return false; }

				outFile << "\n";
			}

			/// saveFile
			global.Save(outFile, buffer); // cf) friend

			outFile.close();

			return true;
		}
	};

}


#endif

