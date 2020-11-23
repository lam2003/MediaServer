// #include <handles/tcp_connection.h>

// using namespace sms;
// TcpConnection::TcpConnection(size_t buffer_size)
// {
//     buffer_size_ = buffer_size;

//     uv_handle_ = new uv_tcp_t;
//     uv_handle_->data = static_cast<void *>(this);
// }

// TcpConnection::~TcpConnection()
// {
//     // delete
// }

// void TcpConnection::Close()
// {
//     if (this->closed_)
//     {
//         return;
//     }

//     closed_ = true;
//     uv_handle_->data = nullptr;

//     int ret = uv_read_stop(reinterpret_cast<uv_stream_t *>(uv_handle_));
//     if (ret != 0)
//     {
//     }
// }