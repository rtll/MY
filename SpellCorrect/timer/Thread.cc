 ///
 /// @file    Thread.cc
 /// @author  lemon(haohb13@gmail.com)
 /// @date    2019-06-08 10:02:21
 ///
 
#include "Thread.h"

#include <stdio.h>
#include <errno.h>
#include <iostream>
using std::cout;
using std::endl;

namespace wd
{

Thread::~Thread()
{
	if(_isRunning) {
		pthread_detach(_pthid);	
		_isRunning = false;
	}
	cout << "~Thread()" << endl;
}

void Thread::start()
{
	if(pthread_create(&_pthid, nullptr, threadFunc, this)) {
		perror("pthread_create");
		return;
	}

	_isRunning = true;
}

void Thread::join()
{
	if(_isRunning) {
		pthread_join(_pthid, nullptr);
		_isRunning = false;
	}
}

void * Thread::threadFunc(void * arg)
{
	Thread * pthread = static_cast<Thread*>(arg);
	if(pthread)
		pthread->_cb();

	return nullptr;
}

}//end of namespace wd
