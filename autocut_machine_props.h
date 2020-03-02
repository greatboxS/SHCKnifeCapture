#ifndef autocut_machine_props_h
#define autocut_machine_props_h
#include <EEPROM.h>
#include "global_config.h"
#include "global_scope.h"

struct knife_head_def
{
    int old_rp_knife = 0; // old replace knife
    int new_rp_knife = 0; // new replace knife
    void reset()
    {
        old_rp_knife = 0;
        new_rp_knife = 0;
    }

    // false not equal
    // true equal
    bool operator==(knife_head_def &knife_head)
    {
        if (old_rp_knife != knife_head.old_rp_knife)
            return false;
        if (new_rp_knife != knife_head.new_rp_knife)
            return false;

        return true;
    }
};

typedef struct machine_resp_info
{
    char machine_name[20];
    knife_head_def left_knife, right_knife;
    char last_update_time[64];
    bool reset_counter = false;

    machine_resp_info()
    {
        clear();
    }

    machine_resp_info(JsonObject &json_doc)
    {
        clear();
        json_parse_member(json_doc);
    }

    void clear()
    {
        left_knife.reset();
        right_knife.reset();
        memset(machine_name, 0, sizeof(machine_name));
        memset(last_update_time, 0, sizeof(last_update_time));
    }

    void json_parse_member(JsonDocument &json_doc)
    {
        function_log();
        JsonParse_Element(json_doc, "ResetCounter", reset_counter);
        JsonObject left = json_doc.getMember("LeftKnife");
        JsonObject right = json_doc.getMember("RightKnife");

        JsonParse_Element(json_doc, "LastUpdateTime", last_update_time, sizeof(last_update_time));
        JsonParse_Element(json_doc, "MachineName", machine_name, sizeof(machine_name));

        JsonParse_Element(left, "NewKnife", left_knife.old_rp_knife);
        JsonParse_Element(left, "OldKnife", left_knife.old_rp_knife);
        JsonParse_Element(right, "OldKnife", right_knife.old_rp_knife);
        JsonParse_Element(right, "NewKnife", right_knife.new_rp_knife);
    }

    void json_parse_member(JsonObject &json_object)
    {
        function_log();
        printf("Get knife object\r\n");
        JsonParse_Element(json_object, "ResetCounter", reset_counter);
        JsonObject left = json_object.getMember("LeftKnife");
        JsonObject right = json_object.getMember("RightKnife");

        JsonParse_Element(json_object, "LastUpdateTime", last_update_time, sizeof(last_update_time));
        JsonParse_Element(json_object, "MachineName", machine_name, sizeof(machine_name));

        JsonParse_Element(left, "NewKnife", left_knife.old_rp_knife);
        JsonParse_Element(left, "OldKnife", left_knife.old_rp_knife);
        JsonParse_Element(right, "OldKnife", right_knife.old_rp_knife);
        JsonParse_Element(right, "NewKnife", right_knife.new_rp_knife);

        printf("ResetCounter %d, MachineName %s, LastUpdateTime %s\r\n", reset_counter, machine_name, last_update_time);
        printf("LeftKnife old %d, new %d\r\n", left_knife.old_rp_knife, left_knife.new_rp_knife);
        printf("RightKnife old %d, new %d\r\n", left_knife.old_rp_knife, left_knife.new_rp_knife);
    }
};
// typedef resp_type
// {
//     //machine_name
//     //left_knife
//         // old
//         // new
//     //right_knife
//     //las_update_time
//     //eop knife_capture
// };

typedef struct knife_capture_resp_info
{
    std::vector<machine_resp_info> machines;
    // bool reset_machine_counter = false;
    String eop;

    void clear()
    {
        machines.clear();
        // reset_machine_counter = false;
        eop = "";
    }
    void json_parse_resp_data(JsonDocument &json_doc)
    {
        function_log();
        clear();
        JsonArray jsonArr = json_doc.getMember("Machines");
        size_t size = jsonArr.size();
        printf("Machines size = %d\r\n", size);
        //JsonParse_Element(json_doc, "ResetCounter", reset_machine_counter);

        for (size_t i = 0; i < size; i++)
        {
            JsonObject machineJsonObj = jsonArr[i];
            printf("Get Array element OK\r\n");
            machine_resp_info resp_machine(machineJsonObj);
            machines.push_back(resp_machine);
        }
    }
};

struct request_def
{
    char machine_name[16]{0};
    int knife_position = 0; //0left 1right
    int knife_type = 0;     //0old 1new
    int retry_time = 5;
    int knife_picked = 0;
    char url[64]{0};
    bool valid = false;
    void clear()
    {
        memset(machine_name, 0, sizeof(machine_name));
        memset(url, 0, sizeof(url));
        knife_position = 0; //0left 1right
        knife_type = 0;     //0old 1new
        retry_time = 5;
        valid = false;
    }

    void update_request_pr(char *name, int device_id, int pos, int type,int knife_picker, int local_value )
    {
        function_log();

        printf("Counter value %d\r\n", local_value);
        valid = true;
        memccpy(machine_name, name, 0, sizeof(machine_name));
        knife_position = pos;
        knife_type = type;
        knife_picked = knife_picker;
        // submit type
        // machine_name/knife_position/knife_type/knife_picked/local_value
        snprintf(url, sizeof(url), "kc_api/%s/%d/%d/%d/%d/%d", machine_name, device_id, knife_position, knife_type, knife_picked, local_value);
        printf("update new url: %s\rn", url);
    }
};

struct autocut_machine_def
{
private:
    int machine_name_addr;
    int last_update_addr;
    int left_knife_addr;
    int right_knife_addr;
    int max_storage_size = 96;
    int storage_addr = 0;

    void eeprom_read_last_update_time(int addr)
    {
        function_log();
        EEPROM.begin(EEPROM_MAX_SIZE);
        for (size_t i = 0; i < sizeof(last_update_time); i++)
        {
            last_update_time[i] = eeprom_read<char>(addr + i);
        }
        EEPROM.end();
    }

    void eeprom_save_last_update_time(int addr)
    {
        function_log();
        EEPROM.begin(EEPROM_MAX_SIZE);
        for (size_t i = 0; i < sizeof(last_update_time); i++)
        {
            eeprom_write<char>(last_update_time[i], addr + i);
        }
        EEPROM.end();
    }
    //********************************************************************
    void eeprom_save_machine_name(int addr)
    {
        function_log();
        EEPROM.begin(EEPROM_MAX_SIZE);
        for (size_t i = 0; i < sizeof(m_name); i++)
        {
            eeprom_write<char>(m_name[i], addr + i);
        }
        EEPROM.end();
    }

    void eeprom_read_machine_name(int addr)
    {
        function_log();
        EEPROM.begin(EEPROM_MAX_SIZE);
        for (size_t i = 0; i < sizeof(m_name); i++)
        {
            m_name[i] = eeprom_read<char>(addr + i);
        }
        EEPROM.end();
    }
    //********************************************************************
    void eeprom_save_knife_head(knife_head_def &knife_head, int addr)
    {
        function_log();
        EEPROM.begin(EEPROM_MAX_SIZE);
        // new rp knife value
        eeprom_write<int>(knife_head.new_rp_knife, addr);
        // old rp knife value
        eeprom_write<int>(knife_head.old_rp_knife, addr + 4);
        EEPROM.end();
    }

    void eeprom_read_knife_head(knife_head_def &knife_head, int addr)
    {
        function_log();
        EEPROM.begin(EEPROM_MAX_SIZE);
        // new rp knife value
        knife_head.new_rp_knife = eeprom_read<int>(addr);
        // old rp knife value
        knife_head.old_rp_knife = eeprom_read<int>(addr + 4);
        EEPROM.end();
    }

public:
    knife_head_def left_knife, right_knife;
    char last_update_time[64]{0};
    char m_name[MACHINE_MAX_NAME_LENGTH]{0}; // machine name exp: L1
    bool reset_counter = false;
    // storage specifications:
    // machine_name storage_addr
    // last_update_time storage_addr+sizeof(machine_name)
    // left_knife =  storage_addr+sizeof(last_update_addr)
    // right_knife = left_knife + sizeof(left_knife)

    autocut_machine_def()
    {
        left_knife.reset();
        right_knife.reset();
    }
    // eeprom_addr : the address of name's storage
    autocut_machine_def(int eeprom_addr)
    {
        printf("new autocut_machine_initialized\r\n");
        storage_addr = eeprom_addr;
        storage_address_specify(eeprom_addr);
        initializes();
    }

    autocut_machine_def(int eeprom_addr, int place, char *machine_name)
    {
        printf("New autocut_machine_initialized\r\n");
        storage_address_specify(eeprom_addr);
        update_machine_name(machine_name);
    }

    bool operator==(const char *machine_name)
    {
        if (strncmp(m_name, machine_name, sizeof(m_name)) == 0)
            return true;
        else
            return false;
    }

    void relocate(int eeprom_addr)
    {
        function_log();
        printf("Relocate machine in eeprom\r\n");
        storage_address_specify(eeprom_addr);
        backup_all_data();
    }

    void storage_address_specify(int addr)
    {
        function_log();
        // storage specifications:
        // machine_name storage_addr
        // last_update_time storage_addr+sizeof(machine_name)
        // left_knife storage_addr+sizeof
        // right_knife
        storage_addr = addr;
        machine_name_addr = storage_addr;
        last_update_addr = machine_name_addr + sizeof(m_name);
        left_knife_addr = last_update_addr + sizeof(last_update_time);
        right_knife_addr = left_knife_addr + 8;
        printf("EEprom specified: [machine %d], [time %d], [left %d], [right %d]\r\n",
               machine_name_addr, last_update_addr, left_knife_addr, right_knife_addr);
    }

    void update_parameters(int eeprom_addr, char *machine_name)
    {
        printf("update machine parameters\r\n");
        storage_addr = eeprom_addr;
        printf("Machine eeprom address %d\r\n", eeprom_addr);
        storage_address_specify(eeprom_addr);
        update_machine_name(machine_name);
        left_knife.reset();
        right_knife.reset();
        backup_all_data();
    }

    // 0 left, 1 right
    //0 old, 1 new
    //return current counter value
    int new_knife_changed(int pos, int type)
    {
        int current_counter_value = 0;
        switch (pos)
        {
        case 0: //left
            if (type)
            {
                // new knife
                left_knife.new_rp_knife++;
                current_counter_value = left_knife.new_rp_knife;
            }
            else
            {
                left_knife.old_rp_knife++;
                current_counter_value = left_knife.old_rp_knife;
            }
            eeprom_save_knife_head(0);
            break;

        case 1: //right
            if (type)
            {
                // new knife
                right_knife.new_rp_knife++;
                current_counter_value = right_knife.new_rp_knife;
            }
            else
            {
                right_knife.old_rp_knife++;
                current_counter_value = right_knife.old_rp_knife;
            }
            eeprom_save_knife_head(1);
            break;
        }

        return current_counter_value;
    }

    // reset value to 0 and save them
    void reset_local_data(bool force = false)
    {
        if (reset_counter || force)
        {
            reset_counter = false;
            left_knife.reset();
            right_knife.reset();
            eeprom_save_all_knife_head();
        }
    }

    void update_data(JsonDocument json_doc)
    {
        JsonParse_Element(json_doc, "machine_name", m_name, sizeof(m_name));
        JsonParse_Element(json_doc, "last_update_time", last_update_time, sizeof(last_update_time));
        JsonObject left = json_doc.getMember("left_knife");
        JsonObject right = json_doc.getMember("right_knife");

        JsonParse_Element(left, "old", left_knife.old_rp_knife);
        JsonParse_Element(left, "new", left_knife.new_rp_knife);

        JsonParse_Element(right, "old", right_knife.old_rp_knife);
        JsonParse_Element(right, "new", right_knife.new_rp_knife);

        backup_all_data();
    }

    void set_update_time(machine_resp_info &rec_machine)
    {
        function_log();
        memccpy(last_update_time, rec_machine.last_update_time, 0, sizeof(last_update_time));
        reset_counter = rec_machine.reset_counter;
        eeprom_save_last_update_time();
    }

    bool checking_data_from_server(machine_resp_info &rec_machine)
    {
        memccpy(last_update_time, rec_machine.last_update_time, 0, sizeof(last_update_time));
        bool result = (left_knife == rec_machine.left_knife && right_knife == rec_machine.left_knife);
        reset_counter = rec_machine.reset_counter;
        return result;
        // if (left_knife == rec_machine.left_knife)
        // {
        //     // equal then save received data
        //     // dont need to do anything
        // }
        // else
        // {
        //     //nex_send_message("Received data different from local data");
        // }
    }

    void update_machine_name(char *name)
    {
        memccpy(m_name, name, 0, sizeof(m_name));
    }

    void nx_display_machine(int slot)
    {
        function_log();
        printf("Update machine %s at slot %d\r\n", m_name, slot);

        switch (slot)
        {
        case 0:
            setStringProperty("SCREEN1", "t0.txt", m_name);
            setStringProperty("SCREEN1", "t3.txt", last_update_time);

            setNumberProperty("SCREEN1", "n0.val", left_knife.new_rp_knife);
            setNumberProperty("SCREEN1", "n1.val", right_knife.new_rp_knife);
            setNumberProperty("SCREEN1", "n6.val", left_knife.old_rp_knife);
            setNumberProperty("SCREEN1", "n7.val", right_knife.old_rp_knife);
            break;

        case 1:
            setStringProperty("SCREEN1", "t1.txt", m_name);
            setStringProperty("SCREEN1", "t4.txt", last_update_time);

            setNumberProperty("SCREEN1", "n2.val", left_knife.new_rp_knife);
            setNumberProperty("SCREEN1", "n3.val", right_knife.new_rp_knife);
            setNumberProperty("SCREEN1", "n8.val", left_knife.old_rp_knife);
            setNumberProperty("SCREEN1", "n9.val", right_knife.old_rp_knife);
            break;

        case 2:
            setStringProperty("SCREEN1", "t2.txt", m_name);
            setStringProperty("SCREEN1", "t5.txt", last_update_time);

            setNumberProperty("SCREEN1", "n4.val", left_knife.new_rp_knife);
            setNumberProperty("SCREEN1", "n5.val", right_knife.new_rp_knife);
            setNumberProperty("SCREEN1", "n10.val", left_knife.old_rp_knife);
            setNumberProperty("SCREEN1", "n11.val", right_knife.old_rp_knife);
            break;

        default:
            break;
        }
    }

    void initializes()
    {
        function_log();
        // read the machine name from eeprom
        eeprom_read_machine_name();
        //read knife head properties
        eeprom_read_all_knife_head();
        // read the last update time
        eeprom_read_last_update_time();
    }

    void backup_all_data()
    {
        function_log();
        //save machine name
        eeprom_save_machine_name();
        // save knife head
        eeprom_save_all_knife_head();
        // save last_update_time
        eeprom_save_last_update_time();
    }

    void eeprom_save_machine_name()
    {
        function_log();
        eeprom_save_machine_name(machine_name_addr);
    }

    void eeprom_save_last_update_time()
    {
        function_log();
        eeprom_save_last_update_time(last_update_addr);
    }

    void eeprom_save_left_knife_head()
    {
        function_log();
        eeprom_save_knife_head(left_knife, left_knife_addr);
    }

    void eeprom_save_right_knife_head()
    {
        function_log();
        eeprom_save_knife_head(right_knife, right_knife_addr);
    }

    // read informations to eeprom
    void eeprom_read_machine_name()
    {
        function_log();
        eeprom_read_machine_name(machine_name_addr);
    }

    void eeprom_read_last_update_time()
    {
        function_log();
        eeprom_read_last_update_time(last_update_addr);
    }

    void eeprom_read_left_knife_head()
    {
        function_log();
        eeprom_read_knife_head(left_knife, left_knife_addr);
    }

    void eeprom_read_right_knife_head()
    {
        function_log();
        eeprom_read_knife_head(right_knife, right_knife_addr);
    }

    void eeprom_save_knife_head(int pos)
    {
        function_log();
        switch (pos)
        {
        case 0: //left
            // save left knife head
            eeprom_save_left_knife_head();
            break;

        case 1: //right
            //save right knife head
            eeprom_save_right_knife_head();
            break;
        }
    }

    void eeprom_read_all_knife_head()
    {
        function_log();
        eeprom_read_left_knife_head();
        eeprom_read_right_knife_head();
    }

    void eeprom_save_all_knife_head()
    {
        function_log();
        eeprom_save_left_knife_head();
        eeprom_save_right_knife_head();
    }
};

struct machine_handler
{
    int total_machine = MAX_MACHINE_PER_PAGE;
    int current_page = 0;
    int storage_address = 0;
    std::vector<autocut_machine_def> machines;
    const uint8_t max_machine_size = 96;
    request_def new_request;

    //
    void add_new_machine(char *new_machine_name)
    {
        function_log();

        int current_place = machines.size();
        printf("machines.size() = %d\r\n", current_place);

        printf("Update new machine name %s\r\n", new_machine_name);

        int addr = 0;
        autocut_machine_def new_machine;

        if (current_place >= total_machine)
        {
            new_machine.update_machine_name(new_machine_name);
            machines.insert(machines.begin(), new_machine);
            machines.pop_back();
            printf("Insert new machine name: %s, machines.size() = %d\r\n", new_machine_name, machines.size());
        }
        else
        {
            new_machine.update_machine_name(new_machine_name);
            machines.push_back(new_machine);
            printf("Add new machine name: %s, machines.size() = %d\r\n", new_machine_name, machines.size());
        }

        relocate_machine_storage_addr();
        setStringProperty("SETTING", "t22.txt", "Success");
    }

    void relocate_machine_storage_addr()
    {
        function_log();
        for (size_t i = 0; i < machines.size(); i++)
        {
            int current_eeprom_addr = max_machine_size * i + MACHINE_START_ADDR + sizeof(total_machine);
            machines[i].relocate(current_eeprom_addr);
        }
    }

    // constructure new machines handler
    machine_handler(int storage_addr)
    {
        function_log();
        storage_address = storage_addr;
    }

    autocut_machine_def *get_display_machine(int current_page, int slot)
    {
        function_log();
        uint8_t index = (current_page - 1) * MAX_MACHINE_PER_PAGE + slot;
        if (index >= machines.size())
        {
            printf("Can not get display machine\r\n");
            return NULL;
        }
        else
            return &machines[index];
    }

    void nx_clear_machine_screen()
    {
        function_log();
        setStringProperty("SCREEN1", "t0.txt", "");
        setStringProperty("SCREEN1", "t3.txt", "");

        setNumberProperty("SCREEN1", "n0.val", 0);
        setNumberProperty("SCREEN1", "n1.val", 0);
        setNumberProperty("SCREEN1", "n6.val", 0);
        setNumberProperty("SCREEN1", "n7.val", 0);

        setStringProperty("SCREEN1", "t1.txt", "");
        setStringProperty("SCREEN1", "t4.txt", "");

        setNumberProperty("SCREEN1", "n2.val", 0);
        setNumberProperty("SCREEN1", "n3.val", 0);
        setNumberProperty("SCREEN1", "n8.val", 0);
        setNumberProperty("SCREEN1", "n9.val", 0);

        setStringProperty("SCREEN1", "t2.txt", "");
        setStringProperty("SCREEN1", "t5.txt", "");

        setNumberProperty("SCREEN1", "n4.val", 0);
        setNumberProperty("SCREEN1", "n5.val", 0);
        setNumberProperty("SCREEN1", "n10.val", 0);
        setNumberProperty("SCREEN1", "n11.val", 0);
    }

    void nx_display_machine(int slot, autocut_machine_def *machine)
    {
        function_log();
        if (machine == NULL)
        {
            printf("Can not display null machine\r\n");
            return;
        }

        printf("Update machine %s at slot %d\r\n", machine->m_name, slot);
        //clear screen first
        switch (slot)
        {
        case 0:
            setStringProperty("SCREEN1", "t0.txt", machine->m_name);
            setStringProperty("SCREEN1", "t3.txt", machine->last_update_time);

            setNumberProperty("SCREEN1", "n0.val", machine->left_knife.new_rp_knife);
            setNumberProperty("SCREEN1", "n1.val", machine->right_knife.new_rp_knife);
            setNumberProperty("SCREEN1", "n6.val", machine->left_knife.old_rp_knife);
            setNumberProperty("SCREEN1", "n7.val", machine->right_knife.old_rp_knife);
            break;

        case 1:
            setStringProperty("SCREEN1", "t1.txt", machine->m_name);
            setStringProperty("SCREEN1", "t4.txt", machine->last_update_time);

            setNumberProperty("SCREEN1", "n2.val", machine->left_knife.new_rp_knife);
            setNumberProperty("SCREEN1", "n3.val", machine->right_knife.new_rp_knife);
            setNumberProperty("SCREEN1", "n8.val", machine->left_knife.old_rp_knife);
            setNumberProperty("SCREEN1", "n9.val", machine->right_knife.old_rp_knife);
            break;

        case 2:
            setStringProperty("SCREEN1", "t2.txt", machine->m_name);
            setStringProperty("SCREEN1", "t5.txt", machine->last_update_time);

            setNumberProperty("SCREEN1", "n4.val", machine->left_knife.new_rp_knife);
            setNumberProperty("SCREEN1", "n5.val", machine->right_knife.new_rp_knife);
            setNumberProperty("SCREEN1", "n10.val", machine->left_knife.old_rp_knife);
            setNumberProperty("SCREEN1", "n11.val", machine->right_knife.old_rp_knife);
            break;

        default:
            break;
        }
    }

    //
    int get_page_number(bool last_page)
    {
        if (last_page)
        {
            if (machines.size() == 0)
                return 1;
            else
                return (machines.size() - 1) / MAX_MACHINE_PER_PAGE + 1;
        }
        else
            return 1; // return first page
    }

    void nx_display_new_page(int page_number)
    {
        function_log();

        nx_clear_machine_screen();

        nex_set_page_number(page_number);

        for (size_t i = 0; i < MAX_MACHINE_PER_PAGE; i++)
        {
            autocut_machine_def *machine = get_display_machine(page_number, i);
            if (machine != NULL)
            {
                nx_display_machine(i, machine);
            }
        }
    }

    // increase the value of changed knifes
    // result: @ request_def, new request handler
    request_def &new_knife_captured(int machine_slot, int current_page, int device_id, int pos, int type, int knife_picker)
    {
        function_log();
        new_request.clear();

        autocut_machine_def *current_machine = get_display_machine(current_page, machine_slot);

        if (current_machine == NULL)
            return new_request;

        int current_local_value = current_machine->new_knife_changed(pos, type);

        new_request.update_request_pr(current_machine->m_name, device_id, pos, type, knife_picker, current_local_value);

        return new_request;
    }

    void machine_force_reset_data()
    {
        for (size_t i = 0; i < machines.size(); i++)
        {
            machines[i].reset_local_data(true);
        }
    }

    // reset all local data, and begin new week or new month
    void machine_reset_data()
    {
        for (size_t i = 0; i < machines.size(); i++)
        {
            machines[i].reset_local_data();
        }
    }

    // setting , save total of machine number
    void update_total_machine_num(int num)
    {
        if (num > MAX_MACHINE_NUM || num < 0)
        {
            printf("invalid total machine number, rewrite this value\r\n");
            total_machine = MAX_MACHINE_PER_PAGE;
            eeprom_save_total_machine();
        }
        else
        {
            total_machine = num;
        }

        eeprom_save_total_machine();
        eeprom_read_machines();
    }

    void eeprom_save_total_machine()
    {
        epprom_save_total_machine(storage_address);
    }

    void eeprom_read_total_machine()
    {
        epprom_read_total_machine(storage_address);
    }

    void eeprom_save_machines(autocut_machine_def &machine)
    {
        machine.backup_all_data();
    }

    void eeprom_read_machines()
    {
        function_log();
        machines.clear();
        for (size_t i = 0; i < total_machine; i++)
        {
            int current_eeprom_addr = max_machine_size * i + MACHINE_START_ADDR + sizeof(total_machine);
            autocut_machine_def machine(current_eeprom_addr);
            machines.push_back(machine);
        }
    }

private:
    void epprom_read_total_machine(int address)
    {
        function_log();
        EEPROM.begin(EEPROM_MAX_SIZE);
        int temp_total_machine = eeprom_read<int>(address);
        update_total_machine_num(temp_total_machine);
        EEPROM.end();
    }

    void epprom_save_total_machine(int address)
    {
        function_log();
        EEPROM.begin(EEPROM_MAX_SIZE);
        eeprom_write<int>(total_machine, address);
        EEPROM.end();
    }
};
#endif