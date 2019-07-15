#ifndef __TASKQUEUE_H__
#define __TASKQUEUE_H__
#include "MutexLock.h"
#include "Condition.h"

#include <queue>
#include <functional>
using namespace std;

using Task = std::function<void()>;
using ElemType = Task ;

class TaskQueue
{
public:
	TaskQueue(size_t sz);

	bool empty() const;
	bool full() const;
	void push(ElemType elem);
	ElemType pop();
	void wakeup();

private:
	size_t _sz;
	queue<ElemType> _que;
	MutexLock _mutex;
	Condition _notEmpty;
	Condition _notFull;
	bool _flag;
};

#endif
