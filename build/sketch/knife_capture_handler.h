#ifndef knife_capture_handler_h
#define knife_capture_handler_h

#include <EEPROM.h>
#include "global_config.h"
#include "global_scope.h"
#include "ethernet_handler.h"
#include "autocut_machine_props.h"

class knife_capture_class
{
public:
    machine_handler machine_handle = machine_handler(MACHINE_START_ADDR);
    ethernet_handler ethernet_handle = ethernet_handler(ETHERNET_START_ADDR);
    http_header_t http_header;
    knife_capture_resp_info knife_capture_resp;

    int local_device_id = 0;
    uint8_t current_machine_page = 1;
    bool sys_requesting = false;

    int knife_position = 0;
    int machine_slot = 0;
    int knife_type = 0;
    int knife_picker = 0;
    volatile bool submit_recheck_flag = false;
    bool knife_capture_submit = false;

    std::queue<request_def> request_list;

    // Initializes all background data
    void init()
    {
        eeprom_read_local_id();
        //
        machine_handle.eeprom_read_total_machine();
        //
        machine_handle.eeprom_read_machines();
        //
        ethernet_handle.ethernet_pr.eeprom_read_all_parameters();
    }

    bool ethernet_queue_available()
    {
        return (request_list.size() > 0 && !sys_requesting && !knife_capture_submit);
    }

    // result: @1 = successed
    //         @0 = request_list fullrecheck_submit_click
    //         @-1 = falied to capture knife
    int add_new_request_to_queue()
    {
        function_log();

        request_def new_request = machine_handle.new_knife_captured(machine_slot, current_machine_page, local_device_id,
                                                                    knife_position, knife_type, knife_picker);

        if (!new_request.valid)
        {
            printf("Add new request falied\r\n");
            nex_send_message("Failed to update current machine parameters");
            return -1;
        }

        // queue is full
        if (request_list.size() > MAX_QUEUE_LENGTH)
        {
            nex_send_message("Request list is full");
            return 0;
        }

        nex_send_message("Updated Successfully");
        request_list.push(new_request);
        // success
        return 1;
    }

    void checking_reset_counter_request()
    {
        function_log();
        char url[256]{0};
        String cmd = "kc_api/initial/";
        for (size_t i = 0; i < machine_handle.machines.size(); i++)
        {
            cmd += String(machine_handle.machines[i].m_name) + ",";
        }
        memccpy(url, cmd.c_str(), 0, sizeof(url));

        printf("First request url: %s\r\n", url);
        ethernet_handle.make_request_to_server(url, HTTP_GET);
        sys_requesting = true;
    }

    bool ethernet_request_next()
    {
        function_log();

        if (!ethernet_handle.cable_connected)
            return false;

        request_list.front().retry_time--;
        if (request_list.front().retry_time < 0)
        {
            printf("Request failed after retry\r\n");
            request_list.pop();
        }

        if (ethernet_handle.start_connect_to_server())
        {
            ethernet_handle.ethernet_pr.ethernet_send_request(ethernet_handle.client, request_list.front().url, HTTP_GET);
            sys_requesting = true;
            knife_capture_submit = true;
            printf("Sent new request successfull\r\n");
            return true;
        }
        else
        {
            printf("Connect to server failed\r\n");
            return false;
        }
        return false;
    }

    bool ethernet_post_capture()
    {
        function_log();

        request_list.front().retry_time--;
        if (request_list.front().retry_time < 0)
        {
            printf("Request failed after retry\r\n");
            request_list.pop();
        }

        if (ethernet_handle.start_connect_to_server())
        {
            ethernet_handle.ethernet_pr.ethernet_send_request(ethernet_handle.client, request_list.front().url, request_list.front().data, HTTP_POST);
            sys_requesting = true;
            knife_capture_submit = true;
            printf("Sent new request successfull\r\n");
            return true;
        }
        else
        {
            printf("Connect to server failed\r\n");
            return false;
        }
        return false;
    }

    void machine_reset_counter()
    {
        function_log();
        machine_handle.machine_reset_data();
    }

    void nx_save_local_mac_id()
    {
        function_log();
        nex_send_message("Exeption: Saving...");

        char *props[6] = {"n1.val", "n2.val", "n3.val", "n4.val", "n5.val", "n6.val"};
        uint8_t mac[6]{0};
        for (int i = 0; i < 6; i++)
        {
            uint32_t mac_element = getNumberProperty("SETTING2", props[i]);
            printf("mac[%d]: %d\r\n", i, mac_element);
            if (mac_element > 255 || mac_element == 0 || mac_element == -1)
            {
                setStringProperty("SETTING2", "t1.txt", "Error: Invalid MAC");
                return;
            }
            mac[i] = (uint8_t)mac_element;
        }

        ethernet_handle.ethernet_pr.device_setup_mac(mac);
        ethernet_handle.ethernet_pr.eeprom_save_MAC();
        nex_send_message("Exeption: Saved successfully");
    }

    void nx_save_server_info()
    {
        function_log();
        char server[20]{0};
        int port = 0;
        nex_send_message("Exeption: Saving...");
        getStringProperty("SETTING2", "t0.txt", server, sizeof(server));
        port = getNumberProperty("SETTING2", "n0.val");

        if (strlen(server) == 0 || port == 0)
        {
            setStringProperty("SETTING2", "t1.txt", "Error: Invalid data");
            return;
        }

        ethernet_handle.ethernet_pr.ethernet_setup_server(server, port);
        ethernet_handle.ethernet_pr.eeprom_save_ethernet_parameters();
        nex_send_message("Exeption: Saved successfully");
    }

    void nx_save_new_machine()
    {
        function_log();
        char m_name[20]{0};
        nex_send_message("Exeption: Adding new machine...");
        getStringProperty("SETTING", "t0.txt", m_name, sizeof(m_name));
        machine_handle.add_new_machine(m_name);
        nex_send_message("Exeption: Saved successfully");
    }

    void nx_save_local_id()
    {
        function_log();
        nex_send_message("Exeption: Saving...");
        local_device_id = getNumberProperty("SETTING", "n1.val");
        eeprom_save_local_id();
        nex_send_message("Exeption: Saved successfully");
    }

    void nx_save_total_machine_num()
    {
        function_log();
        int temp_num = getNumberProperty("SETTING", "n0.val");
        nex_send_message("Exeption: Saving...");
        machine_handle.update_total_machine_num(temp_num);
        nex_send_message("Exeption: Saved successfully");
    }

    // 1 - go next
    // 0 - go back
    void nx_display_next_page(bool go_next)
    {
        function_log();

        if (go_next)
        {
            printf("Current page: %d\r\n", current_machine_page);
            if (machine_handle.get_page_number(true) > current_machine_page)
            {
                current_machine_page++;
                printf("Go next: %d\r\n", current_machine_page);
                nx_update_home_screen();
            }
        }
        else
        {
            printf("Current page: %d\r\n", current_machine_page);
            if (current_machine_page > machine_handle.get_page_number(false))
            {
                current_machine_page--;
                printf("Go back: %d\r\n", current_machine_page);
                nx_update_home_screen();
            }
        }
    }

    void nx_update_home_screen()
    {
        function_log();
        machine_handle.nx_display_new_page(current_machine_page);
    }

    void nx_update_new_setting_screen()
    {
        function_log();

        nx_update_total_machine();

        nx_update_local_id_num();

        nx_update_machine_list();
    }

    void nx_update_setting2_screen()
    {
        function_log();
        setStringProperty("SETTING2", "t0.txt", ethernet_handle.ethernet_pr.ServerIp);
        setNumberProperty("SETTING2", "n0.val", ethernet_handle.ethernet_pr.Port);

        char *props[6] = {"n1.val", "n2.val", "n3.val", "n4.val", "n5.val", "n6.val"};
        for (int i = 0; i < sizeof(ethernet_handle.ethernet_pr.MAC); i++)
        {
            setNumberProperty("SETTING2", props[i], ethernet_handle.ethernet_pr.MAC[i]);
        }

        setStringProperty("SETTING2", "t2.txt", ethernet_handle.localIp);
    }

    void nx_update_total_machine()
    {
        function_log();
        setNumberProperty("SETTING", "n0.val", machine_handle.total_machine);
    }

    void nx_update_local_id_num()
    {
        function_log();
        setNumberProperty("SETTING", "n1.val", local_device_id);
    }

    // display the list of saved machines
    void nx_update_machine_list()
    {
        function_log();
        uint8_t size = machine_handle.machines.size();
        printf("machines.size = %d\r\n", size);

        for (size_t i = 0; i < size; i++)
        {
            char temp[10]{};
            snprintf(temp, sizeof(temp), "t%d.txt", (i + 1) * 2 - 1);
            char num[3]{0};
            snprintf(num, sizeof(num), "%d", i + 1);
            setStringProperty("SETTING", temp, num);

            snprintf(temp, sizeof(temp), "t%d.txt", (i + 1) * 2);
            setStringProperty("SETTING", temp, machine_handle.machines[i].m_name);
        }
    }

    void eeprom_save_local_id()
    {
        EEPROM.begin(EEPROM_MAX_SIZE);
        eeprom_write(local_device_id, LOCAL_DEVICE_ID_ADDR);
        printf("read: %d\r\n", eeprom_read<int>(LOCAL_DEVICE_ID_ADDR));
        EEPROM.end();
    }

    void eeprom_read_local_id()
    {
        EEPROM.begin(EEPROM_MAX_SIZE);
        local_device_id = eeprom_read<int>(LOCAL_DEVICE_ID_ADDR);
        EEPROM.end();
    }

    void ethernet_parse_all_received_data(JsonDocument &json_doc)
    {
        function_log();
        knife_capture_resp.json_parse_resp_data(json_doc);
        handle_synchronize_data(knife_capture_resp);
        nex_goto_page("SCREEN1");
    }

    void handle_synchronize_data(knife_capture_resp_info &resp_data)
    {
        for (size_t i = 0; i < resp_data.machines.size(); i++)
        {
            for (size_t k = 0; k < machine_handle.machines.size(); k++)
            {
                if (machine_handle.machines[k] == resp_data.machines[i].machine_name)
                {
                    // update mached machine
                    machine_handle.machines[k].set_update_time(resp_data.machines[i]);
                    // if (!machine_handle.machines[i].checking_data_from_server(resp_data.machines[i]))
                    // {
                    //     nex_send_message("Received data different from local data");
                    // }
                    printf("Reset machine counter now\r\n");
                    machine_reset_counter();
                    break;
                }
            }
        }
    }

    void received_data_callback(EthernetClient &stream)
    {
        int receivedBytes = stream.available();
        memset(http_header.buf, 0, sizeof(http_header.buf));
        stream.readBytesUntil('\r\n', http_header.buf, receivedBytes);
        bool read_data = false;

        printf("New data received: %s", http_header.buf);

        if (strstr(http_header.buf, "200 OK") != nullptr)
        {
            printf("response: Success");
            printf(http_header.buf);
        }

        if (strstr(http_header.buf, "404 Not Found") != nullptr)
        {
            printf("response: Not Found");
            printf(http_header.buf);
        }

        if (strstr(http_header.buf, "400 Bad Request") != nullptr)
        {
            printf("response: Bad Request");
            printf(http_header.buf);
        }

        if (strstr(http_header.buf, "{") != nullptr)
        {
            printf("new data is comming");
            DynamicJsonDocument json_doc = DynamicJsonDocument(1024 * 3);
            DeserializationError JsonErr = deserializeJson(json_doc, http_header.buf);
            printf("parse json object coed: %s\r\n", JsonErr.c_str());

            if (!JsonErr)
            {
                String eop = json_doc.getMember("eop").as<const char *>();
                if (eop == "knife_capture")
                {
                    ethernet_parse_all_received_data(json_doc);
                }
            }
        }
    }

    void local_device_post_data()
    {
        function_log();
        if (!ethernet_handle.cable_connected)
            return;

        printf("Post all local data to server\r\n");
        serialize_local_data();
        char post_url[32]{0};
        memccpy(post_url, "kc_api/post", 0, sizeof(post_url));
        ethernet_handle.make_post_to_server(post_url, http_header.buf);
    }

    void serialize_local_data()
    {
        function_log();
        int LocalMachinesSize = machine_handle.machines.size();
        DynamicJsonDocument json_doc = DynamicJsonDocument(1024 * 5);

        memset(http_header.buf, 0, sizeof(http_header.buf));

        json_doc["KC_DeviceId"] = local_device_id;
        json_doc["EOP"] = "knife_capture";
        json_doc["CurrentTime"] = "";

        JsonArray Machines = json_doc.createNestedArray("Machines");

        printf("Create machines list successed\r\n");
        printf("Local machines list size %d\r\n", LocalMachinesSize);

        for (size_t i = 0; i < LocalMachinesSize; i++)
        {
            JsonObject machine = Machines.createNestedObject();
            machine["MachineName"] = machine_handle.machines[i].m_name;
            machine["LastUpdateTime"] = "";
            JsonObject left = machine.createNestedObject("LeftKnife");
            JsonObject right = machine.createNestedObject("RightKnife");

            left["OldKnife"] = machine_handle.machines[i].left_knife.old_rp_knife;
            left["NewKnife"] = machine_handle.machines[i].left_knife.new_rp_knife;
            right["OldKnife"] = machine_handle.machines[i].right_knife.old_rp_knife;
            right["NewKnife"] = machine_handle.machines[i].right_knife.new_rp_knife;

            printf("Create machine[%d]: success\r\n", i);
        }

        serializeJson(json_doc, http_header.buf);
        printf("Serialize local device data: success\r\n");
        //printf("Serialize Json: %s\r\n",  http_header.buf);
    }
};
#endif