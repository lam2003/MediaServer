#ifndef SMS_SIGNAL_HANDLER_H
#define SMS_SIGNAL_HANDLER_H

#include <common/global_inc.h>

#include <uv.h>

namespace sms
{
    class SignalsHandler
    {

    public:
        using SignalCB = std::function<void(SignalsHandler *, int)>;

    public:
        explicit SignalsHandler(SignalCB &&cb);
        ~SignalsHandler();

    public:
        void Close();
        int AddSignal(int signo);
        void SetSignalCB(SignalCB &&cb);

    public:
        void OnUvSignal(int signo);

    private:
        SignalCB cb_{nullptr};
        std::vector<uv_signal_t *> uv_handles_;
        bool closed_{false};
    };
} // namespace sms

#endif