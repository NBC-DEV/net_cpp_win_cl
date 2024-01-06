#pragma once

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/bind.hpp>

#include "AsioNetDef.h"
#include "BlockBuffer.h"

namespace AsioNet {
	// doc:https://www.boost.org/doc/libs/1_84_0/doc/html/boost_asio/reference/ip__tcp/socket.html

// ����Խӿڵ���ʽ���ظ����棬��Ӧ�����������ʹ�ýӿڵ�ʱ����ָ�����Ч�Եõ���֤��shared_ptr?
struct AN_INTERFACE IANConn : public boost::noncopyable {
	virtual bool Write(const char* acData, int iSize) = 0;
	virtual bool Read() = 0;
};

struct IConnectHandler{
	virtual void operator()(const error_code& error) = 0;
	virtual ~IConnectHandler() {};
};
struct IAcceptHandler {
	virtual void operator()(const error_code& error) = 0;
	virtual ~IAcceptHandler() {};
};
struct IWriteHandler {
	virtual void operator()(const error_code& error, std::size_t bytes_transferred) = 0;
	virtual ~IWriteHandler() {};
};
struct IReadHandler {
	virtual void operator()(const error_code& error, std::size_t bytes_transferred) = 0;
	virtual ~IReadHandler() {};
};

struct AN_INTERFACE IANCompleteHandler {
	virtual ~IANCompleteHandler() {}
	virtual void connect_handler(const error_code& error) = 0;
	virtual void accept_handler(const error_code& error) = 0;
	virtual void read_handler(const error_code& error, std::size_t bytes_transferred) = 0;
	virtual void write_handler(const error_code& error, std::size_t bytes_transferred) = 0;
};


class TcpConn {
	friend class TcpClient;
	friend class TcpServer;
public:
	TcpConn() = delete;
	TcpConn(io_context&);

	void Close();
	bool Write(const char* data, size_t trans);
	void StartRead();	// start read loop

protected:
	void read_head_handler(const error_code&, size_t);
	void read_body_handler(const error_code&, size_t);
	void write_handler(const error_code&, size_t);
private:
	ip::tcp::socket sock_;
	std::mutex sendLock;
	BlockSendBuffer<DEFAULT_SEND_BUFFER_SIZE, DEFAULT_SEND_BUFFER_POOL_EXTEND_SIZE> sendBuffer;
	char readBuffer[1 << (sizeof(AN_Msg::len)*8)];
};


// NewTCPConn()
// NewUDPConn()
// NewKCPConn()
//class TcpConnFactory {
//
//};

}