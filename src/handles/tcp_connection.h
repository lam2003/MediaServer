// #ifndef TCP_CONNECTION_H
// #define TCP_CONNECTION_H

// #include <common/noncopyable.h>
// #include <common/global_inc.h>

// #include <uv.h>

// namespace sms
// {

//     class TcpConnection : public NonCopyable
//     {
//     public:
//         typedef std::function<void(bool)> OnSendCB;

//     public:
//         class Listener
//         {
//         public:
//             virtual ~Listener() = default;

//         public:
//             virtual void OnTcpConnectionClosed(TcpConnection *connection) = 0;
//         };

//         class UvWriteData : public NonCopyable
//         {
//             explicit UvWriteData(size_t store_size)
//             {
//                 this->store = new uint8_t[store_size];
//             }

//             ~UvWriteData()
//             {
//                 delete[] store;
//             }

//             uv_write_t req;
//             uint8_t *store{nullptr};
//             TcpConnection::OnSendCB cb{nullptr};
//         };

//     public:
//         explicit TcpConnection(size_t buffer_size);
//         virtual ~TcpConnection();

//     public:
//         void Close();
//         virtual void Dump() const;
//         void Setup(Listener *listener,
//                    struct sockaddr_storage *local_addr,
//                    const std::string &local_ip,
//                    uint16_t local_port);
//         void Start();
//         void Write(const uint8_t *data, size_t len, OnSendCB &&cb);
//         const struct sockaddr *GetLocalAddr() const;
//         int GetLocalFamily() const;
//         uint16_t GetLocalPort() const;
//         const struct sockaddr *GetPeerAddr() const;
//         uint16_t GetPeerPort() const;
//         bool IsClosed() const;
//         uv_tcp_t *GetUVHandle() const;
//         size_t GetSentBytes() const;
//         size_t GetRecvBytes() const;

//     public:
//         void OnUvReadAlloc(size_t suggested_size, uv_buf_t *buf);
//         void OnUvRead(ssize_t nread, const uv_buf_t *buf);
//         void OnUvWrite(int status, OnSendCB &&cb);

//     protected:
//         virtual void UserOnTcpConnectionRead() = 0;

//     private:
//         bool set_peer_addr();

//     protected:
//         size_t buffer_size_{0u};

//     private:
//         bool closed_{false};
//         uv_tcp_t *uv_handle_{nullptr};
//         size_t recv_bytes_{0u};
//         size_t sent_bytes_{0u};
//     };

// } // namespace sms

// #endif