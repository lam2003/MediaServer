#ifndef SMS_TASK_H
#define SMS_TASK_H

#include <common/global_inc.h>

namespace sms
{
    class TaskCancelAble
    {
    public:
        TaskCancelAble() = default;

        virtual ~TaskCancelAble() = default;

    public:
        virtual void Cancel() = 0;
    };

    template <typename Ret, typename... ArgsType>
    class TaskCancelAbleImpl
    {
    };

    template <typename Ret, typename... ArgsType>
    class TaskCancelAbleImpl<Ret(ArgsType...)> : public TaskCancelAble
    {
    public:
        using Ptr = std::shared_ptr<TaskCancelAbleImpl>;

        using FuncType = std::function<Ret(ArgsType...)>;

        template <typename Func>
        TaskCancelAbleImpl(Func &&f)
        {
            strong_task_ = std::make_shared<FuncType>(std::forward<Func>(f));
            weak_task_ = strong_task_;
        }

        ~TaskCancelAbleImpl() = default;

        operator bool()
        {
            return strong_task_ && *strong_task_;
        }

        Ret operator()(ArgsType... args) const
        {
            auto strong_task = weak_task_.lock();
            if (strong_task && *strong_task)
            {
                return (*strong_task)(std::forward<ArgsType>(args)...);
            }

            return default_value<Ret>();
        }

        template <typename T>
        static typename std::enable_if<std::is_void<T>::value, void>::type default_value()
        {
            return;
        }

        template <typename T>
        static typename std::enable_if<std::is_integral<T>::value, T>::type default_value()
        {
            return 0;
        }

        template <typename T>
        static typename std::enable_if<std::is_pointer<T>::value, T>::type default_value()
        {
            return nullptr;
        }

    public:
        void Cancel() override
        {
            strong_task_ = nullptr;
        }

    private:
        std::shared_ptr<FuncType> strong_task_{nullptr};
        std::weak_ptr<FuncType> weak_task_;
    };

    using Task = TaskCancelAbleImpl<void()>;
} // namespace sms

#endif