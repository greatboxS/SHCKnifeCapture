#ifndef ethernet_extension_h
#define ethernet_extension_h

#include "global_config.h"
#include <Ethernet.h>
#include <EEPROM.h>
#include "eeprom_extensions.h"

typedef enum HttpMethod_t
{
    HTTP_GET,
    HTTP_POST,
    HTTP_PUT,
    HTTP_DELETE,
};

struct ethernet_pa_def
{
private:
    char RootUrl[256];
    char Host[32] = "Host: 10.4.3.41:32765";
    const char *ConnectionClose = "Connection: close";
    int Storage_address;
    int eeprom_server_ip_addr = 0;
    int eeprom_port_addr = 0;
    int eeprom_mac_addr = 0;

    void eeprom_save_MAC(int address)
    {
        EEPROM.begin(EEPROM_MAX_SIZE);
        for (size_t i = 0; i < 6; i++)
        {
            eeprom_write<uint8_t>(MAC[i], address + i);
        }
        EEPROM.end();
    }

    void eeprom_read_MAC(int address)
    {
        EEPROM.begin(EEPROM_MAX_SIZE);
        uint8_t mac[6]{0};
        for (size_t i = 0; i < sizeof(mac); i++)
        {
            mac[i] = eeprom_read<uint8_t>(address + i);
        }
        EEPROM.end();
        memccpy(MAC, mac, 0, sizeof(MAC));
    }

    void eeprom_save_server_ip(int address)
    {
        function_log(ethernet_extension.h);
        EEPROM.begin(EEPROM_MAX_SIZE);
        for (size_t i = 0; i < sizeof(ServerIp); i++)
        {
            eeprom_write<char>(ServerIp[i], address + i);
        }
        EEPROM.end();
    }

    void eeprom_save_server_port(int address)
    {
        function_log(ethernet_extension.h);
        EEPROM.begin(EEPROM_MAX_SIZE);
        eeprom_write<int>(Port, address);
        EEPROM.end();
    }

    void eeprom_read_server_ip(int address)
    {
        function_log(ethernet_extension.h);
        EEPROM.begin(EEPROM_MAX_SIZE);
        for (size_t i = 0; i < sizeof(ServerIp); i++)
        {
            ServerIp[i] = eeprom_read<char>(address + i);
        }
        EEPROM.end();
    }

    void eeprom_read_server_port(int address)
    {
        function_log(ethernet_extension.h);
        EEPROM.begin(EEPROM_MAX_SIZE);
        Port = eeprom_read<int>(address);
        EEPROM.end();
    }

public:
    char ServerIp[20] = "10.4.3.41";
    int Port = 32765;
    uint8_t MAC[6] = {0xF6, 0x6C, 0x08, 0x62, 0x05, 0x06};

    void init(int storage_address)
    {
        function_log(ethernet_extension.h);
        printf("Initializes ethernet parameter\r\n");
        Storage_address = storage_address;
        memset(RootUrl, 0, sizeof(RootUrl));
        storage_address_specify();
    }

    // port//server_ip//mac
    void storage_address_specify()
    {
        function_log(ethernet_extension.h);
        eeprom_port_addr = Storage_address;
        eeprom_server_ip_addr = eeprom_port_addr + sizeof(Port);
        eeprom_mac_addr = eeprom_server_ip_addr + sizeof(ServerIp);
    }

    void eeprom_read_all_parameters()
    {
        function_log(ethernet_extension.h);
        eeprom_read_ethernet_parameters();
        eeprom_read_MAC();
    }

    void ethernet_make_url(char *url, uint8_t method)
    {
        function_log(ethernet_extension.h);
        memset(RootUrl, 0, sizeof(RootUrl));
        switch (method)
        {
        case HTTP_GET:
            snprintf(RootUrl, sizeof(RootUrl), "GET /%s HTTP/1.1", url);
            break;
        case HTTP_POST:
            snprintf(RootUrl, sizeof(RootUrl), "POST /%s HTTP/1.1", url);
            break;
        case HTTP_PUT:
            snprintf(RootUrl, sizeof(RootUrl), "PUT /%s HTTP/1.1", url);
            break;
        case HTTP_DELETE:
            snprintf(RootUrl, sizeof(RootUrl), "DELETE /%s HTTP/1.1", url);
            break;
        }
    }

    void ethernet_make_url(const String &url, uint8_t method)
    {
        function_log(ethernet_extension.h);
        memset(RootUrl, 0, sizeof(RootUrl));
        switch (method)
        {
        case HTTP_GET:
            snprintf(RootUrl, sizeof(RootUrl), "GET /%s HTTP/1.1", url.c_str());
            break;
        case HTTP_POST:
            snprintf(RootUrl, sizeof(RootUrl), "POST /%s HTTP/1.1", url.c_str());
            break;
        case HTTP_PUT:
            snprintf(RootUrl, sizeof(RootUrl), "PUT /%s HTTP/1.1", url.c_str());
            break;
        case HTTP_DELETE:
            snprintf(RootUrl, sizeof(RootUrl), "DELETE /%s HTTP/1.1", url.c_str());
            break;
        }
    }

    void ethernet_send_request(EthernetClient &client, char *request_url, uint8_t method)
    {
        function_log(ethernet_extension.h);
        ethernet_make_url(request_url, HTTP_GET);
        client.println(RootUrl);
        client.println(Host);
        client.println(ConnectionClose);
        client.println();

        printf("Request url: %s,  %s\r\n", RootUrl, Host);
    }

    void ethernet_send_request(EthernetClient &client, char *request_url, char *data, uint8_t method)
    {
        function_log(ethernet_extension.h);
        ethernet_make_url(request_url, method);
        client.println(RootUrl);
        client.println(Host);
        client.println(ConnectionClose);

        printf("Request url: %s,  %s\r\n", RootUrl, Host);

        if (data != NULL)
        {
            char temp[32]{0};
            int length = strlen(data);
            snprintf(temp, sizeof(temp), "Content-Length: %d", length);
            client.println("Content-Type: application/json");
            client.println(temp);
            client.println();
            client.println(data);

            printf("Post data: %s\r\n", data);
        }
        else
        {
            client.println();
        }
    }

    void ethernet_post(EthernetClient &client, char *url, char *data)
    {
        char temp[32]{0};
        int length = strlen(data);
        snprintf(temp, sizeof(temp), "Content-Length: %d", length);

        ethernet_make_url(url, HTTP_POST);
        //
        client.println(RootUrl);
        client.println(Host);
        client.println(ConnectionClose);
        client.println("Content-Type: application/json");
        client.println(temp);
        client.println();
        client.println(data);

        printf("Post url %s, %s\r\n", RootUrl, Host);
        printf("Content-Type: application/json\r\n");
        printf(temp);
        printf("Data: %s\r\n", data);
    }

    void device_setup_mac(uint8_t *mac)
    {
        function_log(ethernet_extension.h);
        memccpy(MAC, mac, 0, sizeof(MAC));
    }

    void ethernet_setup_server(char *server_name, uint16_t port)
    {
        function_log(ethernet_extension.h);
        Port = port;
        memccpy(ServerIp, server_name, 0, sizeof(ServerIp));
        snprintf(Host, sizeof(Host), "Host: %s:%d", ServerIp, Port);
        printf("New server profile: %s : %d\r\n", ServerIp, Port);
    }

    void eeprom_save_MAC()
    {
        function_log(ethernet_extension.h);
        eeprom_save_MAC(eeprom_mac_addr);
        printf("Save MAC: %s\r\n", MAC);
    }

    void eeprom_read_MAC()
    {
        function_log(ethernet_extension.h);
        eeprom_read_MAC(eeprom_mac_addr);
        printf("Read MAC: %s\r\n", MAC);
    }

    void eeprom_read_ethernet_parameters()
    {
        function_log(ethernet_extension.h);
        eeprom_read_server_ip(eeprom_server_ip_addr);
        eeprom_read_server_port(eeprom_port_addr);
        printf("Read server IP: %s, Port: %d\r\n", ServerIp, Port);
        snprintf(Host, sizeof(Host), "Host: %s:%d", ServerIp, Port);
    }

    void eeprom_save_ethernet_parameters()
    {
        function_log(ethernet_extension.h);
        eeprom_save_server_ip(eeprom_server_ip_addr);
        eeprom_save_server_port(eeprom_port_addr);
    }
};

#endif