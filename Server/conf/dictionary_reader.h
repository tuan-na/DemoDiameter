#ifndef DICTIONARY_READER_H_INCLUDED
#define DICTIONARY_READER_H_INCLUDED

#include "../datastruct/hashmap.h"
#include "../datastruct/vector.h"
#include "../message_data.h"

extern map_t vender_code_def;//use to get vender id from vender name
extern map_t data_type_def;//use to get basic type from other type
extern map_t avp_map;
extern map_t avp_code_name;
extern map_t avp_in_msg;//use to get all avp in command
extern map_t cmd_map_name;


int read_config();

#endif // DICTIONARY_READER_H_INCLUDED
