#include "NetworkManager.h"

int NetworkManager::connectToServer(const std::string &address, const std::string &port = "27015") {
    conPhase = ConnectionPhase::DISCONNECTED;

    //Prepare the Windows socket api
    startResult = WSAStartup(REQ_SOCK_VERSION, &wsadata);
    if (startResult != 0) {
        printf(ANSI_RED "[SockServer] WSAStartup failed with error: %d\n" ANSI_RESET, startResult);
        return startResult;
    }

    conPhase = ConnectionPhase::ESTABLISHING;

    //Websocket configuration
    struct addrinfo *result = nullptr, *ptr = nullptr, requested{};

    ZeroMemory(&requested, sizeof(requested));
    requested.ai_family = AF_INET;
    requested.ai_socktype = SOCK_STREAM;
    requested.ai_protocol = IPPROTO_TCP;

    startResult = getaddrinfo(address.c_str(), port.c_str(), &requested, &result);
    if (startResult != 0) {
        printf(ANSI_RED "[SockClient] getaddrinfo failed with error: %d\n" ANSI_RESET, startResult);
        WSACleanup();
        return startResult;
    }

    printf(ANSI_CYAN "[SockClient] Result %p" ANSI_RESET "\n", result->ai_addr);

    //Create the socket
    clientSocket = INVALID_SOCKET;
    clientSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (clientSocket == INVALID_SOCKET) {
        printf("[SockClient] socket failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return startResult;
    }

    //Connect to the server
    startResult = connect(clientSocket, result->ai_addr, static_cast<int>(result->ai_addrlen));
    if (startResult != 0) {
        printf(ANSI_RED "[SockClient] connect failed with error: %d" ANSI_RESET "\n", startResult);
        freeaddrinfo(result);
        WSACleanup();
        return startResult;
    }

    //Make the socket non-blocking
    u_long mode = 1;
    ioctlsocket(clientSocket, FIONBIO, &mode);

    //Free the address configuration
    freeaddrinfo(result);
    conPhase = ConnectionPhase::ESTABLISHED;
    printf(ANSI_GREEN "[SockClient] Connection established" ANSI_RESET "\n");
    return startResult;
}

void NetworkManager::disconnect() {
    shutdown(clientSocket, SD_SEND);
    closesocket(clientSocket);
    clientSocket = INVALID_SOCKET;
    WSACleanup();
}

bool NetworkManager::pollPacket(PacketHeader &outHeader, std::vector<char> &outPayload) {
    char tempBuffer[DEFAULT_BUFFER_LEN];

    const int bytesReceived = recv(clientSocket, tempBuffer, sizeof(tempBuffer), 0);

    if (bytesReceived > 0) {
        receiveBuffer.insert(receiveBuffer.end(), tempBuffer, tempBuffer + bytesReceived);
    } else if (bytesReceived == 0) {
        conPhase = ConnectionPhase::DISCONNECTED;
        return false;
    } else {
        const int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK) {
            //
        }
    }

    if (receiveBuffer.size() < sizeof(PacketHeader)) {
        // We don't have enough for a full header, wait for more bytes
        return false;
    }

    const auto *pendingHeader = reinterpret_cast<PacketHeader *>(receiveBuffer.data());

    const size_t totalPacketSize = sizeof(PacketHeader) + pendingHeader->payloadSize;

    if (receiveBuffer.size() < totalPacketSize) {
        // We have the header, but not the full data
        return false;
    }

    outHeader = *pendingHeader;
    outPayload.clear();
    if (outHeader.payloadSize > 0) {
        outPayload.insert(
            outPayload.end(),
            receiveBuffer.begin() + sizeof(PacketHeader),
            receiveBuffer.begin() + totalPacketSize);
    }

    receiveBuffer.erase(receiveBuffer.begin(), receiveBuffer.begin() + totalPacketSize);
    return true;
}
