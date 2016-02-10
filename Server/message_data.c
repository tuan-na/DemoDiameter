#include "message_data.h"

avp_represent* create_avp_represent(char* name, int code, char* in_rule_enc, char* in_rule_man,
    char* rule_protect, char* rule_vendor, char* type){
    avp_represent* result = (avp_represent *)malloc(sizeof(avp_represent));
    result->name = name;
    result->code = code;
    if(strcmp(in_rule_man, "must") == 0){
        result->mandatory = 1;
    }else{
        result->mandatory = 0;
    }

    if(strcmp(rule_protect, "must") == 0){
        result->protect = 1;
    }else{
        result->protect = 0;
    }
    if(strcmp(rule_vendor, "must") == 0){
        result->vendor_bit = 1;
    }else{
        result->vendor_bit = 0;
    }
    if(strcmp(in_rule_enc, "yes") == 0){
        result->may_encrypt = 1;
    }else{
        result->may_encrypt = 0;
    }
    int i;
    //set type of avp
    for(i = 0; i < NUM_TYPE; i++){
        if(strcmp(DIA_TYPE_STR[i], type) == 0){
            result->type = i;
            break;
        }
    }

    for(i = 0; i < NUM_RULE; i++){
        if(strcmp(DIA_RULE_STR[i], in_rule_man) == 0){
            result->rule_mandatory = i;
            break;
        }
    }
    for(i = 0; i < NUM_RULE; i++){
        if(strcmp(DIA_RULE_STR[i], rule_protect) == 0){
            result->rule_protected = i;
            break;
        }
    }
    for(i = 0; i < NUM_RULE; i++){
        if(strcmp(DIA_RULE_STR[i], rule_vendor) == 0){
            result->rule_vendor_bit = i;
            break;
        }
    }
    result->is_enumT = 0;
    return result;
}

void set_type_enum(avp_represent* avpRep, vector* listEnum){
    avpRep->is_enumT = 1;
    avpRep->enumT = listEnum;
}

void set_avp_type(avp_represent* avpRep, char* in_type){
    int i;
    for(i = 0; i < NUM_TYPE; i++){
        if(strcmp(DIA_TYPE_STR[i], in_type) == 0){
            avpRep->type = i;
            break;
        }
    }
}

dia_multiplicity convert_multiplicity(char* mul_str){
    int i;
    for(i = 0; i < NUM_MULTI; i++){
        if(strcmp(DIA_MULTI_STR[i], mul_str) == 0){
            return i;
        }
    }
    return -1;
}

void set_type_group(avp_represent* avpRep, vector* listGroup){
    avpRep->grouped = 1;
    avpRep->childrens = listGroup;
}

type_avp_info_msg* create_avp_info_msg(char* name, int code, int vendor, char* mul, int index){
    type_avp_info_msg* result = (type_avp_info_msg *)malloc(sizeof(type_avp_info_msg));
    result->name = name;
    result->code = code;
    result->vendor = vendor;
    dia_multiplicity m = convert_multiplicity(mul);
    result->multiplicity = m;
    result->index = index;
    return result;
}

type_avp_in_cmd* create_avp_in_cmd(char* name, int code, char isReq, int app_id, vector* all_avp){
    type_avp_in_cmd* result = (type_avp_in_cmd*) malloc(sizeof(type_avp_in_cmd));
    result->name = name;
    result->code = code;
    result->is_request = isReq;
    result->app_id = app_id;
    result->all_avp = all_avp;
    return result;
}

int convert_typestr_int(char* type){
    int i;
    for(i = 0; i < NUM_TYPE; i++){
        if(strcmp(DIA_TYPE_STR[i], type) == 0){
            return i;
        }
    }
}
