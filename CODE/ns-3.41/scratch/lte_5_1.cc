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

int main() {
  // Create 4 eNodeBs (towers)
  NodeContainer enbNodes;
  enbNodes.Create(4);

  // Position eNodeBs evenly
  for (uint32_t i = 0; i < 4; i++) {
    Ptr<Node> enbNode = enbNodes.Get(i);
    enbNode->AddDevice(LteEnbNetDevice::Create());
    enbNode->GetObject<LteEnbNetDevice>()->SetPosition(Vector(i * 100, 0, 0)); // evenly spaced
  }

  // Create 5 UE nodes (user devices) around each eNodeB
  NodeContainer ueNodes;
  ueNodes.Create(20); // 5 UEs per eNodeB

  // Position UEs around their respective eNodeBs
  for (uint32_t i = 0; i < 20; i++) {
    Ptr<Node> ueNode = ueNodes.Get(i);
    ueNode->AddDevice(LteUeNetDevice::Create());
    uint32_t enbIndex = i / 5; // which eNodeB is this UE associated with?
    Ptr<Node> enbNode = enbNodes.Get(enbIndex);
    ueNode->GetObject<LteUeNetDevice>()->SetPosition(Vector(enbNode->GetObject<LteEnbNetDevice>()->GetPosition().x + (i % 5) * 20, 0, 0));
  }

  // Install LTE devices on nodes
  LteHelper lteHelper;
  lteHelper.InstallEnbDevice(enbNodes);
  lteHelper.InstallUeDevice(ueNodes);

  // Create internet connectivity
  InternetStackHelper internet;
  internet.Install(enbNodes);
  internet.Install(ueNodes);

  // Run the simulation
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}