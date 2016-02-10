#ifndef DATA_STRUCT_H_INCLUDED
#define DATA_STRUCT_H_INCLUDED

#define NUM_TYPE 21
#define NUM_RULE 4
#define NUM_MULTI 5
#include "datastruct/hashmap.h"
#include "stdlib.h"
#include "string.h"
#include "datastruct/vector.h"

typedef enum {must, may, mustnot, shouldnot} dia_rule;
static const char *DIA_RULE_STR[] = {
    "must",
    "may",
    "mustnot",
    "shouldnot"
};

typedef enum {OctetString,
    Integer32,
    Integer64,
    Unsigned32,
    Unsigned64,
    Float32,
    Float64,
    Grouped,
    Address,
    Time,
    UTF8String,
    DiameterIdentity,
    DiameterURI,
    Enumerated,
    IPFilterRule,
    QOSFilterRule,
    Unsigned32Enumerated,
    MIPRegistrationRequest,
    AppId,
    VendorId,
    IPAddress} dia_type;

static const char* DIA_TYPE_STR[] = {
    "OctetString",
    "Integer32",
    "Integer64",
    "Unsigned32",
    "Unsigned64",
    "Float32",
    "Float64",
    "Grouped",
    "Address",
    "Time",
    "UTF8String",
    "DiameterIdentity",
    "DiameterURI",
    "Enumerated",
    "IPFilterRule",
    "QOSFilterRule",
    "Unsigned32Enumerated",
    "MIPRegistrationRequest",
    "AppId",
    "VendorId",
    "IPAddress"
};

typedef enum {zero, zeroplus, zeroone, one, oneplus} dia_multiplicity;

static const char* DIA_MULTI_STR[] = {"0","0+", "0-1", "1", "1+"};

typedef struct _avp_represent{
    char* name;
    int code;
    char mandatory;
    char protect;
    char may_encrypt;
    char vendor_bit;
    int vendor;
    char grouped;
    char is_enumT;
    //map_t enumT;
    dia_rule rule_mandatory;
    dia_rule rule_protected;
    dia_rule rule_vendor_bit;
    dia_type type;
    vector* enumT;
    vector* childrens;
} avp_represent;

typedef struct _type_avp_enum{
    char* name;
    int code;
} type_avp_enum;

typedef struct _type_avp_group{
    char* avp_name;
    dia_multiplicity multiplicity;
} type_avp_group;

typedef struct _type_avp_info_msg{
    char* name;
    int code;
    int vendor;
    dia_multiplicity multiplicity;
    int index;
    char is_visible;
} type_avp_info_msg;

typedef struct _type_avp_in_cmd{
    char* name;
    int code;
    char is_request;
    int app_id;
    vector* all_avp;//type_avp_info_msg
} type_avp_in_cmd;

avp_represent* create_avp_represent(char* name, int code, char* in_rule_enc, char* in_rule_man,
    char* rule_protect, char* rule_vendor, char* type);
void set_type_enum(avp_represent* avpRep, vector* listEnum);
void set_avp_type(avp_represent* avpRep, char* in_type);
dia_multiplicity convert_multiplicity(char* mul_str);
void set_type_group(avp_represent* avpRep, vector* listGroup);
type_avp_info_msg* create_avp_info_msg(char* name, int code, int vendor, char* mul, int index);
type_avp_in_cmd* create_avp_in_cmd(char* name, int code, char isReq, int app_id, vector* all_avp);
int convert_typestr_int(char* type);

#endif // DATA_STRUCT_H_INCLUDED
