#include <net/signals_handler.h>
#include <net/dep_libuv.h>
#include <common/logger.h>

namespace sms
{
    inline static void on_close(uv_handle_t *handle)
    {
        handle->data = nullptr;
        delete handle;
    }

    inline static void on_signal(uv_signal_t *handle, int signo)
    {
        SignalsHandler *sigs_hdl = reinterpret_cast<SignalsHandler *>(handle->data);
        if (sigs_hdl)
        {
            sigs_hdl->OnUvSignal(signo);
        }
    }

    SignalsHandler::SignalsHandler(SignalCB &&cb)
    {
        SetSignalCB(std::move(cb));
    }

    SignalsHandler::~SignalsHandler()
    {
        Close();
    }

    void SignalsHandler::Close()
    {
        if (closed_)
        {
            return;
        }

        closed_ = true;

        for (uv_signal_t *handle : uv_handles_)
        {
            uv_close(reinterpret_cast<uv_handle_t *>(handle), on_close);
        }
    }

    int SignalsHandler::AddSignal(int signo)
    {
        if (closed_)
        {
            return -1;
        }

        uv_signal_t *handle = new uv_signal_t;
        int ret = uv_signal_init(DepLibUV::GetLoop(), handle);
        if (ret != 0)
        {
            LOG_E << "uv_signal_init() failed: " << uv_strerror(ret);
            delete handle;
            return -1;
        }

        handle->data = this;
        ret = uv_signal_start(handle, static_cast<uv_signal_cb>(on_signal), signo);
        if (ret != 0)
        {
            LOG_E << "uv_signal_start() failed: " << uv_strerror(ret);
            uv_close(reinterpret_cast<uv_handle_t *>(handle), static_cast<uv_close_cb>(on_close));
            return -1;
        }

        uv_handles_.emplace_back(handle);
        return 0;
    }

    void SignalsHandler::SetSignalCB(SignalCB &&cb)
    {
        if (cb)
        {
            cb_ = std::move(cb);
        }
        else
        {
            cb_ = [](SignalsHandler *handler, int signo) {
                LOG_W << "on signal callback not set! ignore signo: " << signo;
            };
        }
    }

    inline void SignalsHandler::OnUvSignal(int signo)
    {
        cb_(this, signo);
    }

} // namespace sms