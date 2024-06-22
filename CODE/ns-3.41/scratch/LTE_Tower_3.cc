#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/lte-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/simulator.h"
#include "ns3/mobility-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("LTE_Simulation");

void SendPacket(Ptr<Socket> socket, Ptr<Packet> packet, Time time) {
  socket->Send(packet);
  NS_LOG_INFO("Packet sent at time " << time.GetSeconds() << " seconds");
}

int main ()
{

  // LTE network configuration
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();

  NodeContainer ueNodes;
  ueNodes.Create (10); // Create 10 UE devices
  NS_LOG_INFO("Created 10 UE nodes");

  // Set mobility model for UE nodes
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel");
  mobility.Install(ueNodes);
  NS_LOG_INFO("Installed mobility model on UE nodes");

  NetDeviceContainer ueDevs;
  ueDevs = lteHelper->InstallUeDevice (ueNodes);
  NS_LOG_INFO("Installed UE devices");

  // Install Internet stack on ueNodes
  InternetStackHelper internet;
  internet.Install (ueNodes);
  NS_LOG_INFO("Installed Internet stack on UE nodes");

  // Point-to-point connections
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("10Mbps"))); // Reduced data rate
  p2p.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2))); // Reduced delay

  NodeContainer gwRouterNodes;
  gwRouterNodes.Create (2);
  NS_LOG_INFO("Created 2 gateway router nodes");

  internet.Install (gwRouterNodes);
  NS_LOG_INFO("Installed Internet stack on gateway router nodes");

  NetDeviceContainer gwRouterDevs;
  gwRouterDevs = p2p.Install (gwRouterNodes);
  NS_LOG_INFO("Installed gateway router devices");

  NodeContainer routerRhsNodes;
  routerRhsNodes.Add (gwRouterNodes.Get (1));
  routerRhsNodes.Create (1);
  NS_LOG_INFO("Created 1 router RHS node");

  internet.Install (routerRhsNodes);
  NS_LOG_INFO("Installed Internet stack on router RHS node");

  NetDeviceContainer routerRhsDevs;
  routerRhsDevs = p2p.Install (routerRhsNodes);
  NS_LOG_INFO("Installed router RHS devices");

  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer routerRhsInterfaces = ipv4.Assign (routerRhsDevs);
  NS_LOG_INFO("Assigned IP addresses to router RHS interfaces");

  // Data communication
  uint16_t port = 9;

  for (uint16_t i = 0; i < 10; ++i) {
    Ptr<Node> ueNode = ueNodes.Get (i);
    Ptr<Socket> socket = Socket::CreateSocket(ueNode, TcpSocketFactory::GetTypeId());
    InetSocketAddress remoteAddr (routerRhsInterfaces.GetAddress (1), port);
    socket->Connect (remoteAddr);

    // Send packets
    Ptr<Packet> packet = Create<Packet>(1024);
    for (Time t = Seconds (0); t <= Seconds (60); t += Seconds (1)) {
      Simulator::Schedule (t, &SendPacket, socket, packet, t);
    }
  }

  // Simulation time
  Simulator::Stop (Seconds (60.0)); // Reduced simulation time to 1 minute
  NS_LOG_INFO("Simulation stopping at 1 minute");

  // Run simulation
  Simulator::Run ();
  NS_LOG_INFO("Simulation started");
  Simulator::Destroy ();
  NS_LOG_INFO("Simulation destroyed");

  return 0;
}