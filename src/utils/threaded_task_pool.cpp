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

#include "threaded_task_pool.h"

#include <functional> // for bind()

ThreadedTaskPool::ThreadedTaskPool() : 
    m_active(false),
	m_numThreads(1)
{
	
}

ThreadedTaskPool::~ThreadedTaskPool()
{
	
}

void ThreadedTaskPool::addTask(Task* pTask)
{
	m_aTasks.push(pTask);
}

void ThreadedTaskPool::start(unsigned int threads)
{
	m_active = true;
	
	m_numThreads = threads;
	
	for (unsigned int i = 0; i < m_numThreads; i++)
	{
		std::thread newThread = std::thread(std::bind(&ThreadedTaskPool::workerThreadFunctionProcess, this));
		m_aWorkerThreads.emplace_back(std::move(newThread));
	}

	for (std::thread& workerThread : m_aWorkerThreads)
	{
		workerThread.join();
	}
}

void ThreadedTaskPool::workerThreadFunctionProcess()
{
	Task* pThisTask = nullptr;
	
	while (m_active)
	{
		{
			std::unique_lock<std::mutex> lock(m_lock);
	
			if (m_aTasks.empty())
			{
				m_active = false;
				return;
			}
			
			pThisTask = std::move(m_aTasks.front());
			m_aTasks.pop();
		}
		
		processTask(pThisTask);
	}
}

void ThreadedTaskPool::workerThreadFunctionEvent()
{
	Task* pThisTask = nullptr;
	
	while (m_active)
	{
		{
			std::unique_lock<std::mutex> lock(m_lock);
			m_newTaskEvent.wait(lock);
			
			pThisTask = std::move(m_aTasks.front());
			m_aTasks.pop();
		}
		
		processTask(pThisTask);
	}
}
