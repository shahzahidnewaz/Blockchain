#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/netanim-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/udp-server.h"
#include "ns3/udp-client.h"
#include "ns3/tcp-sink-helper.h"
#include "ns3/tcp-source.h"
#include "ns3/tcp-sink.h"
#include "ns3/udp-sink.h"

using namespace ns3;

int main ()
{
  // Create two eNodeBs
  Ptr<LteEnbNetDevice> enb1 = CreateObject<LteEnbNetDevice> ();
  Ptr<LteEnbNetDevice> enb2 = CreateObject<LteEnbNetDevice> ();

  // Create 10 UEs
  Ptr<LteUeNetDevice> ue1 = CreateObject<LteUeNetDevice> ();
  Ptr<LteUeNetDevice> ue2 = CreateObject<LteUeNetDevice> ();
  Ptr<LteUeNetDevice> ue3 = CreateObject<LteUeNetDevice> ();
  Ptr<LteUeNetDevice> ue4 = CreateObject<LteUeNetDevice> ();
  Ptr<LteUeNetDevice> ue5 = CreateObject<LteUeNetDevice> ();
  Ptr<LteUeNetDevice> ue6 = CreateObject<LteUeNetDevice> ();
  Ptr<LteUeNetDevice> ue7 = CreateObject<LteUeNetDevice> ();
  Ptr<LteUeNetDevice> ue8 = CreateObject<LteUeNetDevice> ();
  Ptr<LteUeNetDevice> ue9 = CreateObject<LteUeNetDevice> ();
  Ptr<LteUeNetDevice> ue10 = CreateObject<LteUeNetDevice> ();

  // Create nodes for eNodeBs and UEs
  Ptr<Node> enbNode1 = CreateObject<Node> ();
  Ptr<Node> enbNode2 = CreateObject<Node> ();
  Ptr<Node> ueNode1 = CreateObject<Node> ();
  Ptr<Node> ueNode2 = CreateObject<Node> ();
  Ptr<Node> ueNode3 = CreateObject<Node> ();
  Ptr<Node> ueNode4 = CreateObject<Node> ();
  Ptr<Node> ueNode5 = CreateObject<Node> ();
  Ptr<Node> ueNode6 = CreateObject<Node> ();
  Ptr<Node> ueNode7 = CreateObject<Node> ();
  Ptr<Node> ueNode8 = CreateObject<Node> ();
  Ptr<Node> ueNode9 = CreateObject<Node> ();
  Ptr<Node> ueNode10 = CreateObject<Node> ();

  // Install eNodeBs on nodes
  enbNode1->AddDevice (enb1);
  enbNode2->AddDevice (enb2);

  // Install UEs on nodes
  ueNode1->AddDevice (ue1);
  ueNode2->AddDevice (ue2);
  ueNode3->AddDevice (ue3);
  ueNode4->AddDevice (ue4);
  ueNode5->AddDevice (ue5);
  ueNode6->AddDevice (ue6);
  ueNode7->AddDevice (ue7);
  ueNode8->AddDevice (ue8);
  ueNode9->AddDevice (ue9);
  ueNode10->AddDevice (ue10);

  // Position eNodeBs and UEs
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (enbNode1);
  mobility.Install (enbNode2);
  mobility.Install (ueNode1);
  mobility.Install (ueNode2);
  mobility.Install (ueNode3);
  mobility.Install (ueNode4);
  mobility.Install (ueNode5);
  mobility.Install (ueNode6);
  mobility.Install (ueNode7);
  mobility.Install (ueNode8);
  mobility.Install (ueNode9);
  mobility.Install (ueNode10);

  // Configure LTE simulation
  LteHelper lteHelper;
  lteHelper.SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (25));
  lteHelper.SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (25));
  lteHelper.SetUeDeviceAttribute ("DlBandwidth", UintegerValue (25));
  lteHelper.SetUeDeviceAttribute ("UlBandwidth", UintegerValue (25));

  // Install LTE devices on nodes
  lteHelper.InstallEnbDevice (enbNode1);
  lteHelper.InstallEnbDevice (enbNode2);
  lteHelper.InstallUeDevice (ueNode1);
  lteHelper.InstallUeDevice (ueNode2);
  lteHelper.InstallUeDevice (ueNode3);
  lteHelper.InstallUeDevice (ueNode4);
  lteHelper.InstallUeDevice (ueNode5);
  lteHelper.InstallUeDevice (ueNode6);
  lteHelper.InstallUeDevice (ueNode7);
  lteHelper.InstallUeDevice (ueNode8);
  lteHelper.InstallUeDevice (ueNode9);
  lteHelper.InstallUeDevice (ueNode10);

  // Create Internet stack
  InternetStackHelper internet;
  internet.Install (enbNode1);
  internet.Install (enbNode2);
  internet.Install (ueNode1);
  internet.Install (ueNode2);
  internet.Install (ueNode3);
  internet.Install (ueNode4);
  internet.Install (ueNode5);
  internet.Install (ueNode6);
  internet.Install (ueNode7);
  internet.Install (ueNode8);
  internet.Install (ueNode9);
  internet.Install (ueNode10);

  // Create point-to-point links between eNodeBs and UEs
  PointToPointHelper p2p;
  
  // p2p.SetDeviceAttribute ("DataRate", DataRate ("10Mbps"));
  p2p.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("10Mbps")));
  
  // p2p.SetChannelAttribute ("Delay", Time ("2ms"));
  p2p.SetChannelAttribute ("Delay", TimeValue (Time ("2ms")));
  
  NetDeviceContainer enb1Ue1 = p2p.Install (enbNode1, ueNode1);
  NetDeviceContainer enb1Ue2 = p2p.Install (enbNode1, ueNode2);
  NetDeviceContainer enb2Ue7 = p2p.Install (enbNode2, ueNode7);
  NetDeviceContainer enb2Ue8 = p2p.Install (enbNode2, ueNode8);

  // Create a UDP server application
  Ptr<UdpSink> sink = CreateObject<UdpServer> ();
  ueNode8->AddApplication (sink);
  sink->Start (Seconds (1.0));

  // Create a UDP client application
  Ptr<UdpClient> source = CreateObject<UdpClient> ();
  ueNode1->AddApplication (source);
  source->Start (Seconds (1.0));

  // Create NetAnim animation
  AnimationInterface anim ("lte-simulation.xml");
  anim.SetMobilityPollInterval (Seconds (1));
  anim.EnablePacketMetadata (true);

  // Run the simulation
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
