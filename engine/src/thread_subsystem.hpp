// thread_pool.hpp

#pragma once

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
    class cBuffer;

    using TaskFunction = std::function<void(cBuffer* const data)>;

    class cTask
    {
        cBuffer* _data = nullptr;
        std::shared_ptr<TaskFunction> _function;

    public:
        cTask() = default;
        explicit cTask(cBuffer* data, TaskFunction&& function);
        ~cTask() = default;

        void Run();
        inline cBuffer* GetData() const { return _data; }
        inline std::shared_ptr<TaskFunction> GetFunction() const { return _function; }
    };

    class cThread
    {
    public:
        enum class eType
        {
            NONE = 0,
            RENDER,
            WORKER
        };

    private:
        eType _type = eType::NONE;
        std::thread _thread;
        std::atomic<types::boolean> _pause = types::K_FALSE;
        std::atomic<types::boolean> _stop = types::K_FALSE;
        std::queue<cTask> _tasks = {};
        static std::mutex _mtx;
        static std::condition_variable _cv;

    public:
        enum class eType
        {
            NONE = 0,
            RENDER,
            WORKER
        };

        explicit cThread(cThread::eType type);
        ~cThread();

        void SubmitWork(cTask& task);
        inline void Pause() { _pause.store(types::K_TRUE); }
        inline void Resume() { _pause.store(types::K_FALSE); }
        inline void Stop()
        {
            _stop.store(types::K_TRUE);
            _cv.notify_all();
        }
    };

    class cThreadSubsystem : public iObject
    {
        TRITON_OBJECT(cThreadSubsystem)

        std::vector<cThread> _threads = {};
        types::usize _threadCount = 0;
        types::usize _lastWorkThreadID = 0;

    public:
        explicit cThreadSubsystem(
            cContext* context,
            types::usize threadCount = std::thread::hardware_concurrency()
        );
        ~cThreadSubsystem();
        
        void SubmitWork(cTask& task);
    };
}