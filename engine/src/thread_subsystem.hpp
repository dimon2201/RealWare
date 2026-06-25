// thread_pool.hpp

#pragma once

#include <condition_variable>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <memory>
#include <atomic>
#include "object.hpp"
#include "types.hpp"

namespace triton
{
    class cApplication;
    class XDataBuffer;

    using WorkFunction = std::function<void(XDataBuffer* const data)>;

    class cWorkItem
    {
        XDataBuffer* _data = nullptr;
        std::shared_ptr<WorkFunction> _function;

    public:
        cWorkItem() = default;
        explicit cWorkItem(XDataBuffer* data, WorkFunction&& function);

        void Execute();
        inline XDataBuffer* GetData() const { return _data; }
        inline std::shared_ptr<WorkFunction> GetFunction() const { return _function; }
    };

    class cWorkQueue final : public iObject
    {
        TRITON_OBJECT(cWorkQueue)

        types::usize _threadCount = 0;
        std::queue<cWorkItem> _queue;
        std::mutex _queueMutex;
        std::condition_variable _cv;
        std::atomic<types::boolean> _pause;
        std::atomic<types::boolean> _stop;
            
    public:
        explicit cWorkQueue(
            cContext* context,
            types::usize threadCount = std::thread::hardware_concurrency()
        );

        void SubmitWorkItem(cWorkItem&& task);
        void ProcessWorkItems();
    };

    class cThread : public iObject
    {
        TRITON_OBJECT(cThread)

        std::thread _handle;

    public:
        explicit cThread(cContext* context);
        virtual ~cThread();

        virtual void ThreadFunction() = 0;

        types::boolean Run();
        void Stop();
    };

    class cWorkerThread : public cThread
    {
        TRITON_OBJECT(cWorkerThread)

        cWorkQueue* _owner = nullptr;

    public:
        explicit cWorkerThread(cContext* context, cWorkQueue* owner);

        virtual void ThreadFunction() override;
    };

    class cThreadSubsystem : public iObject
    {
        TRITON_OBJECT(cThreadSubsystem)

        cWorkQueue* _workQueue = nullptr;
        std::vector<cWorkerThread*> _workerThreads = {};

    public:
        explicit cThreadSubsystem(
            cContext* context,
            types::usize threadCount = std::thread::hardware_concurrency()
        );
        virtual ~cThreadSubsystem();
        
        void SubmitWorkItem(cWorkItem&& task);
        void ExecuteWorkItems();

        inline cWorkQueue* GetWorkQueue() const { return _workQueue; }
    };
}