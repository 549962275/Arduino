/*
  WiFiUdp.cpp - Library for Arduino Wifi shield.
  Copyright (c) 2011-2014 Arduino LLC.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

extern "C" {
//  #include "utility/debug.h"
//  #include "utility/wifi_spi.h"
extern int rtl_printf(const char *fmt, ...);

}
#include <string.h>
#include "server_drv.h"
#include "wifi_drv.h"
#include "WiFi.h"
#include "WiFiUdp.h"
#include "WiFiClient.h"
#include "WiFiServer.h"

/* Constructor */
WiFiUDP::WiFiUDP() : _sock(-1), _client_sock(-1) {}

/* Start WiFiUDP socket, listening at local port PORT */
uint8_t WiFiUDP::begin(uint16_t port) {

    if (_port == port && _sock >= 0) {
        return 1;
    }

    _port = port;
    _sock = serverDrv.startServer(port, UDP_MODE);

    if (_sock >=0 ) {
        return 1;
    }

    return 0;
}

/* return number of bytes available in the current packet,
   will return zero if parsePacket hasn't been called yet */
int WiFiUDP::available() {
    if (_sock >= 0)
    {
        return serverDrv.availData(_sock);
    }
    return 0;
}

/* Release any resources being used by this WiFiUDP instance */
void WiFiUDP::stop()
{
    if (_sock < 0)
        return;

    serverDrv.stopClient(_sock);

    _sock = -1;
}

int WiFiUDP::beginPacket(const char *host, uint16_t port)
{
    // Look up the host first
    int ret = 0;
    IPAddress remote_addr;
    if (WiFi.hostByName(host, remote_addr))
    {
        return beginPacket(remote_addr, port);
    }
    return ret;
}

int WiFiUDP::beginPacket(IPAddress ip, uint16_t port)
{
    peer_ip = ip;
    peer_port = port;

    if (_sock >= 0) {
        _client_sock = _sock;
    } else {
        _client_sock = serverDrv.startClient(ip, port, UDP_MODE);
    }

    if (_client_sock < 0) {
        return 0;
    } else {
        return 1;
    }
}

int WiFiUDP::endPacket()
{
    if (_client_sock >= 0 && _client_sock != _sock) {
        serverDrv.stopClient(_client_sock);
    }

    peer_ip = 0;
    peer_port = 0;
    _client_sock = -1;

    return true;
}

size_t WiFiUDP::write(uint8_t byte)
{
  return write(&byte, 1);
}

size_t WiFiUDP::write(const uint8_t *buffer, size_t size)
{
    writeImmediately(buffer, size);
    return size;
}

size_t WiFiUDP::writeImmediately(const uint8_t *buffer, size_t size)
{
    serverDrv.sendtoData(_client_sock, buffer, size, peer_ip, peer_port);
    return size;
}

int WiFiUDP::parsePacket()
{
    return available();
}

int WiFiUDP::read()
{
    uint8_t b;
    if (available())
    {
        serverDrv.getData(_sock, &b);
        return b;
    } else {
        return -1;
    }
}

int WiFiUDP::read(unsigned char* buffer, size_t len)
{
    if (available()) {
        return serverDrv.getDataBuf(_sock, buffer, len);
    } else {
        return -1;
    }
}

int WiFiUDP::peek()
{
    uint8_t b;
    if (!available())
        return -1;

    serverDrv.getData(_sock, &b, 1);
    return b;
}

void WiFiUDP::flush()
{
    while (available())
        read();
}

IPAddress WiFiUDP::remoteIP()
{
    uint32_t _remoteIp;
    uint16_t _remotePort;

    serverDrv.getRemoteData(_sock, &_remoteIp, &_remotePort);

    IPAddress ip(_remoteIp);
    return ip;
}

uint16_t WiFiUDP::remotePort()
{
    uint32_t _remoteIp;
    uint16_t _remotePort;

    serverDrv.getRemoteData(_sock, &_remoteIp, &_remotePort);

    return _remotePort;
}
