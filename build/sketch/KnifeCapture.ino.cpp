#include <Arduino.h>
#line 1 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino"
// Creator: khale
// Date:    16/2/2020 , 19:37:20
// Author: Techteam
#include "global_config.h"
#include "global_scope.h"
#include "sys_config.h"

inline void user_handler_task_callback_func();
inline void ethernet_handler_task_callback_fnc();
inline void main_task_callback_func();

void main_task(void *pr);
void ethernet_handler_task(void *pr);
void user_handler_task(void *pr);

int CirclePostTime_Ticks = 0;
int EthernetMaintain_Ticks = 0;
bool EthernetMaintainNow = false;
bool DevicePostNow = false;
/******************************************************************************************/
#line 21 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino"
void runtime_timer(TimerHandle_t pxTimer);
#line 39 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino"
void setup();
#line 84 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino"
void loop();
#line 270 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino"
void ethernet_data_received_callback(EthernetClient &stream);
#line 21 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino"
void runtime_timer(TimerHandle_t pxTimer)
{
    pinMode(BUILTIN_LED, !digitalRead(BUILTIN_LED));
    CirclePostTime_Ticks++;
    EthernetMaintain_Ticks++;
    if (EthernetMaintain_Ticks >= 60 * 30)
    {
        EthernetMaintainNow = true;
        EthernetMaintain_Ticks = 0;
    }
    if (CirclePostTime_Ticks >= 60 * 5)
    {
        DevicePostNow = true;
        CirclePostTime_Ticks = 0;
    }
}
/******************************************************************************************/

void setup()
{
    // getting things ready first
    start_up();

    TimerHandle_t Runtime_timer = xTimerCreate("runtime_timer", (1000), pdTRUE, NULL, runtime_timer);

    if (Runtime_timer == NULL)
    {
    }
    else
    {
        if (xTimerStart(Runtime_timer, 0) != pdPASS)
        {
            printf("Can not start request timer\r\n");
        }
        else
            printf("start request timer successfully\r\n");
    }

    sys_create_request_timeout_timer();
    sys_create_confirm_timeout_timer();

    xSemaphore = xSemaphoreCreateCounting(3, 3);

    if (xSemaphore != NULL)
    {
        printf("The semaphore was created successfully\r\n");
        if (xSemaphoreGive(xSemaphore) != pdTRUE)
        {
            // We would expect this call to fail because we cannot give
            // a semaphore without first "taking" it!
            printf("Semaphore is free now\r\n");
        }
    }

    // create all the task
    BaseType_t error = xTaskCreatePinnedToCore(main_task, "main_task", 1024 * 100, NULL, 1, NULL, 1);
    printf("Create main_task code: %d\r\n", error);

    error = xTaskCreatePinnedToCore(user_handler_task, "user_handler_task", 1024 * 20, NULL, 1, NULL, 0);
    printf("Create user_handler_task code: %d\r\n", error);
}

/******************************************************************************************/
void loop()
{
    // all the things are done in tasks
    vTaskSuspend(NULL); // stop this task
}

/******************************************************************************************/
void ethernet_handler_task(void *pr)
{
    if (xSemaphoreTake(xSemaphore, (TickType_t)0))
    {
        printf("ethernet_handler_task has taken the semaphored, semaphore value: %d\r\n", uxSemaphoreGetCount(xSemaphore));

        ethernet_handler_task_callback_fnc();

        vTaskDelete(RequestTaskHandle);
        if (xSemaphoreGive(xSemaphore) == pdTRUE)
        {
            printf("ethernet_handler_task has given the semaphored\r\n");
            printf("Task end\r\n");
        }
    }
}

/******************************************************************************************/
void user_handler_task(void *pr)
{
    int ticks = 0;
    for (;;)
    {
        if (xSemaphoreTake(xSemaphore, (TickType_t)0))
        {
            //printf("user_handler_task has taken the semaphored, semaphore value: %d\r\n",
            //       uxSemaphoreGetCount(xSemaphore));

            user_handler_task_callback_func();
            // check ethernet cable

            if (millis() - ticks > 500)
            {
                ticks = millis();
                knife_capture.ethernet_handle.checking_ethernet_module();
            }

            xSemaphoreGive(xSemaphore);
        }

        vTaskDelay(2);
    }
}

/******************************************************************************************/
void main_task(void *pr)
{
    for (;;)
    {
        if (!InitializeFinish && InitialTimes > 0 && !knife_capture.sys_requesting)
        {
            InitialTimes--;
            printf("Retry to initializes the device\r\n");

            if (RequestTimeOut_TimerHandle == NULL)
            {
                if (!sys_create_request_timeout_timer())
                {
                    knife_capture.checking_reset_counter_request();
                }
            }
            else
            {
                xTimerReset(RequestTimeOut_TimerHandle, (TickType_t)0);
                knife_capture.checking_reset_counter_request();
            }
        }

        if (xSemaphoreTake(xSemaphore, (TickType_t)0))
        {
            //printf("main_task has taken the semaphored, semaphore value: %d\r\n",
            //       uxSemaphoreGetCount(xSemaphore));
            // listen to nextion screen
            //nex_listening();

            // ethernet_waiting for data coming in
            //knife_capture.ethernet_handle.running();
            main_task_callback_func();
        }

        xSemaphoreGive(xSemaphore);

        vTaskDelay(2);
    }
}

/******************************************************************************************/
inline void user_handler_task_callback_func()
{
    nex_listening();

    // ethernet_waiting for data coming in
    knife_capture.ethernet_handle.running();

    if (DevicePostNow)
    {
        printf("Post local data now\r\n");
        DevicePostNow = false;
        knife_capture.local_device_post_data();
    }

    if (EthernetMaintainNow)
    {
        EthernetMaintainNow = false;
        knife_capture.ethernet_handle.ethernet_maintain();
    }
}

inline void ethernet_handler_task_callback_fnc()
{
    function_log();
    // if create timer success, then make new request

    if (RequestTimeOut_TimerHandle == NULL)
    {
        if (!sys_create_request_timeout_timer())
        {
            if (!knife_capture.ethernet_request_next())
            {
                printf("Connect to server failed\r\n");

                BaseType_t excp = xTimerStop(RequestTimeOut_TimerHandle, (TickType_t)0);
                printf("Stop request waiting timer, Excp code: %d\r\n", excp);
            }
        }
    }

    if (RequestTimeOut_TimerHandle != NULL)
    {
        BaseType_t excp = xTimerReset(RequestTimeOut_TimerHandle, (TickType_t)0);
        printf("Reset request waiting timer, Excp code: %d\r\n", excp);
        if (!knife_capture.ethernet_request_next())
        {
            printf("Connect to server failed\r\n");

            BaseType_t excp = xTimerStop(RequestTimeOut_TimerHandle, (TickType_t)0);
            printf("Stop request waiting timer, Excp code: %d\r\n", excp);
        }
    }
}

/******************************************************************************************/
inline void main_task_callback_func()
{
    // if ethernet queue exsit, create new request task
    if (knife_capture.ethernet_queue_available())
    {
        // BaseType_t error = xTaskCreatePinnedToCore(
        //     ethernet_handler_task, "ethernet_handler_task", 1024 * 20, NULL, 1, &RequestTaskHandle, 1);
        // printf("Create ethernet_handler_task code: %d\r\n", error);
        if (RequestTimeOut_TimerHandle == NULL)
        {
            if (!sys_create_request_timeout_timer())
            {
                if (!knife_capture.ethernet_post_capture())
                {
                    printf("Connect to server failed\r\n");

                    BaseType_t excp = xTimerStop(RequestTimeOut_TimerHandle, (TickType_t)0);
                    printf("Stop request waiting timer, Excp code: %d\r\n", excp);
                }
            }
        }

        if (RequestTimeOut_TimerHandle != NULL)
        {
            BaseType_t excp = xTimerReset(RequestTimeOut_TimerHandle, (TickType_t)0);
            printf("Reset request waiting timer, Excp code: %d\r\n", excp);
            if (!knife_capture.ethernet_post_capture())
            {
                printf("Connect to server failed\r\n");

                BaseType_t excp = xTimerStop(RequestTimeOut_TimerHandle, (TickType_t)0);
                printf("Stop request waiting timer, Excp code: %d\r\n", excp);
            }
        }
    }
}

void ethernet_data_received_callback(EthernetClient &stream)
{
    function_log();
    int receivedBytes = stream.available();
    memset(knife_capture.http_header.buf, 0, sizeof(knife_capture.http_header.buf));
    stream.readBytesUntil('\r\n', knife_capture.http_header.buf, receivedBytes);
    bool read_data = false;

    printf("New data received: %s", knife_capture.http_header.buf);

    if (strstr(knife_capture.http_header.buf, "200 OK") != nullptr)
    {
        printf("response: Success\n\n");
        printf(knife_capture.http_header.buf);
        if (knife_capture.knife_capture_submit)
            knife_capture.knife_capture_submit = false;
    }

    if (strstr(knife_capture.http_header.buf, "404 Not Found") != nullptr)
    {
        printf("response: Not Found\n\n");
        printf(knife_capture.http_header.buf);
        if (knife_capture.knife_capture_submit)
            knife_capture.knife_capture_submit = false;
    }

    if (strstr(knife_capture.http_header.buf, "400 Bad Request") != nullptr)
    {
        printf("response: Bad Request\n\n");
        printf(knife_capture.http_header.buf);
        if (knife_capture.knife_capture_submit)
            knife_capture.knife_capture_submit = false;
    }

    if (strstr(knife_capture.http_header.buf, "{") != nullptr)
    {
        printf("new data is comming\r\n");
        DynamicJsonDocument json_doc = DynamicJsonDocument(1024 * 2);
        DeserializationError JsonErr = deserializeJson(json_doc, knife_capture.http_header.buf);
        printf("parse json object code: %s\r\n", JsonErr.c_str());

        if (strstr(knife_capture.http_header.buf, "knife_capture") != NULL)
        {
            if (knife_capture.knife_capture_submit)
                knife_capture.knife_capture_submit = false;
        }

        if (!JsonErr)
        {
            String eop = json_doc.getMember("EOP").as<const char *>();
            char mesg[100]{0};
            String CurrentTime = "";

            if (eop == "knife_capture")
            {
                printf("Reviced new Json data\r\n");
                CurrentTime = json_doc.getMember("CurrentTime").as<const char *>();
                snprintf(mesg, sizeof(mesg), "Last request time: %s", CurrentTime.c_str());
                nex_send_message(mesg);

                knife_capture.ethernet_parse_all_received_data(json_doc);

                printf("Remove request from queue\r\n");
                knife_capture.request_list.pop();

                printf("Post local data\r\n");
                knife_capture.local_device_post_data();

                knife_capture.knife_capture_submit = false;
            }

            if (eop == "kc_initial") // initial resp
            {
                InitializeFinish = true;
                printf("Reviced new Json data\r\n");
                CurrentTime = json_doc.getMember("CurrentTime").as<const char *>();
                snprintf(mesg, sizeof(mesg), "Last initializes time: %s", CurrentTime.c_str());
                nex_send_message(mesg);

                knife_capture.ethernet_parse_all_received_data(json_doc);

                printf("Post local data now\r\n");
                knife_capture.local_device_post_data();
            }

            if (eop == "post_resp") // post
            {
                printf("Reviced new Json data\r\n");

                CurrentTime = json_doc.getMember("CurrentTime").as<const char *>();
                snprintf(mesg, sizeof(mesg), "Last post time: %s", CurrentTime.c_str());
                nex_send_message(mesg);
            }
        }
    }
    printf("Stop request timer\r\n");
    knife_capture.sys_requesting = false;
    BaseType_t excp = xTimerStop(RequestTimeOut_TimerHandle, (TickType_t)0);
    printf("Stop request timeout timer, Excp code: %d\r\n", excp);
}

