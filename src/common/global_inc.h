#ifndef SMS_DEF_H
#define SMS_DEF_H

#include <algorithm>
#include <cinttypes>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <sstream>
#include <iostream>
#include <chrono>
#include <atomic>
#include <cstring>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/syscall.h> 
#include <sys/time.h>
#include <sys/types.h>
#include <linux/limits.h>
#include <unistd.h>
#include <sched.h>


#ifndef gettid
#define gettid() syscall(SYS_gettid)
#endif


#endif