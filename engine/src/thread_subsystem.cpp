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
    _cv.notify_all();
    _thread.join();
}

triton::cThreadSubsystem::cThreadSubsystem(cContext* context, usize threadCount) : iObject(context), _stop(K_FALSE)
{
    for (usize i = 0; i < threadCount; ++i)
    {
        _threads.emplace_back([this] {
            while (K_TRUE)
            {
                if (_pause.load() == K_TRUE)
                    continue;

                cTask task;
                {
                    std::unique_lock<std::mutex> lock(_mtx);
                    _cv.wait(lock, [this] {
                        return !_tasks.empty() || _stop;
                    });
                    if (_stop && _tasks.empty())
                        return;
                    task = _tasks.front();
                    _tasks.pop();
                }
                task.Run();
            }
        });
    }
}

triton::cThreadSubsystem::~cThreadSubsystem()
{
    Stop();

    _cv.notify_all();

    for (auto& thread : _threads)
        thread.join();
}

void triton::cThreadSubsystem::Pause()
{
    _pause.store(K_TRUE);
}

void triton::cThreadSubsystem::Resume()
{
    _pause.store(K_FALSE);
}

void triton::cThreadSubsystem::Submit(cTask& task)
{
    {
        std::unique_lock<std::mutex> lock(_mtx);
        _tasks.emplace(task);
    }

    _cv.notify_one();
}

void triton::cThreadSubsystem::Stop()
{
    std::unique_lock<std::mutex> lock(_mtx);
    _stop = K_TRUE;
}