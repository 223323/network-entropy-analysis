#include "TcpSynFloodHelper.h"
#include "TcpSynFloodSocketFactory.h"
namespace ns3 {

TcpSynFloodHelper::TcpSynFloodHelper() {
	m_object = CreateObject<TcpSynFloodSocketFactory>();
}
void TcpSynFloodHelper::Install(Ptr<Node> n) {
	n->AggregateObject(CopyObject(m_object));
}
void TcpSynFloodHelper::Install(NodeContainer n) {
	for(auto it = n.Begin(); it != n.End(); it++) {
		Install(*it);
	}
}
void TcpSynFloodHelper::SetAttribute(std::string name, const AttributeValue &value) {
	m_object->SetAttribute(name, value);
}

}
