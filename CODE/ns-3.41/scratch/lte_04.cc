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

/*4. Internet stack for the remote host */

    InternetStackHelper internet;
    internet.Install(remoteHostContainer);

/*5. Point-to-point link configuration */
  
    PointToPointHelper p2ph;
    p2ph.SetDeviceAttribute("DataRate", StringValue("100Gb/s"));
    p2ph.SetChannelAttribute("Delay", StringValue("1ms"));
    NetDeviceContainer internetDevices = p2ph.Install(epcHelper->GetPgwNode(), remoteHostContainer.Get(0));

    // IP addressing for P2P link
    Ipv4AddressHelper ipv4h;
    ipv4h.SetBase("1.0.0.0", "255.255.255.0");
    ipv4h.Assign(internetDevices);

/*6. Set up the mobility model    */

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

/*7 Install LTE devices */
    NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice(enbNodes);
    NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice(ueNodes);

    // Assign IP addresses to UEs and install the internet stack
    internet.Install(ueNodes);
    epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueLteDevs));
    for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {
        lteHelper->Attach(ueLteDevs.Get(u), enbLteDevs.Get(0));
    }

    // Route configuration for UEs accessing the remote host
    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ptr<Node> remoteHost = remoteHostContainer.Get(0);
    Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting(remoteHost->GetObject<Ipv4>());
    remoteHostStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(), 1);

    // Assuming ueIpIface is your Ipv4InterfaceContainer for UEs as previously defined
    Ipv4InterfaceContainer ueIpIface = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueLteDevs));

/*8 Setup a UDP application to send messages from the remote host to a UE */

    uint16_t dlPort = 1234;
    UdpEchoServerHelper echoServer(dlPort);
    ApplicationContainer serverApps = echoServer.Install(ueNodes.Get(1)); // Install server on UE2
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(9.0));

/*9 Assume you want to send messages from the remote host to UE2 (the second UE node) */

    UdpEchoClientHelper echoClient(ueIpIface.GetAddress(1), dlPort); // Use the IP address of UE2
    echoClient.SetAttribute("MaxPackets", UintegerValue(5));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));
    ApplicationContainer clientApps = echoClient.Install(remoteHostContainer.Get(0)); // Install client on remote host
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(9.0));

/*
10

Setting up a UDP server on UE 2 and a UDP client on UE 1, 
configuring the client to send exactly 5 packets to the server. 

Set up the UDP server on UE 2: This will listen for incoming messages.
Set up the UDP client on UE 1: This will send messages to the server (UE 2).

it's important that UE 1 knows the IP address of UE 2 to send messages directly to it.

Install and start the UDP echo server on UE 2
*/
    uint16_t port = 4000; // Port number for the UDP server
    UdpEchoServerHelper server(port);
    ApplicationContainer serverApp = server.Install(ueNodes.Get(1)); // Install server on UE2
    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(10.0));

    // Set the IP address of UE 2 for the UDP client on UE 1
    Ipv4Address ue2Address = ueIpIface.GetAddress(1); // Get UE 2's IP address from the Ipv4InterfaceContainer
    UdpEchoClientHelper client(ue2Address, port);
    client.SetAttribute("MaxPackets", UintegerValue(5)); // Set to send exactly 5 packets
    client.SetAttribute("Interval", TimeValue(Seconds(1.0))); // 1-second interval between packets
    client.SetAttribute("PacketSize", UintegerValue(1024)); // Size of each packet

    ApplicationContainer clientApp = client.Install(ueNodes.Get(0)); // Install client on UE1
    clientApp.Start(Seconds(2.0));
    clientApp.Stop(Seconds(10.0));


/*11 Enable tracing */
    
    lteHelper->EnableTraces();
    p2ph.EnablePcapAll("lte-04/packets/lte-04");

/*12 Flow monitor */
    
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

/*13 NetAnim animator setup */
    
    AnimationInterface anim("lte-04/lte-animation.xml");

    anim.EnablePacketMetadata(true);

    anim.SetMobilityPollInterval(Seconds(1));    

    anim.UpdateNodeDescription(ueNodes.Get(0), "UE_A1");
    anim.UpdateNodeDescription(ueNodes.Get(1), "UE_A2");
    anim.UpdateNodeDescription(remoteHostContainer.Get(0), "RemoteHost");
    anim.UpdateNodeDescription(enbNodes.Get(0), "eNodeB");

    anim.UpdateNodeImage(ueNodes.Get(0)->GetId(), anim.AddResource("/home/tanvir/Documents/ns-allinone-3.41/netanim-3.109/smartphone.png"));
    anim.UpdateNodeImage(ueNodes.Get(1)->GetId(), anim.AddResource("/home/tanvir/Documents/ns-allinone-3.41/netanim-3.109/smartphone.png"));
    
    
    anim.UpdateNodeSize(remoteHostContainer.Get(0), 5, 5);
    anim.UpdateNodeSize(ueNodes.Get(0), 5, 5);
    anim.UpdateNodeSize(ueNodes.Get(1), 5, 5);
    anim.UpdateNodeSize(enbNodes.Get(0), 5, 5);

    //anim.UpdateNodeImage(remoteHostContainer.Get(0)->GetId(), anim.AddResource("/home/tanvir/Documents/ns-allinone-3.41/netanim-3.109/mobile.jpg"));
    
    anim.UpdateNodeColor(ueNodes.Get(0), 0, 255, 0);  // Green for UE1
    anim.UpdateNodeColor(ueNodes.Get(1), 255, 0, 0);  // Red for UE2
    anim.UpdateNodeColor(remoteHostContainer.Get(0), 0, 0, 255); // Blue for RemoteHost

        
    anim.UpdateNodeColor(enbNodes.Get(0), 255, 165, 0);  // Orange for eNodeB
    //anim.UpdateNodeImage(enbNodes.Get(0)->GetId(), anim.AddResource("/home/tanvir/Documents/ns-allinone-3.41/netanim-3.109/mobile.png"));

    Simulator::Stop(Seconds(10.0));
    Simulator::Run();

    // Flow monitor statistics output
    monitor->SerializeToXmlFile("lte-04/lte-flowmon.xml", true, true);

    Simulator::Destroy();
    return 0;
}
