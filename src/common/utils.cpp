#include <common/once_token.h>
#include <common/utils.h>

namespace sms
{

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

    void split_string(const std::string &str, std::vector<std::string> &vec, const std::string &trim)
    {
        vec.clear();

        size_t prev = 0, cur = str.length();

        do
        {
            cur = str.find(trim, prev);
            if (cur != std::string::npos)
            {
                vec.emplace_back(str.substr(prev, cur - prev));
                prev = cur + trim.length();
            }
        } while (cur != std::string::npos);

        vec.emplace_back(str.substr(prev, cur - prev));
    }

    std::string &trim(std::string &s, const std::string &chars)
    {
        std::string map(0xFF, '\0');
        for (const char &ch : chars)
        {
            map[static_cast<const uint8_t &>(ch)] = '\1';
        }

        while (s.size() && map.at(static_cast<const uint8_t &>(s.back())))
        {
            s.pop_back();
        }
        while (s.size() && map.at(static_cast<const uint8_t &>(s.front())))
        {
            s.erase(0, 1);
        }

        return s;
    }

} // namespace sms