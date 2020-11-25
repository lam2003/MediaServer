#include <common/dep_libuv.h>
#include <common/logger.h>

namespace sms
{
    uv_loop_t *DepLibUV::loop_{nullptr};

    void DepLibUV::ClassInit()
    {
        loop_ = new uv_loop_t;
        int err = uv_loop_init(loop_);
        if (err != 0)
        {
            SMS_ABORT("libuv initialization failed");
        }
    }

    void DepLibUV::ClassDestory()
    {
        if (loop_ != nullptr)
        {
            uv_loop_close(loop_);
            delete loop_;
        }
    }

    void DepLibUV::PrintVersion()
    {
        LOG_I << "libuv version: " << uv_version_string();
    }

    void DepLibUV::RunLoop(uv_run_mode mode)
    {
        uv_run(loop_, mode);
    }

    uv_loop_t *DepLibUV::GetLoop()
    {
        return loop_;
    }
    uint64_t DepLibUV::GetTimeMs()
    {
        return static_cast<uint64_t>(uv_hrtime() / 1000000u);
    }
    uint64_t DepLibUV::GetTimeUs()
    {
        return static_cast<uint64_t>(uv_hrtime() / 1000u);
    }
    uint64_t DepLibUV::GetTimeNs()
    {
        return uv_hrtime();
    }

    int64_t DepLibUV::GetTimeMsInt64()
    {
        return static_cast<int64_t>(DepLibUV::GetTimeMs());
    }

    int64_t DepLibUV::GetTimeUsInt64()
    {
        return static_cast<int64_t>(DepLibUV::GetTimeUs());
    }

} // namespace sms