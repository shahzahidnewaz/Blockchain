#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/aodv-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/log.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/wifi-module.h"
#include "ns3/tcp-header.h"
#include "ns3/tcp-socket.h"

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

  // Create nodes for IoT devices, router, and gateway
  NodeContainer devices;
  devices.Create(10);

  NodeContainer router;
  router.Create(1);

  NodeContainer gateway;
  gateway.Create(1);

  // Install internet stack on all nodes
  InternetStackHelper internet;
  internet.Install(devices);
  internet.Install(router);
  internet.Install(gateway);

  // Set device properties
  WifiHelper wifi;
  wifi.SetStandard(WifiStandard(WIFI_STANDARD_80211g));

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss("ns3::FixedRssLossModel", "Rss", DoubleValue(-60));

  YansWifiPhyHelper wifiPhy;
  wifiPhy.SetChannel(wifiChannel.Create());

  WifiMacHelper wifiMac;
  wifiMac.SetType("ns3::StaWifiMac");

  NetDeviceContainer deviceNetDevices;
  for (uint32_t i = 0; i < devices.GetN(); ++i) {
    NetDeviceContainer deviceNetDevice = wifi.Install(wifiPhy, wifiMac, devices.Get(i));
    deviceNetDevices.Add(deviceNetDevice);
  }

  // Set up point-to-point connections
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", DataRateValue(DataRate("100kbps")));
  p2p.SetChannelAttribute("Delay", TimeValue(Time("10ms")));

  // Devices
  Ipv4AddressHelper addressHelper;
  addressHelper.SetBase("10.0.0.0", "255.255.255.0");
  for (uint32_t i = 0; i < devices.GetN(); ++i) {
    NetDeviceContainer deviceNetDevice = p2p.Install(devices.Get(i), router.Get(0));
    Ipv4InterfaceContainer deviceInterface = addressHelper.Assign(deviceNetDevice);
  }

  // Router to gateway
  Ipv4AddressHelper routerToGatewayAddressHelper;
  routerToGatewayAddressHelper.SetBase("10.0.1.0", "255.255.255.0");
  NetDeviceContainer routerNetDevice = p2p.Install(router.Get(0), gateway.Get(0));
  Ipv4InterfaceContainer routerInterface = routerToGatewayAddressHelper.Assign(routerNetDevice);

  // Basic authentication
  std::string username = "iot-device";
  std::string password = "password";

  // Create a socket for authentication
  Ptr<TcpSocket> authSocket = Socket::CreateSocket(devices.Get(0), TcpSocketFactory::GetTypeId()); 
  // Connect to the  server
  InetSocketAddress serverAddress(Ipv4Address("10.0.0.1"), 8080);
  authSocket->Connect(serverAddress);

  // Send authentication request
  std::string packetData = "AUTH " + username + " " + password;
  Ptr<TcpSocket> authSocket = Object::DynamicCast<TcpSocket>(Socket::CreateSocket(devices.Get(0), TcpSocketFactory::GetTypeId()));
  
  authSocket->Send(packet);

  // Receive authentication response
  Ptr<Packet> response = authSocket->Recv();

  if (response->GetSize() > 0) {
    std::string responseStr = response->ToString();
    if (responseStr == "AUTH_OK") {
      NS_LOG_INFO("Authentication successful!");
    } else {
      NS_LOG_INFO("Authentication failed!");
    }
  }

  // Set up animation
  AnimationInterface anim("wifi_outputs/wifi_iot_authentication_2/wifi_iot_2-animation.xml"); // Create animation interface
  anim.EnableIpv4RouteTracking("wifi_outputs/wifi_iot_authentication_2/wifi_iot_2-routingtable.xml", Seconds(0), Seconds(600), Seconds(1)); // Enable IPv4 route tracking
  
  anim.SetConstantPosition(devices.Get(0), 10.0, 10.0); // Set device position
  anim.SetConstantPosition(router.Get(0), 20.0, 20.0); // Set router position
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
  monitor->SerializeToXmlFile("wifi_outputs/wifi_iot_authentication_2/wifi_iot_2-flowmon.xml", true, true);

  Simulator::Destroy();

  return 0;
}