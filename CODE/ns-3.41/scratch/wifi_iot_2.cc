#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/aodv-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/log.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"

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

  NodeContainer routers;
  routers.Create(1); // Only one router

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

  // Router
  Ipv4AddressHelper routerAddressHelper;
  routerAddressHelper.SetBase("10.0.1.0", "255.255.255.0");
  NetDeviceContainer routerNetDevice = p2p.Install(routers.Get(0), gateway.Get(0));
  Ipv4InterfaceContainer routerInterface = routerAddressHelper.Assign(routerNetDevice);

  // Gateway
  Ipv4AddressHelper gatewayAddressHelper;
  gatewayAddressHelper.SetBase("10.0.2.0", "255.255.255.0");
  NetDeviceContainer gatewayNetDevice = p2p.Install(gateway.Get(0), routers.Get(0));
  Ipv4InterfaceContainer gatewayInterface = gatewayAddressHelper.Assign(gatewayNetDevice);

  // Set up AODV routing protocol
  AodvHelper aodv;
  Ipv4ListRoutingHelper list;
  list.Add(aodv, 0);

  // Set routing protocol for devices and router
  internet.SetRoutingHelper(list);
  internet.Install(routers.Get(0));
  for (uint32_t i = 0; i < devices.GetN(); ++i) {
    internet.Install(devices.Get(i));
  }

  // Set up animation
  AnimationInterface anim("wifi_outputs/wifi_iot_2/wifi_iot_2-animation.xml"); // Create animation interface
  
  anim.EnableIpv4RouteTracking("wifi_outputs/wifi_iot_2/wifi_iot_2-routingtable.xml", Seconds(0), Seconds(600), Seconds(1)); // Enable IPv4 route tracking
  
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
  monitor->SerializeToXmlFile("wifi_outputs/wifi_iot_2/wifi_iot_2-flowmon.xml", true, true);

  Simulator::Destroy();

  return 0;
}

