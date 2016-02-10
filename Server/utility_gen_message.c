#include "utility_gen_message.h"
#include "message/base_message.h"

void manual_set_all_avp_value(avp_message* avp, int code, char is_req){
    int* parent_type;
    if(avp->avp_rep->type == Enumerated){
        parent_type = malloc(sizeof(int));
        *parent_type = Enumerated;
    }else{
        hashmapi_get(data_type_def, avp->avp_rep->type, (void**)(&parent_type));
    }

    switch(*parent_type){
        case OctetString:
        case UTF8String:
            manual_set_str(avp, code, is_req);
            break;
        case Integer32:
            manual_set_int32(avp, code, is_req);
            break;
        case Unsigned32:
            manual_set_uint32(avp, code, is_req);
            break;
        case Integer64:
            manual_set_int64(avp, code, is_req);
            break;
        case Unsigned64:
            manual_set_uint64(avp, code, is_req);
            break;
        case Grouped:
            manual_set_grouped(avp, code, is_req);
            break;
        case Enumerated:
            manual_set_enum(avp, code, is_req);
            free(parent_type);
            break;
    }
}

void manual_set_int32(avp_message* avp, int code, char is_req){
    int* temp = (int*)malloc(sizeof(int));
    switch(code){
        case CAPABILITIES_EXCHANGE:
            if(is_req){
                *temp = 1241;
                avp->value = (void*)temp;
            }else{
                *temp = 6856;
                avp->value = (void*)temp;
            }
            break;
        case CREDIT_CONTROL:
            if(is_req){
                *temp = 232654;
                avp->value = (void*)temp;
            }else{
                *temp = 428711;
                avp->value = (void*)temp;
            }
            break;
        case DEVICE_WATCHDOG:
            if(is_req){
                *temp = 77455;
                avp->value = (void*)temp;
            }else{
                *temp = 898878;
                avp->value = (void*)temp;
            }
            break;
    }
}

void manual_set_uint32(avp_message* avp, int code, char is_req){
    unsigned int* temp = (unsigned int*)malloc(sizeof(unsigned int));
    switch(code){
        case CAPABILITIES_EXCHANGE:
            if(is_req){
                *temp = 2233;
                avp->value = (void*)temp;
            }else{
                *temp = 8447;
                avp->value = (void*)temp;
            }
            break;
        case CREDIT_CONTROL:
            if(is_req){
                *temp = 48521;
                avp->value = (void*)temp;
            }else{
                *temp = 69899;
                avp->value = (void*)temp;
            }
            break;
        case DEVICE_WATCHDOG:
            if(is_req){
                *temp = 23111;
                avp->value = (void*)temp;
            }else{
                *temp = 10102;
                avp->value = (void*)temp;
            }
            break;
    }
}
void manual_set_int64(avp_message* avp, int code, char is_req){
    long* temp = (long*)malloc(sizeof(long));
    switch(code){
        case CAPABILITIES_EXCHANGE:
            if(is_req){
                *temp = 555623;
                avp->value = (void*)temp;
            }else{
                *temp = 44145;
                avp->value = (void*)temp;
            }
            break;
        case CREDIT_CONTROL:
            if(is_req){
                *temp = 82161;
                avp->value = (void*)temp;
            }else{
                *temp = 1986;
                avp->value = (void*)temp;
            }
            break;
        case DEVICE_WATCHDOG:
            if(is_req){
                *temp = 44;
                avp->value = (void*)temp;
            }else{
                *temp = 784;
                avp->value = (void*)temp;
            }
            break;
    }
}
void manual_set_uint64(avp_message* avp, int code, char is_req){
    unsigned long* temp = (unsigned long*)malloc(sizeof(unsigned long));
    switch(code){
        case CAPABILITIES_EXCHANGE:
            if(is_req){
                *temp = 20;
                avp->value = (void*)temp;
            }else{
                *temp = 1111;
                avp->value = (void*)temp;
            }
            break;
        case CREDIT_CONTROL:
            if(is_req){
                *temp = 8987;
                avp->value = (void*)temp;
            }else{
                *temp = 3211;
                avp->value = (void*)temp;
            }
            break;
        case DEVICE_WATCHDOG:
            if(is_req){
                *temp = 1;
                avp->value = (void*)temp;
            }else{
                *temp = 5;
                avp->value = (void*)temp;
            }
            break;
    }
}
void manual_set_str(avp_message* avp, int code, char is_req){
    if(avp->avp_rep->type == Time){
        avp->value = "1211";
        return;
    }
    switch(code){
        case CAPABILITIES_EXCHANGE:
            if(is_req){
                avp->value = "This is test String for CER";
            }else{
                avp->value = "This is test String for CEA";
            }
            break;
        case CREDIT_CONTROL:
            if(is_req){
                avp->value = "This is test String for CCR";
            }else{
                avp->value = "This is test String for CCA";
            }
            break;
        case DEVICE_WATCHDOG:
            if(is_req){
                avp->value = "This is test String for DWR";
            }else{
                avp->value = "This is test String for DWA";
            }
            break;
    }
}

void manual_set_enum(avp_message* avp, int code, char is_req){
    int* temp = malloc(sizeof(int));
    type_avp_enum* enumT;
    if(avp->avp_rep->is_enumT > 0 && vector_count(avp->avp_rep->enumT) > 0){
        enumT = (type_avp_enum*)vector_get(avp->avp_rep->enumT, 0);
        *temp = enumT->code;
        avp->value = (int*)temp;
    }else{
        *temp = 0;
        avp->value = (int*)temp;
    }
}

void manual_set_grouped(avp_message* avp, int code, char is_req){
    int i = 0;
    avp_message* child;
    for(i = 0; i < avp->child_avp->count; i++){
        child = (avp_message *)vector_get(avp->child_avp, i);
        manual_set_all_avp_value(child, code, is_req);
    }
}
