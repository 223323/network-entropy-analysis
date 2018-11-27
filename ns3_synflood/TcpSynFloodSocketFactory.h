#ifndef TCPSYNFLOODFACTORY_H
#define TCPSYNFLOODFACTORY_H
#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/socket-factory.h"
namespace ns3 {

class TcpSynFloodSocketFactory : public SocketFactory {
	public:
		static TypeId GetTypeId (void);
		TcpSynFloodSocketFactory ();
		void SetAttribute(std::string name, const AttributeValue &value);
		virtual Ptr<Socket> CreateSocket (void);
	private:
		ObjectFactory m_factory;
};

}
#endif
