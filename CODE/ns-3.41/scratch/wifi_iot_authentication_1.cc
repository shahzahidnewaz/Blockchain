#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/aodv-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/log.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/wifi-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("MyScript"); // Define the logging component

// Callback function for successful connection
void ConnectionSucceeded(Ptr<Socket> socket) {
  NS_LOG_INFO("Device authentication successful.");
}

// Callback function for failed connection
void ConnectionFailed(Ptr<Socket> socket) {
  NS_LOG_INFO("Device authentication failed.");
}

int main() {
  LogComponentEnable("MyScript", LOG_LEVEL_INFO); // Enable logging for MyScript component at INFO level

  // Create nodes for IoT devices, routers, and gateway
  NodeContainer devices;
  devices.Create(10);

  NodeContainer routers;
  routers.Create(2);

  NodeContainer gateway;
  gateway.Create(1);

  // Install internet stack on all nodes
  InternetStackHelper internet;
  internet.Install(devices);
  internet.Install(routers);
  internet.Install(gateway);

  // Set device properties
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", DataRateValue(DataRate("100kbps")));
  p2p.SetChannelAttribute("Delay", TimeValue(Time("10ms")));

  Ipv4AddressHelper addressHelper;

  // Devices
  addressHelper.SetBase("10.0.0.0", "255.255.255.0");
  for (uint32_t i = 0; i < devices.GetN(); ++i) {
    NetDeviceContainer deviceNetDevice = p2p.Install(devices.Get(i), routers.Get(0));
    Ipv4InterfaceContainer deviceInterface = addressHelper.Assign(deviceNetDevice);
  }

  // Routers
  Ipv4AddressHelper routerAddressHelper;
  routerAddressHelper.SetBase("10.0.1.0", "255.255.255.0");
  NetDeviceContainer router0NetDevice = p2p.Install(routers.Get(0), routers.Get(1));
  Ipv4InterfaceContainer router0Interface = routerAddressHelper.Assign(router0NetDevice);

  // Gateway
  Ipv4AddressHelper gatewayAddressHelper;
  gatewayAddressHelper.SetBase("10.0.2.0", "255.255.255.0");
  NetDeviceContainer router1NetDevice = p2p.Install(routers.Get(1), gateway.Get(0));
  Ipv4InterfaceContainer router1Interface = gatewayAddressHelper.Assign(router1NetDevice);

  // Gateway to device
  Ipv4AddressHelper gatewayToDeviceAddressHelper;
  gatewayToDeviceAddressHelper.SetBase("10.0.3.0", "255.255.255.0");
  NetDeviceContainer gatewayNetDevice = p2p.Install(gateway.Get(0), devices.Get(0));
  Ipv4InterfaceContainer gatewayInterface = gatewayToDeviceAddressHelper.Assign(gatewayNetDevice);

// Set up WiFi
WifiHelper wifi;
wifi.SetStandard(ns3::WIFI_STANDARD_80211g); // Change to 802.11g standard

YansWifiChannelHelper wifiChannel;
wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
wifiChannel.AddPropagationLoss("ns3::FixedRssLossModel", "Rss", DoubleValue(-60));

YansWifiPhyHelper wifiPhy;
wifiPhy.SetChannel(wifiChannel.Create());

WifiMacHelper wifiMac;
wifiMac.SetType("ns3::AdhocWifiMac");

// Create an access point (AP)
NodeContainer apNode;
apNode.Create(1);
NetDeviceContainer apDevice = wifi.Install(wifiPhy, wifiMac, apNode.Get(0));

// Create a WiFi MAC layer for the devices
wifiMac.SetType("ns3::StaWifiMac");
  
// Connect devices to the AP
for (uint32_t i = 0; i < devices.GetN(); ++i) {
    NetDeviceContainer deviceNetDevice = wifi.Install(wifiPhy, wifiMac, devices.Get(i));
}

  // Set up animation
  AnimationInterface anim("wifi_outputs/wifi_iot_authentication_1/wifi_iot_auth_1-animation.xml"); // Create animation interface
  anim.EnableIpv4RouteTracking("wifi_outputs/wifi_iot_authentication_1/wifi_iot_1-routingtable.xml", Seconds(0), Seconds(600), Seconds(1)); // Enable IPv4 route tracking
  
  anim.SetConstantPosition(devices.Get(0), 10.0, 10.0); // Set device position
  anim.SetConstantPosition(routers.Get(0), 20.0, 20.0); // Set router position
  anim.SetConstantPosition(gateway.Get(0), 30.0, 30.0); // Set gateway position

  // Set up flow monitor
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();

  // Set up simulation scenario
  Simulator::Stop(Seconds(600)); // Set simulation duration to 10 minutes

  // Run simulation
  Simulator::Run();
  NS_LOG_INFO("Simulation completed successfully!"); // Log simulation completion
  std::cout << "Simulation completed successfully!" << std::endl;

  // Flow monitor statistics output
  monitor->SerializeToXmlFile("wifi_outputs/wifi_iot_authentication_1/wifi_iot_1-flowmon.xml", true, true);

  Simulator::Destroy();

  return 0;
}

