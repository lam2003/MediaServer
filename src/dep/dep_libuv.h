#ifndef SMS_DEP_LIBUV_H
#define SMS_DEP_LIBUV_H

#include <common/global_inc.h>

#include <uv.h>

namespace sms
{
    class DepLibUV
    {
    public:
        static void ClassInit();
        static void ClassDestroy();
        static void PrintVersion();
        static void RunLoop(uv_run_mode mode = UV_RUN_DEFAULT);
        static uv_loop_t *GetLoop();
        static uint64_t GetTimeMs();
        static uint64_t GetTimeUs();
        static uint64_t GetTimeNs();
        static int64_t GetTimeMsInt64();
        static int64_t GetTimeUsInt64();

    private:
        static uv_loop_t *loop_;
    };
} // namespace sms
#endif