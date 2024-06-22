
// Exponential Traffic Generators


#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ExponentialTrafficGenerator");

int main ()
{
  LogComponentEnable ("ExponentialTrafficGenerator", LOG_LEVEL_INFO);

  uint32_t packetSize = 1000; // bytes
  double dataRate = 5000000; // bps
  double interval = 0.1; // seconds

  NS_LOG_INFO ("Creating nodes");
  NodeContainer nodes;
  nodes.Create (2);

  NS_LOG_INFO ("Creating point-to-point link");
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", DataRateValue (DataRate (dataRate)));
  pointToPoint.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (10)));

  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);

  NS_LOG_INFO ("Installing internet stack");
  InternetStackHelper stack;
  stack.Install (nodes);

  NS_LOG_INFO ("Assigning IP addresses");
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces = address.Assign (devices);

  NS_LOG_INFO ("Creating exponential traffic generator");
  Ptr<ExponentialRandomVariable> randomVariable = CreateObject<ExponentialRandomVariable> ();
  randomVariable->SetAttribute ("Mean", DoubleValue (interval));

  OnOffHelper onOffHelper ("ns3::UdpSocketFactory", Address ());
  onOffHelper.SetAttribute ("OnTime", PointerValue (randomVariable));
  onOffHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  onOffHelper.SetAttribute ("DataRate", DataRateValue (DataRate (dataRate)));
  onOffHelper.SetAttribute ("PacketSize", UintegerValue (packetSize));

  NS_LOG_INFO ("Installing sender");
  AddressValue remoteAddress (InetSocketAddress (interfaces.GetAddress (1), 9));
  onOffHelper.SetAttribute ("Remote", remoteAddress);
  ApplicationContainer senderApp = onOffHelper.Install (nodes.Get (0));
  senderApp.Start (Seconds (1.0));
  senderApp.Stop (Seconds (10.0));

  NS_LOG_INFO ("Installing receiver");
  PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 9));
  ApplicationContainer receiverApp = packetSinkHelper.Install (nodes.Get (1));
  receiverApp.Start (Seconds (0.0));
  receiverApp.Stop (Seconds (10.0));

  NS_LOG_INFO ("Starting simulation");
  Simulator::Stop (Seconds (10.0));
  Simulator::Run ();
  Simulator::Destroy ();

  NS_LOG_INFO ("Simulation completed");

  return 0;
}

