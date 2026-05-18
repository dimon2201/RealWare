// thread_pool.cpp

#pragma once

#include <iostream>
#include "application.hpp"
#include "thread_subsystem.hpp"
#include "buffer.hpp"

using namespace types;

triton::cTask::cTask(cBuffer* data, TaskFunction&& function) : _data(data), _function(std::make_shared<TaskFunction>(std::move(function)))
{
}

void triton::cTask::Run()
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

                    cTask task;
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

void triton::cThread::SubmitWork(cTask& task)
{
    _tasks.emplace(task);
    _cv.notify_one();
}

triton::cThreadSubsystem::cThreadSubsystem(cContext* context, usize threadCount)
    : iObject(context),
    _threadCount(threadCount)
{
    for (usize i = 0; i < _threadCount; ++i)
        _threads.emplace_back(cThread(cThread::eType::WORKER));
}

triton::cThreadSubsystem::~cThreadSubsystem()
{
    for (usize i = 0; i < _threadCount; ++i)
        _threads.at(i).Stop();
}

void triton::cThreadSubsystem::SubmitWork(cTask& task)
{
    _threads.at(_lastWorkThreadID).SubmitWork(task);
    _lastWorkThreadID = (_lastWorkThreadID + 1) % _threadCount;
}