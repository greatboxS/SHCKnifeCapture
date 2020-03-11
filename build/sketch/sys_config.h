#ifndef sys_config_h
#define sys_config_h
#include <EEPROM.h>
#include "global_config.h"
#include "global_scope.h"
#include "knife_capture_handler.h"

SemaphoreHandle_t xSemaphore;
TaskHandle_t RequestTaskHandle;

TimerHandle_t RequestTimeOut_TimerHandle;
TimerHandle_t ConfirmTimeOut_TimerHandle;

volatile bool InitializeFinish = false;
volatile bool InitialRequestNow = false;
int InitialTimes = 2;

void confirm_timeout_timer(TimerHandle_t pxTimer);
void request_timeout_timer(TimerHandle_t pxTimer);

bool sys_create_confirm_timeout_timer();
bool sys_create_request_timeout_timer();

knife_capture_class knife_capture;

void ethernet_data_received_callback(EthernetClient &stream);

// nextion callback fucntions
void nx_knife_capture_submit(uint8_t but_id);
void PAGE_LOADING_EVENT_CALLBACK(uint8_t pageId, uint8_t componentId, uint8_t eventType);

void start_up();
void io_init();
void ethernet_init();
void local_data_init();

void start_up()
{
    io_init();
    PAGE_LOADING_EVENT.root_attachCallback(PAGE_LOADING_EVENT_CALLBACK);
    local_data_init();
    ethernet_init();
}

void io_init()
{
    Serial.begin(115200);
    while (!Serial)
        ;
    printf("Initializes system IO\r\n");
    pinMode(GPIO_NUM_13, OUTPUT);
    digitalWrite(GPIO_NUM_13, HIGH);
    nex_init();
}

void ethernet_init()
{
    printf("Initializing ethernet module\r\n");
    nex_send_message("Setting up ethernet module...");
    knife_capture.ethernet_handle.init(ETHERNET_CS_PIN, ETHERNET_RST_PIN, ethernet_data_received_callback);
    //initialize ethernet module
    int exception = knife_capture.ethernet_handle.setting_up_ethernet_module();
    printf("Initialize ethernet module exception code: %d\r\n", exception);
    nex_send_message("Setting up done");
}

void local_data_init()
{
    nex_send_message("Initializing local device data...");
    printf("Initializing local system data\r\n");
    knife_capture.init();
    nex_send_message("Local data initialized successfully");
}

void PAGE_LOADING_EVENT_CALLBACK(uint8_t pageId, uint8_t componentId, uint8_t eventType)
{
    function_log();
    printf("Nextion event callback:\r\nPage: %d, ComponentId: %d\r\n", pageId, componentId);

    switch (pageId)
    {
    case 0: //home page
        if (componentId >= 0x00 && componentId <= 0x0B)
        {
            nx_knife_capture_submit(componentId);
        }

        if (componentId == NEX_BUT_SETTING)
        {
            nex_goto_page("SETTING");
            knife_capture.nx_update_new_setting_screen();
        }

        if (componentId == NEX_PAGE_INIT)
        {
            //update screen
            knife_capture.nx_update_home_screen();
        }

        if (componentId == NEX_BUT_NEXT)
        {
            // go to next page if machines list is more than 3
            knife_capture.nx_display_next_page(true);
        }

        if (componentId == NEX_BUT_BACK)
        {
            // go to privious page if machines list is more than 3
            knife_capture.nx_display_next_page(false);
        }

        break;

    case 1: //page setting1
        if (componentId == BUT_RESET_MODULE)
            esp_restart();

        if (componentId == NEX_PAGE_INIT)
            knife_capture.nx_update_machine_list();

        if (componentId == BUT_SAVE_TOTAL_MACHINE)
        {
            knife_capture.nx_save_total_machine_num();
            knife_capture.nx_update_machine_list();
        }

        if (componentId == BUT_SAVE_MACHINE_NAME)
        {
            knife_capture.nx_save_new_machine();
            // display the list of saved machines
            knife_capture.nx_update_machine_list();
        }

        if (componentId == BUT_SAVE_LOCAL_DEVICE_ID)
            knife_capture.nx_save_local_id();

        if (componentId == BUT_GOTO_SETTING2)
        {
            knife_capture.nx_update_setting2_screen();
            nex_goto_page("SETTING2");
        }
        break;

    case 2: // page setting2
        if (componentId == BUT_SAVE_MAC)
            knife_capture.nx_save_local_mac_id();

        if (componentId == BUT_SAVE_SERVER_IP)
            knife_capture.nx_save_server_info();

        if (componentId == BUT_RESET_MACHINE_COUNTER)
        {
            knife_capture.machine_handle.machine_force_reset_data();
        }

        break;

    case 7: // recheck page// check for the confirmation after click but
        if (knife_capture.submit_recheck_flag)
        {

            if (componentId == BUT_NO)
            {
                knife_capture.submit_recheck_flag = false;
                nex_goto_page("SCREEN1");
                BaseType_t excp = xTimerStop(ConfirmTimeOut_TimerHandle, (TickType_t)0);
                printf("Stop confirm timeout Timer, Excp code: %d\r\n", excp);
            }

            if (componentId < 0xEA && componentId >= 0xE0)
            {
                bool right_pick = false;
                componentId &= 0x0F;

                if (knife_capture.local_device_id < 10 && componentId < 4)
                    right_pick = true;

                if (knife_capture.local_device_id >= 10 && componentId >= 4)
                    right_pick = true;

                if (right_pick)
                {
                    knife_capture.knife_picker = componentId + 1;
                    knife_capture.add_new_request_to_queue();
                    nex_goto_page("SCREEN1");
                    BaseType_t excp = xTimerStop(ConfirmTimeOut_TimerHandle, (TickType_t)0);
                    printf("Stop confirm timeout Timer, Excp code: %d\r\n", excp);
                }
            }
        }
        break;
    }
}

// /machine_name/lef-right/old-new
void nx_knife_capture_submit(uint8_t but_id)
{
    int machine_slot = 0;
    int knife_position = 0; // 0 left, 1 right
    int knife_type = 0;     //0 old, 1 new

    switch (but_id)
    {
        //slot 1
    case NEX_BUT_1_NEW_LEFT:
        machine_slot = 0;
        knife_position = 0;
        knife_type = 1;
        break;
    case NEX_BUT_1_OLD_LEFT:
        machine_slot = 0;
        knife_position = 0;
        knife_type = 0;
        break;
    case NEX_BUT_1_NEW_RIGHT:
        machine_slot = 0;
        knife_position = 1;
        knife_type = 1;
        break;
    case NEX_BUT_1_OLD_RIGHT:
        machine_slot = 0;
        knife_position = 1;
        knife_type = 0;
        break;

        //slot 2
    case NEX_BUT_2_NEW_LEFT:
        machine_slot = 1;
        knife_position = 0;
        knife_type = 1;
        break;
    case NEX_BUT_2_OLD_LEFT:
        machine_slot = 1;
        knife_position = 0;
        knife_type = 0;
        break;
    case NEX_BUT_2_NEW_RIGHT:
        machine_slot = 1;
        knife_position = 1;
        knife_type = 1;
        break;
    case NEX_BUT_2_OLD_RIGHT:
        machine_slot = 1;
        knife_position = 1;
        knife_type = 0;
        break;

        //slot 3
    case NEX_BUT_3_NEW_LEFT:
        machine_slot = 2;
        knife_position = 0;
        knife_type = 1;
        break;
    case NEX_BUT_3_OLD_LEFT:
        machine_slot = 2;
        knife_position = 0;
        knife_type = 0;
        break;
    case NEX_BUT_3_NEW_RIGHT:
        machine_slot = 2;
        knife_position = 1;
        knife_type = 1;
        break;
    case NEX_BUT_3_OLD_RIGHT:
        machine_slot = 2;
        knife_position = 1;
        knife_type = 0;
        break;
    }

    printf("New knife captured: slot=%d,pos=%d, type=%d\r\n",
           machine_slot, knife_position, knife_type);

    knife_capture.knife_position = knife_position;
    knife_capture.knife_type = knife_type;
    knife_capture.machine_slot = machine_slot;

    // check the confirmaion
    if (ConfirmTimeOut_TimerHandle == NULL)
    {
        if (!sys_create_confirm_timeout_timer())
        {
            setStringProperty("SCREEN1", "t6.txt", "Error: Please press button again. Thank you");
        }
    }
    else
    {
        xTimerReset(ConfirmTimeOut_TimerHandle, (TickType_t)0);
    }
    nex_goto_page("KNIFE_PICKER");
    knife_capture.submit_recheck_flag = true;
}

bool sys_create_confirm_timeout_timer()
{
    //create 2s timer to check the timeout of button clicked
    ConfirmTimeOut_TimerHandle = xTimerCreate("ConfirmTimeOut_TimerHandle", (10000), pdTRUE, NULL, confirm_timeout_timer);

    if (ConfirmTimeOut_TimerHandle == NULL)
    {
        printf("Failded to initialize root Timer\r\n");
        return false;
    }
    else
    {
        // Start the timer.  No block time is specified, and even if one was
        // it would be ignored because the scheduler has not yet been
        // started.
        if (xTimerStart(ConfirmTimeOut_TimerHandle, 0) != pdPASS)
        {
            printf("Can not start root timer\r\n");
            return false;
        }
        else
        {
            printf("start root timer successfully\r\n");
            return true;
        }
    }
}

// true =  success
// false = failded
bool sys_create_request_timeout_timer()
{
    // create 10s timer to check the request timeout
    RequestTimeOut_TimerHandle = xTimerCreate("RequestTimeOut_TimerHandle", (10000), pdTRUE, NULL, request_timeout_timer);

    if (RequestTimeOut_TimerHandle == NULL)
    {
        printf("failded to initialize root Timer\r\n");
        return false;
    }
    else
    {
        // Start the timer.  No block time is specified, and even if one was
        // it would be ignored because the scheduler has not yet been
        // started.
        if (xTimerStart(RequestTimeOut_TimerHandle, 0) != pdPASS)
        {
            printf("Can not start root timer\r\n");
            return false;
        }
        else
        {
            printf("start root timer successfully\r\n");
            return true;
        }
    }
}

void request_timeout_timer(TimerHandle_t pxTimer)
{
    function_log();
    knife_capture.sys_requesting = false;
    if (!InitializeFinish)
        InitialRequestNow = true;
    xTimerStop(pxTimer, (TickType_t)0);
}

void confirm_timeout_timer(TimerHandle_t pxTimer)
{
    function_log();
    knife_capture.submit_recheck_flag = false;
    xTimerStop(pxTimer, (TickType_t)0);
}

#endif