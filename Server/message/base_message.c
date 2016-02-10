#include "base_message.h"

unsigned char gen_flag_value(char r, char p, char e, char t){
    unsigned char flag = 0;//create a flag
    //set r flag (bit 7)
    flag |= (r << 7);
    //set p flag (bit 6)
    flag |= (p << 6);
    //set e flag (bit 5)
    flag |= (e << 5);
    //set t flag (bit 4)
    flag |= (t << 4);
    return flag;
}

int calculate_len(base_send *msg){
    //length = version(1) + len(3) + flag(1) + command code(3) + appId(4) + ...
    // ... + hopbyhopID(4) + endbyendID(4) + AVP(...)
    int val = 1 + 3 + 1 + 3 + 4 + 4 + 4;
    return val;
}

void calculate_avp_length(avp_message* avp, int len){
    avp->length = len;
    int padL = 4 - len % 4;
    if(padL == 4) padL = 0;
    avp->pad_len = padL;
    //avp->length += padL;
}

int put_flex_int(msg_packet* des_arr, int value, unsigned char num_byte){
    int pos = des_arr->position;
    char i = 0;
    if(pos + num_byte > des_arr->max_size){
        des_arr->max_size += des_arr->max_size;
        des_arr->data = (unsigned char*) realloc(des_arr->data, sizeof(char) * des_arr->max_size);
    }
    for(i = 0; i < num_byte; i++){
        *(des_arr->data + pos + i) = (value >> ((num_byte - i - 1) << 3) & 0xFF);
    }
    des_arr->position += num_byte;
    return pos;
}

msg_packet* create_new_msg_packet(int init_size){
    msg_packet* result = malloc(sizeof(msg_packet));
    result->data = malloc(init_size);
    result->position = 0;
    result->max_size = init_size;
    return result;
}

/**
gen all avp for base_send
*/
void generate_avp_msg(base_send* msg){
    hashmap_get(avp_in_msg, msg->name, (void**)(&msg->avp_info));
    int i;
    int len = msg->avp_info->all_avp->count;
    vector* v = msg->avp_info->all_avp;
    type_avp_info_msg* avp_info_msg;
    msg->all_avp_msg = vector_init();
    avp_message* temp_avp;
    for(i = 0; i < len; i++){//duyet het cac avp trong msg gen ra cac avp
        avp_info_msg = (type_avp_info_msg*)vector_get(v, i);
        if(avp_info_msg->multiplicity == one || avp_info_msg->multiplicity == oneplus || avp_info_msg->multiplicity == zeroplus){
            temp_avp = create_new_avp_msg(avp_info_msg->name);
            temp_avp->packet = msg->packet;
            gen_avp_child(temp_avp);
            vector_add(msg->all_avp_msg, (void*)temp_avp);
        }
    }
}

/**
base on value of message and avp, create byte array and pack that to packet
*/
void create_send_data(base_send* msg){
    msg_packet* packet = msg->packet;
    put_byte(packet, msg->version);
    int pos_len = put_flex_int(packet, 0, 3);
    put_byte(packet, msg->flag);
    put_flex_int(packet, msg->command_code, 3);
    put_int32(packet, msg->app_id);
    put_int32(packet, msg->hop_hop_id);
    put_int32(packet, msg->end_end_id);
    int i, len;
    len = msg->all_avp_msg->count;
    avp_message* avp_msg;
    for(i = 0; i < len; i++){
        avp_msg = vector_get(msg->all_avp_msg, i);
        create_avp_data(avp_msg);
    }
    msg->length = packet->position;
    *(packet->data + pos_len) = (msg->length >> 16 & 0xFF);
    *(packet->data + pos_len + 1) = (msg->length >> 8 & 0xFF);
    *(packet->data + pos_len + 2) = (msg->length & 0xFF);
}

/**
return a new base_send, need free this
*/
base_send* create_new_send_msg(unsigned char ver, int cmd_code, char r, char e, char p, char t){
    base_send* msg = (base_send*)malloc(sizeof(base_send));
    msg->version = ver;
    msg->command_code = cmd_code;
    msg->rFlag = r;
    msg->eFlag = e;
    msg->pFlag = p;
    msg->tFlag = t;
    msg->flag = gen_flag_value(r, p, e, t);
    hashmapi_get(cmd_map_name, cmd_code*10 + r, (void**)(&msg->name));
    msg->app_id = 0;
    msg->hop_hop_id = 12345;
    msg->end_end_id = 56823;
    msg->packet = create_new_msg_packet(20);
    return msg;
}

/**
gen fake value for all avp(include children avp) of this message
*/
void auto_set_all_avp(base_send* msg){
    int i, len;
    len = msg->all_avp_msg->count;
    avp_message* avp_msg;
    for(i = 0; i < len; i++){
        avp_msg = (avp_message*)vector_get(msg->all_avp_msg, i);
        manual_set_all_avp_value(avp_msg, msg->command_code, msg->rFlag);
    }
}

unsigned char gen_avp_flag(avp_represent* avp){
    unsigned char flag = 0;
    flag |= (avp->protect << 7);
    flag |= (avp->mandatory << 6);
    flag |= (avp->vendor_bit << 5);
    return flag;
}

void expand_msg_packet(msg_packet* des_arr){
    des_arr->max_size += des_arr->max_size;
    des_arr->data = (unsigned char*) realloc(des_arr->data, des_arr->max_size);
}

void put_int32(msg_packet* des_arr, int value){
    int pos = des_arr->position;
    if(pos + 4 > des_arr->max_size){
        expand_msg_packet(des_arr);
    }
    *(des_arr->data + pos) = (value >> 24 & 0xFF);
    *(des_arr->data + pos+ 1) = (value >> 16 & 0xFF);
    *(des_arr->data + pos+ 2) = (value >> 8 & 0xFF);
    *(des_arr->data + pos+ 3) = (value & 0xFF);
    des_arr->position += 4;
}

void put_int64(msg_packet* des_arr, long value){
    int pos = des_arr->position;
    if(pos + 8 > des_arr->max_size){
        expand_msg_packet(des_arr);
    }
    *(des_arr->data + pos) = (value >> 56 & 0xFF);
    *(des_arr->data + pos+ 1) = (value >> 48 & 0xFF);
    *(des_arr->data + pos+ 2) = (value >> 40 & 0xFF);
    *(des_arr->data + pos+ 3) = (value >> 32 & 0xFF);
    *(des_arr->data + pos+ 4) = (value >> 24 & 0xFF);
    *(des_arr->data + pos+ 5) = (value >> 16 & 0xFF);
    *(des_arr->data + pos+ 6) = (value >> 8 & 0xFF);
    *(des_arr->data + pos+ 7) = (value & 0xFF);
    des_arr->position += 8;
}

int put_byte(msg_packet* des_arr, unsigned char value){
    int pos = des_arr->position;
    if(pos + 1 > des_arr->max_size){
        expand_msg_packet(des_arr);
    }
    *(des_arr->data + pos) = value & 0xFF;
    des_arr->position += 1;
    return pos;
}

void put_str(msg_packet* des_arr, char* str){
    int len = strlen(str);
    int pos = des_arr->position;
    if(len + pos > des_arr->max_size){
        expand_msg_packet(des_arr);
    }
    memcpy(des_arr->data + pos, str, len);
    des_arr->position += len;
}

avp_message* create_new_avp_msg(char* avp_name){
    avp_message* avp = (avp_message *)malloc(sizeof(avp_message));
    avp_represent* avp_rep;
    hashmap_get(avp_map, avp_name, (void**)(&avp_rep));
    avp->avp_rep = avp_rep;
    //avp->child_avp = NULL;
    return avp;
}

void gen_avp_child(avp_message* avp_msg){
    if(avp_msg->avp_rep->type != Grouped) return;
    avp_msg->child_avp = vector_init();
    int i;
    int len = avp_msg->avp_rep->childrens->count;
    avp_message* child;
    type_avp_group* avp_g;
    for(i = 0; i < len; i++){
        avp_g = (type_avp_group *)vector_get(avp_msg->avp_rep->childrens, i);
        child = create_new_avp_msg(avp_g->avp_name);
        child->packet = avp_msg->packet;
        gen_avp_child(child);
        vector_add(avp_msg->child_avp, (void*)child);
    }
}

/**
set value for avp, call from base_send
*/
void set_avp_message(base_send* msg, void* val, int avp_code){
    int i;
    avp_message* avp;
    for(i = 0; i < msg->all_avp_msg->count; i++) {
        avp = vector_get(msg->all_avp_msg, i);
        int as = avp->avp_rep->code;
        if(avp->avp_rep->code == avp_code){
            set_avp_value(avp, val);
            break;
        }
    }
}

void set_avp_value(avp_message* avp, void* val){
    //clear old value;
    dia_type type = avp->avp_rep->type;
    if(avp->avp_rep->type != UTF8String && avp->avp_rep->type != OctetString && avp->avp_rep->type != Grouped){
        free(avp->value);
    }
    avp->value = val;
}

void put_avp_value_data(avp_represent* avp_rep, msg_packet* des_arr, void* val){
    int* parent_type;
    hashmapi_get(data_type_def, avp_rep->type, (void**)(&parent_type));

    switch(*parent_type){
        case Integer32:
            put_int32(des_arr, *((int*)val));
            break;
        case Unsigned32:
            put_int32(des_arr, *((unsigned int*)val));
            break;
        case Integer64:
            put_int64(des_arr, *((long*)val));
            break;
        case Unsigned64:
            put_int64(des_arr, *((unsigned long*)val));
            break;
        case OctetString:
        case UTF8String:
            put_str(des_arr, (char*)val);
            break;
        default:
            break;
    }
}

void create_avp_data(avp_message* avp){
    int i;
    int code = avp->avp_rep->code;
    msg_packet* packet = avp->packet;
    int beforePos = packet->position;
    avp_represent* avp_rep = avp->avp_rep;
    put_int32(packet, avp_rep->code);
    avp->flag = gen_avp_flag(avp_rep);
    put_byte(packet, avp->flag);
    avp->length = 0;//temp length. modify after have all value
    int pos_len = put_flex_int(packet, 0, 3);
    if(avp_rep->vendor_bit > 0){
        put_int32(packet, avp_rep->vendor);
    }
    if(avp_rep->type == Grouped){
        avp_message* child;
        if(avp->child_avp != NULL){
            for(i = 0; i < avp->child_avp->count; i++){
                child = (avp_message*)vector_get(avp->child_avp, i);
                create_avp_data(child);
            }
        }
    }else{
        put_avp_value_data(avp_rep, packet, avp->value);
    }
    calculate_avp_length(avp, packet->position - beforePos);
    *(packet->data + pos_len) = (avp->length >> 16 & 0xFF);
    *(packet->data + pos_len + 1) = (avp->length >> 8 & 0xFF);
    *(packet->data + pos_len + 2) = (avp->length & 0xFF);
    int pad = avp->pad_len;
    for(i = 0; i < pad; i++){
        put_byte(packet, 0);
    }
}

void free_avp_send(avp_message* avp_msg){
    int i;
    avp_message* child;
    //free child
    if(avp_msg->avp_rep->type == Grouped){
        vector* v = avp_msg->child_avp;
        for(i = 0; i < v->count; i++){
            child = (avp_message*)vector_get(v, i);
            free_avp_send(child);
        }
        vector_free(v);
        free(avp_msg->child_avp);
    }
    int* parent_type;
    hashmapi_get(data_type_def, avp_msg->avp_rep->type, (void**)(&parent_type));
    if(*parent_type != OctetString && *parent_type != UTF8String && *parent_type != Grouped){
        free(avp_msg->value);
    }
}

void free_base_send(base_send* msg){
    int i;
    avp_message* avp;
    //free child
    vector* v = msg->all_avp_msg;
    for(i = 0; i < v->count; i++){
        avp = (avp_message*)vector_get(v, i);
        free_avp_send(avp);
    }
    vector_free(v);
    free(msg->all_avp_msg);
    free(msg->packet->data);
    free(msg->packet);
}

avp_message* get_avp_message_by_code(base_send* msg, int code){
    int i, size;
    size = msg->all_avp_msg->count;
    avp_message* res;
    for(i = 0; i < size; i++){
        res = (avp_message*)vector_get(msg->all_avp_msg, i);
        if(res->avp_rep->code == code){
            return res;
        }
    }
    return 0;
}
