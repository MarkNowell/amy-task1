#include "settings.h"

float Settings::TARGET_TEMP = 22.5;
float Settings::TARGET_HUMIDITY = 55.0;
float Settings::TARGET_CO2 = 350.0;
float Settings::MAXTEMP = Settings::TARGET_TEMP + 3;
float Settings::MINCO2 = Settings::TARGET_CO2 - 50;


json Settings::getSettingsFromServer() {
    WSADATA wsaData;
    SOCKET sock = INVALID_SOCKET;
    struct sockaddr_in server;
    char buffer[1024] = {};

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        WSACleanup();
        throw std::runtime_error("Socket creation failed");
    }

    // Set up server address
    server.sin_family = AF_INET;
    server.sin_port = htons(5001);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    // Connect to server
    if (connect(sock, (SOCKADDR*)&server, sizeof(server)) == SOCKET_ERROR) {
        closesocket(sock);
        WSACleanup();
        throw std::runtime_error("Connection failed");
    }

    // Receive data
    int bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
    if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
        closesocket(sock);
        WSACleanup();
        throw std::runtime_error("Failed to receive data");
    }

    // Clean up
    closesocket(sock);
    WSACleanup();

    // Parse and return JSON
    return json::parse(std::string(buffer, static_cast<std::string::size_type>(bytesReceived)));
}

void Settings::currentSettings()
{
    try
    {
        json settings =getSettingsFromServer();
        TARGET_TEMP=settings["target_temperature"];
        TARGET_HUMIDITY =settings["target_humidity"];
        TARGET_CO2 = settings["target_co2"];
    }catch(const std::exception& e){std::cerr <<"TCP Client error: "<<e.what()<<'\n';}

    MAXTEMP=TARGET_TEMP+3;
    MINCO2=TARGET_CO2-50;
}

std::ostream& operator<<(std::ostream& out,const Settings&)
{
    out<<"\nTarget Temp: "<<Settings::TARGET_TEMP<<'\t'<<"Max Temp: "<<Settings::MAXTEMP<<'\n';
    out<<"Target Humidity: "<<Settings::TARGET_HUMIDITY<<'\n';
    out<<"Target Co2: "<<Settings::TARGET_CO2<<'\t'<<'\t'<<"Min Co2: "<<Settings::MINCO2<<'\n'<<'\n';
    return out;
}
