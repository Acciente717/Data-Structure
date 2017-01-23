#include <iostream>
#include <cstring>
using namespace std;

template <class Type>

class Queue
{
protected:
	Type *pointer;
	int head;
	int rear;
	int cap; // Real-capacity == cap - 1

public:
	// Default Constructor
	Queue()
	{
		pointer = new Type[11];
		if (!pointer)
		{
			cout << "Out of memory!" << endl;
			exit(0);
		}
		head = 0;
		rear = 0;
		cap = 11;
	}

	// Customized Constructor
	Queue(int initial_capacity)
	{
		if (initial_capacity <= 0)
		{
			cout << "Capacity must be greater than 0!" << endl;
			exit(0);
		}
		pointer = new Type[initial_capacity + 1];
		if (!pointer)
		{
			cout << "Out of memory!" << endl;
			exit(0);
		}
		head = 0;
		rear = 0;
		cap = initial_capacity + 1;
	}

	// Copy Constructor
	Queue(Queue const &source)
	{
		pointer = new Type[source.cap];
		if (!pointer)
		{
			cout << "Out of memory!" << endl;
			exit(0);
		}
		cap = source.cap;
		head = source.head;
		rear = source.rear;
		memcpy(pointer, source.pointer, sizeof(Type) * cap);
	}

	// Destructor
	~Queue()
	{
		delete[] pointer;
	}

	// Operation: Get size
	int size()
	{
		if (rear >= head)
			return rear - head;
		else
			return rear + cap - head;
	}

	// Operation: Get capacity
	int capacity()
	{
		return cap - 1;
	}

	// Operation: Enqueue
	void enqueue(Type x)
	{
		if (size() >= cap - 1)
		{
			Type *newPointer = new Type[cap * 2]; // Overflow! Allocating a larger space.
			if (!newPointer)
			{
				cout << "Out of memory!" << endl;
				exit(0);
			}
			if (head <= rear)
			{
				memcpy(newPointer, pointer + head, sizeof(Type) * (rear - head));
				rear = rear - head;
				head = 0;
				cap *= 2;
			}
			else
			{
				memcpy(newPointer, pointer + head, sizeof(Type) * (cap - head));
				memcpy(newPointer + cap - head, pointer, sizeof(Type) * rear);
				rear = rear + cap - head;
				head = 0;
				cap *= 2;
			}
			delete[] pointer;
			pointer = newPointer;
		}
		pointer[rear] = x;
		rear = (rear + 1) % cap;
	}

	// Operation: Dequeue
	void dequeue()
	{
		if (head == rear)
		{
			cout << "Queue underflow!" << endl;
			exit(0);
		}
		head = (head + 1) % cap;
	}

	// Operation: Front
	Type front()
	{
		if (head == rear)
		{
			cout << "Empty queue!" << endl;
			exit(0);
		}
		return pointer[head];
	}

	// Operation: TakeFront
	Type takeFront()
	{
		if (head == rear)
		{
			cout << "Empty queue!" << endl;
			exit(0);
		}
		int mem = head;
		head = (head + 1) % cap;
		return pointer[mem];
	}

	// Operation: Clear
	void clear()
	{
		head = rear;
	}

	// Operator: =
	Queue& operator=(Queue const &source)
	{
		delete[] pointer;
		pointer = new Type[source.cap];
		if (!pointer)
		{
			cout << "Out of memory!" << endl;
			exit(0);
		}
		memcpy(pointer, source.pointer, sizeof(Type) * source.cap);
		head = source.head;
		rear = source.rear;
		cap = source.cap;
	}
};
