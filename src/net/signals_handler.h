#ifndef SMS_SIGNAL_HANDLER_H
#define SMS_SIGNAL_HANDLER_H

#include <common/global_inc.h>

#include <uv.h>

namespace sms
{
    class SignalsHandler
    {
    public:
        class Listener
        {
        public:
            virtual ~Listener() {}
            virtual void OnSignal(SignalsHandler *sig_hdl, int signo) = 0;
        };

    public:
        explicit SignalsHandler(Listener *listener);
        ~SignalsHandler();

    public:
        void Close();
        int AddSignal(int signo);

    public:
        void OnUvSignal(int signo);

    private:
        Listener *listener_{nullptr};
        std::vector<uv_signal_t *> uv_handles_;
        bool closed_{false};
    };
} // namespace sms

#endif