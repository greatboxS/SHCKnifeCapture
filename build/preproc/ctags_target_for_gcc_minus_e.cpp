# 1 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino"
// Creator: khale
// Date:    16/2/2020 , 19:37:20
// Author: Techteam
# 5 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino" 2
# 6 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino" 2
# 7 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino" 2

inline void user_handler_task_callback_func();
inline void ethernet_handler_task_callback_fnc();
inline void main_task_callback_func();

void main_task(void *pr);
void ethernet_handler_task(void *pr);
void user_handler_task(void *pr);

int CirclePostTime_Ticks = 0;
bool DevicePostNow = false;
/******************************************************************************************/
void runtime_timer(TimerHandle_t pxTimer)
{
    pinMode(LED_BUILTIN /* backward compatibility*/, !digitalRead(LED_BUILTIN /* backward compatibility*/));
    CirclePostTime_Ticks++;
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

    xSemaphore = xQueueCreateCountingSemaphore( ( 3 ), ( 3 ) );

    if (xSemaphore != 
# 38 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino" 3 4
                     __null
# 38 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino"
                         )
    {
        printf("The semaphore was created successfully\r\n");
        if (xQueueGenericSend( ( QueueHandle_t ) ( xSemaphore ), 
# 41 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino" 3 4
           __null
# 41 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino"
           , ( ( TickType_t ) 0U ), ( ( BaseType_t ) 0 ) ) != ( ( BaseType_t ) 1 ))
        {
            // We would expect this call to fail because we cannot give
            // a semaphore without first "taking" it!
            printf("Semaphore is free now\r\n");
        }
    }

    // create all the task
    BaseType_t error = xTaskCreatePinnedToCore(main_task, "main_task", 1024 * 100, 
# 50 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino" 3 4
                                                                                  __null
# 50 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino"
                                                                                      , 1, 
# 50 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino" 3 4
                                                                                           __null
# 50 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino"
                                                                                               , 1);
    printf("Create main_task code: %d\r\n", error);

    error = xTaskCreatePinnedToCore(user_handler_task, "user_handler_task", 1024 * 20, 
# 53 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino" 3 4
                                                                                      __null
# 53 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino"
                                                                                          , 1, 
# 53 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino" 3 4
                                                                                               __null
# 53 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino"
                                                                                                   , 0);
    printf("Create user_handler_task code: %d\r\n", error);

    TimerHandle_t Runtime_timer = xTimerCreate("runtime_timer", (1000), ( ( BaseType_t ) 1 ), 
# 56 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino" 3 4
                                                                               __null
# 56 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino"
                                                                                   , runtime_timer);

    if (Runtime_timer == 
# 58 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino" 3 4
                        __null
# 58 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino"
                            )
    {
    }
    else
    {
        if (xTimerGenericCommand( ( Runtime_timer ), ( ( BaseType_t ) 1 ), ( xTaskGetTickCount() ), 
# 63 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino" 3 4
           __null
# 63 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino"
           , ( 0 ) ) != ( ( ( BaseType_t ) 1 ) ))
        {
            printf("Can not start request timer\r\n");
        }
        else
            printf("start request timer successfully\r\n");
    }

    sys_create_request_timeout_timer();
    sys_create_confirm_timeout_timer();

    knife_capture.checking_reset_counter_request();
}

/******************************************************************************************/
void loop()
{
    // all the things are done in tasks
    vTaskSuspend(
# 81 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino" 3 4
                __null
# 81 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino"
                    ); // stop this task
}

/******************************************************************************************/
void ethernet_handler_task(void *pr)
{
    if (xQueueGenericReceive( ( QueueHandle_t ) ( xSemaphore ), 
# 87 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino" 3 4
       __null
# 87 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino"
       , ( (TickType_t)0 ), ( ( BaseType_t ) 0 ) ))
    {
        printf("ethernet_handler_task has taken the semaphored, semaphore value: %d\r\n", uxQueueMessagesWaiting( ( QueueHandle_t ) ( xSemaphore ) ));

        ethernet_handler_task_callback_fnc();

        vTaskDelete(RequestTaskHandle);
        if (xQueueGenericSend( ( QueueHandle_t ) ( xSemaphore ), 
# 94 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino" 3 4
           __null
# 94 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino"
           , ( ( TickType_t ) 0U ), ( ( BaseType_t ) 0 ) ) == ( ( BaseType_t ) 1 ))
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
        if (xQueueGenericReceive( ( QueueHandle_t ) ( xSemaphore ), 
# 108 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino" 3 4
           __null
# 108 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino"
           , ( (TickType_t)0 ), ( ( BaseType_t ) 0 ) ))
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

            xQueueGenericSend( ( QueueHandle_t ) ( xSemaphore ), 
# 122 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino" 3 4
           __null
# 122 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino"
           , ( ( TickType_t ) 0U ), ( ( BaseType_t ) 0 ) );
        }

        vTaskDelay(2);
    }
}

/******************************************************************************************/
void main_task(void *pr)
{
    for (;;)
    {
        if (!InitializeFinish && InitialTimes > 0)
        {
            if (InitialRequestNow)
            {
                InitialTimes--;
                printf("Retry to initializes the device\r\n");
                InitialRequestNow = false;
                xTimerGenericCommand( ( RequestTimeOut_TimerHandle ), ( ( BaseType_t ) 2 ), ( xTaskGetTickCount() ), 
# 141 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino" 3 4
               __null
# 141 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino"
               , ( (TickType_t)0 ) );
                knife_capture.checking_reset_counter_request();
            }
        }

        if (xQueueGenericReceive( ( QueueHandle_t ) ( xSemaphore ), 
# 146 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino" 3 4
           __null
# 146 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino"
           , ( (TickType_t)0 ), ( ( BaseType_t ) 0 ) ))
        {
            //printf("main_task has taken the semaphored, semaphore value: %d\r\n",
            //       uxSemaphoreGetCount(xSemaphore));
            // listen to nextion screen
            //nex_listening();

            // ethernet_waiting for data coming in
            //knife_capture.ethernet_handle.running();

            main_task_callback_func();
        }

        xQueueGenericSend( ( QueueHandle_t ) ( xSemaphore ), 
# 159 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino" 3 4
       __null
# 159 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino"
       , ( ( TickType_t ) 0U ), ( ( BaseType_t ) 0 ) );

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
}

inline void ethernet_handler_task_callback_fnc()
{
    printf("\r\n[USER_DEBUG] ----> Func: %s at line: %d\r\n", __func__, 183);
    // if create timer success, then make new request

    if (RequestTimeOut_TimerHandle == 
# 186 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino" 3 4
                                     __null
# 186 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino"
                                         )
    {
        if (!sys_create_request_timeout_timer())
        {
            if (!knife_capture.ethernet_request_next())
            {
                printf("Connect to server failed\r\n");

                BaseType_t excp = xTimerGenericCommand( ( RequestTimeOut_TimerHandle ), ( ( BaseType_t ) 3 ), 0U, 
# 194 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino" 3 4
                                 __null
# 194 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino"
                                 , ( (TickType_t)0 ) );
                printf("Stop request waiting timer, Excp code: %d\r\n", excp);
            }
        }
    }

    if (RequestTimeOut_TimerHandle != 
# 200 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino" 3 4
                                     __null
# 200 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino"
                                         )
    {
        BaseType_t excp = xTimerGenericCommand( ( RequestTimeOut_TimerHandle ), ( ( BaseType_t ) 2 ), ( xTaskGetTickCount() ), 
# 202 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino" 3 4
                         __null
# 202 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino"
                         , ( (TickType_t)0 ) );
        printf("Reset request waiting timer, Excp code: %d\r\n", excp);
        if (!knife_capture.ethernet_request_next())
        {
            printf("Connect to server failed\r\n");

            BaseType_t excp = xTimerGenericCommand( ( RequestTimeOut_TimerHandle ), ( ( BaseType_t ) 3 ), 0U, 
# 208 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino" 3 4
                             __null
# 208 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino"
                             , ( (TickType_t)0 ) );
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
        if (RequestTimeOut_TimerHandle == 
# 223 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino" 3 4
                                         __null
# 223 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino"
                                             )
        {
            if (!sys_create_request_timeout_timer())
            {
                if (!knife_capture.ethernet_request_next())
                {
                    printf("Connect to server failed\r\n");

                    BaseType_t excp = xTimerGenericCommand( ( RequestTimeOut_TimerHandle ), ( ( BaseType_t ) 3 ), 0U, 
# 231 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino" 3 4
                                     __null
# 231 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino"
                                     , ( (TickType_t)0 ) );
                    printf("Stop request waiting timer, Excp code: %d\r\n", excp);
                }
            }
        }

        if (RequestTimeOut_TimerHandle != 
# 237 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino" 3 4
                                         __null
# 237 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino"
                                             )
        {
            BaseType_t excp = xTimerGenericCommand( ( RequestTimeOut_TimerHandle ), ( ( BaseType_t ) 2 ), ( xTaskGetTickCount() ), 
# 239 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino" 3 4
                             __null
# 239 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino"
                             , ( (TickType_t)0 ) );
            printf("Reset request waiting timer, Excp code: %d\r\n", excp);
            if (!knife_capture.ethernet_request_next())
            {
                printf("Connect to server failed\r\n");

                BaseType_t excp = xTimerGenericCommand( ( RequestTimeOut_TimerHandle ), ( ( BaseType_t ) 3 ), 0U, 
# 245 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino" 3 4
                                 __null
# 245 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino"
                                 , ( (TickType_t)0 ) );
                printf("Stop request waiting timer, Excp code: %d\r\n", excp);
            }
        }
    }
}

void ethernet_data_received_callback(EthernetClient &stream)
{
    printf("\r\n[USER_DEBUG] ----> Func: %s at line: %d\r\n", __func__, 254);
    int receivedBytes = stream.available();
    memset(knife_capture.http_header.buf, 0, sizeof(knife_capture.http_header.buf));
    stream.readBytesUntil('\r\n', knife_capture.http_header.buf, receivedBytes);
    bool read_data = false;

    printf("New data received: %s", knife_capture.http_header.buf);

    if (strstr(knife_capture.http_header.buf, "200 OK") != nullptr)
    {
        printf("response: Success\n\n");
        printf(knife_capture.http_header.buf);
    }

    if (strstr(knife_capture.http_header.buf, "404 Not Found") != nullptr)
    {
        printf("response: Not Found\n\n");
        printf(knife_capture.http_header.buf);
    }

    if (strstr(knife_capture.http_header.buf, "400 Bad Request") != nullptr)
    {
        printf("response: Bad Request\n\n");
        printf(knife_capture.http_header.buf);
    }

    if (strstr(knife_capture.http_header.buf, "{") != nullptr)
    {
        printf("new data is comming\r\n");
        DynamicJsonDocument json_doc = DynamicJsonDocument(1024 * 3);
        DeserializationError JsonErr = deserializeJson(json_doc, knife_capture.http_header.buf);
        printf("parse json object coed: %s\r\n", JsonErr.c_str());

        if (!JsonErr)
        {
            InitializeFinish = true;

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

                knife_capture.knife_capture_submit=false;
            }

            if (eop == "kc_initial") // initial resp
            {
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
    BaseType_t excp = xTimerGenericCommand( ( RequestTimeOut_TimerHandle ), ( ( BaseType_t ) 3 ), 0U, 
# 338 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino" 3 4
                     __null
# 338 "e:\\Visual Code\\KnifeCapture\\KnifeCapture.ino"
                     , ( (TickType_t)0 ) );
    printf("Stop request timeout timer, Excp code: %d\r\n", excp);
}
