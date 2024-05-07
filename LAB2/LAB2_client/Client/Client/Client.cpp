#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
//#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <iptypes.h>
#include <winsock.h>
#include <Assert.h>
#include <iphlpapi.h>
#include <stdlib.h>
#include "processthreadsapi.h"
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "IPHLPAPI.lib")
#define THR_AM 7
std::atomic_bool done;
std::queue<int> work_queue;
std::vector<std::thread> threads;
std::atomic_int last;
std::mutex g_lock;
std::string username;
std::vector <char> clientBuff(100);





void message_receiver(SOCKET Server){
	while (1) {
		g_lock.lock();
		//	char y = 'y';
		int packet_size = recv(Server, clientBuff.data(), clientBuff.size(), 0);
		if (packet_size != -1) {
			if (clientBuff.data() == "") {
				//send(Server, &y, 1, 0);
			}
			else {
				std::cout <<"\n"<< clientBuff.data()<<"\n";
				g_lock.unlock();
				if (clientBuff[0] == '-') {
					shutdown(Server, SD_BOTH);
					closesocket(Server);
				}
			}
		}
	}

}
int message_sender(SOCKET Client) {
	while (1) {
		//g_lock.lock();
		//std::cout << "give me the frase: ";
		std::string buffinp;
		std::string receiver;
		//std::cin>> buffinp;
		while (!buffinp.size()) {
			std::getline(std::cin, buffinp);
		}
		std::cout << "to whom do you want it to send ";
		std::getline(std::cin, receiver);
		std::string data =username+": "+ buffinp+" :" + receiver;
		for (int i = 0; i < data.size(); i++) {
			clientBuff[i] = data[i];
		}
		 // +clientBuff.data();
		int packet_size = send(Client, clientBuff.data(), clientBuff.size(), 0);
		std::cout << "\n";
		//g_lock.unlock();
		if (clientBuff[0] == '-') {
			shutdown(Client, SD_BOTH);
			closesocket(Client);
		}
	}
}





int main() {

	//std::thread thr(func);
	//thr.join();
	std::string IP;
	int port;
	std::cout << "give me the IP of server\n";
	std::cin >> IP;
	std::cout << "give me the port number\n";
	std::cin>>port;
	WSADATA wsDATA;
	int err_st = WSAStartup(0x202, &wsDATA);
	if (err_st) {
		std::cout << "failed sockets access. invalid version";
		return 1;
	}
	SOCKET soc = socket(AF_INET6, SOCK_STREAM, 0);
	if (soc == INVALID_SOCKET) {
		WSACleanup();
		std::cout << "Couldn't get socket";
		return 2;
	}
	in_addr IP_Mask;
	in_addr6 IP_Goer;
	ZeroMemory(&IP_Goer, sizeof(IP_Goer));
	//FIXED_INFO IPdata;

	//unsigned long size = sizeof(FIXED_INFO);
	//int i = GetNetworkParams(&IPdata, &size);
	//PCSTR IP_addr;
	//PCSTR IP_mask;

	//	IP_addr=IPdata.DnsServerList.IpAddress.String;
	//	IP_mask=IPdata.DnsServerList.IpMask.String;
	//	IPdata.DnsServerList = *IPdata.DnsServerList.Next;
		//PCSTR IP_str = IPdata.DnsServerList.IpAddress.String;	
		//std::cout << IP_addr << "\n";
	PSTR ip = const_cast<char*>(IP.c_str());
	//"127.0.0.1";
	err_st = inet_pton(AF_INET6, ip, &IP_Goer);
	if (err_st <= 0) {
		std::cout << "Error in IP translation to special numeric format\n";
		return 3;
	}
	sockaddr_in6 info;
	ZeroMemory(&info, sizeof(info));
	info.sin6_family = AF_INET6;
	info.sin6_addr = IP_Goer;
	info.sin6_port = htons(port);
	err_st = connect(soc, (sockaddr*)&info, sizeof(info));
	if (err_st) {
		std::cout << "Couldn't access server\n";
		return 4;
	}
	//__try {
	std::cout << "enter your username\n";
	std::cin >> username;
		std::thread a=std::thread(message_receiver,soc);
		message_sender(soc);
	//}
	//threads.push_back(std::th()read(server_beh, soc, max));

//	threads.push_back(std::thread(client_beh, soc, (sockaddr*)&info, sizeof(info)));
	//main_id = GetCurrentThreadId();
	//pool_thread(IP_Goer, IP_Mask);
	//BSTR* pbszMacAddress;
//HRESULT ADDR= IWdsTransportClient::get_MacAddress(pbszMacAddress);
//	__finally {
	closesocket(soc);
	WSACleanup();

	//}
}