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
std::vector<SOCKET> Clients;
std::string Usernames[5];
std::vector<char> isHere;





void check_active(int num) {


	char y;
	send(Clients[num], NULL, 0, 0);
	recv(Clients[num], &y, 1, 0);
	if (y != 'y') {
		g_lock.lock();
		isHere[num] = false;
		g_lock.unlock();
	}

}

SOCKET socket_of_receiver(std::string receiver) {
	int i = 0;
	while(i < 5 && receiver.compare(Usernames[i])) {
		i++;
		
	}
	if (i == 5) {
		return INVALID_SOCKET;
	}
	else {
		return Clients[i];
	}
}

void message_receiver(SOCKET Client, int num) {
	std::vector <char> servBuff(100), clientBuff(100);
	while (isHere[num]) {
		for (int i = 0; i < servBuff.size(); i++) {
			servBuff[i] = 0;
		}
		if (!isHere[num]) { return; }
		int packet_size = recv(Client, servBuff.data(), servBuff.size(), 0);
		if (packet_size!=-1) {
			std::cout << servBuff.data()<<". ";
			int i = 0;
			std::string name;

			while (servBuff[i] != ':') {
				name += servBuff[i];
				i++;
			}
			if (!strcmp((char*)&(Usernames[num]), (char*)&name) || !Usernames[num].size()) {
				for (int i = 0; i < name.size(); i++) {
					Usernames[num] += name[i];
				}
			}
			if (servBuff[0] == '-') {
				shutdown(Client, SD_BOTH);
				closesocket(Client);
			}
			std::string receiver;
			int len = 0;
			while (servBuff[servBuff.size() - len - 1] != ':') { len++; }
			int j = 0;
			servBuff[servBuff.size() - len - 1] = 0;
			while(j < len && servBuff[servBuff.size() - len +j] != 0) {
				receiver += servBuff[servBuff.size() - len + j];
				servBuff[servBuff.size() - len + j] = 0;
				j++;
			}
			SOCKET rec = socket_of_receiver(receiver);
			int done=send(rec, servBuff.data(), servBuff.size(),0);
			if (done == -1) {
				g_lock.lock();
				std::cout << "couldn't send to some user\n";
				g_lock.unlock();
			}

		}
	//	check_active(num);
		else { isHere[num] = false; }
	}

}

//void message_receiver(SOCKET Client) {

//}
std::string NodeName = "ABC";
std::string PortName = "4234";
void v() {
	SOCKET ConnSocket;
	DWORD ipv6only = 0;
	int iResult;
	BOOL bSuccess;
	SOCKADDR_STORAGE LocalAddr = { 0 };
	SOCKADDR_STORAGE RemoteAddr = { 0 };
	DWORD dwLocalAddr = sizeof(LocalAddr);
	DWORD dwRemoteAddr = sizeof(RemoteAddr);

	ConnSocket = socket(AF_INET6, SOCK_STREAM, 0);
	if (ConnSocket == INVALID_SOCKET) {
		//return INVALID_SOCKET;
	}

	iResult = setsockopt(ConnSocket, IPPROTO_IPV6,
		IPV6_V6ONLY, (char*)&ipv6only, sizeof(ipv6only));
	if (iResult == SOCKET_ERROR) {
		closesocket(ConnSocket);
		//	return INVALID_SOCKET;
	}

	bSuccess = WSAConnectByName(ConnSocket, (LPWSTR)&NodeName,
		(LPWSTR)&PortName, &dwLocalAddr,
		(SOCKADDR*)&LocalAddr,
		&dwRemoteAddr,
		(SOCKADDR*)&RemoteAddr,
		NULL,
		NULL);
	if (bSuccess) {
	}

}

int main() {
	std::string IP="fe80:0000:0000:0000:9526:0b04:3f2a:486e";
	int port=4234;
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
	PSTR ip = const_cast<char*>(IP.c_str());
	//"127.0.0.1";
	err_st = inet_pton(AF_INET6, ip, &IP_Goer);
	if (err_st <= 0) {
		std::cout << "Error in IP translation to special numeric format\n";
		return 3;
	}
	struct sockaddr_in6 info;
	ZeroMemory(&info, sizeof(info));
	info.sin6_family = AF_INET6;
	info.sin6_addr = IP_Goer;
	info.sin6_port = htons(port);
	int num = 0;
	err_st = bind(soc,(sockaddr*)(&info),sizeof(info));
	if (err_st) {
		std::cout << "Couldn,t bind\n"<< WSAGetLastError();
		closesocket(soc);
		WSACleanup();
		return 4;
	}
	while (1) {
			int err_st = listen(soc, 4);
			if (err_st) {
				std::cout << "Listening failed\n";
		//		return 5;
			}
			sockaddr_in6 clientInf;
			ZeroMemory(&clientInf, sizeof(clientInf));
			 int len = sizeof(clientInf);
			SOCKET Client= accept(soc,(sockaddr*) & clientInf, &len);
			if (Client==INVALID_SOCKET) {
				std::cout << "Couldn't get client's socket";
			//	return 6;
			}
			else{ Clients.push_back(Client);
			threads.push_back(std::thread(message_receiver, Client, num));
			isHere.push_back(true);
			num++;
			}
     }
	
	//threads.push_back(std::th()read(server_beh, soc, max));
	
//	threads.push_back(std::thread(client_beh, soc, (sockaddr*)&info, sizeof(info)));
	//main_id = GetCurrentThreadId();
	//pool_thread(IP_Goer, IP_Mask);
	//BSTR* pbszMacAddress;
//HRESULT ADDR= IWdsTransportClient::get_MacAddress(pbszMacAddress);
//	__finally {
		closesocket(soc);
		WSACleanup();
}

	  
	//  }
