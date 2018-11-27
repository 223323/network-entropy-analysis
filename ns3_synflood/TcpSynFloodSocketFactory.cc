#include "TcpSynFloodSocketFactory.h"
#include "TcpSynFloodSocket.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TcpSynFloodSocketFactory");
NS_OBJECT_ENSURE_REGISTERED (TcpSynFloodSocketFactory);

TypeId TcpSynFloodSocketFactory::GetTypeId (void) {
	static TypeId tid = TypeId ("ns3::TcpSynFloodSocketFactory")
		.SetParent<SocketFactory> ()
		.AddConstructor<TcpSynFloodSocketFactory> ()
		.SetGroupName("Test");
	return tid;
}

TcpSynFloodSocketFactory::TcpSynFloodSocketFactory () {
	m_factory.SetTypeId("ns3::TcpSynFloodSocket");
}

void TcpSynFloodSocketFactory::SetAttribute(std::string name, const AttributeValue &value) {
	m_factory.Set(name, value);
}

Ptr<Socket> TcpSynFloodSocketFactory::CreateSocket (void) {
	auto sock = m_factory.Create<TcpSynFloodSocket>();
	auto node = GetObject<Node>();
	if(!node) {
		return 0;
	}
	auto tcpl4proto = GetObject<TcpL4Protocol>();
	sock->SetNode(node);
	sock->SetTcp(tcpl4proto);
	return sock;
}

}
