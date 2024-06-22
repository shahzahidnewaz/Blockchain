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

class MyApp : public Application {
public:
  MyApp();
  virtual ~MyApp();

  void Setup(Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);
  void StartApplication();
  void StopApplication();

private:
  void SendPacket();
  void ScheduleTx();
  Ptr<Socket> m_socket;
  Address m_peer;
  uint32_t m_packetSize;
  uint32_t m_nPackets;
  DataRate m_dataRate;
  EventId m_sendEvent;
  uint32_t m_packetsSent;
};

MyApp::MyApp()
  : m_socket(0), m_peer(), m_packetSize(0), m_nPackets(0), m_dataRate(0), m_sendEvent(), m_packetsSent(0)
{
  NS_LOG_INFO("MyApp constructor called");
}

MyApp::~MyApp()
{
  NS_LOG_INFO("MyApp destructor called");
}

void
MyApp::Setup(Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
  NS_LOG_INFO("MyApp setup called");
}

void
MyApp::StartApplication()
{
  m_sendEvent = Simulator::Schedule(Seconds(1.0), &MyApp::SendPacket, this);
  NS_LOG_INFO("MyApp start application called");
}

void
MyApp::StopApplication()
{
  Simulator::Cancel(m_sendEvent);
  NS_LOG_INFO("MyApp stop application called");
}

void
MyApp::SendPacket()
{
  Ptr<Packet> packet = Create<Packet>(m_packetSize);
  m_socket->Send(packet);
  m_packetsSent++;
  if (m_packetsSent < m_nPackets) {
    ScheduleTx();
  }
  NS_LOG_INFO("Packet sent");
}

void
MyApp::ScheduleTx()
{
  Time tNext(Seconds(m_packetSize * 8 / static_cast<double>(m_dataRate.GetBitRate())));
  m_sendEvent = Simulator::Schedule(tNext, &MyApp::SendPacket, this);
  NS_LOG_INFO("Scheduling next packet transmission");
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
  p2p.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Mbps")));
  p2p.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (5)));

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
    Ptr<MyApp> app = CreateObject<MyApp> ();
    app->Setup (socket, remoteAddr, 1024, 1000000, DataRate ("1Mbps"));
    ueNode->AddApplication (app);
    app->SetStartTime (Seconds (1.0));
    app->SetStopTime (Seconds (10.0));
    NS_LOG_INFO("Added application to UE node");
  }

  // Simulation time
  Simulator::Stop (Seconds (10.0));
  NS_LOG_INFO("Simulation stopping at 10 seconds");

  // Run simulation
  Simulator::Run ();
  NS_LOG_INFO("Simulation started");
  Simulator::Destroy ();
  NS_LOG_INFO("Simulation destroyed");

  return 0;
}