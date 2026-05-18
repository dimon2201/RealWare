// thread_pool.cpp

#pragma once

#include <iostream>
#include "application.hpp"
#include "thread_subsystem.hpp"
#include "buffer.hpp"

using namespace types;

triton::cWork::cWork(cBuffer* data, WorkFunction&& function)
    :
    _data(data),
    _function(std::make_shared<WorkFunction>(std::move(function)))
{
}

void triton::cWork::Run()
{
    if (_function)
        _function->operator()(_data);
}

triton::cThread::cThread(cThread::eType type) : _type(type)
{
    if (_type == eType::WORKER)
    {
        _thread = std::thread(
            [this]
            {
                while (K_TRUE)
                {
                    if (_pause.load() == K_TRUE)
                        continue;

                    cWork task;
                    {
                        std::unique_lock<std::mutex> lock(_mtx);
                        _cv.wait(lock, [this] {
                            return !_tasks.empty() || _stop.load();
                        });
                        if (_stop.load() && _tasks.empty())
                            return;
                        task = _tasks.front();
                        _tasks.pop();
                    }
                    task.Run();
                }
            }
        );
    }
}

triton::cThread::~cThread()
{
    Stop();
    _thread.join();
}

void triton::cThread::SubmitWork(cWork& task)
{
    _tasks.emplace(task);
    _cv.notify_one();
}

triton::cThreadSubsystem::cThreadSubsystem(cContext* context, usize threadCount)
    : iObject(context),
    _threadCount(threadCount)
{
    _pThreads = (cThread*)malloc(threadCount * sizeof(cThread));
    for (usize i = 0; i < _threadCount; ++i)
        new (&_pThreads[i]) cThread(cThread::eType::WORKER);
}

triton::cThreadSubsystem::~cThreadSubsystem()
{
    for (usize i = 0; i < _threadCount; ++i)
        _pThreads[i].~cThread();
    free(_pThreads);
}

void triton::cThreadSubsystem::SubmitWork(cWork& task)
{
    _pThreads[_lastWorkThreadID].SubmitWork(task);
    _lastWorkThreadID = (_lastWorkThreadID + 1) % _threadCount;
}