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
        virtual char *Data() const = 0;

        virtual size_t Size() const = 0;

        virtual std::string ToString() const
        {
            return std::string(Data(), Size());
        }

        virtual size_t Capacity() const
        {
            return Size();
        }
    };

    class BufferLikeString final : public Buffer
    {
    public:
        BufferLikeString() = default;

        BufferLikeString(std::string &&str)
        {
            str_ = std::move(str);
        }

        BufferLikeString(const std::string &str)
        {
            str_ = str;
        }

        BufferLikeString(const char *str)
        {
            str_ = str;
        }

        BufferLikeString &operator=(std::string &&str)
        {
            str_ = std::move(str);
            return *this;
        }

        BufferLikeString &operator=(const std::string &str)
        {
            str_ = str;
            return *this;
        }

        BufferLikeString &operator=(const char *str)
        {
            str_ = str;
            return *this;
        }

        ~BufferLikeString() = default;

    public:
        // class Buffer;
        char *Data() const override
        {
            return const_cast<char *>(str_.data()) + erase_head_;
        }

        size_t Size() const override
        {
            return str_.size() - erase_tail_ - erase_head_;
        }

        size_t Capacity() const override
        {
            return str_.capacity();
        }

    public:
        BufferLikeString &Erase(std::string::size_type pos = 0, std::string::size_type n = std::string::npos)
        {
            if (pos == 0)
            {
                if (n != std::string::npos)
                {
                    if (n > Size())
                    {
                        n = Size();
                    }

                    erase_head_ += n;
                    Data()[Size()] = '\0';
                    return *this;
                }

                erase_head_ = 0;
                erase_tail_ = str_.size();
                Data()[Size()] = '\0';
                return *this;
            }

            if (n == std::string::npos || pos + n >= Size())
            {
                if (pos >= Size())
                {
                    pos = Size();
                }

                erase_tail_ += Size() - pos;
                Data()[Size()] = '\0';
                return *this;
            }

            str_.erase(erase_head_ + pos, n);

            return *this;
        }

        BufferLikeString &Append(const char *data, int len)
        {
            if (len <= 0)
            {
                return *this;
            }

            if (erase_head_ > str_.capacity() / 2)
            {
                move_data();
            }

            if (erase_tail_ == 0)
            {
                str_.append(data, len);
                return *this;
            }

            str_.insert(erase_head_ + Size(), data, len);
            return *this;
        }

        BufferLikeString &Append(const BufferLikeString &str)
        {
            Append(str.Data(), str.Size());
            return *this;
        }

        BufferLikeString &Append(const std::string &str)
        {
            Append(str.data(), str.size());
            return *this;
        }

        BufferLikeString &Append(const char *str)
        {
            Append(str, strlen(str));
            return *this;
        }

        void PushBack(char c)
        {
            if (erase_tail_ == 0)
            {
                str_.push_back(c);
                return;
            }
            Data()[Size()] = c;
            --erase_tail_;
            Data()[Size()] = '\0';
        }

        BufferLikeString &Insert(std::string::size_type pos, const char *str, std::string::size_type n)
        {
            str_.insert(erase_head_ + pos, str, n);
            return *this;
        }

        BufferLikeString &Assign(const char *data, int len)
        {
            if (len <= 0)
            {
                return *this;
            }

            if (str_.data() <= data && data < str_.data() + str_.size())
            {
                erase_head_ = data - str_.data();
                if (data + len > str_.data() + str_.size())
                {
                    len = str_.data() + str_.size() - data;
                }

                erase_tail_ = str_.data() + str_.size() - data - len;
                return *this;
            }

            str_.assign(data, len);
            erase_head_ = 0;
            erase_tail_ = 0;
            return *this;
        }

        void Clear()
        {
            erase_head_ = 0;
            erase_tail_ = 0;
            str_.clear();
        }

        char &operator[](std::string::size_type pos)
        {
            if (pos > Size())
            {
                return zero_;
            }

            return Data()[pos];
        }

        const char &operator[](std::string::size_type pos) const
        {
            if (pos > Size())
            {
                return zero_;
            }

            return Data()[pos];
        }

        void Reserve(std::string::size_type size)
        {
            str_.reserve(size);
        }

        bool Empty() const
        {
            return Size() <= 0;
        }

        std::string SubStr(std::string::size_type pos, std::string::size_type n = std::string::npos) const
        {
            if (n == std::string::npos)
            {
                if (pos > Size())
                {
                    return "";
                }
                return str_.substr(erase_head_ + pos, Size() - pos);
            }

            if (pos > Size())
            {
                return "";
            }

            if (pos + n > Size())
            {
                return str_.substr(erase_head_ + pos, Size() - pos);
            }

            return str_.substr(erase_head_ + pos, n);
        }

    private:
        void move_data()
        {
            if (erase_head_)
            {
                str_.erase(0, erase_head_);
                erase_head_ = 0;
            }
        }

    private:
        uint32_t erase_head_{0};
        uint32_t erase_tail_{0};
        std::string str_;
        char zero_{'\0'};
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

        char *Data() const override
        {
            return const_cast<char *>(data_.data()) + offset_;
        }

        size_t Size() const override
        {
            return size_;
        }

        std::string ToString() const override
        {
            return std::string(Data(), Size());
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
        char *Data() const override
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
            data_ = new char[capacity];
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
        char *data_{nullptr};
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