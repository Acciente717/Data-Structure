#include <iostream>
#include <memory>
using namespace std;

template <class Type>

class Stack_seq
{
protected:
	Type* pointer;
	int stackTop;
	int cap;

public:
	// Default Constructor
	Stack_seq()
	{
		pointer = new Type[10];
		stackTop = -1;
		cap = 10;
	}

	// Constructor with customized capacity
	Stack_seq(int initial_capacity)
	{
		if (initial_capacity <= 0)
		{
			cout << "Invalid capacity! Must greater than 0!" << endl;
			cout << "Using default capacity 10." << endl;
			pointer = new Type[10];
			stackTop = -1;
			cap = 10;
		}
		else
		{
			pointer = new Type[initial_capacity];
			stackTop = -1;
			cap = initial_capacity;
		}
	}

	// Copy Constructor
	Stack_seq(Stack_seq const &source)
	{
		cap = source.cap;
		stackTop = source.stackTop;
		pointer = new Type[cap];
		memcpy(pointer, source.pointer, (stackTop + 1) * sizeof(Type));
	}

	// Destructor
	~Stack_seq()
	{
		delete[] pointer;
	}

	// Operation: push
	bool push(Type element)
	{
		if (stackTop == cap - 1)
		{
			Type *newPointer = new Type[cap * 2];
			if (newPointer == 0)
			{
				cout << "Out of memory!" << endl;
				return false;
			}
			memcpy(newPointer, pointer, sizeof(Type) * (cap + 1));
			delete[] pointer;
			pointer = newPointer;
			cap *= 2;
		}
		pointer[++stackTop] = element;
		return true;
	}

	// Operation: pop
	bool pop()
	{
		if (stackTop == -1)
		{
			cout << "The stack is already empty!" << endl;
			return false;
		}
		stackTop--;
		return true;
	}

	// Operation: top
	Type top()
	{
		if (stackTop == -1)
		{
			cout << "Empty stack!" << endl;
			exit(0);
		}
		return pointer[stackTop];
	}

	// Operator: = 
	Stack_seq& operator=(Stack_seq const &source)
	{
		if (source.stackTop > cap - 1)
		{
			Type *newPointer = new Type[source.stackTop + 1];
			if (newPointer == 0)
			{
				cout << "Out of memory!" << endl;
				exit(0);
			}
			stackTop = source.stackTop;
			cap = source.stackTop + 1;
			delete[] pointer;
			pointer = newPointer;
			memcpy(pointer, source.pointer, sizeof(Type) * (stackTop + 1));
		}
		else
		{
			stackTop = source.stackTop;
			memcpy(pointer, source.pointer, sizeof(Type) * (stackTop + 1));
		}
	}

	// Operation: clear
	void clear()
	{
		stackTop = -1;
	}

	// Operation: size
	int size()
	{
		return stackTop + 1;
	}

	// Operation: capacity
	int capacity()
	{
		return capacity;
	}
};
