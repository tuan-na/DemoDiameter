#include "packet_process.h"
#include "diameter_queue_input.h"

base_receive* process_read(int fd){
    //create new base receive
    int count;
    base_receive* rev_temp;
    rev_temp = create_new_receive_packet();
    rev_temp->packet = create_new_msg_packet(MESSAGE_HEADER_LEN);
    unsigned char* buf = rev_temp->packet->data;
    int hha = rev_temp->packet->max_size;
    //read header
    memset(buf, 0, MESSAGE_HEADER_LEN);
    count = read(fd, buf, MESSAGE_HEADER_LEN);
    if(count == -1){
        if(errno != EAGAIN){
            return READ_ERROR;
        }else{
            return READ_DONE;
        }
    }
    else if(count == 0){
        return READ_ERROR;
    }
    unpack_message_header(rev_temp);
    free(rev_temp->packet->data);
    free(rev_temp->packet);
    int data_len = rev_temp->data_len;
    rev_temp->packet = create_new_msg_packet(data_len);
    buf = rev_temp->packet->data;
    memset(buf, 0, data_len);
    count = read(fd, buf, data_len);
    if(count == -1){
        return READ_ERROR;
    }else if(count == 0){
        return READ_ERROR;
    }
    return rev_temp;
}

base_send* process_gen_response_error(base_receive* rev){
    base_send* rep_msg;
    int code = rev->command_code;
    unsigned char is_req = rev->r_flag ? 0 : 1;
    if(code == CAPABILITIES_EXCHANGE || code == CREDIT_CONTROL){
        rep_msg = create_new_send_msg(1, code, is_req, 0, 0, 0);
    }else{
        return 0;
    }
    generate_avp_msg(rep_msg);
    auto_set_all_avp(rep_msg);
    int* val = (int*)malloc(sizeof(int));
    *val = 3001;
    set_avp_message(rep_msg, (void*)val, 268);
    create_send_data(rep_msg);
    return rep_msg;
}

/**
gen keep connection message
*/
base_send* process_gen_keep_con(base_receive* rev){
    unpack_message_data(rev);
    base_send* send;
    unsigned char is_req = rev->r_flag ? 0 : 1;
    send = create_new_send_msg(1, DEVICE_WATCHDOG, is_req, 0, 0, 0);
    generate_avp_msg(send);
    auto_set_all_avp(send);
    int* val = (int*)malloc(sizeof(int));
    *val = 2001;
    set_avp_message(send, (void*)val, 268);
    create_send_data(send);
    return send;
}

base_send* process_gen_init_con(base_receive* rev){
    unpack_message_data(rev);
    base_send* send;
    unsigned char is_req = rev->r_flag ? 0 : 1;
    send = create_new_send_msg(1, CAPABILITIES_EXCHANGE, is_req, 0, 0, 0);
    generate_avp_msg(send);
    auto_set_all_avp(send);
    int* val = (int*)malloc(sizeof(int));
    *val = 2001;
    set_avp_message(send, (void*)val, 268);

    create_send_data(send);
    return send;
}

base_send* process_gen_response(struct d_queue_data *data){
    base_send* send;
    base_receive* rev = data->rev;
    int code = rev->command_code;
    unsigned char is_req = rev->r_flag ? 0 : 1;
    send = create_new_send_msg(1, code, is_req, 0, 0, 0);
    generate_avp_msg(send);
    auto_set_all_avp(send);
    int* val = (int*)malloc(sizeof(int));
    *val = 2001;
    set_avp_message(send, (void*)val, 268);
    create_send_data(send);
    return send;
}
