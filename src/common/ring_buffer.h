#ifndef SMS_RING_BUFFER_H
#define SMS_RING_BUFFER_H

#include <common/utils.h>

namespace sms
{
    template <typename typeT>
    class RingDelegate
    {
    public:
        RingDelegate()
        {
            static_assert(HasPtr<typeT>::value, "typeT::Ptr not defined");
        }

        ~RingDelegate()
        {
        }

    private:
        std::list<typename typeT::Ptr> buf_;
    };

    class A
    {
    public:
        using Ptr = std::shared_ptr<A>;
    };

    RingDelegate<A> a;
}

#endif