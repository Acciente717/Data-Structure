/*
#####################
###   bigint.h    ###
### Author: Acc.  ###
###  2017-03-12   ###
###  Version 1.0  ###
#####################

***** WARNING *****
bigint.h uses AVX & AVX2 instructions by default.
Running on CPUs which don't have these two instructions may cause crush.
Flags can be set in the "Define Block" below.
When "AVX" is defined, AVX will be used in doing "+", "-", and AVX2 will be used in doing, "*", "%".
When "AVX" is not defined and "SSE" is defined, SSE2 will be used in doing "+", "-", and SSE4.1 will be used in doing, "*", "%".
When neither of the flag is defined, nothing will be used to speed up.

***** ANOTHER WARNING *****
bigint.h uses C++11 feature. (right value reference)
It may cause compile error when compiling on compilers which don't support C++11.


***** USAGE *****

Part 1: load, extract, and print
	To load a big integer from a string:
		Bigint example;
		char yourstring[] = "123456";
		example.load(yourstring);
	To extract the number from a bigint to string:
		example.extract(yourstring);
	To print your number directly:
		example.print();

Part 2: operators
	bigint supports operator "+", "-", "*", "%", "=", "==", "<".
	They function as normal "int", but they should be used between two bigint.
	Correct:
		Bigint a, b;
		a.load("2"); b.load("1");
		a = a - b;
		a.print();
	Incorrect:
		Bigint a;
		a.load("2");
		a = a - 1;
		a.print();

Part 3: little tricks
	cutHalf:
		a.cutHalf() works like "a /= 2";
	isOdd:
		Returns a bool. When the number is odd, return true, otherwise return false.
		It's much faster than using "a % two == one".
	isZero:
		If the number is zero, return true, otherwise false.
*/

// ***** Define Block Start *****
#define AVX
#define SSE
// ***** Define Block End *****

#include <cstring>
#include <iostream>
#include <cstdio>
#include <stack>
#include <immintrin.h>
#include <emmintrin.h>

class Bigint
{
	int size;
	int capacity;
	bool isNegative;
	int *row;
	void __eliminate_double_zero()
	{
		if (size == 1)
			if (row[0] == 0)
				isNegative = false;
	}
	bool __abs_smaller_than(const Bigint &a, const Bigint &b);
	Bigint __plain_add(const Bigint &a, const Bigint &b);
	Bigint __plain_subtract(const Bigint &a, const Bigint &b);
	Bigint __plain_multiply(const Bigint &a, const Bigint &b);
	Bigint(int iniCapacity) : size(1), capacity(iniCapacity), isNegative(false)
	{
		row = new int[iniCapacity];
		memset(row, 0, sizeof(int) * iniCapacity);
		row[0] = 0;
	}

public:
	Bigint() : size(1), capacity(32), isNegative(false)
	{
		row = new int[32];
		memset(row, 0, sizeof(int) * 32);
		row[0] = 0;
	}
	Bigint(const Bigint &x)
	{
		row = new int[x.capacity];
		memcpy(row, x.row, sizeof(int) * x.capacity);
		size = x.size;
		capacity = x.capacity;
		isNegative = x.isNegative;
	}
	Bigint(Bigint &&x)
	{
		row = x.row;
		x.row = new int[1];
		size = x.size;
		capacity = x.capacity;
		isNegative = x.isNegative;
	}
	~Bigint()
	{
		delete[] row;
	}
	void load(const char* input_);
	void print();
	void extract(char *target_);
	void cutHalf();
	bool operator<(const Bigint &x);
	bool operator==(const Bigint &x);
	bool isOdd()
	{
		if (row[0] == 0 || row[0] == 2 || row[0] == 4 || row[0] == 6 || row[0] == 8)
			return false;
		else
			return true;
	}
	bool isZero()
	{
		if (size == 1 && row[0] == 0)
			return true;
		else
			return false;
	}
	Bigint& operator=(const Bigint &x);
	Bigint& operator=(Bigint &&x);
	Bigint operator+(const Bigint &x);
	Bigint operator-(const Bigint &x);
	Bigint operator*(const Bigint &x);
	Bigint operator%(const Bigint &x);
};

bool Bigint::__abs_smaller_than(const Bigint &a, const Bigint &b)
{
	if (a.size < b.size)
		return true;
	else if (a.size > b.size)
		return false;
	else
	{
		for (int i = a.size - 1; i >= 0; --i)
		{
			if (a.row[i] < b.row[i])
				return true;
			else if (a.row[i] > b.row[i])
				return false;
		}
		return false;
	}
}

Bigint Bigint::__plain_add(const Bigint &a, const Bigint &b)
{
	if (a.size > b.size)
	{
		Bigint temp(a.size + 2);
		temp = a;

#if defined AVX
		int round = b.size / 32;
		int rest = b.size - round * 32;
		__m256i loader1, loader2, loader3, loader4;
		__m256i *pb = reinterpret_cast<__m256i*>(b.row);
		__m256i *ptemp = reinterpret_cast<__m256i*>(temp.row);
		for (int i = 0; i < round; ++i)
		{
			loader1 = _mm256_add_epi32(_mm256_loadu_si256(pb), _mm256_loadu_si256(ptemp));
			loader2 = _mm256_add_epi32(_mm256_loadu_si256(pb + 1), _mm256_loadu_si256(ptemp + 1));
			loader3 = _mm256_add_epi32(_mm256_loadu_si256(pb + 2), _mm256_loadu_si256(ptemp + 2));
			loader4 = _mm256_add_epi32(_mm256_loadu_si256(pb + 3), _mm256_loadu_si256(ptemp + 3));
			_mm256_storeu_si256(ptemp, loader1);
			_mm256_storeu_si256(ptemp + 1, loader2);
			_mm256_storeu_si256(ptemp + 2, loader3);
			_mm256_storeu_si256(ptemp + 3, loader4);
			pb += 4;
			ptemp += 4;
		}
		round = (b.size - round * 32) / 8;
		rest -= round * 8;
		for (int i = 0; i < round; ++i)
		{
			loader1 = _mm256_add_epi32(_mm256_loadu_si256(pb), _mm256_loadu_si256(ptemp));
			_mm256_storeu_si256(ptemp, loader1);
			pb += 1;
			ptemp += 1;
		}
		for (int i = b.size - rest; i < b.size; ++i)
			temp.row[i] += b.row[i];

#elif defined SSE
		int round = b.size / 16;
		int rest = b.size - round * 16;
		__m128i loader1, loader2, loader3, loader4;
		__m128i *pb = reinterpret_cast<__m128i*>(b.row);
		__m128i *ptemp = reinterpret_cast<__m128i*>(temp.row);
		for (int i = 0; i < round; ++i)
		{
			loader1 = _mm_add_epi32(_mm_loadu_si128(pb), _mm_loadu_si128(ptemp));
			loader2 = _mm_add_epi32(_mm_loadu_si128(pb + 1), _mm_loadu_si128(ptemp + 1));
			loader3 = _mm_add_epi32(_mm_loadu_si128(pb + 2), _mm_loadu_si128(ptemp + 2));
			loader4 = _mm_add_epi32(_mm_loadu_si128(pb + 3), _mm_loadu_si128(ptemp + 3));
			_mm_storeu_si128(ptemp, loader1);
			_mm_storeu_si128(ptemp + 1, loader2);
			_mm_storeu_si128(ptemp + 2, loader3);
			_mm_storeu_si128(ptemp + 3, loader4);
			pb += 4;
			ptemp += 4;
		}
		round = (b.size - round * 16) / 4;
		rest -= round * 4;
		for (int i = 0; i < round; ++i)
		{
			loader1 = _mm_add_epi32(_mm_loadu_si128(pb), _mm_loadu_si128(ptemp));
			_mm_storeu_si128(ptemp, loader1);
			pb += 1;
			ptemp += 1;
		}
		for (int i = b.size - rest; i < b.size; ++i)
			temp.row[i] += b.row[i];
#else
		for (int i = 0; i < b.size; ++i)
			temp.row[i] += b.row[i];
#endif

		for (int i = 0; i < a.size; ++i)
		{
			if (temp.row[i] >= 10)
			{
				temp.row[i] -= 10;
				temp.row[i + 1] += 1;
			}
		}
		if (temp.row[a.size] != 0)
			temp.size = a.size + 1;
		return temp;
	}
	else
	{
		Bigint temp(b.size + 2);
		temp = b;

#if defined AVX
		int round = a.size / 32;
		int rest = a.size - round * 32;
		__m256i loader1, loader2, loader3, loader4;
		__m256i *pa = reinterpret_cast<__m256i*>(a.row);
		__m256i *ptemp = reinterpret_cast<__m256i*>(temp.row);
		for (int i = 0; i < round; ++i)
		{
			loader1 = _mm256_add_epi32(_mm256_loadu_si256(pa), _mm256_loadu_si256(ptemp));
			loader2 = _mm256_add_epi32(_mm256_loadu_si256(pa + 1), _mm256_loadu_si256(ptemp + 1));
			loader3 = _mm256_add_epi32(_mm256_loadu_si256(pa + 2), _mm256_loadu_si256(ptemp + 2));
			loader4 = _mm256_add_epi32(_mm256_loadu_si256(pa + 3), _mm256_loadu_si256(ptemp + 3));
			_mm256_storeu_si256(ptemp, loader1);
			_mm256_storeu_si256(ptemp + 1, loader2);
			_mm256_storeu_si256(ptemp + 2, loader3);
			_mm256_storeu_si256(ptemp + 3, loader4);
			pa += 4;
			ptemp += 4;
		}
		round = (a.size - round * 32) / 8;
		rest -= round * 8;
		for (int i = 0; i < round; ++i)
		{
			loader1 = _mm256_add_epi32(_mm256_loadu_si256(pa), _mm256_loadu_si256(ptemp));
			_mm256_storeu_si256(ptemp, loader1);
			pa += 1;
			ptemp += 1;
		}
		for (int i = a.size - rest; i < a.size; ++i)
			temp.row[i] += a.row[i];
#elif defined SSE
		int round = a.size / 16;
		int rest = a.size - round * 16;
		__m128i loader1, loader2, loader3, loader4;
		__m128i *pa = reinterpret_cast<__m128i*>(a.row);
		__m128i *ptemp = reinterpret_cast<__m128i*>(temp.row);
		for (int i = 0; i < round; ++i)
		{
			loader1 = _mm_add_epi32(_mm_loadu_si128(pa), _mm_loadu_si128(ptemp));
			loader2 = _mm_add_epi32(_mm_loadu_si128(pa + 1), _mm_loadu_si128(ptemp + 1));
			loader3 = _mm_add_epi32(_mm_loadu_si128(pa + 2), _mm_loadu_si128(ptemp + 2));
			loader4 = _mm_add_epi32(_mm_loadu_si128(pa + 3), _mm_loadu_si128(ptemp + 3));
			_mm_storeu_si128(ptemp, loader1);
			_mm_storeu_si128(ptemp + 1, loader2);
			_mm_storeu_si128(ptemp + 2, loader3);
			_mm_storeu_si128(ptemp + 3, loader4);
			pa += 4;
			ptemp += 4;
		}
		round = (a.size - round * 16) / 4;
		rest -= round * 4;
		for (int i = 0; i < round; ++i)
		{
			loader1 = _mm_add_epi32(_mm_loadu_si128(pa), _mm_loadu_si128(ptemp));
			_mm_storeu_si128(ptemp, loader1);
			pa += 1;
			ptemp += 1;
		}
		for (int i = a.size - rest; i < a.size; ++i)
			temp.row[i] += a.row[i];
#else
		for (int i = 0; i < a.size; ++i)
			temp.row[i] += a.row[i];
#endif
		for (int i = 0; i < b.size; ++i)
		{
			if (temp.row[i] >= 10)
			{
				temp.row[i] -= 10;
				temp.row[i + 1] += 1;
			}
		}
		if (temp.row[b.size] != 0)
			temp.size = b.size + 1;
		return temp;
	}
}

Bigint Bigint::__plain_subtract(const Bigint &a, const Bigint &b)
{

	Bigint temp(a.size + 2);
	temp = a;
#if defined AVX
	int round = b.size / 32;
	int rest = b.size - round * 32;
	__m256i loader1, loader2, loader3, loader4;
	__m256i *pb = reinterpret_cast<__m256i*>(b.row);
	__m256i *ptemp = reinterpret_cast<__m256i*>(temp.row);
	for (int i = 0; i < round; ++i)
	{
		loader1 = _mm256_sub_epi32(_mm256_loadu_si256(ptemp), _mm256_loadu_si256(pb));
		loader2 = _mm256_sub_epi32(_mm256_loadu_si256(ptemp + 1), _mm256_loadu_si256(pb + 1));
		loader3 = _mm256_sub_epi32(_mm256_loadu_si256(ptemp + 2), _mm256_loadu_si256(pb + 2));
		loader4 = _mm256_sub_epi32(_mm256_loadu_si256(ptemp + 3), _mm256_loadu_si256(pb + 3));
		_mm256_storeu_si256(ptemp, loader1);
		_mm256_storeu_si256(ptemp + 1, loader2);
		_mm256_storeu_si256(ptemp + 2, loader3);
		_mm256_storeu_si256(ptemp + 3, loader4);
		pb += 4;
		ptemp += 4;
	}
	round = (b.size - round * 32) / 8;
	rest -= round * 8;
	for (int i = 0; i < round; ++i)
	{
		loader1 = _mm256_sub_epi32(_mm256_loadu_si256(ptemp), _mm256_loadu_si256(pb));
		_mm256_storeu_si256(ptemp, loader1);
		pb += 1;
		ptemp += 1;
	}
	for (int i = b.size - rest; i < b.size; ++i)
		temp.row[i] -= b.row[i];
#elif defined SSE
	int round = b.size / 16;
	int rest = b.size - round * 16;
	__m128i loader1, loader2, loader3, loader4;
	__m128i *pb = reinterpret_cast<__m128i*>(b.row);
	__m128i *ptemp = reinterpret_cast<__m128i*>(temp.row);
	for (int i = 0; i < round; ++i)
	{
		loader1 = _mm_sub_epi32(_mm_loadu_si128(ptemp), _mm_loadu_si128(pb));
		loader2 = _mm_sub_epi32(_mm_loadu_si128(ptemp + 1), _mm_loadu_si128(pb + 1));
		loader3 = _mm_sub_epi32(_mm_loadu_si128(ptemp + 2), _mm_loadu_si128(pb + 2));
		loader4 = _mm_sub_epi32(_mm_loadu_si128(ptemp + 3), _mm_loadu_si128(pb + 3));
		_mm_storeu_si128(ptemp, loader1);
		_mm_storeu_si128(ptemp + 1, loader2);
		_mm_storeu_si128(ptemp + 2, loader3);
		_mm_storeu_si128(ptemp + 3, loader4);
		pb += 4;
		ptemp += 4;
	}
	round = (b.size - round * 16) / 4;
	rest -= round * 4;
	for (int i = 0; i < round; ++i)
	{
		loader1 = _mm_sub_epi32(_mm_loadu_si128(ptemp), _mm_loadu_si128(pb));
		_mm_storeu_si128(ptemp, loader1);
		pb += 1;
		ptemp += 1;
	}
	for (int i = b.size - rest; i < b.size; ++i)
		temp.row[i] -= b.row[i];
#else
	for (int i = b.size - 1; i >= 0; --i)
		temp.row[i] -= b.row[i];
#endif
	for (int i = 0; i < a.size; ++i)
	{
		if (temp.row[i] < 0)
		{
			temp.row[i] += 10;
			temp.row[i + 1] -= 1;
		}
	}
	for (int i = a.size - 1; i >= 0; --i)
	{
		if (temp.row[i] != 0)
		{
			temp.size = i + 1;
			break;
		}
		if (i == 0)
			temp.size = 1;
	}
	return temp;
}

Bigint Bigint::__plain_multiply(const Bigint &a, const Bigint &b)
{
	Bigint temp(a.size + b.size + 3);
	if (__abs_smaller_than(a, b))
	{
#if defined AVX
		__m256i loader1, loader2, loader3, loader4, factor;
		__m256i *pb = reinterpret_cast<__m256i*>(b.row);
		__m256i *ptemp = reinterpret_cast<__m256i*>(temp.row);
		int bigRound = b.size / 32;
		int smallRound = (b.size - bigRound * 32) / 8;
		int rest = b.size - bigRound * 32 - smallRound * 8;
		int threshold = 0;
		for (int i = 0; i < a.size; ++i)
		{
			pb = reinterpret_cast<__m256i*>(b.row);
			ptemp = reinterpret_cast<__m256i*>(temp.row + i);
			factor = _mm256_set1_epi32(a.row[i]);
			for (int j = 0; j < bigRound; ++j)
			{
				loader1 = _mm256_mullo_epi32(_mm256_loadu_si256(pb), factor);
				loader2 = _mm256_mullo_epi32(_mm256_loadu_si256(pb + 1), factor);
				loader3 = _mm256_mullo_epi32(_mm256_loadu_si256(pb + 2), factor);
				loader4 = _mm256_mullo_epi32(_mm256_loadu_si256(pb + 3), factor);
				loader1 = _mm256_add_epi32(loader1, *ptemp);
				loader2 = _mm256_add_epi32(loader2, *(ptemp + 1));
				loader3 = _mm256_add_epi32(loader3, *(ptemp + 2));
				loader4 = _mm256_add_epi32(loader4, *(ptemp + 3));
				_mm256_storeu_si256(ptemp, loader1);
				_mm256_storeu_si256(ptemp + 1, loader2);
				_mm256_storeu_si256(ptemp + 2, loader3);
				_mm256_storeu_si256(ptemp + 3, loader4);
				pb += 4;
				ptemp += 4;
				threshold += 32;
				if (threshold > 1000000)
				{
					threshold = 0;
					for (int i = 0; i < temp.capacity; ++i)
					{
						if (temp.row[i] >= 10)
						{
							temp.row[i + 1] += temp.row[i] / 10;
							temp.row[i] %= 10;
						}
					}
				}
			}
			for (int j = 0; j < smallRound; ++j)
			{
				loader1 = _mm256_mullo_epi32(_mm256_loadu_si256(pb), factor);
				loader1 = _mm256_add_epi32(loader1, *ptemp);
				_mm256_storeu_si256(ptemp, loader1);
				pb += 1;
				ptemp += 1;
				threshold += 8;
				if (threshold > 1000000)
				{
					threshold = 0;
					for (int i = 0; i < temp.capacity; ++i)
					{
						if (temp.row[i] >= 10)
						{
							temp.row[i + 1] += temp.row[i] / 10;
							temp.row[i] %= 10;
						}
					}
				}
			}
			for (int j = b.size - rest; j < b.size; ++j)
			{
				temp.row[i + j] += b.row[j] * a.row[i];
				threshold += 1;
				if (threshold > 1000000)
				{
					threshold = 0;
					for (int i = 0; i < temp.capacity; ++i)
					{
						if (temp.row[i] >= 10)
						{
							temp.row[i + 1] += temp.row[i] / 10;
							temp.row[i] %= 10;
						}
					}
				}
			}
		}
#elif defined SSE
		__m128i loader1, loader2, loader3, loader4, factor;
		__m128i *pb = reinterpret_cast<__m128i*>(b.row);
		__m128i *ptemp = reinterpret_cast<__m128i*>(temp.row);
		int bigRound = b.size / 16;
		int smallRound = (b.size - bigRound * 16) / 4;
		int rest = b.size - bigRound * 16 - smallRound * 4;
		int threshold = 0;
		for (int i = 0; i < a.size; ++i)
		{
			factor = _mm_set1_epi32(a.row[i]);
			pb = reinterpret_cast<__m128i*>(b.row);
			ptemp = reinterpret_cast<__m128i*>(temp.row + i);
			for (int j = 0; j < bigRound; ++j)
			{
				loader1 = _mm_mullo_epi32(_mm_loadu_si128(pb), factor);
				loader2 = _mm_mullo_epi32(_mm_loadu_si128(pb + 1), factor);
				loader3 = _mm_mullo_epi32(_mm_loadu_si128(pb + 2), factor);
				loader4 = _mm_mullo_epi32(_mm_loadu_si128(pb + 3), factor);
				loader1 = _mm_add_epi32(loader1, *ptemp);
				loader2 = _mm_add_epi32(loader2, *(ptemp + 1));
				loader3 = _mm_add_epi32(loader3, *(ptemp + 2));
				loader4 = _mm_add_epi32(loader4, *(ptemp + 3));
				_mm_storeu_si128(ptemp, loader1);
				_mm_storeu_si128(ptemp + 1, loader2);
				_mm_storeu_si128(ptemp + 2, loader3);
				_mm_storeu_si128(ptemp + 3, loader4);
				pb += 4;
				ptemp += 4;
				threshold += 16;
				if (threshold > 1000000)
				{
					threshold = 0;
					for (int i = 0; i < temp.capacity; ++i)
					{
						if (temp.row[i] >= 10)
						{
							temp.row[i + 1] += temp.row[i] / 10;
							temp.row[i] %= 10;
						}
					}
				}
			}
			for (int j = 0; j < smallRound; ++j)
			{
				loader1 = _mm_mullo_epi32(_mm_loadu_si128(pb), factor);
				loader1 = _mm_add_epi32(loader1, *ptemp);
				_mm_storeu_si128(ptemp, loader1);
				pb += 1;
				ptemp += 1;
				threshold += 4;
				if (threshold > 1000000)
				{
					threshold = 0;
					for (int i = 0; i < temp.capacity; ++i)
					{
						if (temp.row[i] >= 10)
						{
							temp.row[i + 1] += temp.row[i] / 10;
							temp.row[i] %= 10;
						}
					}
				}
			}
			for (int j = b.size - rest; j < b.size; ++j)
			{
				temp.row[i + j] += b.row[j] * a.row[i];
				threshold += 1;
				if (threshold > 1000000)
				{
					threshold = 0;
					for (int i = 0; i < temp.capacity; ++i)
					{
						if (temp.row[i] >= 10)
						{
							temp.row[i + 1] += temp.row[i] / 10;
							temp.row[i] %= 10;
						}
					}
				}
			}
		}
#else
		int threshold = 0;
		for (int i = 0; i < a.size; ++i)
		{
			for (int j = 0; j < b.size; ++j)
			{
				temp.row[i + j] += b.row[j] * a.row[i];
				threshold += 1;
				if (threshold > 1000000)
				{
					threshold = 0;
					for (int i = 0; i < temp.capacity; ++i)
					{
						if (temp.row[i] >= 10)
						{
							temp.row[i + 1] += temp.row[i] / 10;
							temp.row[i] %= 10;
						}
					}
				}
			}
		}
#endif
	}
	else
	{
#if defined AVX
		__m256i loader1, loader2, loader3, loader4, factor;
		__m256i *pa = reinterpret_cast<__m256i*>(a.row);
		__m256i *ptemp = reinterpret_cast<__m256i*>(temp.row);
		int bigRound = a.size / 32;
		int smallRound = (a.size - bigRound * 32) / 8;
		int rest = a.size - bigRound * 32 - smallRound * 8;
		int threshold = 0;
		for (int i = 0; i < b.size; ++i)
		{
			pa = reinterpret_cast<__m256i*>(a.row);
			ptemp = reinterpret_cast<__m256i*>(temp.row + i);
			factor = _mm256_set1_epi32(b.row[i]);
			for (int j = 0; j < bigRound; ++j)
			{
				loader1 = _mm256_mullo_epi32(_mm256_loadu_si256(pa), factor);
				loader2 = _mm256_mullo_epi32(_mm256_loadu_si256(pa + 1), factor);
				loader3 = _mm256_mullo_epi32(_mm256_loadu_si256(pa + 2), factor);
				loader4 = _mm256_mullo_epi32(_mm256_loadu_si256(pa + 3), factor);
				loader1 = _mm256_add_epi32(loader1, *ptemp);
				loader2 = _mm256_add_epi32(loader2, *(ptemp + 1));
				loader3 = _mm256_add_epi32(loader3, *(ptemp + 2));
				loader4 = _mm256_add_epi32(loader4, *(ptemp + 3));
				_mm256_storeu_si256(ptemp, loader1);
				_mm256_storeu_si256(ptemp + 1, loader2);
				_mm256_storeu_si256(ptemp + 2, loader3);
				_mm256_storeu_si256(ptemp + 3, loader4);
				pa += 4;
				ptemp += 4;
				threshold += 32;
				if (threshold > 1000000)
				{
					threshold = 0;
					for (int i = 0; i < temp.capacity; ++i)
					{
						if (temp.row[i] >= 10)
						{
							temp.row[i + 1] += temp.row[i] / 10;
							temp.row[i] %= 10;
						}
					}
				}
			}
			for (int j = 0; j < smallRound; ++j)
			{
				loader1 = _mm256_mullo_epi32(_mm256_loadu_si256(pa), factor);
				loader1 = _mm256_add_epi32(loader1, *ptemp);
				_mm256_storeu_si256(ptemp, loader1);
				pa += 1;
				ptemp += 1;
				threshold += 8;
				if (threshold > 1000000)
				{
					threshold = 0;
					for (int i = 0; i < temp.capacity; ++i)
					{
						if (temp.row[i] >= 10)
						{
							temp.row[i + 1] += temp.row[i] / 10;
							temp.row[i] %= 10;
						}
					}
				}
			}
			for (int j = a.size - rest; j < a.size; ++j)
			{
				temp.row[i + j] += a.row[j] * b.row[i];
				threshold += 1;
				if (threshold > 1000000)
				{
					threshold = 0;
					for (int i = 0; i < temp.capacity; ++i)
					{
						if (temp.row[i] >= 10)
						{
							temp.row[i + 1] += temp.row[i] / 10;
							temp.row[i] %= 10;
						}
					}
				}
			}
		}
#elif defined SSE
		__m128i loader1, loader2, loader3, loader4, factor;
		__m128i *pa = reinterpret_cast<__m128i*>(a.row);
		__m128i *ptemp = reinterpret_cast<__m128i*>(temp.row);
		int bigRound = a.size / 16;
		int smallRound = (a.size - bigRound * 16) / 4;
		int rest = a.size - bigRound * 16 - smallRound * 4;
		int threshold = 0;
		for (int i = 0; i < b.size; ++i)
		{
			factor = _mm_set1_epi32(b.row[i]);
			pa = reinterpret_cast<__m128i*>(a.row);
			ptemp = reinterpret_cast<__m128i*>(temp.row + i);
			for (int j = 0; j < bigRound; ++j)
			{
				loader1 = _mm_mullo_epi32(_mm_loadu_si128(pa), factor);
				loader2 = _mm_mullo_epi32(_mm_loadu_si128(pa + 1), factor);
				loader3 = _mm_mullo_epi32(_mm_loadu_si128(pa + 2), factor);
				loader4 = _mm_mullo_epi32(_mm_loadu_si128(pa + 3), factor);
				loader1 = _mm_add_epi32(loader1, *ptemp);
				loader2 = _mm_add_epi32(loader2, *(ptemp + 1));
				loader3 = _mm_add_epi32(loader3, *(ptemp + 2));
				loader4 = _mm_add_epi32(loader4, *(ptemp + 3));
				_mm_storeu_si128(ptemp, loader1);
				_mm_storeu_si128(ptemp + 1, loader2);
				_mm_storeu_si128(ptemp + 2, loader3);
				_mm_storeu_si128(ptemp + 3, loader4);
				pa += 4;
				ptemp += 4;
				threshold += 16;
				if (threshold > 1000000)
				{
					threshold = 0;
					for (int i = 0; i < temp.capacity; ++i)
					{
						if (temp.row[i] >= 10)
						{
							temp.row[i + 1] += temp.row[i] / 10;
							temp.row[i] %= 10;
						}
					}
				}
			}
			for (int j = 0; j < smallRound; ++j)
			{
				loader1 = _mm_mullo_epi32(_mm_loadu_si128(pa), factor);
				loader1 = _mm_add_epi32(loader1, *ptemp);
				_mm_storeu_si128(ptemp, loader1);
				pa += 1;
				ptemp += 1;
				threshold += 4;
				if (threshold > 1000000)
				{
					threshold = 0;
					for (int i = 0; i < temp.capacity; ++i)
					{
						if (temp.row[i] >= 10)
						{
							temp.row[i + 1] += temp.row[i] / 10;
							temp.row[i] %= 10;
						}
					}
				}
			}
			for (int j = a.size - rest; j < a.size; ++j)
			{
				temp.row[i + j] += a.row[j] * b.row[i];
				threshold += 1;
				if (threshold > 1000000)
				{
					threshold = 0;
					for (int i = 0; i < temp.capacity; ++i)
					{
						if (temp.row[i] >= 10)
						{
							temp.row[i + 1] += temp.row[i] / 10;
							temp.row[i] %= 10;
						}
					}
				}
			}
		}
#else
		int threshold = 0;
		for (int i = 0; i < b.size; ++i)
		{
			for (int j = 0; j < a.size; ++j)
			{
				temp.row[i + j] += a.row[j] * b.row[i];
				threshold += 1;
				if (threshold > 1000000)
				{
					threshold = 0;
					for (int i = 0; i < temp.capacity; ++i)
					{
						if (temp.row[i] >= 10)
						{
							temp.row[i + 1] += temp.row[i] / 10;
							temp.row[i] %= 10;
						}
					}
				}
			}
		}
#endif
	}
	for (int i = 0; i < temp.capacity; ++i)
	{
		if (temp.row[i] >= 10)
		{
			temp.row[i + 1] += temp.row[i] / 10;
			temp.row[i] %= 10;
		}
	}
	temp.size = 0;
	for (int i = temp.capacity - 1; i >= 0; --i)
	{
		if (temp.row[i] != 0)
		{
			temp.size = i + 1;
			break;
		}
	}
	if (temp.size == 0)
		temp.size = 1;
	return temp;
}

Bigint Bigint::operator-(const Bigint &x)
{
	if (isNegative)
	{
		if (x.isNegative)
		{
			if (__abs_smaller_than(*this, x))
			{
				Bigint temp = __plain_subtract(x, *this);
				temp.isNegative = false;
				temp.__eliminate_double_zero();
				return temp;
			}
			else
			{
				Bigint temp = __plain_subtract(*this, x);
				temp.isNegative = true;
				temp.__eliminate_double_zero();
				return temp;
			}
		}
		else
		{
			Bigint temp = __plain_add(*this, x);
			temp.isNegative = true;
			temp.__eliminate_double_zero();
			return temp;
		}
	}
	else
	{
		if (x.isNegative)
		{
			Bigint temp = __plain_add(*this, x);
			temp.isNegative = false;
			temp.__eliminate_double_zero();
			return temp;
		}
		else
		{
			if (__abs_smaller_than(*this, x))
			{
				Bigint temp = __plain_subtract(x, *this);
				temp.isNegative = true;
				temp.__eliminate_double_zero();
				return temp;
			}
			else
			{
				Bigint temp = __plain_subtract(*this, x);
				temp.isNegative = false;
				temp.__eliminate_double_zero();
				return temp;
			}
		}
	}
}

Bigint Bigint::operator+(const Bigint &x)
{
	if (isNegative)
	{
		if (x.isNegative)
		{
			Bigint temp = __plain_add(*this, x);
			temp.isNegative = true;
			temp.__eliminate_double_zero();
			return temp;
		}
		else
		{
			if (__abs_smaller_than(*this, x))
			{
				Bigint temp = __plain_subtract(x, *this);
				temp.isNegative = false;
				temp.__eliminate_double_zero();
				return temp;
			}
			else
			{
				Bigint temp = __plain_subtract(*this, x);
				temp.isNegative = true;
				temp.__eliminate_double_zero();
				return temp;
			}
		}
	}
	else
	{
		if (x.isNegative)
		{
			if (__abs_smaller_than(*this, x))
			{
				Bigint temp = __plain_subtract(x, *this);
				temp.isNegative = true;
				temp.__eliminate_double_zero();
				return temp;
			}
			else
			{
				Bigint temp = __plain_subtract(*this, x);
				temp.isNegative = false;
				temp.__eliminate_double_zero();
				return temp;
			}
		}
		else
		{
			Bigint temp = __plain_add(*this, x);
			temp.isNegative = false;
			temp.__eliminate_double_zero();
			return temp;
		}
	}
}

Bigint Bigint::operator*(const Bigint &x)
{
	Bigint temp(size + x.size + 3);
	temp = __plain_multiply(*this, x);
	if (isNegative == !x.isNegative)
		temp.isNegative = true;
	else
		temp.isNegative = false;
	temp.__eliminate_double_zero();
	return temp;
}

Bigint Bigint::operator%(const Bigint &x)
{
	Bigint result = *this;
	Bigint temp = x, two;
	two.load("2");
	std::stack<Bigint> sequence;
	for (;;)
	{
		if (!(*this < temp))
		{
			sequence.push(temp);
			temp = temp * two;
		}
		else
			break;
	}
	while (!sequence.empty())
	{
		if (!(result < sequence.top()))
			result = result - sequence.top();
		sequence.pop();
	}
	return result;
}

Bigint& Bigint::operator=(Bigint &&x)
{
	delete[] row;
	row = x.row;
	size = x.size;
	capacity = x.capacity;
	isNegative = x.isNegative;
	x.row = new int[1];
	return *this;
}

Bigint& Bigint::operator=(const Bigint &x)
{
	if (capacity < x.size + 1)
	{
		delete[] row;
		row = new int[x.size + 1];
		memcpy(row, x.row, sizeof(int) * x.size);
		row[x.size - 1] = 0;
		size = x.size;
		capacity = size + 1;
		isNegative = x.isNegative;
	}
	else
	{
		memset(row, 0, sizeof(int) * capacity);
		memcpy(row, x.row, sizeof(int) * x.size);
		size = x.size;
		isNegative = x.isNegative;
	}
	return *this;
}

bool Bigint::operator<(const Bigint &x)
{
	if (isNegative)
	{
		if (!x.isNegative)
			return true;
		else
		{
			if (size < x.size)
				return false;
			else if (size > x.size)
				return true;
			else
			{
				for (int i = size - 1; i >= 0; --i)
				{
					if (row[i] > x.row[i])
						return true;
					else if (row[i] < x.row[i])
						return false;
				}
				return false;
			}
		}
	}
	else
	{
		if (x.isNegative)
			return false;
		else
		{
			if (size < x.size)
				return true;
			else if (size > x.size)
				return false;
			else
			{
				for (int i = size - 1; i >= 0; --i)
				{
					if (row[i] < x.row[i])
						return true;
					else if (row[i] > x.row[i])
						return false;
				}
				return false;
			}
		}
	}
}

bool Bigint::operator==(const Bigint &x)
{
	if (size != x.size)
		return false;
	for (int i = 0; i < size; ++i)
		if (row[i] != x.row[i])
			return false;
	return true;
}

void Bigint::print()
{
	if (isNegative)
		putchar('-');
	for (int i = size - 1; i >= 0; --i)
		putchar(char(row[i] + '0'));
	putchar('\n');
}

void Bigint::load(const char* input_)
{
	int length = strlen(input_);
	if (input_[0] == '-')
	{
		if (length - 1 >= capacity)
		{
			delete[] row;
			row = new int[length];
			capacity = length;
		}
		memset(row, 0, sizeof(int) * capacity);
		for (int i = length - 1; i >= 1; --i)
			row[length - 1 - i] = input_[i] - '0';
		size = length - 1;
		isNegative = true;
	}
	else
	{
		if (length >= capacity)
		{
			delete[] row;
			row = new int[length + 1];
			capacity = length + 1;
		}
		memset(row, 0, sizeof(int) * capacity);
		for (int i = length - 1; i >= 0; --i)
			row[length - 1 - i] = input_[i] - '0';
		size = length;
		isNegative = false;
	}
	__eliminate_double_zero();
}

void Bigint::extract(char *target_)
{
	if (isNegative)
	{
		target_[0] = '-';
		for (int i = 0; i < size; ++i)
			target_[size - i] = row[i] + '0';
		target_[size + 1] = '\0';
	}
	else
	{
		for (int i = 0; i < size; ++i)
			target_[size - 1 - i] = row[i] + '0';
		target_[size] = '\0';
	}
}

void Bigint::cutHalf()
{
	for (int i = size - 1; i >= 1; --i)
	{
		if (row[i] % 2)
			row[i - 1] += 10;
		row[i] /= 2;
	}
	row[0] /= 2;
	if (row[size - 1] == 0 && size != 1)
		--size;
}
