/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: George F. Riley<riley@ece.gatech.edu>
 */

#include <iostream>

#include "ns3/core-module.h"
//#include "ns3/simulator-module.h"
//#include "ns3/node-module.h"
#include "ns3/network-module.h"
//#include "ns3/helper-module.h"
#include "ns3/point-to-point-dumbbell.h"
#include "ns3/on-off-helper.h"
//#include "ns3/random-variable.h"
#include "ns3/animation-interface.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/packet-sink-helper.h"

#include "TcpSynFloodHelper.h"

NS_LOG_COMPONENT_DEFINE ("SynFlood");


// ---- dodato ------
#undef NS_LOG_COMPONENT_DEFINE
#define NS_LOG_COMPONENT_DEFINE(x)
#include "TcpSynFloodHelper.cc"
#include "TcpSynFloodFactory.cc"
#include "TcpSynFloodSocket.cc"
// -----------------

using namespace ns3;



int main(int argc, char **argv)
{

	// Options
	GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));
	// Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpVegas"));
	   
	LogComponentEnable("SynFlood", LOG_LEVEL_INFO);

	Time::SetResolution (Time::MS);
	//
	// Setting up network topology
	// It is a dumbbell network with two leaves on each side
	
	// ptp router, leaf
	PointToPointHelper PTPRouter;
	PointToPointHelper PTPLeaf;

	PTPRouter.SetDeviceAttribute("DataRate", StringValue("20Mbps"));
	PTPRouter.SetChannelAttribute("Delay", StringValue("25ms"));

	PTPLeaf.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
	PTPLeaf.SetChannelAttribute("Delay", StringValue("1ms"));
	
	int num_clients = 100;
	int num_attackers = 20;
	int num_servers = 20;

	// ptpdumbell
	PointToPointDumbbellHelper dumbbell(num_clients+num_attackers, PTPLeaf, num_servers, PTPLeaf, PTPRouter);

	// set internet stack
	InternetStackHelper stack;
	dumbbell.InstallStack(stack);

	// assign some ips
	dumbbell.AssignIpv4Addresses(Ipv4AddressHelper("10.0.0.0", "255.255.255.0"),
								 Ipv4AddressHelper("11.0.0.0", "255.255.255.0"),
								 Ipv4AddressHelper("12.0.0.0", "255.255.255.0"));

	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

	// ------ print ip addresses
	/*
	std::cout << "Left leaves: " << std::endl;
	std::cout << "\tUpper: " << dumbbell.GetLeftIpv4Address(0) << std::endl;
	std::cout << "\tLower: " << dumbbell.GetLeftIpv4Address(1) << std::endl;
	std::cout << "\tLower2: " << dumbbell.GetLeftIpv4Address(2) << std::endl;

	std::cout << "Right leaves: " << std::endl;
	std::cout << "\tUpper: " << dumbbell.GetRightIpv4Address(0) << std::endl;
	std::cout << "\tLower: " << dumbbell.GetRightIpv4Address(1) << std::endl;
    */
   
	for(uint i=0; i < dumbbell.LeftCount(); i++) {
		Names::Add(std::string("Left(")+std::to_string(i)+")", dumbbell.GetLeft(i));
	}
	
	for(uint i=0; i < dumbbell.RightCount(); i++) {
		Names::Add(std::string("Right(")+std::to_string(i)+")", dumbbell.GetRight(i));
	}
	// ---------------
	
	//
	// Setting up applications
	ApplicationContainer clientApps;
	ApplicationContainer serverApps;
	
	OnOffHelper clientHelperAttack("ns3::TcpSynFloodSocketFactory", Address());
	clientHelperAttack.SetAttribute("OnTime",  StringValue("ns3::ConstantRandomVariable[Constant=2]"));
	clientHelperAttack.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=5]"));
	clientHelperAttack.SetAttribute("DataRate", DataRateValue(DataRate("60Kbps")));
	clientHelperAttack.SetAttribute("PacketSize", UintegerValue(100));
	
	int server_port = 50000;
	
	// attack 1st server
	AddressValue attack_target(InetSocketAddress(dumbbell.GetRightIpv4Address(0), server_port));
	clientHelperAttack.SetAttribute("Remote", attack_target);
	
	TcpSynFloodHelper synflood;
	synflood.SetAttribute("SrcPort", StringValue("ns3::UniformRandomVariable[Min=5000][Max=50000]"));
	synflood.SetAttribute("DstPort", StringValue("ns3::ConstantRandomVariable[Constant=" + std::to_string(server_port) + "]"));
	synflood.SetAttribute("RandomizeSourceAddress", BooleanValue(false));
	
	// setup attackers
	for(int i=0; i < num_attackers; i++) {
		synflood.Install(dumbbell.GetLeft(i));
		clientApps.Add(clientHelperAttack.Install(dumbbell.GetLeft(i)));
	}
	
	srand(65654647);
	int clients_ofs = num_attackers;
	// setup clients
	for(int i=clients_ofs; i < clients_ofs+num_clients; i++) {
		
		// TCP Clients
		OnOffHelper clientHelperTcp("ns3::TcpSocketFactory", Address());
		
		clientHelperTcp.SetAttribute("OnTime",  StringValue("ns3::UniformRandomVariable[Min=1][Max=15]"));
		if(i < clients_ofs+30) {
			clientHelperTcp.SetAttribute("OnTime",  StringValue("ns3::ConstantRandomVariable[Constant=15.0]"));
			clientHelperTcp.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));
		} else {
			clientHelperTcp.SetAttribute("OffTime", StringValue("ns3::UniformRandomVariable[Min=0.1][Max=7]"));
		}
		clientHelperTcp.SetAttribute("PacketSize", UintegerValue(1000 + rand() % 500));
		std::string s_drate = std::to_string(100 + rand() % 400) + std::string("Kbps");
		clientHelperTcp.SetAttribute("DataRate", DataRateValue(DataRate( s_drate.c_str() )) );

		// talk with random server
		AddressValue remote(InetSocketAddress(dumbbell.GetRightIpv4Address(rand() % num_servers), server_port));
		
		// set remote
		clientHelperTcp.SetAttribute("Remote", remote);
		
		clientApps.Add(clientHelperTcp.Install(dumbbell.GetLeft(i)));
	}
	
	// TCP Servers
	PacketSinkHelper serverHelper("ns3::TcpSocketFactory", Address());
	
	// setup servers
	for(int i=0; i < num_servers; i++) {
		AddressValue local(InetSocketAddress(dumbbell.GetRightIpv4Address(i), server_port));
		serverHelper.SetAttribute("Local", local);
		serverApps.Add(serverHelper.Install(dumbbell.GetRight(i)));
	}
	
	Ptr<Node> n = dumbbell.GetRight();
	for(unsigned i=0; i < n->GetNDevices(); i++) {
		PTPLeaf.EnablePcap("pcap", n->GetDevice(i));
	}

	// ascii print
	// AsciiTraceHelper ascii;
	// PTPRouter.EnableAsciiAll (ascii.CreateFileStream ("dumbbell-tcp.tr"));

	// Setting up simulation
	serverApps.Start(Seconds(0.0));
	serverApps.Stop(Seconds(50.0));

	clientApps.Start(Seconds(0.0));
	clientApps.Stop(Seconds(50.0));

	// PTPRouter.EnablePcapAll("PTPRouter");
	// PTPLeaf.EnablePcapAll("PTPRouter");
	
	// dumbbell.BoundingBox (1, 1, 100, 100);
	
	// AnimationInterface anim ("dumbbell-tcp-animation.xml");

	Simulator::Run();
	Simulator::Destroy();

	std::cout << "Done." << std::endl;

	return 0;
}
