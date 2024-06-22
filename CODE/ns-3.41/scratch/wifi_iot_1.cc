#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/aodv-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/log.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("MyScript"); // Define the logging component

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


  // Set up AODV routing protocol
  AodvHelper aodv;
  Ipv4ListRoutingHelper list;
  list.Add(aodv, 0);

  // Set routing protocol for devices and routers
  for (uint32_t i = 0; i < devices.GetN(); ++i) {
    internet.SetRoutingHelper(list);
    internet.Install(devices.Get(i));
  }
  for (uint32_t i = 0; i < routers.GetN(); ++i) {
    internet.SetRoutingHelper(list);
    internet.Install(routers.Get(i));
  }

  // Set up simulation scenario
  Simulator::Stop(Seconds(600)); // Set simulation duration to 10 minutes

  // Run simulation
  Simulator::Run();
  NS_LOG_INFO("Simulation completed successfully!"); // Log simulation completion
  std::cout << "Simulation completed successfully!" << std::endl;
  
  Simulator::Destroy();

  return 0;
}

