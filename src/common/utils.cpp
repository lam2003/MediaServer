#include <common/once_token.h>
#include <common/utils.h>

namespace sms
{
    static inline uint64_t get_current_microseconds_origin()
    {
        return std::chrono::duration_cast<std::chrono::microseconds>(
                   std::chrono::system_clock::now().time_since_epoch())
            .count();
    }

    static std::atomic<uint64_t>
        s_now_microseconds(get_current_microseconds_origin());
    static std::atomic<uint64_t>
        s_now_milliseconds(get_current_microseconds_origin() / 1000);
    static std::atomic<uint64_t> s_now_seconds(get_current_microseconds_origin() /
                                               1000000);

    static inline bool init_update_ts_thread()
    {
        // C++11局部静态变量构造是线程安全的
        static std::thread s_update_ts_thread([]() {
            uint64_t now_microseconds;
            set_thread_name("update_time");
            while (true)
            {
                now_microseconds = get_current_microseconds_origin();
                s_now_microseconds.store(now_microseconds,
                                         std::memory_order_release);
                s_now_milliseconds.store(now_microseconds / 1000,
                                         std::memory_order_release);
                s_now_seconds.store(now_microseconds / 1000000,
                                    std::memory_order_release);
                std::this_thread::sleep_for(std::chrono::microseconds(500));
            }
        });

        static OnceToken s_once_token([&]() { s_update_ts_thread.detach(); });

        return true;
    }

    uint64_t get_current_microseconds()
    {
        static bool __attribute__((unused)) s_flag = init_update_ts_thread();
        return s_now_microseconds.load(std::memory_order_acquire);
    }

    uint64_t get_current_milliseconds()
    {
        static bool __attribute__((unused)) s_flag = init_update_ts_thread();
        return s_now_milliseconds.load(std::memory_order_acquire);
    }

    uint64_t get_current_seconds()
    {
        static bool __attribute__((unused)) s_flag = init_update_ts_thread();
        return s_now_seconds.load(std::memory_order_acquire);
    }

    std::string print_time(const timeval &tv)
    {
        time_t sec = tv.tv_sec;
        tm *tm = localtime(&sec);

        char buf[128];
        snprintf(buf, sizeof(buf), "%d-%02d-%02d %02d:%02d:%02d.%03d",
                 1900 + tm->tm_year, 1 + tm->tm_mon, tm->tm_mday, tm->tm_hour,
                 tm->tm_min, tm->tm_sec, (int)(tv.tv_usec / 1000));
        return buf;
    }

    std::string get_exe_path()
    {
        std::string path;
        char buf[PATH_MAX * 2 + 1] = {0};
        int n = readlink("/proc/self/exe", buf, sizeof(buf));
        if (n <= 0)
        {
            path = "./";
        }
        else
        {
            path = buf;
        }
        return path;
    }

    std::string get_exe_name()
    {
        std::string path = get_exe_path();
        return path.substr(path.rfind("/") + 1);
    }

    bool set_thread_priority(ThreadPriority priority, pthread_t tid)
    {
        static int min = sched_get_priority_min(SCHED_OTHER);
        if (min == -1)
        {
            return false;
        }
        static int max = sched_get_priority_max(SCHED_OTHER);
        if (max == -1)
        {
            return false;
        }

        static int priorities[] = {min, min + (max - min) / 4,
                                   min + (max - min) / 2, min + (max - min) * 3 / 4,
                                   max};

        if (tid == 0)
        {
            tid = pthread_self();
        }
        struct sched_param params;
        params.sched_priority = priorities[priority];
        return pthread_setschedparam(tid, SCHED_OTHER, &params) == 0;
    }

    bool set_thread_name(const std::string &name)
    {
        return (pthread_setname_np(pthread_self(), name.c_str()) == 0);
    }

} // namespace sms