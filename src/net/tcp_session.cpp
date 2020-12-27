#include <net/tcp_session.h>

namespace sms
{
    TcpSession::TcpSession(TcpConnection *conn)
    {
        conn_ = conn;
        conn_->SetErrorCB([this](const SockException &err) {
            OnError(err);
        });
    }

    TcpSession::~TcpSession()
    {
        shutdown();
    }

    void TcpSession::send(const BufferList::Ptr &list)
    {
        conn_->Write(list, [](bool) {
            // ignore
        });
    }

    void TcpSession::send(const Buffer::Ptr &buf)
    {
        conn_->Write(buf, [](bool) {
            // ignore
        });
    }

    void TcpSession::shutdown()
    {
        conn_->Close();
    }

} // namespace sms