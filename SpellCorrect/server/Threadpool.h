#ifndef __WD_THREADPOOL_H__
#define __WD_THREADPOOL_H__
#include "TaskQueue.h"
#include "Thread.h"

#include <vector>
#include <memory>
#include <functional>
using namespace std;

using Task = function<void()>;
class Threadpool
{
public:
	Threadpool(size_t, size_t);

	void start();
	void stop();

	void addTask(Task && task);

private:
	void threadFunc();
	Task getTask();

private:
	size_t _threadNum;
	size_t _queSize;
	vector<unique_ptr<Thread>> _threads;
	TaskQueue _que;
	bool _isExit;
};

#endif 
