#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
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

  // Create nodes for IoT devices and gateway
  NodeContainer devices;
  devices.Create(10);

  NodeContainer gateway;
  gateway.Create(1);

  // Install internet stack on all nodes
  InternetStackHelper internet;
  internet.Install(devices);
  internet.Install(gateway);

  // Set device properties
  WifiHelper wifi;
  wifi.SetStandard(WifiStandard(WIFI_STANDARD_80211ah)); // Update WiFi standard to 802.11ah

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

  // Devices to gateway
  Ipv4AddressHelper addressHelper;
  addressHelper.SetBase("10.0.0.0", "255.255.255.0");
  for (uint32_t i = 0; i < devices.GetN(); ++i) {
    NetDeviceContainer deviceNetDevice = p2p.Install(devices.Get(i), gateway.Get(0));
    Ipv4InterfaceContainer deviceInterface = addressHelper.Assign(deviceNetDevice);
  }

  // Gateway to internet
  Ipv4AddressHelper gatewayAddressHelper;
  gatewayAddressHelper.SetBase("10.0.1.0", "255.255.255.0");
  NetDeviceContainer gatewayNetDevice = p2p.Install(gateway.Get(0), gateway.Get(0)); // Connect gateway to itself (internet)
  Ipv4InterfaceContainer gatewayInterface = gatewayAddressHelper.Assign(gatewayNetDevice);

  // Set up MQTT communication (instead of CoAP)
  MqttHelper mqtt;
  mqtt.SetAttribute("Server", Ipv4Address("10.0.1.1")); // Set MQTT server address
  mqtt.SetAttribute("Port", UintegerValue(1883)); // Set MQTT port

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