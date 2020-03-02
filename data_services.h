#ifndef data_services_h
#define data_services_h

#include "global_config.h"
#include "global_scope.h"
#include <ArduinoJson.h>

static void update_machine_responsed_data(JsonDocument json_doc, knife_capture_resp &resp_machine)
{
    function_log();
    resp_machine.json_parse_member(json_doc);
}

#endif