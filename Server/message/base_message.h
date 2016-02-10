#ifndef BASE_MESSAGE_H_INCLUDED
#define BASE_MESSAGE_H_INCLUDED

#include "../message_data.h"
#include "../datastruct/hashmap.h"
#include "../datastruct/hashmapi.h"
#include <string.h>
#include "../conf/dictionary_reader.h"
#include "../datastruct/vector.h"
#include "../utility_gen_message.h"

#define CAPABILITIES_EXCHANGE 257
#define CREDIT_CONTROL 272
#define DEVICE_WATCHDOG 280

typedef struct _msg_packet{
    unsigned char* data;
    int max_size;
    int position;
} msg_packet;

typedef struct _base_send{
    type_avp_in_cmd* avp_info;
    int command_code;
    unsigned char version;
    int length;
    unsigned char flag;
    int app_id;
    int hop_hop_id;
    int end_end_id;
    char rFlag;
    char pFlag;
    char eFlag;
    char tFlag;
    char* name;
    msg_packet* packet;//need clean
    vector* all_avp_msg;//avp_message
} base_send;

typedef struct _avp_message{
    avp_represent* avp_rep;
    unsigned char flag;
    int length;
    void* value;
    msg_packet* packet;
    vector* child_avp;//need clean when avp message clean
    int pad_len;
} avp_message;

unsigned char gen_flag_value(char r, char p, char e, char t);
int calculate_len(base_send *msg);
int put_flex_int(msg_packet* des_arr, int value, unsigned char num_byte);
msg_packet* create_new_msg_packet(int init_size);//should be clean
void generate_avp_msg(base_send* msg);
void create_send_data(base_send* msg);//pack all value to byte array
base_send* create_new_send_msg(unsigned char ver, int cmd_code, char r, char e, char p, char t);
void auto_set_all_avp(base_send* msg);
unsigned char gen_avp_flag(avp_represent* avp);
void put_int32(msg_packet* des_arr, int value);
//void put_uint32(msg_packet* des_arr, unsigned int value);
void put_int64(msg_packet* des_arr, long value);
//void put_uint64(msg_packet* des_arr, unsigned long value);
void put_str(msg_packet* des_arr, char* str);
int put_byte(msg_packet* des_arr, unsigned char value);
void expand_msg_packet(msg_packet* des_arr);
void append_padding(msg_packet* des_arr);
avp_message* create_new_avp_msg(char* avp_name);//should be clean
void gen_avp_child(avp_message* avp_msg);
void set_avp_value(avp_message* avp, void* val);
void set_avp_message(base_send* msg, void* val, int avp_code);
void create_avp_data(avp_message* avp);
void calculate_avp_length(avp_message* avp, int len);
void put_avp_value_data(avp_represent* avp_rep, msg_packet* des_arr, void* val);
void free_base_send(base_send* msg);
void free_avp_send(avp_message* avp_msg);
avp_message* get_avp_message_by_code(base_send* msg, int code);

#endif // BASE_MESSAGE_H_INCLUDED
