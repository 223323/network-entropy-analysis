#ifndef TCPSYNFLOODSOCKET_H
#define TCPSYNFLOODSOCKET_H
#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/socket.h"
#include "ns3/ptr.h"
#include "ns3/internet-module.h"

namespace ns3 {

class TcpSynFloodSocket : public Socket {
  public:
  static TypeId GetTypeId (void);

  TcpSynFloodSocket (void);
  virtual ~TcpSynFloodSocket (void);
  virtual enum Socket::SocketErrno GetErrno (void) const;
  virtual enum Socket::SocketType GetSocketType (void) const;
  virtual Ptr<Node> GetNode (void) const;
  void SetNode (Ptr<Node> node);
  
  int SetupEndpoint();
  virtual int Bind (const Address &address);
  virtual int Bind ();
  virtual int Bind6 ();
  virtual int Close (void);
  virtual int ShutdownSend (void);
  virtual int ShutdownRecv (void);
  virtual int Connect (const Address &address);
  virtual int Listen (void);
  virtual uint32_t GetTxAvailable (void) const;
  virtual int Send (Ptr<Packet> p, uint32_t flags);
  virtual int SendTo (Ptr<Packet> p, uint32_t flags, 
                      const Address &toAddress);
  virtual uint32_t GetRxAvailable (void) const;
  virtual Ptr<Packet> Recv (uint32_t maxSize, uint32_t flags);
  virtual Ptr<Packet> RecvFrom (uint32_t maxSize, uint32_t flags,
                                Address &fromAddress);
  virtual int GetSockName (Address &address) const; 
  virtual int GetPeerName (Address &address) const;
  virtual bool SetAllowBroadcast (bool allowBroadcast);
  virtual bool GetAllowBroadcast () const;
	
	void SetTcp(Ptr<TcpL4Protocol> tcp);
	private:
		Ptr<RandomVariableStream> m_srcAddressGenerator;
		Ptr<RandomVariableStream> m_srcPortGenerator;
		Ptr<RandomVariableStream> m_dstPortGenerator;
		SequenceNumber32 m_seq;
		Ipv4EndPoint* m_endPoint;
		Ptr<Node> m_node;
		Ptr<TcpL4Protocol> m_tcp;
		std::ofstream* m_saveAttackTimesFile;
		bool m_randomizeSourceAddress;
		bool m_saveAttackTimes;
		double m_lastAttackTime;
};

}
#endif
