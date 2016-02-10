#include "base_receive.h"

char read_byte(msg_packet* packet){
    if(packet->position + 1 > packet->max_size) return 0;
    char result = packet->data[packet->position];
    packet->position++;
    return result;
}

int read_int32(msg_packet* packet){
    int pos = packet->position;
    if(pos + 4 > packet->max_size) return 0;
    int temp, result, i;
    result = 0;
    for(i = 0; i < 4; i++){
        temp = packet->data[pos + i];
        result += (temp << ((3 - i) * 8));
    }
    packet->position += 4;
    return result;
}

long read_int64(msg_packet* packet){
    int pos = packet->position;
    if(pos + 8 > packet->max_size) return 0;
    long temp, result;
    int i;
    for(i = 0; i < 8; i++){
        temp = packet->data[pos + i];
        result += (temp << ((7 - i) * 8));
    }
    packet->position += 8;
    return result;
}

int read_flex_int(msg_packet* packet, int len){
    int pos = packet->position;
    if(pos + len > packet->max_size) return 0;
    int i, temp, result = 0;
    int t = (len - 1) * 8;
    for(i = 0; i < len; i++){
        temp = *(packet->data + pos + i);
        result += temp << t;
        t -= 8;
    }
    packet->position += len;
    return result;
}

char* read_str(msg_packet* packet, int len){
    char* result = malloc(len);
    if(packet->position + len >= packet->max_size) return 0;
    memcpy(result, (packet->data + packet->position), len);
    packet->position += len;
    return result;
}

void read_group_avp(avp_receive_msg* avp, int len){
    avp->child_avp = vector_init();
    int num_read = 0;
    avp_receive_msg* avp_c;
    while(num_read < len){
        avp_c = (avp_receive_msg*)malloc(sizeof(avp_receive_msg));
        avp_c->packet = avp->packet;
        unpack_avp_msg_header(avp_c);
        unpack_avp_msg_body(avp_c);
        vector_add(avp->child_avp, (void*)avp_c);
        num_read += (avp_c->length + avp_c->pad_len);
    }
}

void unpack_avp_msg_header(avp_receive_msg* msg){
    int code = read_int32(msg->packet);
    msg->flag = read_byte(msg->packet);
    unpack_avp_flag(msg);
    msg->length = read_flex_int(msg->packet, 3);
    char* name;
    hashmapi_get(avp_code_name, code, (void**)(&name));
    hashmap_get(avp_map, name, (void**)(&msg->avp_rep));
    if(msg->avp_rep->vendor_bit != 0){
        msg->vendor = read_int32(msg->packet);
    }
}

void unpack_avp_flag(avp_receive_msg* avp){
    unsigned char flag = avp->flag;
    avp->v_flag = (flag >> 7) & 0x01;
    avp->m_flag= (flag >> 6) & 0x01;
    avp->p_flag= (flag >> 5) & 0x01;
}

void unpack_avp_msg_body(avp_receive_msg* avp){
    int* parent_type;
    hashmapi_get(data_type_def, avp->avp_rep->type, (void**)(&parent_type));
    int value_len = avp->length - AVP_HEADER_LEN;
    if(avp->avp_rep->vendor_bit != 0){
        value_len -= 4;
    }
    avp->pad_len = 4 - avp->length % 4;
    if(avp->pad_len == 4) avp->pad_len = 0;
    switch(*parent_type){
        case Integer32:
            avp->value = malloc(sizeof(int));
            *((int*)avp->value) = read_int32(avp->packet);
            break;
        case Unsigned32:
            avp->value = malloc(sizeof(unsigned int));
            *((unsigned int*)avp->value) = read_int32(avp->packet);
            break;
        case Integer64:
            avp->value = malloc(sizeof(long));
            *((long*)avp->value) = read_int64(avp->packet);
            break;
        case Unsigned64:
            avp->value = malloc(sizeof(unsigned long));
            *((unsigned long*)avp->value) = read_int64(avp->packet);
            break;
        case OctetString:
        case UTF8String:
            avp->value = read_str(avp->packet, value_len);
            break;
        case Grouped:
            read_group_avp(avp, value_len);
            break;
    }
    avp->packet->position += avp->pad_len;
}

void unpack_message_header(base_receive* msg){
    msg_packet* packet = msg->packet;
    msg->version = read_byte(packet);
    msg->length = read_flex_int(packet, 3);
    msg->flag = read_byte(packet);
    unpack_message_flag(msg);
    msg->command_code = read_flex_int(packet, 3);
    msg->app_id = read_int32(packet);
    msg->hop_hop_id = read_int32(packet);
    msg->end_end_id = read_int32(packet);
    msg->data_len = msg->length - MESSAGE_HEADER_LEN;
}

void unpack_message_flag(base_receive* msg){
    unsigned char flag = msg->flag;
    msg->r_flag = (flag >> 7) & 0x01;
    msg->p_flag = (flag >> 6) & 0x01;
    msg->e_flag = (flag >> 5) & 0x01;
    msg->t_flag = (flag >> 4) & 0x01;
}

void unpack_message_data(base_receive* msg){
    avp_receive_msg* avp;
    int data_len;
    msg_packet* packet = msg->packet;
    data_len = msg->data_len;
    while(packet->position < data_len){
        avp = (avp_receive_msg*)malloc(sizeof(avp_receive_msg));
        avp->packet = msg->packet;
        unpack_avp_msg_header(avp);
        unpack_avp_msg_body(avp);
        vector_add(msg->all_avp_msg, (void*)avp);
    }
}

base_receive* create_new_receive_packet(){
    base_receive* rev = (base_receive*)malloc(sizeof(base_receive));
    rev->all_avp_msg = vector_init();
    return rev;
}

void free_avp_receive(avp_receive_msg* avp){
    int i;
    avp_receive_msg* child;
    //free child
    if(avp->avp_rep->type == Grouped){
        vector* v = avp->child_avp;
        for(i = 0; i < v->count; i++){
            child = (avp_receive_msg*)vector_get(v, i);
            free_avp_receive(child);
        }
        vector_free(v);
        free(avp->child_avp);
    }
    int* parent_type;
    hashmapi_get(data_type_def, avp->avp_rep->type, (void**)(&parent_type));
    if(*parent_type != OctetString && *parent_type != UTF8String && *parent_type != Grouped){
        free(avp->value);
    }
}

void free_base_receive(base_receive* rev){
    int i;
    avp_receive_msg* avp;
    //free child
    vector* v = rev->all_avp_msg;
    for(i = 0; i < v->count; i++){
        avp = (avp_receive_msg*)vector_get(v, i);
        free_avp_receive(avp);
    }
    vector_free(v);
    free(rev->all_avp_msg);
    free(rev->packet);
}

avp_receive_msg* get_avp_receive_by_code(base_receive* rev, int code){
    int i;
    vector* v = rev->all_avp_msg;
    avp_receive_msg* result;
    for(i = 0; i < v->count; i++){
        result = (avp_receive_msg*)vector_get(v, i);
        if(result->avp_rep->code == code){
            return result;
        }
    }
    return 0;
}
