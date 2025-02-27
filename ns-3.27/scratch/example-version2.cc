/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

//
// Program description: Create a UDP or TCP packet flow across a
// point-to-point link.  Allow user to select the transport protocol.
//
//  node 0 ----------------------- node 1
//  10.1.1.1                       10.1.1.2
//         <----- direction of data flow 
//
 

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/netanim-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Wns3Example");

void PacketSinkTraceSink (Ptr<const Packet> packet, const Address &from)
{
  std::cout << "Packet received; UID = " << packet->GetUid () << std::endl;
}

int main (int argc, char *argv[])
{
  // Defaults and command-line arguments
  std::string protocol = "ns3::UdpSocketFactory";  
  uint16_t port = 5009;

  CommandLine cmd;
  cmd.AddValue ("transport", "TypeId for socket factory", protocol);
  cmd.Parse (argc,argv);

  // Nodes
  NodeContainer nodes;
  nodes.Create (2);

  // Devices
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);

  // Internet stack
  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces;
  interfaces = address.Assign (devices);

  // Applications
  InetSocketAddress destinationAddress = InetSocketAddress (Ipv4Address ("10.1.1.1"), port);
  InetSocketAddress sinkAddress = InetSocketAddress (Ipv4Address::GetAny (), port); 

  OnOffHelper onOff (protocol, destinationAddress);
  onOff.SetConstantRate (DataRate ("1Mb/s"));
  PacketSinkHelper sink (protocol, sinkAddress);
  
  ApplicationContainer serverApps = onOff.Install (nodes.Get (1));
  ApplicationContainer clientApps = sink.Install (nodes.Get (0));
  
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (11.0));

  clientApps.Start (Seconds (1.0));
  clientApps.Stop (Seconds (12.0));

  // Add some visualization and output data capabilities
  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator", 
                                 "DeltaX", DoubleValue (100));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);

  Config::ConnectWithoutContext ("/NodeList/0/ApplicationList/0/$ns3::PacketSink/Rx", MakeCallback (&PacketSinkTraceSink));

  // Output config store to txt format
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("example-2-attributes.txt"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("RawText"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  ConfigStore outputConfig;
  outputConfig.ConfigureDefaults ();
  outputConfig.ConfigureAttributes ();

  Simulator::Stop (Seconds (13.0));

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}

