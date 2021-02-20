#ifndef SMS_NONCOPYABLE_H
#define SMS_NONCOPYABLE_H

namespace sms
{
    class NonCopyable
    {
    protected:
        // 定义为protected原因是：noncopyable只有在被继承时才有意义，
        // 防止其被单独实例化
        NonCopyable() {}
        ~NonCopyable() {}

    private:
        // 禁止拷贝
        NonCopyable(const NonCopyable &that) = delete;
        NonCopyable(NonCopyable &&that) = delete;
        NonCopyable &operator=(const NonCopyable &that) = delete;
        NonCopyable &operator=(NonCopyable &&that) = delete;
    };

#define SMS_NONCOPYABLE_DESTRUCTOR(C) \
    C(const C &) = delete;            \
    C(C &&) = delete;                 \
    C &operator=(const C &) = delete; \
    C &operator=(C &&) = delete;      \
    ~C()

} // namespace sms

#endif