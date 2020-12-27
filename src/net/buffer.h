#ifndef SMS_BUFFER_H
#define SMS_BUFFER_H

#include <common/noncopyable.h>
#include <common/global_inc.h>

#include <uv.h>

namespace sms
{
    class Buffer : public NonCopyable
    {
    public:
        using Ptr = std::shared_ptr<Buffer>;

        Buffer() = default;

        virtual ~Buffer() = default;

    public:
        virtual uint8_t *Data() const = 0;

        virtual size_t Size() const = 0;

        virtual std::string ToString() const
        {
            return std::string(reinterpret_cast<const char *>(Data()),
                               Size());
        }

        virtual size_t Capacity() const
        {
            return Size();
        }
    };

    class BufferString final : public Buffer
    {
    public:
        BufferString(const std::string &data, size_t offset = 0, size_t len = 0)
        {
            data_ = data;
            offset_ = offset;
            size_ = len;
            setup();
        }

        BufferString(std::string &&data, size_t offset = 0, size_t len = 0)
        {
            data_ = std::move(data);
            offset_ = offset;
            size_ = len;
            setup();
        }

        uint8_t *Data() const override
        {
            return const_cast<uint8_t *>((reinterpret_cast<const uint8_t *>(data_.data() + offset_)));
        }

        size_t Size() const override
        {
            return size_;
        }

        std::string ToString() const override
        {
            return std::string(reinterpret_cast<const char *>(Data()), Size());
        }

    private:
        void setup()
        {
            if (offset_ > data_.size())
            {
                offset_ = data_.size();
            }
            if (size_ == 0)
            {
                size_ = data_.size();
            }
            if (size_ + offset_ > data_.size())
            {
                size_ = data_.size() - offset_;
            }
        }

    private:
        std::string data_;
        size_t offset_{0};
        size_t size_{0};
    };

    class BufferRaw final : public Buffer
    {
    public:
        using Ptr = std::shared_ptr<BufferRaw>;

        BufferRaw(uint32_t capacity = 0)
        {
            if (capacity)
            {
                SetCapacity(capacity);
            }
        }
    
        ~BufferRaw()
        {
            if (data_)
            {
                delete[] data_;
            }
        }

    public:
        uint8_t *Data() const override
        {
            return data_;
        }

        size_t Size() const override
        {
            return size_;
        }

        size_t Capacity() const override
        {
            return capacity_;
        }

    public:
        void SetCapacity(size_t capacity)
        {
            if (data_)
            {
                do
                {
                    if (capacity > capacity_)
                    {
                        // 需要重新分配内存
                        break;
                    }

                    // 例如：1500 * 2 = 3000 > 2048
                    // 此时没必要重新分配，一页内存4096KB
                    if (capacity < 2 * 1024)
                    {
                        // 2k以下，不需要重复开辟
                        return;
                    }

                    if (capacity * 2 > capacity_)
                    {
                        return;
                    }

                } while (0);

                delete[] data_;
            }
            data_ = new uint8_t[capacity];
            capacity_ = capacity;
        }

        void SetSize(size_t size)
        {
            if (size > capacity_)
            {
                throw std::invalid_argument("Buffer::SetSize out of range");
            }
            size_ = size;
        }

        void Assign(const char *data, size_t size = 0)
        {
            if (size <= 0)
            {
                // 字符串
                size = strlen(data);
            }

            SetCapacity(size + 1);
            memcpy(data_, data, size);
            data_[size] = '\0';
            SetSize(size);
        }

    private:
        uint8_t *data_{nullptr};
        size_t capacity_{0};
        size_t size_{0};
    };

    class BufferList : public NonCopyable
    {
    public:
        using Ptr = std::shared_ptr<BufferList>;
        BufferList(std::list<Buffer::Ptr> &list);
        ~BufferList();

    public:
        bool Empty() const;
        uv_buf_t *GetUvBuf() const;
        int GetUvBufNum() const;
        int GetRemainSize() const;
        void ReOffset(int n);

    private:
        struct uv_buf_t *buf_{nullptr};
        int buf_num_{0};
        int buf_off_{0};
        int remain_size_{0};
        std::list<Buffer::Ptr> pkt_list_;
    };

} // namespace sms

#endif