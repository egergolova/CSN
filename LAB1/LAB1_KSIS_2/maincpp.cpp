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
//#include "pull_thr.h"
#include "processthreadsapi.h"
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "IPHLPAPI.lib")
#define THR_AM 7

std::atomic_bool done;
std::queue<int> work_queue;
std::vector<std::thread> threads;
std::atomic_int last;
std::mutex g_lock;
const char name[] = "dbM.txt";
FILE* vendors;

void ARP_require() {
	while (!done) {
		while (work_queue.empty() && !done) {
			std::this_thread::yield();
		}
		if (done) { return; }
			g_lock.lock();
			std::atomic_int IP_Goer = work_queue.front();
			work_queue.pop();
			g_lock.unlock();
		
		//ULONG  pMacAddr[2];
		unsigned char pMacAddr[6];
		ULONG PhysAddrLen = 6;//PULONG phyAddrlen = (PULONG)malloc(sizeof(ULONG));
		DWORD err = SendARP((IPAddr)IP_Goer, 0, &pMacAddr, &PhysAddrLen);
		char addr[16];
		inet_ntop(AF_INET, (in_addr*)(&IP_Goer), addr, 16);
		if (err != NO_ERROR) {
			g_lock.lock();
			std::cout << "didn't access  " << addr << "\n";
			g_lock.unlock();
		}
		else {
			g_lock.lock();
			fopen_s(&vendors, name, "r");
			std::cout << "the mac addr is ";
			for (int i = 0; i < 6; i++) { std::cout<<std::hex << (unsigned)pMacAddr[i]<<" "; } std::cout << "  ";
			
			std::cout << addr << "\n";
			g_lock.unlock();
		}
		
		g_lock.lock();
		last++;
		g_lock.unlock();

	}
	//std::terminate();
}
int main_id;

int pool_thread(in_addr IP_Goer, in_addr IP_Mask) {
	int amThr = 0;
	int numThr = 0;
	//pool_thr pool;
	last = 0;
	//std::vector<std::thread> threads;
	//in_addr Mask;
	IP_Mask.S_un.S_addr&= 0x00FFFFFF;
	
	IP_Goer.S_un.S_addr &= IP_Mask.S_un.S_addr;// 0x00FFFFFF;
	IP_Goer.S_un.S_un_b.s_b4 += 2;
	done = 0;
	for (unsigned i = 0; i < std::thread::hardware_concurrency(); ++i) {
		//pool.threads[i];
		//if (GetCurrentThreadId() != main_id) {
		//	pool.ARP_require(&pool);
	//	}
		//std::thread vect(do_ntg);
		//	std::thread thr(ARP_require);
			threads.push_back(std::thread(ARP_require));
	}
	int a =  (IP_Mask.S_un.S_un_b.s_b4^0xFF+ ((IP_Mask.S_un.S_un_b.s_b3^0xFF)<<8));
	for (int i = 0; i < a-2; i++) {
		work_queue.push(*(int*)(&IP_Goer));
		IP_Goer.S_un.S_un_b.s_b4++;
		if (!IP_Goer.S_un.S_un_b.s_b4) {
			IP_Goer.S_un.S_un_b.s_b3++;
			IP_Goer.S_un.S_un_b.s_b4+=2;
		}

	}
	while (!work_queue.empty()) {
	}
	done = 1;
	for (int i = 0; i < std::thread::hardware_concurrency(); ++i) {
		threads[i].join();//TerminateThread(threads[i].native_handle());
	}
	//pool.~pool_thr();
	return 0;

}

int main() {
	fopen_s(&vendors, name, "r");

	//std::thread thr(func);
	//thr.join();
	WSADATA wsDATA;
	int err_st = WSAStartup(0x202, &wsDATA);
	if (err_st) {
		std::cout << "failed sockets access. invalid version";
		return 1;
	}
	SOCKET soc = socket(AF_INET, SOCK_STREAM, 0);
	if (soc == INVALID_SOCKET) {
		WSACleanup();
		std::cout << "Couldn't get socket";
		return 2;
	}
	in_addr IP_Mask;
	in_addr IP_Goer;
	FIXED_INFO IPdata;

	unsigned long size = sizeof(FIXED_INFO);
	int i = GetNetworkParams(&IPdata, &size);
	std::vector<PCSTR> IP_addr;
	std::vector<PCSTR> IP_mask;
	while (IPdata.DnsServerList.Next != NULL) {
		IP_addr.push_back(IPdata.DnsServerList.IpAddress.String);
		IP_mask.push_back(IPdata.DnsServerList.IpMask.String);
		IPdata.DnsServerList = *IPdata.DnsServerList.Next;
			//PCSTR IP_str = IPdata.DnsServerList.IpAddress.String;
	}
	IP_addr.push_back(IPdata.DnsServerList.IpAddress.String);
	IP_mask.push_back(IPdata.DnsServerList.IpMask.String);
	for (int i = 0; i < IP_addr.size(); i++) {
		std::cout << IP_addr[i]<<"\n";
	}
	std::cout << "which local network do you want to be pooled\n";
	int num;
	std::cin >> num;
	//PCSTR IP_str = IPdata.DnsServerList.Next->IpAddress.String;
	err_st = inet_pton(AF_INET, IP_mask[num], &IP_Mask);
	if (err_st <= 0) {
		std::cout << "Error in IP translation to special numeric format\n";
		return 3;
	}
	err_st = inet_pton(AF_INET, IP_addr[num], &IP_Goer);
	if (err_st <= 0) {
		std::cout << "Error in IP translation to special numeric format\n";
		return 3;
	}
	//IP.S_un.S_addr &= 0xFFFFFE00;
	//IP.S_un.S_addr += 1;
	sockaddr_in info;
	//info = (sockaddr_in*)malloc(sizeof(sockaddr_in));
	info.sin_family = AF_INET;
	info.sin_addr = IP_Goer;
	info.sin_port = htons(1234);
	//err_st=bind(soc,(sockaddr*)&info,sizeof(info));
	if (!err_st) {
		std::cout << "couldn't bind \n" << WSAGetLastError();
		closesocket(soc);
		WSACleanup();
		return 4;
	}
	main_id = GetCurrentThreadId();
	pool_thread(IP_Goer, IP_Mask);
	//BSTR* pbszMacAddress;
//HRESULT ADDR= IWdsTransportClient::get_MacAddress(pbszMacAddress);
	WSACleanup();
	closesocket(soc);
}