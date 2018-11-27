#include <iostream>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-dumbbell.h"
#include "ns3/on-off-helper.h"
#include "ns3/animation-interface.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/packet-sink-helper.h"


NS_LOG_COMPONENT_DEFINE ("SynFlood");

#undef NS_LOG_COMPONENT_DEFINE
#define NS_LOG_COMPONENT_DEFINE(x)
#include "TcpSynFloodHelper.h"
#include "TcpSynFloodSocketFactory.h"
#include "TcpSynFloodSocket.h"

using namespace ns3;

int main(int argc, char **argv)
{
	// basic configuration
	Config::SetDefault ("ns3::Ipv4GlobalRouting::RespondToInterfaceEvents", BooleanValue (true));
	GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));
	// Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpVegas"));
	Time::SetResolution (Time::MS);
	LogComponentEnable("SynFlood", LOG_LEVEL_INFO);
	srand(time(0));
	
	// -------
	PointToPointHelper PTPRouter;
	PointToPointHelper PTPLeaf;

	PTPRouter.SetDeviceAttribute("DataRate", StringValue("20Mbps"));
	PTPRouter.SetChannelAttribute("Delay", StringValue("25ms"));

	PTPLeaf.SetDeviceAttribute("DataRate", StringValue("20Mbps"));
	PTPLeaf.SetChannelAttribute("Delay", StringValue("10ms"));
	
	// left side
	int num_clients = 100;
	int num_attackers = 20;
	
	// right side
	int num_servers = 20;

	PointToPointDumbbellHelper dumbbell(num_clients+num_attackers, PTPLeaf,  // left side
										num_servers, PTPLeaf, 				 // right side
										PTPRouter); // middle

	// install internet stack to all nodes in dumbbell network
	InternetStackHelper stack;
	dumbbell.InstallStack(stack);

	// assign ips to dumbbell nodes
	dumbbell.AssignIpv4Addresses(Ipv4AddressHelper("10.0.0.0", "255.255.255.0"),
								 Ipv4AddressHelper("11.0.0.0", "255.255.255.0"),
								 Ipv4AddressHelper("12.0.0.0", "255.255.255.0"));

	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
	// --------

	// setup clients
	ApplicationContainer clientApps;
	int server_port = 50000;
	for(int i=0; i < num_clients; i++) {
		// TCP Clients
		OnOffHelper clientHelper("ns3::TcpSocketFactory", Address());
		
		clientHelper.SetAttribute("OnTime",  StringValue("ns3::UniformRandomVariable[Min=1][Max=15]"));
		if(i < 30) {
			clientHelper.SetAttribute("OnTime",  StringValue("ns3::ConstantRandomVariable[Constant=15.0]"));
			clientHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));
		} else {
			clientHelper.SetAttribute("OffTime", StringValue("ns3::UniformRandomVariable[Min=0.1][Max=7]"));
		}
		
		clientHelper.SetAttribute("PacketSize", UintegerValue(1000 + rand() % 500));
		std::string s_drate = std::to_string(100 + rand() % 400) + "Kbps";
		clientHelper.SetAttribute("DataRate", DataRateValue(DataRate(s_drate.c_str())) );

		// talk with random server
		// AddressValue remote(InetSocketAddress(dumbbell.GetRightIpv4Address(rand() % num_servers), server_port));
		AddressValue remote(InetSocketAddress(dumbbell.GetRightIpv4Address(i % num_servers), server_port));
		
		// set remote
		clientHelper.SetAttribute("Remote", remote);
		clientApps.Add(clientHelper.Install(dumbbell.GetLeft(i)));
	}
	
	// setup servers
	ApplicationContainer serverApps;
	PacketSinkHelper serverHelper("ns3::TcpSocketFactory", Address());
	for(int i=0; i < num_servers; i++) {
		AddressValue local(InetSocketAddress(dumbbell.GetRightIpv4Address(i), server_port));
		serverHelper.SetAttribute("Local", local);
		serverApps.Add(serverHelper.Install(dumbbell.GetRight(i)));
	}
	// -------
	
	
	
	// setup attackers
	int attackers_offset = num_clients;
	for(int i=attackers_offset; i < attackers_offset+num_attackers; i++) {
		
		OnOffHelper clientHelperAttack("ns3::TcpSynFloodSocketFactory", Address());
		clientHelperAttack.SetAttribute("OnTime",  StringValue("ns3::ConstantRandomVariable[Constant=2]"));
		clientHelperAttack.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=5]"));
		clientHelperAttack.SetAttribute("DataRate", DataRateValue(DataRate("30Kbps")));
		clientHelperAttack.SetAttribute("PacketSize", UintegerValue(100));
		
		// attack 1st server
		AddressValue attack_target(InetSocketAddress(dumbbell.GetRightIpv4Address(0), server_port));
		clientHelperAttack.SetAttribute("Remote", attack_target);
		
		// install syn flood socket factory to nodes which are going to attack server
		TcpSynFloodHelper synflood;
		synflood.SetAttribute("SrcPort", StringValue("ns3::UniformRandomVariable[Min=5000][Max=50000]"));
		synflood.SetAttribute("RandomizeSourceAddress", BooleanValue(true));
		synflood.Install(dumbbell.GetLeft(i));
		
		clientApps.Add(clientHelperAttack.Install(dumbbell.GetLeft(i)));
	}
	// ----------
	
	// setup output
	Ptr<Node> n = dumbbell.GetRight();
	for(unsigned i=0; i < n->GetNDevices(); i++) {
		PTPLeaf.EnablePcap("pcap", n->GetDevice(i));
	}

	// Setting up simulation
	serverApps.Start(Seconds(0.0));
	serverApps.Stop(Seconds(50.0));

	clientApps.Start(Seconds(0.0));
	clientApps.Stop(Seconds(50.0));
	
	// dumbbell.BoundingBox (1, 1, 100, 100);
	// AnimationInterface anim ("dumbbell-tcp-animation.xml");

	Simulator::Run();
	Simulator::Destroy();

	return 0;
}
