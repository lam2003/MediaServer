#include <common/utils.h>
#include <net/tcp_connection.h>

namespace sms
{

    class ConnectionManager : public NonCopyable
    {
    public:
        ~ConnectionManager() = default;
        static ConnectionManager &Instance();

    public:
        void Register(TcpConnection *conn)
        {
            if (!IsExist(conn))
            {
                mapping_.emplace(conn);
            }
        }
        void Remove(TcpConnection *conn)
        {
            mapping_.erase(conn);
        }

        bool IsExist(TcpConnection *conn)
        {
            return mapping_.count(conn);
        }

    private:
        ConnectionManager() = default;

    private:
        std::unordered_set<TcpConnection *> mapping_;
    };

    INSTANCE_IMPL(ConnectionManager);
} // namespace sms