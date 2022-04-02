// An NS3 code to simulate 3-sector LTE base stations
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/config-store.h"
#include <ns3/buildings-propagation-loss-model.h>
#include <ns3/buildings-helper.h>
#include <ns3/radio-environment-map-helper.h>
#include <vector>

using namespace ns3;
using std::vector;

// # Function to create nodes (eNB) with multiple sectors
vector<NodeContainer> createMultiSectorNodes(int numberOfNodes, int numberOfSectors){
  std::cout << "Creating multi-sector node";
  vector < NodeContainer > l;
  for(int i=0; i<numberOfNodes; i++){
    NodeContainer _3SecNode;
    _3SecNode.Create(3);
    l.push_back(_3SecNode);
  }
  return l;
}

int
main (int argc, char *argv[])
{
int NUMBER_OF_ENBS = 4;
int NUMBER_OF_UES = 1;
double ENB_TRANSMIT_POWER_DBM = 50;
double ANTENNA_1_ORIENTATION = 0;
double ANTENNA_2_ORIENTATION = 120;
double ANTENNA_3_ORIENTATION = 240;
double DISTANCE_BETWEEN_ENBS = 200;
int SIM_STOP_TIME = 10;

Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
epcHelper = ns.lte.PointToPointEpcHelper();
lteHelper->SetEpcHelper(epcHelper);
lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::FriisSpectrumPropagationLossModel"));
lteHelper->Initialize();
lteHelper->SetSchedulerType("ns3::RrFfMacScheduler");
lteHelper->SetHandoverAlgorithmType("ns3::A2A4RsrqHandoverAlgorithm");
lteHelper->SetHandoverAlgorithmAttribute("ServingCellThreshold", ns.core.UintegerValue(30));
lteHelper->SetHandoverAlgorithmAttribute("NeighbourCellOffset", ns.core.UintegerValue(1));

pgw = epcHelper->GetPgwNode();

// Create a single RemoteHost
std::cout << "Creating remote host";
remoteHostContainer = ns.network.NodeContainer();
remoteHostContainer.Create(1);
remoteHost = remoteHostContainer.Get(0);
internet = ns.internet.InternetStackHelper();
internet.Install(remoteHostContainer);

// Create the Internet
p2ph = ns.point_to_point.PointToPointHelper();
p2ph.SetDeviceAttribute("DataRate", ns.core.StringValue("100Gb/s"));
p2ph.SetDeviceAttribute("Mtu", ns.core.UintegerValue(1500));
p2ph.SetChannelAttribute("Delay", ns.core.TimeValue(ns.core.Seconds(0.010)));
internetDevices = p2ph.Install(pgw, remoteHost);
ipv4h = ns.internet.Ipv4AddressHelper();
ipv4h.SetBase(ns.network.Ipv4Address("1.0.0.0"), ns.network.Ipv4Mask("255.0.0.0"));
internetIpIfaces = ipv4h.Assign(internetDevices);
remoteHostAddr = internetIpIfaces.GetAddress(1);

// Routing of the Internet Host (towards the LTE network)
ipv4RoutingHelper = ns.internet.Ipv4StaticRoutingHelper();
remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting(ns.internet.Ipv4);

// interface 0 is localhost, 1 is the p2p device
remoteHostStaticRouting->AddNetworkRouteTo(ns.network.Ipv4Address("7.0.0.0"), ns.network.Ipv4Mask("255.0.0.0"), 1);

// Creating Base Stations' (eNBs) with three sectors
NodeContainer ueNodes;
vector < NodeContainer > enbNodes;
enbNodes =  createMultiSectorNodes(NUMBER_OF_ENBS, 3);
ueNodes.Create(NUMBER_OF_UES);

// # Assigning location to each eNB node
MobilityHelper mobility;
mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
for(int  i=0; i<NUMBER_OF_ENBS; i++){
  Ptr < ListPositionAllocator > enbPositionAlloc = CreateObject<ListPositionAllocator> ();
    if( i < 2){
        enbPositionAlloc->Add(Vector(0, DISTANCE_BETWEEN_ENBS*i, 0));
    }
    else{
        enbPositionAlloc->Add(Vector(200, DISTANCE_BETWEEN_ENBS*i, 0));
    }
  mobility.Install(enbNodes.at(i));
}

mobility.SetPositionAllocator(enbPositionAlloc);
mobility.Install(enbNodes);
MobilityHelper mobilityForUe;
mobilityForUe.SetMobilityModel("ns3::ConstantPositionMobilityModel");
mobilityForUe.Install(ueNodes);

// # Installing LTE Protocol Stack on the eNBs
Config::SetDefault("ns3::LteEnbPhy::TxPower", DoubleValue(ENB_TRANSMIT_POWER_DBM));
NetDeviceContainer ueDevs;
vector<NetDeviceContainer> enbDevs;
ueDevs = lteHelper->InstallUeDevice(ueNodes);

// # Installing Antenna Configutation to each eNB
for(int i=0; i<NUMBER_OF_ENBS; i++){
  NetDeviceContainer enbDev;
  lteHelper->SetEnbAntennaModelType("ns3::CosineAntennaModel");
  lteHelper->SetEnbAntennaModelAttribute("Orientation", DoubleValue (ANTENNA_1_ORIENTATION));
  lteHelper->SetEnbAntennaModelAttribute("HorizontalBeamwidth", DoubleValue (60));
  lteHelper->SetEnbAntennaModelAttribute("VerticalBeamwidth", DoubleValue (60));
  lteHelper->SetEnbAntennaModelAttribute("MaxGain", DoubleValue (0.0));
  // # enbDevs.Add(lteHelper.InstallEnbDevice(enbNodes.Get(enbs).Get(0)))
  enbDev.Add(lteHelper->InstallEnbDevice(enbNodes.at(i).Get(0)));

  lteHelper->SetEnbAntennaModelType("ns3::CosineAntennaModel");
  lteHelper->SetEnbAntennaModelAttribute("Orientation", DoubleValue (ANTENNA_2_ORIENTATION));
  lteHelper->SetEnbAntennaModelAttribute("HorizontalBeamwidth", DoubleValue (60));
  lteHelper->SetEnbAntennaModelAttribute("VerticalBeamwidth", DoubleValue (60));
  lteHelper->SetEnbAntennaModelAttribute("MaxGain", DoubleValue (0.0));
  // # enbDevs.Add(lteHelper.InstallEnbDevice(enbNodes.Get(enbs).Get(1)))
  enbDev.Add(lteHelper->InstallEnbDevice(enbNodes.at(i).Get(1)));

  lteHelper->SetEnbAntennaModelType("ns3::CosineAntennaModel");
  lteHelper->SetEnbAntennaModelAttribute("Orientation", DoubleValue (ANTENNA_3_ORIENTATION));
  lteHelper->SetEnbAntennaModelAttribute("HorizontalBeamwidth", DoubleValue (60));
  lteHelper->SetEnbAntennaModelAttribute("VerticalBeamwidth", DoubleValue (60));
  lteHelper->SetEnbAntennaModelAttribute("MaxGain", DoubleValue (0.0));
  // # enbDevs.Add(lteHelper.InstallEnbDevice(enbNodes.Get(enbs).Get(2)))
  enbDev.Add(lteHelper->InstallEnbDevice(enbNodes.at(i).Get(2)));
  enbDevs.push_back(enbDev);
}
std::cout << "UE and ENBs initialized";

// Install the IP stack on the UEs
internet.Install(ueNodes);
ueIpIfaces = ns.internet.Ipv4InterfaceContainer();
ueIpIfaces = epcHelper->AssignUeIpv4Address(ns.network.NetDeviceContainer(ueDevs));
// std::cout << enbDevs.Get(0);

// Attaching UEs to an eNB
lteHelper->Attach(ueDevs, enbDevs.at(0).Get(1));

// Attaching bearer
enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
EpsBearer bearer (q);
lteHelper->ActivateDataRadioBearer(ueDevs, bearer);

// Adding X2 interface for communication among eNBs
lteHelper->AddX2Interface(enbNodes);

Simulator::Stop(Seconds(SIM_STOP_TIME));

// Getting trace files
std::cout << "Generating Trace Files";
lteHelper->EnablePhyTraces();
lteHelper->EnableMacTraces();
lteHelper->EnableRlcTraces();

// Adding UE position
Vector distance_temp (1000,1000,1000);
Vector userDistance (1000,1000,1000);
userDistance->assign(distance_temp, distance_temp + 3);
 
for(int i =0; i < 4; i++){
    mm = ns.mobility.ConstantPositionMobilityModel();
    mm = ueNode->Get(i);
    mm.SetPosition(ns.core.Vector(userDistance[i], 0.0, 0.0));
}

ns.core.Config.Connect("/NodeList/*/DeviceList/*/ComponentCarrierMapUe/*/LteUePhy/ReportCurrentCellRsrpSinr",
                        MakeCallBack(id(ReportCurrentCellRsrpSinrEnb)));
ns.core.Config.Connect("/NodeList/*/DeviceList/*/LteEnbRrc/ConnectionEstablished",
                   MakeCallback(id(NotifyConnectionEstablishedEnb)));

// Generating REM
std::cout << "Generating REM plots";
Ptr<RadioEnvironmentMapHelper> remHelper = CreateObject<RadioEnvironmentMapHelper> ();
// remHelper->SetAttribute("Channel", PointerValue(lteHelper.GetDownlinkSpectrumChannel()));
remHelper->SetAttribute ("ChannelPath", StringValue ("/ChannelList/0"));
remHelper->SetAttribute("OutputFile", StringValue("rem.out"));
remHelper->SetAttribute("XMin", DoubleValue(-400));
remHelper->SetAttribute("XMax", DoubleValue(400.0));
remHelper->SetAttribute("XRes", UintegerValue(100));
remHelper->SetAttribute("YMin", DoubleValue(-400));
remHelper->SetAttribute("YMax", DoubleValue(400.0));
remHelper->SetAttribute("YRes", UintegerValue(75));
remHelper->SetAttribute("Z", DoubleValue(0.0));
remHelper->SetAttribute("UseDataChannel", BooleanValue(true));
remHelper->SetAttribute("RbId", IntegerValue(-1));
remHelper->Install();

// Running the simulator
Simulator::Run();
Simulator::Destroy();
return 0;
}
