/*
 Sniffle
 Copyright 2018 Peter Pearson.

 Licensed under the Apache License, Version 2.0 (the "License");
 You may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 ---------
*/

#ifndef THREADED_TASK_POOL_H
#define THREADED_TASK_POOL_H

#include <thread>
#include <atomic>
#include <condition_variable>

#include <vector>
#include <queue>

class ThreadedTaskPool
{
public:
	ThreadedTaskPool();
	~ThreadedTaskPool();
	
	class Task
	{
	public:
		Task()
		{
		}
		
		virtual ~Task()
		{
		}
	};
	
	void addTask(Task* pTask);
protected:
	
	void start(unsigned int threads);
	
	// it's (currently) the subclass's job to delete the task object when finished
	virtual void processTask(Task* pTask) = 0;	
	
	void workerThreadFunctionProcess(); // just process what's in the list
	void workerThreadFunctionEvent(); // continue.
	
protected:
	std::vector<std::thread>		m_aWorkerThreads;
	
	volatile bool					m_active;
	
	std::mutex						m_lock;
	unsigned int					m_numThreads;

	std::queue<Task*>				m_aTasks;

	std::condition_variable			m_newTaskEvent;
};

#endif // THREADED_TASK_POOL_H
