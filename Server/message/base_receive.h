#ifndef BASE_RECEIVE_H_INCLUDED
#define BASE_RECEIVE_H_INCLUDED

#include "../message_data.h"
#include "../datastruct/vector.h"
#include "base_message.h"
#include "../conf/dictionary_reader.h"

#define AVP_HEADER_LEN 8
#define MESSAGE_HEADER_LEN 20

typedef struct _avp_receive_msg {
    avp_represent* avp_rep;
    unsigned char flag;
    int length;
    int pad_len;
    unsigned char v_flag;
    unsigned char m_flag;
    unsigned char p_flag;
    void* value;
    long vendor;
    vector* child_avp;
    msg_packet* packet;
} avp_receive_msg;

typedef struct _base_receive{
    int command_code;
    char version;
    int length;
    int data_len;
    unsigned char flag;
    int app_id;
    int hop_hop_id;
    int end_end_id;
    unsigned char r_flag;
    unsigned char p_flag;
    unsigned char e_flag;
    unsigned char t_flag;
    unsigned char is_req;
    msg_packet* packet;
    vector* all_avp_msg;//avp_receive_msg
} base_receive;

char read_byte(msg_packet* packet);
int read_int32(msg_packet* packet);
long read_int64(msg_packet* packet);
int read_flex_int(msg_packet* packet, int len);
char* read_str(msg_packet* packet, int len);
void read_group_avp(avp_receive_msg* avp, int len);
void unpack_avp_msg_header(avp_receive_msg* msg);
void unpack_avp_flag(avp_receive_msg* avp);
void unpack_avp_msg_body(avp_receive_msg* avp);
void unpack_message_header(base_receive* msg);
void unpack_message_data(base_receive* msg);
base_receive* create_new_receive_packet();
void free_avp_receive(avp_receive_msg* avp);
void free_base_receive(base_receive* rev);
avp_receive_msg* get_avp_receive_by_code(base_receive* rev, int code);

#endif // BASE_RECEIVE_H_INCLUDED
