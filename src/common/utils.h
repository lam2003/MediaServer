#ifndef SMS_UTILS_H
#define SMS_UTILS_H

#include <common/global_inc.h>

template <typename... T>
struct make_void
{
    using type = void;
};

template <typename... T>
using void_t = typename make_void<T...>::type;

template <typename, template <typename... T> class OP, typename... T>
struct is_detected_impl : std::false_type
{
};

template <template <typename... T> class OP, typename... T>
struct is_detected_impl<void_t<OP<T...>>, OP, T...> : std::true_type
{
};

template <template <typename... T> class OP, typename... T>
using is_detected = is_detected_impl<void, OP, T...>;

template <typename T>
using HasPtr_t = typename T::Ptr;

template <typename T>
using HasPtr = is_detected<HasPtr_t, T>;

#define INSTANCE_IMPL(class_name, ...)                   \
    class_name &class_name::Instance()                   \
    {                                                    \
        static std::shared_ptr<class_name> s_instance(   \
            new class_name(__VA_ARGS__));                \
        static class_name &s_instance_ref = *s_instance; \
        return s_instance_ref;                           \
    }

namespace sms
{
    class __StringPrinter : public std::string
    {
    public:
        __StringPrinter() = default;
        ~__StringPrinter() = default;

    public:
        template <typename T>
        __StringPrinter &operator<<(T &&data)
        {
            ss_ << std::forward<T>(data);
            std::string::operator=(ss_.str());
            return *this;
        }

        std::string operator<<(std::ostream &(*f)(std::ostream &)) const
        {
            return *this;
        }

    private:
        std::stringstream ss_;
    };

#define StringPrinter __StringPrinter()

    std::string print_time(const timeval &tv);

    std::string get_exe_path();

    std::string get_exe_name();

    enum ThreadPriority
    {
        TPRIORITY_LOWEST,
        TPRIORITY_LOW,
        TPRIORITY_NORMAL,
        TPRIORITY_HIGH,
        TPRIORITY_HIGHEST
    };

    bool set_thread_priority(ThreadPriority priority = TPRIORITY_NORMAL,
                             pthread_t tid = 0);

    bool set_thread_name(const std::string &name);

    void split_string(const std::string &str, std::vector<std::string> &vec, const std::string &trim);

    std::string &trim(std::string &str, const std::string &chars = "\r\n\t");

    void to_lower_case(std::string &str);

    uint64_t get_current_microseconds(bool system_time = true);

    uint64_t get_current_milliseconds(bool system_time = true);

} // namespace sms

#endif