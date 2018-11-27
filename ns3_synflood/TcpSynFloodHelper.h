#ifndef TCPSYNFLOODHELPER_H
#define TCPSYNFLOODHELPER_H
#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "TcpSynFloodSocketFactory.h"
namespace ns3 {
class TcpSynFloodHelper {
	public:
		TcpSynFloodHelper();
		void Install(NodeContainer n);
		void Install(Ptr<Node> n);
		void SetAttribute(std::string name, const AttributeValue &value);
	private:
		Ptr<TcpSynFloodSocketFactory> m_object;
};
}
#endif
