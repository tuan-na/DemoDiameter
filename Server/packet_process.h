#ifndef PACKET_PROCESS_H_INCLUDED
#define PACKET_PROCESS_H_INCLUDED

#define READ_DONE 1
#define READ_ERROR 0
#include "message/base_receive.h"
#include "message/base_message.h"
#include "errno.h"

struct d_queue_data;

base_receive* process_read(int fd);
base_send* process_gen_response_error(base_receive* rev);
base_send* process_gen_keep_con(base_receive* rev);
base_send* process_gen_init_con(base_receive* rev);
base_send* process_gen_response(struct d_queue_data *data);

#endif // PACKET_PROCESS_H_INCLUDED
