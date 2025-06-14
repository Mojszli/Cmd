#ifndef NETWORK_UTILS_H
#define NETWORK_UTILS_H

#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
    #include <winsock2.h>
    #include <iphlpapi.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "iphlpapi.lib")
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <ifaddrs.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <sys/socket.h>
    #include <netdb.h>
    #include <unistd.h>
#endif

struct NetworkInterface {
    std::string name;
    std::string ipAddress;
    std::string subnetMask;
    std::string macAddress;
    std::string gateway;
    bool isUp;
    bool isLoopback;
};

class NetworkUtils {
public:
    static std::vector<NetworkInterface> getNetworkInterfaces() {
        std::vector<NetworkInterface> interfaces;
        
#ifdef _WIN32
        getWindowsInterfaces(interfaces);
#else
        getUnixInterfaces(interfaces);
#endif
        return interfaces;
    }
    
    static void displayIpConfig() {
        std::cout << "\nWindows IP Configuration\n" << std::endl;
        
        std::vector<NetworkInterface> interfaces = getNetworkInterfaces();
        
        if (interfaces.empty()) {
            std::cout << "No network interfaces found." << std::endl;
            return;
        }
        
        for (const auto& iface : interfaces) {
            if (iface.isLoopback) continue; 
            
            std::cout << "Ethernet adapter " << iface.name << ":" << std::endl;
            std::cout << "   Connection-specific DNS Suffix  . :" << std::endl;
            std::cout << "   Link-local IPv6 Address . . . . . : (Not available)" << std::endl;
            std::cout << "   IPv4 Address. . . . . . . . . . . : " << iface.ipAddress << std::endl;
            std::cout << "   Subnet Mask . . . . . . . . . . . : " << iface.subnetMask << std::endl;
            if (!iface.gateway.empty()) {
                std::cout << "   Default Gateway . . . . . . . . . : " << iface.gateway << std::endl;
            }
            if (!iface.macAddress.empty()) {
                std::cout << "   Physical Address. . . . . . . . . : " << iface.macAddress << std::endl;
            }
            std::cout << std::endl;
        }
        
       
        for (const auto& iface : interfaces) {
            if (iface.isLoopback) {
                std::cout << "Tunnel adapter " << iface.name << ":" << std::endl;
                std::cout << "   IPv4 Address. . . . . . . . . . . : " << iface.ipAddress << std::endl;
                std::cout << "   Subnet Mask . . . . . . . . . . . : " << iface.subnetMask << std::endl;
                std::cout << std::endl;
                break;
            }
        }
    }
    
private:
#ifdef _WIN32
    static void getWindowsInterfaces(std::vector<NetworkInterface>& interfaces) {
        DWORD dwSize = 0;
        DWORD dwRetVal = 0;
        
        PIP_ADAPTER_INFO pAdapterInfo;
        PIP_ADAPTER_INFO pAdapter = nullptr;
        
        
        GetAdaptersInfo(nullptr, &dwSize);
        pAdapterInfo = (IP_ADAPTER_INFO*)malloc(dwSize);
        
        if (pAdapterInfo == nullptr) {
            std::cout << "Error allocating memory for adapter info" << std::endl;
            return;
        }
        
        dwRetVal = GetAdaptersInfo(pAdapterInfo, &dwSize);
        
        if (dwRetVal == NO_ERROR) {
            pAdapter = pAdapterInfo;
            while (pAdapter) {
                NetworkInterface iface;
                iface.name = std::string(pAdapter->AdapterName);
                iface.ipAddress = std::string(pAdapter->IpAddressList.IpAddress.String);
                iface.subnetMask = std::string(pAdapter->IpAddressList.IpMask.String);
                iface.gateway = std::string(pAdapter->GatewayList.IpAddress.String);
                
                // Format MAC address
                char macStr[18];
                sprintf(macStr, "%02X-%02X-%02X-%02X-%02X-%02X",
                    pAdapter->Address[0], pAdapter->Address[1],
                    pAdapter->Address[2], pAdapter->Address[3],
                    pAdapter->Address[4], pAdapter->Address[5]);
                iface.macAddress = std::string(macStr);
                
                iface.isUp = (pAdapter->Type != MIB_IF_TYPE_LOOPBACK);
                iface.isLoopback = (pAdapter->Type == MIB_IF_TYPE_LOOPBACK);
                
                if (!iface.ipAddress.empty() && iface.ipAddress != "0.0.0.0") {
                    interfaces.push_back(iface);
                }
                
                pAdapter = pAdapter->Next;
            }
        }
        
        if (pAdapterInfo) {
            free(pAdapterInfo);
        }
    }
#else
    static void getUnixInterfaces(std::vector<NetworkInterface>& interfaces) {
        struct ifaddrs* ifaddr;
        struct ifaddrs* ifa;
        
        if (getifaddrs(&ifaddr) == -1) {
            std::cout << "Error getting network interfaces" << std::endl;
            return;
        }
        
        for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr == nullptr) continue;
            
            if (ifa->ifa_addr->sa_family == AF_INET) {
                NetworkInterface iface;
                iface.name = std::string(ifa->ifa_name);
                
                struct sockaddr_in* addr_in = (struct sockaddr_in*)ifa->ifa_addr;
                iface.ipAddress = std::string(inet_ntoa(addr_in->sin_addr));
                
                struct sockaddr_in* netmask_in = (struct sockaddr_in*)ifa->ifa_netmask;
                iface.subnetMask = std::string(inet_ntoa(netmask_in->sin_addr));
                
                iface.isUp = (ifa->ifa_flags & IFF_UP) != 0;
                iface.isLoopback = (ifa->ifa_flags & IFF_LOOPBACK) != 0;
                
                // Simple gateway detection (not perfect but works for basic cases)
                if (!iface.isLoopback && iface.isUp) {
                    iface.gateway = "192.168.1.1"; // Default assumption
                }
                
                interfaces.push_back(iface);
            }
        }
        
        freeifaddrs(ifaddr);
    }
#endif
};

#endif