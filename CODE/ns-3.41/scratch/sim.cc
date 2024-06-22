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

    // Set up the mobility model for eNodeBs
    MobilityHelper enbMobility;
    enbMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

    // Set position of first eNodeB to (32, 32)
    Ptr<UniformDiscPositionAllocator> uePositionAlloc_1 = CreateObject<UniformDiscPositionAllocator>();
    uePositionAlloc_1->SetX(32.0);
    uePositionAlloc_1->SetY(32.0);
    uePositionAlloc_1->SetRho(10.0); // Radius around the eNodeBs

    // Use the RandomWalk2dMobilityModel for UEs with bounds set to keep them in the area
    MobilityHelper ueMobility;
    ueMobility.SetPositionAllocator(uePositionAlloc_1);
    ueMobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                                "Bounds", RectangleValue(Rectangle(22, 42, 22, 42)), // Adjust bounds as necessary
                                "Distance", DoubleValue(5.0), // Distance before changing direction
                                "Speed", StringValue("ns3::ConstantRandomVariable[Constant=5.0]")); // Speed of UEs
    ueMobility.Install(ueNodes);

    // UE Mobility Configuration for the second set of UEs
    // Setup a position allocator that distributes UEs uniformly around the eNodeBs
    Ptr<UniformDiscPositionAllocator> uePositionAlloc_2 = CreateObject<UniformDiscPositionAllocator>();
    uePositionAlloc_2->SetX(62.0); // Adjust X position as needed
    uePositionAlloc_2->SetY(40.0); // Adjust Y position as needed
    uePositionAlloc_2->SetRho(15.0); // Radius around the eNodeBs

    // Use the RandomWalk2dMobilityModel for UEs with bounds set to keep them in the area
    MobilityHelper ueMobility2;
    ueMobility2.SetPositionAllocator(uePositionAlloc_2);
    ueMobility2.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                                  "Bounds", RectangleValue(Rectangle(55, 35, 70, 45)), // Adjust bounds as necessary
                                  "Distance", DoubleValue(5.0), // Distance before changing direction
                                  "Speed", StringValue("ns3::ConstantRandomVariable[Constant=5.0]")); // Speed of UEs
    ueMobility2.Install(ueNodes);

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

    /* 8. Add 20 more UEs and another eNodeB */

    NodeContainer ueNodes2;
    ueNodes2.Create(20);
    NetDeviceContainer ueLteDevs_2 = lteHelper->InstallUeDevice(ueNodes2);
    internet.Install(ueNodes2);
    epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueLteDevs_2));
    for (uint32_t u = 0; u < ueNodes2.GetN(); ++u) {
        lteHelper->Attach(ueLteDevs_2.Get(u), enbLteDevs.Get(1));
    }

    // UE Mobility Configuration for the second set of UEs
    // Setup a position allocator that distributes UEs uniformly around the eNodeBs
    Ptr<UniformDiscPositionAllocator> uePositionAlloc_3 = CreateObject<UniformDiscPositionAllocator>();
    uePositionAlloc_3->SetX(62.0); // Adjust X position as needed
    uePositionAlloc_3->SetY(40.0); // Adjust Y position as needed
    uePositionAlloc_3->SetRho(15.0); // Radius around the eNodeBs

    // Use the RandomWalk2dMobilityModel for UEs with bounds set to keep them in the area
    MobilityHelper ueMobility3;
    ueMobility3.SetPositionAllocator(uePositionAlloc_3);
    ueMobility3.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                                  "Bounds", RectangleValue(Rectangle(65, 65, 60, 60)), // Adjust bounds as necessary
                                  "Distance", DoubleValue(5.0), // Distance before changing direction
                                  "Speed", StringValue("ns3::ConstantRandomVariable[Constant=5.0]")); // Speed of UEs
    ueMobility3.Install(ueNodes2);

    /* 9. Update the NetAnim animator */

    AnimationInterface anim("extended-lte-04/lte-animation_v2_3_stationary.xml");
    anim.EnablePacketMetadata(true);
    anim.SetMobilityPollInterval(Seconds(1));

    // Update Node Description, Image, and Size for UE nodes
    for (uint32_t i = 0; i < ueNodes.GetN(); ++i) {
        anim.UpdateNodeDescription(ueNodes.Get(i), "UE_A" + std::to_string(i + 1));
        anim.UpdateNodeImage(ueNodes.Get(i)->GetId(), anim.AddResource("/home/tanvir/Documents/ns-allinone-3.41/netanim-3.109/phone.png"));
        anim.UpdateNodeSize(ueNodes.Get(i), 5, 5);
    }
    for (uint32_t i = 0; i < ueNodes2.GetN(); ++i) {
        anim.UpdateNodeDescription(ueNodes2.Get(i), "UE_B" + std::to_string(i + 1));
        anim.UpdateNodeImage(ueNodes2.Get(i)->GetId(), anim.AddResource("/home/tanvir/Documents/ns-allinone-3.41/netanim-3.109/phone.png"));
        anim.UpdateNodeSize(ueNodes2.Get(i), 5, 5);
    }

    // Update Node Description and Size for first eNodeB
    anim.UpdateNodeDescription(enbNodes.Get(0), "Tower1");
    anim.UpdateNodeImage(enbNodes.Get(0)->GetId(), anim.AddResource("/home/tanvir/Documents/ns-allinone-3.41/netanim-3.109/tower-pic.png"));
    anim.UpdateNodeSize(enbNodes.Get(0), 7, 7);

    // Update Node Description, Size, and Image for second eNodeB
    anim.UpdateNodeDescription(enbNodes.Get(1), "Tower2");
    anim.UpdateNodeImage(enbNodes.Get(1)->GetId(), anim.AddResource("/home/tanvir/Documents/ns-allinone-3.41/netanim-3.109/tower-pic2.png")); // Use a different image resource
    anim.UpdateNodeSize(enbNodes.Get(1), 9, 9);

    // Update Node Description, Image, and Size for remote host
    anim.UpdateNodeDescription(remoteHostContainer.Get(0), "Remote-Host");
    anim.UpdateNodeImage(remoteHostContainer.Get(0)->GetId(), anim.AddResource("/home/tanvir/Documents/ns-allinone-3.41/netanim-3.109/remote-host.png"));
    anim.UpdateNodeSize(remoteHostContainer.Get(0), 7, 7);

    /* 10. Stop the simulation */
    Simulator::Stop(Seconds(10.0));

    /* 11. Flow monitor statistics output */
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    // Run the simulation
    Simulator::Run();



    /* 13. Destroy the simulator */
    Simulator::Destroy();

    return 0;
}
