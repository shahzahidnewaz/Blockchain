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
    ueNodes.Create(2);
    NodeContainer enbNodes;
    enbNodes.Create(1);
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
    enbPositionAlloc->Add(Vector(18.0, 18.0, 0.0)); // eNodeB position
    enbMobility.SetPositionAllocator(enbPositionAlloc);
    enbMobility.Install(enbNodes);

    // UE Mobility Configuration
    // Setup a position allocator that distributes UEs uniformly around the eNodeB
    Ptr<UniformDiscPositionAllocator> uePositionAlloc = CreateObject<UniformDiscPositionAllocator>();
    uePositionAlloc->SetX(18.0);
    uePositionAlloc->SetY(18.0);
    uePositionAlloc->SetRho(10.0); // Radius around the eNodeB

    // Use the RandomWalk2dMobilityModel for UEs with bounds set to keep them in the area
    MobilityHelper ueMobility;
    ueMobility.SetPositionAllocator(uePositionAlloc);
    ueMobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                                "Bounds", RectangleValue(Rectangle(8, 28, 8, 28)), // Adjust bounds as necessary
                                "Distance", DoubleValue(30), // Distance before changing direction
                                "Speed", StringValue("ns3::ConstantRandomVariable[Constant=5.0]")); // Speed of UEs
    ueMobility.Install(ueNodes);

    // Set up the Mobility model for the remote host
    MobilityHelper remoteHostMobility;
    remoteHostMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    Ptr<ListPositionAllocator> positionAllocRemote = CreateObject<ListPositionAllocator>();
    positionAllocRemote->Add(Vector(10.0, 50.0, 0.0));  // Set position of remote host to (70,70,0)
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
    ueNodes2.Create(3);
    NodeContainer enbNodes2;
    enbNodes2.Create(1);

    // UE Mobility Configuration for the second set of UEs
    // Setup a position allocator that distributes UEs uniformly around the eNodeB
    Ptr<UniformDiscPositionAllocator> uePositionAlloc2 = CreateObject<UniformDiscPositionAllocator>();
    uePositionAlloc2->SetX(20.0); // Adjust X position as needed
    uePositionAlloc2->SetY(20.0); // Adjust Y position as needed
    uePositionAlloc2->SetRho(10.0); // Radius around the eNodeB

    // Use the RandomWalk2dMobilityModel for UEs with bounds set to keep them in the area
    MobilityHelper ueMobility2;
    ueMobility2.SetPositionAllocator(uePositionAlloc2);
    ueMobility2.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                                  "Bounds", RectangleValue(Rectangle(10, 30, 10, 30)), // Adjust bounds as necessary
                                  "Distance", DoubleValue(30), // Distance before changing direction
                                  "Speed", StringValue("ns3::ConstantRandomVariable[Constant=5.0]")); // Speed of UEs
    ueMobility2.Install(ueNodes2);

    // Set up the Mobility model for the second set of eNodeBs
    MobilityHelper enbMobility2;
    enbMobility2.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    Ptr<ListPositionAllocator> enbPositionAlloc2 = CreateObject<ListPositionAllocator>();
    enbPositionAlloc2->Add(Vector(20.0, 20.0, 0.0)); // eNodeB position
    enbMobility2.SetPositionAllocator(enbPositionAlloc2);
    enbMobility2.Install(enbNodes2);

    /* 9. Update the NetAnim animator */

    AnimationInterface anim("extended-lte-04/lte-animation_v2_1.xml");
    anim.EnablePacketMetadata(true);
    anim.SetMobilityPollInterval(Seconds(1));

    // Update node descriptions for the new nodes
    anim.UpdateNodeDescription(ueNodes2.Get(0), "UE_B1");
    anim.UpdateNodeDescription(ueNodes2.Get(1), "UE_B2");
    anim.UpdateNodeDescription(ueNodes2.Get(2), "UE_B3");
    anim.UpdateNodeDescription(enbNodes2.Get(0), "eNodeB2");

    // Update node positions for the new nodes
    anim.SetConstantPosition(ueNodes2.Get(0), 10, 20, 0);
    anim.SetConstantPosition(ueNodes2.Get(1), 15, 25, 0);
    anim.SetConstantPosition(ueNodes2.Get(2), 20, 30, 0);
    anim.SetConstantPosition(enbNodes2.Get(0), 20, 20, 0);

    // Set colors, images, or any other attributes for the new nodes as needed


    /* 10. Stop the simulation */
    Simulator::Stop(Seconds(10.0));

    /* 11. Run the simulation */
    Simulator::Run();

    /* 12. Flow monitor statistics output */
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();
    monitor->SerializeToXmlFile("extended-lte-04/lte-flowmon_v2_1.xml", true, true);

    /* 13. Destroy the simulator */
    Simulator::Destroy();

    return 0;
}
