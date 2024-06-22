#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h"
#include <iostream>
#include <fstream>
#include <streambuf>
#include <sstream>
#include <vector>
#include <map>
#include <cstdio>
#include <random>


#include <curl/curl.h>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LteSimulation");

// Declare ofstream object globally so it can be accessed from any function
// std::ofstream logFile("extended-lte-04/log_output.txt");


/* data collection - job-2 - authentication and handover events portion-1 start */ 
struct EventLog {
    std::string eventType; // "Authentication" or "Handover"
    std::string sourceAddress;
    std::string destinationAddress;
    double timestamp;
};

// Function to call a URL using curl
void callCurl(const std::string &url) {
    printf("______________\n");
    std::string command = "curl -s \"" + url + "\"";
    std::system(command.c_str());
    printf("\n --------------\n");
}

std::string generateUniqueSuffix() {
    std::time_t now = std::time(nullptr);
   
    // Random number generation
    std::random_device rd; // Obtain a random number from hardware
    std::mt19937 rng(rd()); // Seed the generator
    std::uniform_int_distribution<int> dist(1000, 9999); // Define the range
    int random_number = dist(rng);
    
    return std::to_string(now) + "_" + std::to_string(random_number);
}

// Map to store authentication and handover events
std::map<double, EventLog> eventLogs;

        /*   */
        // Declare ofstream object globally so it can be accessed from any function
        struct PacketCount {
            uint32_t packetsTransmitted;
            uint32_t packetsReceived;
        };

        // Map to store packet counts for each UE node or eNodeB
        std::map<double, PacketCount> packetCounts;
        /* data collection - job-3 - packet count portion-1 end */

        void PacketTransmitted(Ptr< const Packet > packet, const uint16_t rbId, const LteMacSapUser & macSapUser) {
            // Increment packets transmitted count
            double now = Simulator::Now().GetSeconds();
            packetCounts[now].packetsTransmitted++;
        }

        void PacketReceived(Ptr< const Packet > packet, const uint16_t rbId, const LteMacSapProvider & macSapProvider) {
            // Increment packets received count
            double now = Simulator::Now().GetSeconds();
            packetCounts[now].packetsReceived++;
        }
        /*   */

/* data collection - job-2 - authentication and handover events portion-1 end */ 


// Define a trace function to print source and destination addresses
void TracePacket(Ptr<const Packet> packet, const Address& source, const Address& destination)
{
    // Get source and destination addresses
    InetSocketAddress sourceAddr = InetSocketAddress::ConvertFrom(source);
    InetSocketAddress destAddr = InetSocketAddress::ConvertFrom(destination);

    // Print source and destination addresses along with port number
    std::cout << "Packet transmitted from: " << sourceAddr.GetIpv4() << ":" << sourceAddr.GetPort()
              << " to " << destAddr.GetIpv4() << ":" << destAddr.GetPort() << std::endl;
}


int main(int argc, char *argv[]) {

    printf("Hello, World!\n");

    /* 1. LOG START */
    LogComponentEnable("LteSimulation", LOG_LEVEL_INFO);
    // LogComponentEnable("LteEnbRrc", LOG_LEVEL_INFO);
    // LogComponentEnable("LteUeRrc", LOG_LEVEL_INFO);
    // LogComponentEnable("LteUeRrc", LOG_LEVEL_FUNCTION); // Enable logging for TransmitTrace function
    LogComponentEnable("Ipv4", LOG_LEVEL_INFO);

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
    enbNodes.Create(4); 
    
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

    /* 8. install the internet stack */

    // internet.Install(enbNodes);

    internet.Install(ueNodes_1);
    internet.Install(ueNodes_2);
    internet.Install(ueNodes_3);
    internet.Install(ueNodes_4);

    Ipv4InterfaceContainer ueInterfaces_1 = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueLteDevs_1));
    Ipv4InterfaceContainer ueInterfaces_2 = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueLteDevs_2));
    Ipv4InterfaceContainer ueInterfaces_3 = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueLteDevs_3));
    Ipv4InterfaceContainer ueInterfaces_4 = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueLteDevs_4));

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

    // data collection - for fabric network - job-1 - UE and eNodeB config - start
    // Define data structures to store UE and eNodeB configuration
struct UeConfig {
    uint32_t ueId;
    std::string ipAddress;
    uint32_t attachedEnbId;
};
/*
struct EnbConfig {
    uint32_t enbId;
    std::string ipAddress;
};
*/
 /* Store UE and eNodeB configuration */
    std::vector<UeConfig> ueConfigs;
    //std::vector<EnbConfig> enbConfigs;    

for (uint32_t u = 0; u < ueNodes_1.GetN(); ++u) {
    UeConfig ueConfig;
    ueConfig.ueId = u;
    Ipv4Address addr = ueInterfaces_1.GetAddress(u);
    std::ostringstream oss;
    addr.Print(oss);
    ueConfig.ipAddress = oss.str();
    ueConfig.attachedEnbId = 0; // enbLteDevs.Get(0) is the attached eNodeB
    ueConfigs.push_back(ueConfig);
}
for (uint32_t u = 0; u < ueNodes_2.GetN(); ++u) {
    UeConfig ueConfig;
    ueConfig.ueId = u;
    Ipv4Address addr = ueInterfaces_2.GetAddress(u);
    std::ostringstream oss;
    addr.Print(oss);
    ueConfig.ipAddress = oss.str();
    ueConfig.attachedEnbId = 1; // enbLteDevs.Get(1) is the attached eNodeB
    ueConfigs.push_back(ueConfig);
}
for (uint32_t u = 0; u < ueNodes_3.GetN(); ++u) {
    UeConfig ueConfig;
    ueConfig.ueId = u;
    Ipv4Address addr = ueInterfaces_3.GetAddress(u);
    std::ostringstream oss;
    addr.Print(oss);
    ueConfig.ipAddress = oss.str();
    ueConfig.attachedEnbId = 2; // enbLteDevs.Get(1) is the attached eNodeB
    ueConfigs.push_back(ueConfig);
}
for (uint32_t u = 0; u < ueNodes_4.GetN(); ++u) {
    UeConfig ueConfig;
    ueConfig.ueId = u;
    Ipv4Address addr = ueInterfaces_4.GetAddress(u);
    std::ostringstream oss;
    addr.Print(oss);
    ueConfig.ipAddress = oss.str();
    ueConfig.attachedEnbId = 3; // enbLteDevs.Get(1) is the attached eNodeB
    ueConfigs.push_back(ueConfig);
}
/*
for (uint32_t e = 0; e < enbNodes.GetN(); ++e) {
    EnbConfig enbConfig;
    enbConfig.enbId = e;
    Ipv4Address addr = epcHelper->GetEnbIpv4Address(enbLteDevs.Get(e));
    std::ostringstream oss;
    addr.Print(oss);
    enbConfig.ipAddress = oss.str();
    enbConfigs.push_back(enbConfig);
}
*/

// Print UE configurations
printf("UE Configurations:\n");
for (const auto& ueConfig : ueConfigs) {
    printf("UE ID: %u, IP Address: %s, Attached eNodeB ID: %u\n", ueConfig.ueId, ueConfig.ipAddress.c_str(), ueConfig.attachedEnbId);
}

// add UE and enbNode Data to blockchain - start

    // Step 1: Call the initial URL
    std::string initLedgerUrl = "http://localhost:3000/invoke?channelid=mychannel&chaincodeid=basic&function=InitLedger";
    callCurl(initLedgerUrl);

// add UE and enbNode Data to blockchain - end

// write data for job-1 - UE and eNodeB Configuration
    std::ofstream outputFile("UE_eNodeB_Configuration.txt");
    if (!outputFile.is_open()) {
        std::cerr << "Error: Unable to open file for writing\n";
        return 1; // Return an error code if unable to open the file
    }

    // Write UE configurations to the file
    outputFile << "UE Configurations:\n";

    for (const auto& ueConfig : ueConfigs) {

        std::string suffix = generateUniqueSuffix();

        std::string ueIdWithSuffix = std::to_string(ueConfig.ueId) + "_" + suffix;
        std::string enbIdWithSuffix = std::to_string(ueConfig.attachedEnbId) + "_" + suffix;

        outputFile << "UE ID: " << ueConfig.ueId
                   << ", IP Address: " << ueConfig.ipAddress
                   << ", Attached eNodeB ID: " << ueConfig.attachedEnbId << "\n";

        // Step 2: Call the URL for each UE configuration
        std::string createAssetUrl = "http://localhost:3000/invoke?channelid=mychannel&chaincodeid=basic&function=CreateAsset";
        createAssetUrl += "&args=" + ueIdWithSuffix;
        createAssetUrl += "&args=" + ueConfig.ipAddress;
        createAssetUrl += "&args=" + enbIdWithSuffix;
        callCurl(createAssetUrl);

    }

    // Close the file
    outputFile.close();

/*
// Print eNodeB configurations
printf("eNodeB Configurations:\n");
for (const auto& enbConfig : enbConfigs) {
    printf("eNodeB ID: %u, IP Address: %s\n", enbConfig.enbId, enbConfig.ipAddress.c_str());
}
*/
    // data collection - for fabric network - job-1 - UE and eNodeB config - end

    /* 9. enable packet tracing */

// Enable packet tracing for LTE devices
lteHelper->EnableTraces();

// Enable PCAP capture for the point-to-point link between the EPC and the remote host
p2ph.EnablePcapAll("extended-lte-04/pcap_files/sim_2_data", false);


/*  10. // Exponential Traffic Generators - data transimission simulation start */

// Attach UE nodes in ueNodes_1 to the first eNodeB (enbNodes.Get(0))
/*
for (uint32_t u = 0; u < ueNodes_1.GetN(); ++u) {
    lteHelper->Attach(ueLteDevs_1.Get(u), enbLteDevs.Get(0));
}
*/

// Set up parameters for the traffic generator
uint32_t packetSize = 1024; // bytes
double dataRate = 0.5e6; // bps (0.5 Mbps)
double interval = 0.5; // seconds

// Create exponential random variable
Ptr<ExponentialRandomVariable> randomVariable = CreateObject<ExponentialRandomVariable>();
randomVariable->SetAttribute("Mean", DoubleValue(interval));

// Create OnOffHelper for traffic generation
OnOffHelper onOffHelper("ns3::UdpSocketFactory", Address());
onOffHelper.SetAttribute("OnTime", PointerValue(randomVariable));
onOffHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
onOffHelper.SetAttribute("DataRate", DataRateValue(DataRate(dataRate)));
onOffHelper.SetAttribute("PacketSize", UintegerValue(packetSize));

// Get the IP address of the first eNodeB
Ipv4Address enbIpAddress = epcHelper->GetPgwNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();

// Get an available port number dynamically
Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
uint16_t portNumber = rand->GetInteger(1024, 49151); // Choose a port number in the dynamic/private range (1024-49151)

// Set up packet tracing
// Config::Connect("/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/Tx",MakeCallback(&TracePacket));

// Install OnOff application on each UE node in ueNodes_1
for (uint32_t u = 0; u < ueNodes_1.GetN(); ++u) 
{
    Ipv4Address ipv4Addr = Ipv4Address::ConvertFrom(enbIpAddress);
    AddressValue remoteAddress(InetSocketAddress(ipv4Addr, portNumber));    
    onOffHelper.SetAttribute("Remote", remoteAddress);
    ApplicationContainer app = onOffHelper.Install(ueNodes_1.Get(u)); // Install the app on each UE node
    app.Start(Seconds(1.0));
    app.Stop(Seconds(9.0)); // Stop after 9 seconds
}

// Repeat the same process for other UE nodes and their corresponding eNodeBs
// For ueNodes_2, ueNodes_3, and ueNodes_4

/* 10. // Exponential Traffic Generators - data tranmission simulation end */

/* 11. Update the NetAnim animator */

    AnimationInterface anim("extended-lte-04/sim_2_data.xml");
    anim.EnablePacketMetadata(true);
    anim.SetMobilityPollInterval(Seconds(0.5));

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

    /* 12. Flow monitor statistics output */
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();


            /*   */
            // Install packet tracing for each LTE device
            Ptr<Node> enbNode = enbNodes.Get(0); // getting only the first enbNode
            for (uint32_t i = 0; i < enbNode->GetNDevices(); ++i) {
                Ptr<NetDevice> dev = enbNode->GetDevice(i);
                if (dev->GetInstanceTypeId() == LteEnbNetDevice::GetTypeId()) {
                    Ptr<LteEnbNetDevice> enbDev = dev->GetObject<LteEnbNetDevice>();
                    enbDev->GetPhy()->TraceConnectWithoutContext("PhyTxDrop", MakeCallback(&PacketTransmitted));
                }
            }
            /*   */

    /* 13. Stop the simulation */
    Simulator::Stop(Seconds(10.0));
    // Run the simulation
    Simulator::Run();

            /*   */
            // Output packet counts to file
            std::string filename = "Packet_Count_Output.txt";
            std::ofstream outFile(filename.c_str());
            if (outFile.is_open()) {
                for (const auto &entry : packetCounts) {
                    outFile << "Time: " << entry.first << "s - Packets Transmitted: " << entry.second.packetsTransmitted
                            << ", Packets Received: " << entry.second.packetsReceived << std::endl;
                }
                outFile.close();
                std::cout << "Packet counts written to file: " << filename << std::endl;
            } else {
                std::cerr << "Unable to open file: " << filename << std::endl;
            }

            /*   */

    monitor->SerializeToXmlFile("extended-lte-04/sim_2_data-flowmon.xml", true, true);

    /* 14. Destroy the simulator */
    Simulator::Destroy();


    return 0;
}