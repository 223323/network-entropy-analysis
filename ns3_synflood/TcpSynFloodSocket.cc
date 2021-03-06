#include "TcpSynFloodSocket.h"
#include "ipv4-end-point.h"

namespace ns3 {
NS_LOG_COMPONENT_DEFINE ("TcpSynFloodSocket");

NS_OBJECT_ENSURE_REGISTERED (TcpSynFloodSocket);

TypeId TcpSynFloodSocket::GetTypeId (void) {
	static TypeId tid = TypeId ("ns3::TcpSynFloodSocket")
		.SetParent<Socket> ()
		.AddConstructor<TcpSynFloodSocket> ()
		.AddAttribute("RandomizeSourceAddress", "RandomizeSourceAddress",
					  BooleanValue(false),
					  MakeBooleanAccessor(&TcpSynFloodSocket::m_randomizeSourceAddress),
					  MakeBooleanChecker())
		.AddAttribute("SaveAttackTimes", "SaveAttackTimes",
					  BooleanValue(false),
					  MakeBooleanAccessor(&TcpSynFloodSocket::m_saveAttackTimes),
					  MakeBooleanChecker())
		.AddAttribute ("SrcPort", "",
					   StringValue ("ns3::ConstantRandomVariable[Min=1,Max=50000]"),
					   MakePointerAccessor (&TcpSynFloodSocket::m_srcPortGenerator),
					   MakePointerChecker <RandomVariableStream>())
		;
	return tid;
}

TcpSynFloodSocket::TcpSynFloodSocket () {
	NS_LOG_FUNCTION (this);
	ObjectFactory obj;
	obj.SetTypeId("ns3::UniformRandomVariable");
	obj.Set("Min", DoubleValue(1));
	obj.Set("Max", DoubleValue(50000));
	m_srcPortGenerator = obj.Create<UniformRandomVariable>();
	m_dstPortGenerator = obj.Create<UniformRandomVariable>();

	obj.Set("Min", DoubleValue(1));
	obj.Set("Max", DoubleValue(0xfffffffe));
	m_srcAddressGenerator = obj.Create<UniformRandomVariable>();
	m_saveAttackTimesFile = 0;
	m_lastAttackTime = 0;
}


int TcpSynFloodSocket::Bind (const Address &address) {
	return 0;
}

void TcpSynFloodSocket::SetTcp(Ptr<TcpL4Protocol> tcp) {
	m_tcp = tcp;
}

int TcpSynFloodSocket::Bind () {
	return 0;
}

int TcpSynFloodSocket::Close (void) {
	if(m_saveAttackTimesFile) {
		m_saveAttackTimesFile->close();
	}
	return 0;
}

int TcpSynFloodSocket::Connect (const Address &address) {
	NS_LOG_FUNCTION (this);
	InetSocketAddress transport = InetSocketAddress::ConvertFrom (address);
	Ipv4Address ipv4 = transport.GetIpv4 ();
	m_endPoint = m_tcp->Allocate ();
	m_endPoint->SetPeer(ipv4, transport.GetPort());

	std::stringstream ss;
	ss << "attack_times_" << ipv4 << ".txt";
	m_saveAttackTimesFile = new std::ofstream(ss.str(), std::ios::out);

	SetupEndpoint();
	NotifyConnectionSucceeded();
	return 0;
}

int TcpSynFloodSocket::SetupEndpoint () {
	NS_LOG_FUNCTION (this);
	Ptr<Ipv4> ipv4 = m_node->GetObject<Ipv4> ();
	NS_ASSERT (ipv4 != 0);
	if (ipv4->GetRoutingProtocol () == 0) {
		NS_FATAL_ERROR ("No Ipv4RoutingProtocol in the node");
	}
	// Create a dummy packet, then ask the routing function for the best output
	// interface's address
	Ipv4Header header;
	header.SetDestination (m_endPoint->GetPeerAddress ());
	Socket::SocketErrno errno_;
	Ptr<Ipv4Route> route;
	Ptr<NetDevice> oif = m_boundnetdevice;
	route = ipv4->GetRoutingProtocol ()->RouteOutput (Ptr<Packet> (), header, oif, errno_);
	if (route == 0) {
		NS_LOG_LOGIC ("Route to " << m_endPoint->GetPeerAddress () << " does not exist");
		NS_LOG_ERROR (errno_);
		return -1;
	}
	NS_LOG_LOGIC ("Route exists");
	m_endPoint->SetLocalAddress (route->GetSource ());
	return 0;
}

int TcpSynFloodSocket::Send (Ptr<Packet> p, uint32_t flags) {
	TcpHeader tcphdr;
	// tcphdr.SetDestinationPort(m_dstPortGenerator->GetInteger());
	tcphdr.SetDestinationPort(m_endPoint->GetPeerPort());
	tcphdr.SetFlags(TcpHeader::SYN);
	tcphdr.SetSequenceNumber(++m_seq);
	tcphdr.SetSourcePort(m_srcPortGenerator->GetInteger());
	Ipv4Address s_addr;
	if(m_randomizeSourceAddress) {
		s_addr = Ipv4Address(m_srcAddressGenerator->GetInteger());
	} else {
		s_addr = m_endPoint->GetLocalAddress();
	}
	if(m_saveAttackTimesFile) {
		double sec_now = Simulator::Now().GetSeconds();
		if(sec_now - m_lastAttackTime > 0.1) {
			*m_saveAttackTimesFile << sec_now;
		}
		m_lastAttackTime = sec_now;
	}
	
	m_tcp->SendPacket(p, tcphdr, s_addr, m_endPoint->GetPeerAddress(), m_boundnetdevice);
	int size = p->GetSize() + tcphdr.GetLength();
	NotifyDataSent(size);
	NotifySend(1500);
	return size;
}

// -------------------------

int TcpSynFloodSocket::Bind6 () {
	NS_LOG_FUNCTION (this);
	return -1;
}

int TcpSynFloodSocket::ShutdownSend (void) {
	NS_LOG_FUNCTION (this);
	return 0;
}

int TcpSynFloodSocket::ShutdownRecv (void) {
	NS_LOG_FUNCTION (this);
	return 0;
}

int TcpSynFloodSocket::Listen (void) {
	NS_LOG_FUNCTION (this);
	return -1;
}

uint32_t TcpSynFloodSocket::GetTxAvailable (void) const {
	NS_LOG_FUNCTION (this);
	const uint32_t MTU = 1500;
	return MTU;
}

int TcpSynFloodSocket::SendTo (Ptr<Packet> p, uint32_t flags,
                               const Address &toAddress) {
	NS_LOG_FUNCTION (this);
	NS_ABORT_MSG_IF(true, "should be unused function");
	return 0;
}

uint32_t TcpSynFloodSocket::GetRxAvailable (void) const {
	NS_LOG_FUNCTION (this);
	return 0;
}

Ptr<Packet> TcpSynFloodSocket::Recv (uint32_t maxSize, uint32_t flags) {
	NS_LOG_FUNCTION (this);
	return 0;
}


Ptr<Packet> TcpSynFloodSocket::RecvFrom (uint32_t maxSize, uint32_t flags,
        Address &fromAddress) {
	NS_LOG_FUNCTION (this);
	return 0;
}


TcpSynFloodSocket::~TcpSynFloodSocket (void) {
	NS_LOG_FUNCTION (this);
}


int TcpSynFloodSocket::GetSockName (Address &address) const {
	NS_LOG_FUNCTION (this);
	return 0;
}

int TcpSynFloodSocket::GetPeerName (Address &address) const {
	NS_LOG_FUNCTION (this);
	return 0;
}

bool TcpSynFloodSocket::SetAllowBroadcast (bool allowBroadcast) {
	NS_LOG_FUNCTION (this);
	return false;
}
bool TcpSynFloodSocket::GetAllowBroadcast () const {
	NS_LOG_FUNCTION (this);
	return false;
}

enum Socket::SocketErrno TcpSynFloodSocket::GetErrno (void) const {
	NS_LOG_FUNCTION (this);
	return ERROR_NOTERROR;
}

enum Socket::SocketType TcpSynFloodSocket::GetSocketType (void) const {
	NS_LOG_FUNCTION (this);
	return NS3_SOCK_DGRAM;
}

Ptr<Node> TcpSynFloodSocket::GetNode (void) const {
	NS_LOG_FUNCTION (this);
	return m_node;
}

void TcpSynFloodSocket::SetNode (Ptr<Node> node) {
	NS_LOG_FUNCTION (this);
	m_node = node;
}


}

