#include <net/buffer.h>

namespace sms
{

    BufferList::BufferList(std::list<Buffer::Ptr> &list)
    {
        buf_ = new uv_buf_t[list.size()];

        std::swap(pkt_list_, list);

        int i = 0;
        for (const Buffer::Ptr &ptr : pkt_list_)
        {
            buf_[i].base = reinterpret_cast<char *>(ptr->Data());
            buf_[i].len = ptr->Size();
            remain_size_ += ptr->Size();
            ++i;
        }

        buf_num_ = pkt_list_.size();
    }

    void BufferList::ReOffset(int n)
    {
        if (n <= 0)
        {
            return;
        }

        remain_size_ -= n;
        int offset = 0;
        int last_off = buf_off_;

        for (int i = last_off; i < buf_num_; ++i)
        {
            offset += buf_[i].len;
            if (offset < n)
            {
                continue;
            }

            int remain_size = offset - n;
            buf_->base = reinterpret_cast<char *>(buf_->base + buf_->len - remain_size);
            buf_->len = remain_size;
            buf_off_ = i;
            if (remain_size == 0)
            {
                buf_off_++;
            }
            break;
        }

        for (int i = last_off; i < buf_off_; ++i)
        {
            pkt_list_.pop_front();
        }
    }

    BufferList::~BufferList()
    {
        delete[] buf_;
    }

    bool BufferList::Empty() const
    {
        return buf_off_ == buf_num_;
    }

    uv_buf_t *BufferList::GetUvBuf() const
    {
        return &buf_[buf_off_];
    }

    int BufferList::GetUvBufNum() const
    {
        return buf_num_ - buf_off_;
    }

    int BufferList::GetRemainSize() const
    {
        return remain_size_;
    }
} // namespace sms