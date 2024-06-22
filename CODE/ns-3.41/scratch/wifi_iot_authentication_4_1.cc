#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/log.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/applications-module.h"
#include "ns3/packet.h"
#include "ns3/mobility-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("MyScript"); // Define the logging component

int main() {
  LogComponentEnable("MyScript", LOG_LEVEL_INFO); // Enable logging for MyScript component at INFO level

  // Set mobility model for devices
  MobilityHelper mobility;

  // Create nodes for IoT devices, router, and gateway
  NodeContainer devices;
  devices.Create(10);
  mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                "MinX", DoubleValue(0.0),
                                "MinY", DoubleValue(0.0),
                                "DeltaX", DoubleValue(10.0),
                                "DeltaY", DoubleValue(10.0),
                                "GridWidth", UintegerValue(3),
                                "LayoutType", StringValue("RowFirst"));
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(devices);

  NodeContainer router;
  router.Create(1);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(router);

  NodeContainer gateway;
  gateway.Create(1);
  mobility.Install(gateway);

  // Install internet stack on all nodes
  InternetStackHelper internet;
  internet.Install(devices);
  internet.Install(router);
  internet.Install(gateway);

  // Set up point-to-point connections
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", DataRateValue(DataRate("100kbps")));
  p2p.SetChannelAttribute("Delay", TimeValue(Time("10ms")));

  // Devices to router
  Ipv4AddressHelper addressHelper;
  addressHelper.SetBase("10.0.0.0", "255.255.255.0");
  NetDeviceContainer routerToDeviceNetDevice;
  for (uint32_t i = 0; i < devices.GetN(); ++i) {
    NetDeviceContainer deviceNetDevice = p2p.Install(devices.Get(i), router.Get(0));
    routerToDeviceNetDevice.Add(deviceNetDevice);
    addressHelper.Assign(deviceNetDevice);
  }

  // Router to gateway
  Ipv4InterfaceContainer routerInterface;
  routerInterface = addressHelper.Assign(p2p.Install(router.Get(0), gateway.Get(0)));
  
  // Setup device to gateway communication
  Ipv4Address gatewayIpAddress = routerInterface.GetAddress(1);
  NS_LOG_INFO("Gateway IP Address: " << gatewayIpAddress); // Debugging output
  uint16_t gatewayPort = 8080;

  PacketSinkHelper packetSinkHelper("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), gatewayPort));
  ApplicationContainer sinkApps = packetSinkHelper.Install(gateway);

  UdpClientHelper udpClientHelper(InetSocketAddress(gatewayIpAddress, gatewayPort));
  NS_LOG_INFO("Type of gatewayIpAddress: " << typeid(gatewayIpAddress).name());

  udpClientHelper.SetAttribute("MaxPackets", UintegerValue(1000000));
  udpClientHelper.SetAttribute("Interval", TimeValue(Seconds(0.01)));
  udpClientHelper.SetAttribute("PacketSize", UintegerValue(1024));

  ApplicationContainer clientApps;
  for (uint32_t i = 0; i < devices.GetN(); ++i) {
    clientApps.Add(udpClientHelper.Install(devices.Get(i)));
  }

  // Setup gateway to device communication
  uint16_t devicePort = 9090;

  PacketSinkHelper deviceSinkHelper("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), devicePort));
  ApplicationContainer deviceSinkApps;
  for (uint32_t i = 0; i < devices.GetN(); ++i) {
    deviceSinkApps.Add(deviceSinkHelper.Install(devices.Get(i)));
  }

  UdpClientHelper deviceUdpClientHelper;
  deviceUdpClientHelper.SetAttribute("MaxPackets", UintegerValue(1000000));
  deviceUdpClientHelper.SetAttribute("Interval", TimeValue(Seconds(0.01)));
  deviceUdpClientHelper.SetAttribute("PacketSize", UintegerValue(1024));

  ApplicationContainer gatewayClientApps;
  gatewayClientApps.Add(deviceUdpClientHelper.Install(gateway.Get(0)));

  // Set up flow monitor
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();

  // Set up animation
  AnimationInterface anim("wifi_outputs/wifi_iot_authentication_4/wifi_iot_4-animation.xml"); // Create animation interface
  anim.EnablePacketMetadata(true);
  anim.EnableIpv4RouteTracking("wifi_outputs/wifi_iot_authentication_4/wifi_iot_4-routingtable.xml", Seconds(0), Seconds(600), Seconds(1)); // Enable IPv4 route tracking

  // Set node descriptions for devices
  for (uint32_t i = 0; i < devices.GetN(); ++i) {
    std::string nodeName = "Device_" + std::to_string(i + 1); // Generate a unique name for each device
    anim.UpdateNodeDescription(devices.Get(i), nodeName);

    if ((i + 1) % 2 == 0) {
      anim.UpdateNodeImage(devices.Get(i)->GetId(), anim.AddResource("/home/tanvir/Documents/ns-allinone-3.41/netanim-3.109/lap.png"));
    } else {
      anim.UpdateNodeImage(devices.Get(i)->GetId(), anim.AddResource("/home/tanvir/Documents/ns-allinone-3.41/netanim-3.109/phone.png"));
    }
  }

  anim.SetConstantPosition(router.Get(0), 26.0, 18.0); // Set router position
  anim.UpdateNodeDescription(router.Get(0), "ROUTER");
  anim.UpdateNodeImage(router.Get(0)->GetId(), anim.AddResource("/home/tanvir/Documents/ns-allinone-3.41/netanim-3.109/router.png"));

  anim.SetConstantPosition(gateway.Get(0), 26.0, 26.0); // Set gateway position
  anim.UpdateNodeDescription(gateway.Get(0), "ISP/GATEWAY");
  anim.UpdateNodeImage(gateway.Get(0)->GetId(), anim.AddResource("/home/tanvir/Documents/ns-allinone-3.41/netanim-3.109/gateway.png"));

  // Start the data transmission
  sinkApps.Start(Seconds(1.0));
  clientApps.Start(Seconds(1.0));
  deviceSinkApps.Start(Seconds(1.0));
  gatewayClientApps.Start(Seconds(1.0));

  // Run simulation
  Simulator::Stop(Seconds(30));
  Simulator::Run();
  NS_LOG_INFO("Simulation completed successfully!"); // Log simulation completion
  std::cout << "Simulation completed successfully!" << std::endl;

  // Flow monitor statistics output
  monitor->SerializeToXmlFile("wifi_outputs/wifi_iot_authentication_4/wifi_iot_4-flowmon.xml", true, true);

  // Cleanup
  Simulator::Destroy();

  return 0;
}
