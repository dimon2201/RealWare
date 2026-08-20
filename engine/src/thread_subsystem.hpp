// thread_pool.hpp

#pragma once

#include <condition_variable>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <memory>
#include <atomic>
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

    class cWorkQueue final
    {
        types::usize _threadCount = 0;
        std::queue<cWorkItem> _queue;
        std::mutex _queueMutex;
        std::condition_variable _cv;
        std::atomic<types::boolean> _pause;
        std::atomic<types::boolean> _stop;
            
    public:
        explicit cWorkQueue(types::usize threadCount = std::thread::hardware_concurrency());

        void SubmitWorkItem(cWorkItem&& task);
        void ProcessWorkItems();
    };

    class cThread
    {
        std::thread _handle;

    public:
        explicit cThread();
        virtual ~cThread();

        virtual void ThreadFunction() = 0;

        types::boolean Run();
        void Stop();
    };

    class cWorkerThread : public cThread
    {
        cWorkQueue* _owner = nullptr;

    public:
        explicit cWorkerThread(cWorkQueue* owner);

        virtual void ThreadFunction() override;
    };

    class cThreadSubsystem
    {
        cWorkQueue* _workQueue = nullptr;
        std::vector<cWorkerThread*> _workerThreads = {};

    public:
        explicit cThreadSubsystem(types::usize threadCount = std::thread::hardware_concurrency());
        ~cThreadSubsystem();
        
        void SubmitWorkItem(cWorkItem&& task);
        void ExecuteWorkItems();

        inline cWorkQueue* GetWorkQueue() const { return _workQueue; }
    };
}