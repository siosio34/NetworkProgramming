#pragma once
#include "Configure.h"
#include "Winpcap_Packet_System.h"
#include "Windivert_Packet_System.h"

class Application
{
private:
	Winpcap_Packet_System pcap_obj;
	Windivert_Packet_System divert_ob;

public:
	void PrintRun();
	void ProcessInput(int num);
	void RunPacketCapture();
	void RunArpSpoofing();
	void RunSendRst();
	void RunUDPDelay();
	void RunPacketContentChange();
	

	

};