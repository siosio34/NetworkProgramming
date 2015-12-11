#include "Windivert_Packet_System.h"

void Windivert_Packet_System::Windivert_Open_UDP()
{
	INT16 priority = 0;
	cout << " 시간 입력 : ";
	cin >> input_time;
	//and udp.DstPort == 12345 for Debug Port
	//xx
	handle = WinDivertOpen("udp and outbound", WINDIVERT_LAYER_NETWORK, priority, 0); // 패킷 오픈. 모든 패킷을 다받아와야함.
	if (handle == INVALID_HANDLE_VALUE) // HANDLE 값이 오류일때
	{
		if (GetLastError() == ERROR_INVALID_PARAMETER)
		{
			fprintf(stderr, "error: filter syntax error\n");
			exit(EXIT_FAILURE);
		}
		fprintf(stderr, "error: failed to open the WinDivert device (%d)\n",
			GetLastError());
		exit(EXIT_FAILURE);
	}

	if (!WinDivertSetParam(handle, WINDIVERT_PARAM_QUEUE_LEN, 8192))
	{
		fprintf(stderr, "error: failed to set packet queue length (%d)\n",
			GetLastError());
		exit(EXIT_FAILURE);
	}

}

void Windivert_Packet_System::ReceiveUdp_Packet()
{
	
	unsigned char packet[MAXBUF];
	WINDIVERT_ADDRESS recv_addr;
	UINT packet_len;


	while (TRUE)
	{
		// Read a matching packet.
		if (!WinDivertRecv(handle, packet, sizeof(packet), &recv_addr, // 패킷들을 받아요, 전부 다 받아요
			&packet_len))
		{
			fprintf(stderr, "warning: failed to read packet\n");
			continue;
		}

		// Print info about the matching packet.
		WinDivertHelperParsePacket(packet, packet_len, NULL, // 수정하기 쉽게 패킷들을 파싱한다.
			NULL, NULL, NULL, NULL,
			NULL, NULL, NULL);

		SENDSAVEPACKET UDP_temp; //

		//cout << " 끅 " << endl;
		//for (int i = 0; i < packet_len; i++)
		//	printf("%X ", packet[i]);

		//cout << endl << endl;

		memcpy(&(UDP_temp.buff), &packet, packet_len); // UDP 패킷 복사
		memcpy(&(UDP_temp.send_addr), &recv_addr, sizeof(WINDIVERT_ADDRESS)); //주소 복사
		UDP_temp.len = packet_len;
		UDP_temp.receive_packet_time = chrono::system_clock::now(); // 시간받기

		mtx_lock.lock();
		Saved_UDP_Packet.push_back(UDP_temp); // 패킷 저장
		mtx_lock.unlock();

	}

}

void Windivert_Packet_System::SendUdp_Packet()
{
	while (TRUE)
	{
		if (Saved_UDP_Packet.size() == 0)
			continue;

		else
		{
			//chrono::system_clock::time_point Endtime = chrono::system_clock::now();
			//time_t t = chrono::system_clock::to_time_t(Endtime);
			//time_t l = chrono::system_clock::to_time_t(Saved_UDP_Packet[0].receive_packet_time);

			std::chrono::duration<double> sec = std::chrono::system_clock::now() - Saved_UDP_Packet[0].receive_packet_time;
			
			if (sec.count() < input_time)
				continue;

			else
			{
				SENDSAVEPACKET now = Saved_UDP_Packet[0];
				u_char *tt = (u_char*)now.buff;

				//for (int i = 0; i < now.len; i++)
				//	printf("%X ", tt[i]);

				if (!WinDivertSend(handle, (PVOID)&now.buff, (now.len), &(now.send_addr),
					NULL)) // send_addr 걍 방향만 설정해서 내보내주면 된다. 따라서 필요한건 dnr 배열을 만들어서 저장해야된다.
				{
					fprintf(stderr, "warning: failed to send UDP message "
						"(%d)\n", GetLastError());
				}
				//else
				//{
				//	for (int i = 0; i < now.len; i++)
				//		printf("%X ", tt[i]);
				//
				//}

				mtx_lock.lock();
				Saved_UDP_Packet.erase(Saved_UDP_Packet.begin());
				mtx_lock.unlock();

			}

		}
	}

}

void Windivert_Packet_System::Run_UDP_Delay()
{
	Windivert_Open_UDP();

	std::thread* ReceiveThread;
	std::thread* SendThread;

	ReceiveThread = new std::thread(&Windivert_Packet_System::ReceiveUdp_Packet, this);
	SendThread = new std::thread(&Windivert_Packet_System::SendUdp_Packet, this);

	ReceiveThread->join();
	SendThread->join();
}

