#ifndef ethernet_handler_h
#define ethernet_handler_h
#include <EEPROM.h>
#include "global_config.h"
#include "global_scope.h"

class ethernet_handler
{
private:
    typedef void (*received_data_f)(EthernetClient &);
    received_data_f received_data_callback;
    uint8_t chip_select_pin;
    uint8_t reset_pin;

    bool reset_module_now = false;

    IPAddress local_ip, dns_ip, gateway_ip, subnet_ip;
    bool got_ip = false;

public:
    bool cable_connected = false;
    bool module_connected = false;
    bool ip_initialized = false;
    ethernet_pa_def ethernet_pr;
    EthernetClient client;
    char localIp[20];

    ethernet_handler(int storage_addr)
    {
        printf("Create new ethernet handler\r\n");
        ethernet_pr.init(storage_addr);
    }

    void init(uint8_t cs_pin, uint8_t rst_pin, received_data_f f)
    {
        function_log();
        chip_select_pin = cs_pin;
        reset_pin = rst_pin;
        pinMode(reset_pin, OUTPUT);
        digitalWrite(reset_pin, HIGH);
        received_data_callback = f;
    }

    void running()
    {
        if (client.available() > 0)
        {
            while (client.available() > 0)
            {
                received_data_callback(client);
            }
        }
    }

    uint8_t reset_ethernet_module()
    {
        function_log();
        printf("Ethernet module is resetting...");
        digitalWrite(reset_pin, LOW);
        delay(100);
        digitalWrite(reset_pin, HIGH);
        delay(3000);

        return setting_up_ethernet_module();
    }

    int setting_up_ethernet_module()
    {
        function_log();
        Ethernet.init(chip_select_pin);

        printf("Begin Ethernet configuration using DHCP\r\n");

        printf("Old localIP: %s\r\n", Ethernet.localIP().toString().c_str());

        printf("Device MAC:");
        for (size_t i = 0; i < sizeof(ethernet_pr.MAC); i++)
        {
            printf(" %XX", ethernet_pr.MAC[i]);
        }
        printf("\r\n");

        // IPAddress debug_ip(192, 168, 0, 2);
        // Ethernet.begin(ethernet_pr.MAC, debug_ip);
        // return 1;
        int exception = Ethernet.begin(ethernet_pr.MAC, 5000);

        if (exception == 0)
        {
            printf("Failed to configure Ethernet using DHCP\r\n");
            // Check for Ethernet hardware present
            if (Ethernet.hardwareStatus() == EthernetNoHardware)
            {
                printf("Ethernet shield was not found.  Sorry, can't run without hardware.\r\n");
                nex_send_message("Error: Ethernet module was not found");
                //setStringProperty("SCREEN1", "t6.txt", "Error: Ethernet module was not found");
                cable_connected = false;
                module_connected = false;
            }
            if (Ethernet.linkStatus() == LinkOFF)
            {
                printf("Ethernet cable is not connected.");
                nex_send_message("Error: Cable is disconnected");
                //setStringProperty("SCREEN1", "t6.txt", "Error: Cable is disconnected");
                cable_connected = false;
                module_connected = true;
            }
        }
        else
        {
            got_ip = true;

            local_ip = Ethernet.localIP();
            dns_ip = Ethernet.dnsServerIP();
            gateway_ip = Ethernet.gatewayIP();
            subnet_ip = Ethernet.subnetMask();

            module_connected = true;
            cable_connected = true;
            ip_initialized = true;
            snprintf(localIp, sizeof(localIp), "%s", Ethernet.localIP().toString().c_str());
            printf(localIp);
            nex_send_message(localIp);
            //setStringProperty("SCREEN1", "t6.txt", localIp);
        }

        return exception;
    }

    void resetup_ethernet_module()
    {
        Ethernet.init(chip_select_pin);
        printf("Reinit Ethernet module with static ip\r\n");
        Ethernet.begin(ethernet_pr.MAC, local_ip, dns_ip, gateway_ip, subnet_ip);
    }

    void checking_ethernet_module()
    {
        get_ethernet_module_status();

        if (reset_module_now)
        {
            reset_module_now = false;
            if (got_ip)
                resetup_ethernet_module();
            else
                setting_up_ethernet_module();
        }
    }

    uint8_t get_ethernet_module_status()
    {
        if (Ethernet.hardwareStatus() == EthernetNoHardware)
        {
            printf("Ethernet shield was not found.  Sorry, can't run without hardware\r\n");
            cable_connected = false;
            module_connected = false;
            return 0; // no hardware
        }

        if (Ethernet.linkStatus() == LinkOFF)
        {
            printf("Ethernet cable is not connected\r\n");
            cable_connected = false;
            module_connected = true;
            return 2; // no cable
        }
        module_connected = true;
        if (!cable_connected)
        {
            reset_module_now = true;
        }

        cable_connected = true;
        ip_initialized = true;
        return 1; // cable connect ok
    }

    bool start_connect_to_server(uint32_t timeout = 10000)
    {
        function_log();

        if (client.connected())
            return true;

        if (got_ip)
            resetup_ethernet_module();
        else
            setting_up_ethernet_module();

        client.connect(ethernet_pr.ServerIp, ethernet_pr.Port);
        char temp[64];
        snprintf(temp, sizeof(temp), "Start connect to server : %s:%d\r\n", ethernet_pr.ServerIp, ethernet_pr.Port);
        printf(temp);
        int count = timeout / 100;
        int i = 0;
        while (!client.connected() && i < count)
        {
            i++;
            delay(100);
            printf(".");
        }
        if (client.connected())
        {
            printf("Connected to server %s:%d: successfull\r\n", ethernet_pr.ServerIp, ethernet_pr.Port);
        }
        else
        {
            printf("Connected to server %s:%d: fail\r\n", ethernet_pr.ServerIp, ethernet_pr.Port);
        }
        return client.connected();
    }

    // false: connect falied
    // true: success
    bool make_request_to_server(char *url, uint8_t _method)
    {
        function_log();

        if (!start_connect_to_server(1000))
        {
            printf("Connect: failed\r\n");
            return false;
        }
        printf("Connect: successed\r\n");

        ethernet_pr.ethernet_send_request(client, url, HTTP_GET);
        return true;
    }

    // false: connect falied
    // true: success
    bool make_post_to_server(char *url, char *data)
    {
        function_log();

        if (!start_connect_to_server(1000))
        {
            printf("Connect: failed\r\n");
            return false;
        }
        printf("Connect: successed\r\n");

        ethernet_pr.ethernet_post(client, url, data);
        return true;
    }

    void ethernet_maintain()
    {
        Ethernet.maintain();
    }
};

#endif
