#ifndef UTILITY_GEN_MESSAGE_H_INCLUDED
#define UTILITY_GEN_MESSAGE_H_INCLUDED

#include "conf/dictionary_reader.h"

typedef struct _avp_message avp_message;
void manual_set_all_avp_value(avp_message* avp, int code, char is_req);
void manual_set_int32(avp_message* avp, int code, char is_req);
void manual_set_uint32(avp_message* avp, int code, char is_req);
void manual_set_int64(avp_message* avp, int code, char is_req);
void manual_set_uint64(avp_message* avp, int code, char is_req);
void manual_set_str(avp_message* avp, int code, char is_req);
void manual_set_enum(avp_message* avp, int code, char is_req);
void manual_set_grouped(avp_message* avp, int code, char is_req);

#endif // UTILITY_GEN_MESSAGE_H_INCLUDED
