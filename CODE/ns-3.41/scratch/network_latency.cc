#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LteSimulation");

int main(int argc, char *argv[]) {

    /* 1. LOG START */
    LogComponentEnable("LteSimulation", LOG_LEVEL_INFO);
    LogComponentEnable("LteEnbRrc", LOG_LEVEL_INFO);
    LogComponentEnable("LteUeRrc", LOG_LEVEL_INFO);

    /* 2. LTE and EPC helpers initialization */
    Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();
    Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper>();
    lteHelper->SetEpcHelper(epcHelper);

    /* 3. Node containers setup */
    NodeContainer ueNodes;
    ueNodes.Create(20);
    NodeContainer enbNodes;
    enbNodes.Create(2); // Added one more eNodeB
    NodeContainer remoteHostContainer;
    remoteHostContainer.Create(1);

    /* 4. Internet stack for the remote host */
    InternetStackHelper internet;
    internet.Install(remoteHostContainer);

    /* 5. Point-to-point link configuration */
    PointToPointHelper p2ph;
    p2ph.SetDeviceAttribute("DataRate", StringValue("100Gb/s"));
    p2ph.SetChannelAttribute("Delay", StringValue("1ms"));
    NetDeviceContainer internetDevices = p2ph.Install(epcHelper->GetPgwNode(), remoteHostContainer.Get(0));

    // IP addressing for P2P link
    Ipv4AddressHelper ipv4h;
    ipv4h.SetBase("1.0.0.0", "255.255.255.0");
    ipv4h.Assign(internetDevices);

    /* 6. Set up the mobility model */

    // Set up the mobility for eNodeB at a fixed position
    MobilityHelper enbMobility;
    enbMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator>();
    enbPositionAlloc->Add(Vector(32.0, 32.0, 0.0)); // eNodeB position
    enbMobility.SetPositionAllocator(enbPositionAlloc);
    enbMobility.Install(enbNodes);

    // UE Mobility Configuration
    // Setup a position allocator that distributes UEs uniformly around the eNodeBs
    Ptr<UniformDiscPositionAllocator> uePositionAlloc = CreateObject<UniformDiscPositionAllocator>();
    uePositionAlloc->SetX(32.0);
    uePositionAlloc->SetY(32.0);
    uePositionAlloc->SetRho(10.0); // Radius around the eNodeBs

    // Use the RandomWalk2dMobilityModel for UEs with bounds set to keep them in the area
    MobilityHelper ueMobility;
    ueMobility.SetPositionAllocator(uePositionAlloc);
    ueMobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                                "Bounds", RectangleValue(Rectangle(22, 42, 22, 42)), // Adjust bounds as necessary
                                "Distance", DoubleValue(30), // Distance before changing direction
                                "Speed", StringValue("ns3::ConstantRandomVariable[Constant=5.0]")); // Speed of UEs
    ueMobility.Install(ueNodes);

    // Set up the Mobility model for the remote host
    MobilityHelper remoteHostMobility;
    remoteHostMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    Ptr<ListPositionAllocator> positionAllocRemote = CreateObject<ListPositionAllocator>();
    positionAllocRemote->Add(Vector(10.0, 50.0, 0.0));  // Set position of remote host to (10,50,0)
    remoteHostMobility.SetPositionAllocator(positionAllocRemote);
    remoteHostMobility.Install(remoteHostContainer);

    /* 7. Install LTE devices */
    NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice(enbNodes);
    NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice(ueNodes);

    // Assign IP addresses to UEs and install the internet stack
    internet.Install(ueNodes);
    epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueLteDevs));
    for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {
        lteHelper->Attach(ueLteDevs.Get(u), enbLteDevs.Get(0));
    }

    /* 8. Add three more UEs and another eNodeB */

    NodeContainer ueNodes2;
    ueNodes2.Create(20);

    // UE Mobility Configuration for the second set of UEs
    // Setup a position allocator that distributes UEs uniformly around the eNodeBs
    Ptr<UniformDiscPositionAllocator> uePositionAlloc2 = CreateObject<UniformDiscPositionAllocator>();
    uePositionAlloc2->SetX(62.0); // Adjust X position as needed
    uePositionAlloc2->SetY(40.0); // Adjust Y position as needed
    uePositionAlloc2->SetRho(10.0); // Radius around the eNodeBs

    // Use the RandomWalk2dMobilityModel for UEs with bounds set to keep them in the area
    MobilityHelper ueMobility2;
    ueMobility2.SetPositionAllocator(uePositionAlloc2);
    ueMobility2.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                                  "Bounds", RectangleValue(Rectangle(52, 72, 30, 50)), // Adjust bounds as necessary
                                  "Distance", DoubleValue(30), // Distance before changing direction
                                  "Speed", StringValue("ns3::ConstantRandomVariable[Constant=5.0]")); // Speed of UEs
    ueMobility2.Install(ueNodes2);

    /* 9. Update the NetAnim animator */

    AnimationInterface anim("extended-lte-04/net-latency.xml");
    anim.EnablePacketMetadata(true);
    anim.SetMobilityPollInterval(Seconds(1));

    // Update Node Description, Image, and Size for UE nodes
    for (uint32_t i = 0; i < ueNodes.GetN(); ++i) {
        anim.UpdateNodeDescription(ueNodes.Get(i), "UE_A" + std::to_string(i + 1));
        anim.UpdateNodeImage(ueNodes.Get(i)->GetId(), anim.AddResource("/home/tanvir/Documents/ns-allinone-3.41/netanim-3.109/phone.png"));
        anim.UpdateNodeSize(ueNodes.Get(i), 5, 5);
    }
    for (uint32_t i = 0; i < ueNodes2.GetN(); ++i) {
        anim.UpdateNodeDescription(ueNodes2.Get(i), "UE_A" + std::to_string(i + 1));
        anim.UpdateNodeImage(ueNodes2.Get(i)->GetId(), anim.AddResource("/home/tanvir/Documents/ns-allinone-3.41/netanim-3.109/phone.png"));
        anim.UpdateNodeSize(ueNodes2.Get(i), 5, 5);
    }
    

    // Update Node Description, Image, and Size for eNodeBs
    /*
    for (uint32_t i = 0; i < enbNodes.GetN(); ++i) {
        anim.UpdateNodeDescription(enbNodes.Get(i), "Tower" + std::to_string(i + 1));
        anim.UpdateNodeImage(enbNodes.Get(i)->GetId(), anim.AddResource("/home/tanvir/Documents/ns-allinone-3.41/netanim-3.109/tower-pic.png"));
        anim.UpdateNodeSize(enbNodes.Get(i), 7, 7);
    }
    */
       // Update Node Description and Size for first eNodeB
    anim.UpdateNodeDescription(enbNodes.Get(0), "Tower1");
    anim.UpdateNodeImage(enbNodes.Get(0)->GetId(), anim.AddResource("/home/tanvir/Documents/ns-allinone-3.41/netanim-3.109/tower-pic.png"));
    anim.UpdateNodeSize(enbNodes.Get(0), 7, 7);

    // Update Node Description, Size, and Image for second eNodeB
    anim.UpdateNodeDescription(enbNodes.Get(1), "Tower2");
    anim.UpdateNodeImage(enbNodes.Get(1)->GetId(), anim.AddResource("/home/tanvir/Documents/ns-allinone-3.41/netanim-3.109/tower-pic.png"));
    anim.UpdateNodeSize(enbNodes.Get(1), 7, 7);
    

    // Update Node Description, Image, and Size for remote host
    anim.UpdateNodeDescription(remoteHostContainer.Get(0), "Remote-Host");
    anim.UpdateNodeImage(remoteHostContainer.Get(0)->GetId(), anim.AddResource("/home/tanvir/Documents/ns-allinone-3.41/netanim-3.109/remote-host.png"));
    anim.UpdateNodeSize(remoteHostContainer.Get(0), 7, 7);

    /* 10. Stop the simulation */
    Simulator::Stop(Seconds(10.0));

/* 12. Flow monitor statistics output */
FlowMonitorHelper flowmon;
Ptr<FlowMonitor> monitor = flowmon.InstallAll();

// Run the simulation
Simulator::Run();

// Get the flow stats
std::map<FlowId, FlowMonitor::FlowStats> flowStats = monitor->GetFlowStats();

// Create a vector to store the latency values over time
std::vector<std::pair<double, double>> latencyValues;

// Calculate and store the latency values over time
for (double t = 0; t <= 10; t += 0.1) {
    double latency = 0;
    uint32_t packetsReceived = 0;
    for (auto it = flowStats.begin(); it!= flowStats.end(); ++it) {
        FlowMonitor::FlowStats stats = it->second;
        packetsReceived += stats.rxPackets;
        latency += stats.delaySum.GetSeconds();
    }
    latency /= packetsReceived;
    latencyValues.push_back(std::make_pair(t, latency));
}

// Print latency data
std::cout << "Average Latency: " << latencyValues.back().second << " s" << std::endl;

// Generate graph using gnuplot
std::ofstream plotFile("latency.plot");
plotFile << "set terminal png\n";
plotFile << "set output 'latency.png'\n";
plotFile << "set xlabel 'Time (s)'\n";
plotFile << "set ylabel 'Latency (s)'\n";
plotFile << "set yrange [0:10]\n"; // Set y-range explicitly
plotFile << "plot '-' with lines\n";
for (auto it = latencyValues.begin(); it!= latencyValues.end(); ++it) {
    plotFile << it->first << " " << it->second << "\n";
}
plotFile << "e\n";
plotFile.close();

system("gnuplot -p latency.plot");

    /* 13. Destroy the simulator */
    Simulator::Destroy();

    return 0;
}
