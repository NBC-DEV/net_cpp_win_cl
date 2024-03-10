#include "TcpNetMgr.h"

namespace AsioNet
{
// ************************************************
	std::shared_ptr<TcpServer> TcpServerMgr::GetServer(ServerKey k)
	{
		_lock_guard_(m_lock);
		if(servers.find(k) != servers.end()){
			return servers[k];
		}
		return nullptr;
	}
	void TcpServerMgr::AddServer(std::shared_ptr<TcpServer> s)
	{
		_lock_guard_(m_lock);
		if(!s){
			return;
		}
		if(servers.find(s->GetKey()) == servers.end()){
			servers[s->GetKey()] = s;
		}
	}
	TcpServerMgr::~TcpServerMgr()
	{}

// ************************************************

	TcpNetMgr::TcpNetMgr(size_t th_num, IEventPoller* ptr_poller)
		:m_poller(ptr_poller)
	{
		for (size_t i = 0; i < th_num; i++)
		{
			// std::move
			thPool.push_back(std::thread([self = this]{
				while(true){
					self->m_ctx.run();	// 只要有io事件，这个Run就不会返回
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				} 
			}));
		}
	}

	TcpNetMgr::~TcpNetMgr()
	{
		// 发送结束事件，通知线程退出
		// m_ctx.poll_one();
		for (auto &t : thPool)
		{
			t.join();
		}
	}

	void TcpNetMgr::Connect(std::string ip, uint16_t port,int retry/*,options*/)
	{
		auto conn = std::make_shared<TcpConn>(m_ctx, m_poller);
		// 连接并没有成功建立，这里不应该调用AddConn
		conn->SetOwner(&m_connMgr);
		conn->Connect(ip, port, retry);
	}

	ServerKey TcpNetMgr::Serve(uint16_t port /*,options*/)
	{
		auto s = std::make_shared<TcpServer>(m_ctx, m_poller);
		s->Serve(port);
		m_serverMgr.AddServer(s);
		return s->GetKey();
	}

	bool TcpNetMgr::Send(NetKey k, const char* data, size_t trans)
	{
		auto conn = m_connMgr.GetConn(k);
		if (conn) {
			return conn->Write(data, trans);
		}

		ServerKey sk = k & 0xffff;
		auto server = m_serverMgr.GetServer(sk);
		if(server){
			auto client = server->GetConn(k);
			if (client)
			{
				return client->Write(data,trans);
			}
		}
		return false;
	}

	void TcpNetMgr::Broadcast(ServerKey sk, const char* data, size_t trans)
	{
		auto server = m_serverMgr.GetServer(sk);
		if (server) {
			server->Broadcast(data,trans);
		}
	}

	void TcpNetMgr::Disconnect(NetKey k)
	{
		m_connMgr.DelConn(k);

		ServerKey sk = k & 0xff;
		auto server = m_serverMgr.GetServer(sk);
		if (server) {
			server->Disconnect(k);
		}
	}

}