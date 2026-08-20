// thread_pool.cpp

#pragma once

#include <iostream>
#include "application.hpp"
#include "context.hpp"
#include "thread_subsystem.hpp"
#include "data_buffer.hpp"

using namespace types;

triton::cWorkItem::cWorkItem(XDataBuffer* data, WorkFunction&& function)
    :
    _data(data),
    _function(std::make_shared<WorkFunction>(std::move(function)))
{
}

void triton::cWorkItem::Execute()
{
    if (_function)
        _function->operator()(_data);
}

triton::cWorkQueue::cWorkQueue(types::usize threadCount) : _threadCount(threadCount)
{
}

void triton::cWorkQueue::SubmitWorkItem(cWorkItem&& task)
{
    std::lock_guard<std::mutex> lock(_queueMutex);
    _queue.emplace(std::move(task));
}

void triton::cWorkQueue::ProcessWorkItems()
{
    while (K_TRUE)
    {
        cWorkItem task;
        {
            std::unique_lock<std::mutex> lock(_queueMutex);
            _cv.wait(lock, [this] {
                return (!_queue.empty() && !_pause.load()) || _stop.load();
            });
            if (_stop.load() && _queue.empty())
                return;
            task = _queue.front();
            _queue.pop();
        }
        task.Execute();
    }
}

triton::cThread::cThread()
{
}

triton::cThread::~cThread()
{
    Stop();
}

types::boolean triton::cThread::Run()
{
    if (_handle.joinable())
        return K_FALSE;

    _handle = std::thread([this](){ ThreadFunction(); });
    if (_handle.joinable())
        return K_TRUE;
    else
        return K_FALSE;
}

void triton::cThread::Stop()
{
    if (_handle.joinable())
        _handle.join();
}

triton::cWorkerThread::cWorkerThread(cWorkQueue* owner) : _owner(owner)
{
}

void triton::cWorkerThread::ThreadFunction()
{
    _owner->ProcessWorkItems();
}

triton::cThreadSubsystem::cThreadSubsystem(usize threadCount)
{
    // TODO: rewrite this
    // ||||||||||||||||||
    // VVVVVVVVVVVVVVVVVV
    //_workQueue = _context->Create<cWorkQueue>(_context, threadCount);
    //for (usize i = 0; i < threadCount; i++)
    //    _workerThreads.push_back(_context->Create<cWorkerThread>(_context, _workQueue));
}

triton::cThreadSubsystem::~cThreadSubsystem()
{
    // TODO: rewrite this
    // ||||||||||||||||||
    // VVVVVVVVVVVVVVVVVV
    //for (usize i = 0; i < _workerThreads.size(); i++)
    //    _context->Destroy<cWorkerThread>(_workerThreads.at(i));
    //_context->Destroy<cWorkQueue>(_workQueue);
}

void triton::cThreadSubsystem::SubmitWorkItem(cWorkItem&& task)
{
    _workQueue->SubmitWorkItem(std::move(task));
}

void triton::cThreadSubsystem::ExecuteWorkItems()
{
    for (usize i = 0; i < _workerThreads.size(); i++)
        _workerThreads.at(i)->Run();
}