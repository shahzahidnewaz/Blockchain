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
    NodeContainer ueNodes_1;
    NodeContainer ueNodes_2;
    NodeContainer ueNodes_3;
    NodeContainer ueNodes_4;
    ueNodes_1.Create(10);
    ueNodes_2.Create(10);
    ueNodes_3.Create(10);
    ueNodes_4.Create(10);

    NodeContainer enbNodes;
    enbNodes.Create(4); // Added one more eNodeB
    
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

// Set up the mobility model for eNodeB 1 start
MobilityHelper enbMobility_1;
enbMobility_1.SetMobilityModel("ns3::ConstantPositionMobilityModel");
Ptr<ListPositionAllocator> enbPositionAlloc_1 = CreateObject<ListPositionAllocator>();
enbPositionAlloc_1->Add(Vector(22.0, 22.0, 0.0)); // eNodeB position
enbMobility_1.SetPositionAllocator(enbPositionAlloc_1);
enbMobility_1.Install(enbNodes.Get(0));

// Set up the bounds for UE nodes around eNodeB 1
Ptr<UniformDiscPositionAllocator> uePositionAlloc_1 = CreateObject<UniformDiscPositionAllocator>();
uePositionAlloc_1->SetX(22.0);
uePositionAlloc_1->SetY(22.0);
uePositionAlloc_1->SetRho(20.0); // Radius around the eNodeB
MobilityHelper ueMobility_1;
ueMobility_1.SetPositionAllocator(uePositionAlloc_1);
ueMobility_1.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                                "Bounds", RectangleValue(Rectangle(2, 42, 2, 42)), // Adjust bounds as necessary
                                "Distance", DoubleValue(5.0), // Distance before changing direction
                                "Speed", StringValue("ns3::ConstantRandomVariable[Constant=5.0]")); // Speed of UEs
ueMobility_1.Install(ueNodes_1);
// Set up the mobility model for eNodeB 1 end 
// Set up the mobility model for eNodeB 2 start
MobilityHelper enbMobility_2;
enbMobility_2.SetMobilityModel("ns3::ConstantPositionMobilityModel");
Ptr<ListPositionAllocator> enbPositionAlloc_2 = CreateObject<ListPositionAllocator>();
enbPositionAlloc_2->Add(Vector(65.0, 65.0, 0.0)); // eNodeB position
enbMobility_2.SetPositionAllocator(enbPositionAlloc_2);
enbMobility_2.Install(enbNodes.Get(1));

// Set up the bounds for UE nodes around eNodeB 2
Ptr<UniformDiscPositionAllocator> uePositionAlloc_2 = CreateObject<UniformDiscPositionAllocator>();
uePositionAlloc_2->SetX(65.0);
uePositionAlloc_2->SetY(65.0);
uePositionAlloc_2->SetRho(20.0); // Radius around the eNodeB
MobilityHelper ueMobility_2;
ueMobility_2.SetPositionAllocator(uePositionAlloc_2);
ueMobility_2.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                                "Bounds", RectangleValue(Rectangle(45, 85, 45, 85)), // Adjust bounds as necessary
                                "Distance", DoubleValue(5.0), // Distance before changing direction
                                "Speed", StringValue("ns3::ConstantRandomVariable[Constant=5.0]")); // Speed of UEs
ueMobility_2.Install(ueNodes_2);
// Set up the mobility model for eNodeB 2 end
// Set up the mobility model for eNodeB 3 start
MobilityHelper enbMobility_3;
enbMobility_3.SetMobilityModel("ns3::ConstantPositionMobilityModel");
Ptr<ListPositionAllocator> enbPositionAlloc_3 = CreateObject<ListPositionAllocator>();
enbPositionAlloc_3->Add(Vector(70.0, 20.0, 0.0)); // eNodeB position
enbMobility_3.SetPositionAllocator(enbPositionAlloc_3);
enbMobility_3.Install(enbNodes.Get(2));

// Set up the bounds for UE nodes around eNodeB 2
Ptr<UniformDiscPositionAllocator> uePositionAlloc_3 = CreateObject<UniformDiscPositionAllocator>();
uePositionAlloc_3->SetX(70.0);
uePositionAlloc_3->SetY(20.0);
uePositionAlloc_3->SetRho(20.0); // Radius around the eNodeB
MobilityHelper ueMobility_3;
ueMobility_3.SetPositionAllocator(uePositionAlloc_3);
ueMobility_3.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                                "Bounds", RectangleValue(Rectangle(50, 90, 0, 40)), // Adjust bounds as necessary
                                "Distance", DoubleValue(5.0), // Distance before changing direction
                                "Speed", StringValue("ns3::ConstantRandomVariable[Constant=5.0]")); // Speed of UEs
ueMobility_3.Install(ueNodes_3);
// Set up the mobility model for eNodeB 3 end
// Set up the mobility model for eNodeB 4 start
MobilityHelper enbMobility_4;
enbMobility_4.SetMobilityModel("ns3::ConstantPositionMobilityModel");
Ptr<ListPositionAllocator> enbPositionAlloc_4 = CreateObject<ListPositionAllocator>();
enbPositionAlloc_4->Add(Vector(20.0, 65.0, 0.0)); // eNodeB position
enbMobility_4.SetPositionAllocator(enbPositionAlloc_4);
enbMobility_4.Install(enbNodes.Get(3));

// Set up the bounds for UE nodes around eNodeB 2
Ptr<UniformDiscPositionAllocator> uePositionAlloc_4 = CreateObject<UniformDiscPositionAllocator>();
uePositionAlloc_4->SetX(20.0);
uePositionAlloc_4->SetY(65.0);
uePositionAlloc_4->SetRho(20.0); // Radius around the eNodeB
MobilityHelper ueMobility_4;
ueMobility_4.SetPositionAllocator(uePositionAlloc_4);
ueMobility_4.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                                "Bounds", RectangleValue(Rectangle(0, 40, 45, 85)), // Adjust bounds as necessary
                                "Distance", DoubleValue(5.0), // Distance before changing direction
                                "Speed", StringValue("ns3::ConstantRandomVariable[Constant=5.0]")); // Speed of UEs
ueMobility_4.Install(ueNodes_4);
// Set up the mobility model for eNodeB 4 end

// Set up the Mobility model for the remote host
MobilityHelper remoteHostMobility;
remoteHostMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
Ptr<ListPositionAllocator> positionAllocRemote = CreateObject<ListPositionAllocator>();
positionAllocRemote->Add(Vector(50.0, 5.0, 0.0));  // Set position of remote host to (10,50,0)
remoteHostMobility.SetPositionAllocator(positionAllocRemote);
remoteHostMobility.Install(remoteHostContainer);

    /* 7. Install LTE devices */
    NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice(enbNodes);

    NetDeviceContainer ueLteDevs_1 = lteHelper->InstallUeDevice(ueNodes_1);
    NetDeviceContainer ueLteDevs_2 = lteHelper->InstallUeDevice(ueNodes_2);
    NetDeviceContainer ueLteDevs_3 = lteHelper->InstallUeDevice(ueNodes_3);
    NetDeviceContainer ueLteDevs_4 = lteHelper->InstallUeDevice(ueNodes_4);

    // Assign IP addresses to UEs and install the internet stack
    internet.Install(ueNodes_1);
    internet.Install(ueNodes_2);
    internet.Install(ueNodes_3);
    internet.Install(ueNodes_4);
    epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueLteDevs_1));
    epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueLteDevs_2));
    epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueLteDevs_3));
    epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueLteDevs_4));

    for (uint32_t u = 0; u < ueNodes_1.GetN(); ++u) {
        lteHelper->Attach(ueLteDevs_1.Get(u), enbLteDevs.Get(0));
    }
    for (uint32_t u = 0; u < ueNodes_2.GetN(); ++u) {
        lteHelper->Attach(ueLteDevs_2.Get(u), enbLteDevs.Get(1));
    }
    for (uint32_t u = 0; u < ueNodes_3.GetN(); ++u) {
        lteHelper->Attach(ueLteDevs_3.Get(u), enbLteDevs.Get(2));
    }
    for (uint32_t u = 0; u < ueNodes_4.GetN(); ++u) {
        lteHelper->Attach(ueLteDevs_4.Get(u), enbLteDevs.Get(3));
    }


    /* 9. Update the NetAnim animator */

    AnimationInterface anim("extended-lte-04/sim_2.xml");
    anim.EnablePacketMetadata(true);
    anim.SetMobilityPollInterval(Seconds(1));

    std::vector<NodeContainer> ueNodeContainers = {ueNodes_1, ueNodes_2, ueNodes_3, ueNodes_4};
    for (uint32_t j = 0; j < ueNodeContainers.size(); ++j) {
        char label = 'A' + j; // For labeling UE nodes (A, B, C, D)
        for (uint32_t i = 0; i < ueNodeContainers[j].GetN(); ++i) {
            std::string ueName = "UE_" + std::string(1, label) + std::to_string(i + 1);
            anim.UpdateNodeDescription(ueNodeContainers[j].Get(i), ueName);
            std::string imagePath;
            if (i % 2 == 0) {
                // Even index: Use "lap.png"
                imagePath = "/home/tanvir/Documents/ns-allinone-3.41/netanim-3.109/lap.png";
            } else {
                // Odd index: Use "phone.png"
                imagePath = "/home/tanvir/Documents/ns-allinone-3.41/netanim-3.109/phone.png";
            }
            anim.UpdateNodeImage(ueNodeContainers[j].Get(i)->GetId(), anim.AddResource(imagePath));
            anim.UpdateNodeSize(ueNodeContainers[j].Get(i), 5, 5);
        }
    }


    for (uint32_t i = 0; i < enbNodes.GetN(); ++i) {
        std::string towerName = "Tower" + std::to_string(i + 1);
        std::string imageName = (i % 2 == 0) ? "tower-pic.png" : "tower-pic2.png";
        anim.UpdateNodeDescription(enbNodes.Get(i), towerName);
        anim.UpdateNodeImage(enbNodes.Get(i)->GetId(), anim.AddResource("/home/tanvir/Documents/ns-allinone-3.41/netanim-3.109/" + imageName));
        anim.UpdateNodeSize(enbNodes.Get(i), 9, 9);
    }    

    // Update Node Description, Image, and Size for remote host
    anim.UpdateNodeDescription(remoteHostContainer.Get(0), "Remote-Host");
    anim.UpdateNodeImage(remoteHostContainer.Get(0)->GetId(), anim.AddResource("/home/tanvir/Documents/ns-allinone-3.41/netanim-3.109/remote-host.png"));
    anim.UpdateNodeSize(remoteHostContainer.Get(0), 7, 7);

    /* 10. Stop the simulation */
    Simulator::Stop(Seconds(10.0));

    /* 11. Flow monitor statistics output */
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();
    monitor->SerializeToXmlFile("extended-lte-04/sim_2-flowmon.xml", true, true);

    // Run the simulation
    Simulator::Run();



    /* 13. Destroy the simulator */
    Simulator::Destroy();

    return 0;
}
