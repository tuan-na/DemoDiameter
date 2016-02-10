#include <stdio.h>
#include <stdlib.h>
#include <libxml/xpath.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "dictionary_reader.h"
#include <string.h>

map_t vender_code_def;
map_t data_type_def;
map_t avp_map;
map_t avp_code_name;
map_t avp_in_msg;
map_t cmd_map_name;

int read_config(){
    vender_code_def = hashmap_new(16);
    data_type_def = hashmapi_new(32);
    avp_map = hashmap_new(1024);
    avp_code_name = hashmapi_new(1024);
    avp_in_msg = hashmap_new(128);
    cmd_map_name = hashmapi_new(128);
    xmlInitParser();
    LIBXML_TEST_VERSION
    xmlDocPtr doc = xmlParseFile("config/dictionary.xml");
    if(doc == NULL){
        perror("parse error");
        exit(1);
    }

    xmlXPathContextPtr xpathCtx;
    xmlXPathObjectPtr xpathObj;
    xpathCtx = xmlXPathNewContext(doc);
    if(xpathCtx == NULL){
        perror("error");
        exit(1);
    }
    xpathObj = xmlXPathEvalExpression((xmlChar *)"/dictionary", xpathCtx);//get all element in dictionary
    xmlNodePtr node = xpathObj->nodesetval->nodeTab[0];
    node = node->children;
    if(node == NULL){
        perror("error");
        exit(1);
    }

    xmlNodePtr currNode = NULL;
    xmlAttrPtr tempAtt;
    int* code, *vendor_code, id, *typei_parent;
    char* name, *parent = 0, *mandatory, *protect, *may_enc, *vendor_bit, *vendor_id = 0, *codeStr;
    avp_represent *tempAvp;
    int avp_code, typei_name;
    for(currNode = node; currNode; currNode = currNode->next){//
        if(currNode->type == XML_ELEMENT_NODE){
            if(strcmp((char*)currNode->name, "vendor") == 0){
            //check if this node have name vendor
            //save vendor
                for(tempAtt = currNode->properties; tempAtt; tempAtt = tempAtt->next){
                //get all attribute of this node
                    if(strcmp((char*)tempAtt->name, "vendor-id") == 0){
                        name = (char*)xmlNodeListGetString(currNode->doc, tempAtt->children, 1);
                    }else if(strcmp((char*)tempAtt->name, "code") == 0){
                        code = malloc(sizeof(int));
                        *code = atoi((char *)xmlNodeListGetString(currNode->doc, tempAtt->children, 1));
                    }
                    //xmlFree(tempAtt);
                }
                hashmap_put(vender_code_def, name, code);
            }else if(strcmp((char *)currNode->name, "typedefn") == 0){
                parent = 0;
                for(tempAtt = currNode->properties; tempAtt; tempAtt = tempAtt->next){
                    if(strcmp((char*)tempAtt->name, "type-name") == 0){
                        name = (char*)xmlNodeListGetString(currNode->doc, tempAtt->children, 1);
                    }else if(strcmp((char*)tempAtt->name, "type-parent") == 0){
                        parent = (char*)xmlNodeListGetString(currNode->doc, tempAtt->children, 1);
                    }
                    //xmlFree(tempAtt);
                }
                if(!parent){
                    parent = name;
                    //parent = name;
                }
                typei_name = convert_typestr_int(name);
                typei_parent = malloc(sizeof(int));
                *typei_parent = convert_typestr_int(parent);
                hashmapi_put(data_type_def, typei_name, typei_parent);
            }
        }
        //xmlFree(currNode);
    }
    typei_name = convert_typestr_int("Grouped");
    typei_parent = malloc(sizeof(int));
    *typei_parent = convert_typestr_int("Grouped");
    hashmapi_put(data_type_def, typei_name, typei_parent);

    node = xpathObj->nodesetval->nodeTab[0]->children;
    for(currNode = node; currNode; currNode = currNode->next){
        if(currNode->type == XML_ELEMENT_NODE){
            if(strcmp((char *)currNode->name, "avpdefn") == 0){
                vendor_id = 0;
                for(tempAtt = currNode->properties; tempAtt; tempAtt = tempAtt->next){
                    if(strcmp((char*)tempAtt->name, "name") == 0){
                        //tempName = (char*)xmlNodeListGetString(currNode->doc, tempAtt->children, 1);
                        name = (char*)xmlNodeListGetString(currNode->doc, tempAtt->children, 1);
                    }else if(strcmp((char*)tempAtt->name, "code") == 0){
                        code = malloc(sizeof(int));
                        *code = atoi((char *)xmlNodeListGetString(currNode->doc, tempAtt->children, 1));
                        //sprintf(codeStr, "%d", *code);
                    }else if(strcmp((char*)tempAtt->name, "mandatory") == 0){
                         mandatory = (char *)xmlNodeListGetString(currNode->doc, tempAtt->children, 1);
                    }else if(strcmp((char*)tempAtt->name, "protected") == 0){
                         protect = (char *)xmlNodeListGetString(currNode->doc, tempAtt->children, 1);
                    }else if(strcmp((char*)tempAtt->name, "may-encrypt") == 0){
                         may_enc = (char *)xmlNodeListGetString(currNode->doc, tempAtt->children, 1);
                    }else if(strcmp((char*)tempAtt->name, "vendor-bit") == 0){
                         vendor_bit = (char *)xmlNodeListGetString(currNode->doc, tempAtt->children, 1);
                    }else if(strcmp((char*)tempAtt->name, "vendor-id") == 0){
                         vendor_id = (char *)xmlNodeListGetString(currNode->doc, tempAtt->children, 1);
                    }
                    //xmlFree(tempAtt);
                }
                if(vendor_id){
                    vendor_code = malloc(sizeof(int));
                    hashmap_get(vender_code_def, vendor_id, (void**)&vendor_code);
                    tempAvp->vendor = *vendor_code;
                }
                //check children of node

                tempAvp = create_avp_represent(name, *code, may_enc, mandatory, protect, vendor_bit, "Integer32");
                getTypeAvpNode(currNode->children, tempAvp);
                hashmap_put(avp_map, name, tempAvp);
                hashmapi_put(avp_code_name, *code, name);
            }else if(strcmp((char*)currNode->name, "application") == 0){
                get_command(currNode);
            }
        }
        //xmlFree(currNode);
    }

    //clean doc and xml parser
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return 1;
}

void getTypeAvpNode(xmlNodePtr node, avp_represent* avp){
    xmlNodePtr currNode = NULL;
    xmlAttrPtr att, att1;
    char* type;
    vector* tmpVec;
    xmlNodePtr runNode = NULL;
    int code;
    char* name, *mul;
    type_avp_enum* enumT;
    type_avp_group* group;

    for(currNode = node; currNode; currNode = currNode->next){
        if(currNode->type == XML_ELEMENT_NODE && strcmp((char*)currNode->name, "type") == 0){
            att = currNode->properties;
            if(strcmp((char*)att->name, "type-name") == 0){
                type = (char *)xmlNodeListGetString(currNode->doc, att->children, 1);
                if(strcmp(type, "Enumerated") == 0){
                    //read all enum
                    tmpVec = vector_init();
                    for(runNode = currNode->children; runNode; runNode = runNode->next){
                        if(runNode->type == XML_ELEMENT_NODE){
                            if(strcmp((char*)runNode->name, "enum") == 0){
                                att1 = runNode->properties;
                                if(strcmp((char *)att1->name, "code") == 0){
                                    code = atoi((char *)xmlNodeListGetString(runNode->doc, att1->children, 1));
                                }
                                att1 = att1->next;
                                if(strcmp((char *)att1->name, "name") == 0){
                                    name = (char *)xmlNodeListGetString(runNode->doc, att1->children, 1);
                                }
                                enumT = malloc(sizeof(type_avp_enum));
                                enumT->code = code;
                                enumT->name = name;
                                vector_add(tmpVec, (void*)enumT);
                            }
                        }
                    }
                    set_type_enum(avp, tmpVec);
                }
            }
            set_avp_type(avp, type);
        }else if(currNode->type == XML_ELEMENT_NODE && strcmp((char*)currNode->name, "grouped") == 0){
            tmpVec = vector_init();
            for(runNode = currNode->children; runNode; runNode = runNode->next){
                if(runNode->type == XML_ELEMENT_NODE && strcmp((char*)runNode->name, "avp") == 0){
                    att1 = runNode->properties;
                    if(strcmp((char*)att1->name, "name") == 0){
                        name = (char *)xmlNodeListGetString(runNode->doc, att1->children, 1);
                    }
                    if(att1->next == NULL){
                        int a = 0;
                        int b = 0;
                        a = b+1;
                    }
                    att1 = att1->next;
                    if(att1){
                        if(strcmp((char*)att1->name, "multiplicity") == 0){
                            mul = (char *)xmlNodeListGetString(runNode->doc, att1->children, 1);
                        }
                    }else{
                        mul = "0";
                    }
                    group = malloc(sizeof(type_avp_group));
                    group->avp_name = name;
                    group->multiplicity = convert_multiplicity(mul);
                    vector_add(tmpVec, (void*)group);
                }
            }
            set_type_group(avp, tmpVec);
            set_avp_type(avp, "Grouped");
        }
    }
}

void get_command(xmlNodePtr appNode){
    int id;
    xmlAttrPtr att;
    xmlNodePtr runNode = NULL, avp_node = NULL;
    char* name, *tempChar, *avp_name, *multi;
    int code, vendor, avp_code, avp_index;
    type_avp_info_msg* avp_info;
    vector* list_avp;
    type_avp_in_cmd* cmd;

    char isReq;

    att = appNode->properties;
    if(strcmp((char*)appNode, "id") == 0){
        id = atoi((char *)xmlNodeListGetString(appNode->doc, att->children, 1));
    }
    for(runNode = appNode->children; runNode; runNode = runNode->next){
        if(runNode->type == XML_ELEMENT_NODE && strcmp((char*)runNode->name, "command") == 0){
            list_avp = vector_init();
            att = runNode->properties;
            if(strcmp((char*)att->name, "name") == 0){
                name = (char *)xmlNodeListGetString(runNode->doc, att->children, 1);
            }
            att = att->next;
            if(strcmp((char*)att->name, "code") == 0){
                code = atoi((char *)xmlNodeListGetString(appNode->doc, att->children, 1));
            }
            att = att->next;
            if(strcmp((char*)att->name, "request") == 0){
                tempChar = (char *)xmlNodeListGetString(appNode->doc, att->children, 1);
                if(strcmp(tempChar, "false") == 0){
                    isReq = 0;
                }else if(strcmp(tempChar, "true") == 0){
                    isReq = 1;
                }
            }
            for(avp_node = runNode->children; avp_node; avp_node = avp_node->next){
                if(avp_node->type == XML_ELEMENT_NODE && strcmp((char*)avp_node->name, "avp") == 0){
                    att = avp_node->properties;
                    if(strcmp((char*)att->name, "name") == 0){
                        avp_name = (char *)xmlNodeListGetString(avp_node->doc, att->children, 1);
                    }
                    att = att->next;
                    if(strcmp((char*)att->name,  "code") == 0){
                        avp_code = atoi((char *)xmlNodeListGetString(avp_node->doc, att->children, 1));
                    }
                    att = att->next;
                    if(strcmp((char*)att->name, "vendor") == 0){
                        vendor = atoi((char *)xmlNodeListGetString(avp_node->doc, att->children, 1));
                    }
                    att = att->next;
                    if(strcmp((char*)att->name, "multiplicity") == 0){
                        multi = (char *)xmlNodeListGetString(avp_node->doc, att->children, 1);
                    }
                    att = att->next;
                    if(strcmp((char*)att->name, "index") == 0){
                        avp_index = atoi((char *)xmlNodeListGetString(avp_node->doc, att->children, 1));
                    }
                    avp_info = create_avp_info_msg(avp_name, avp_code, vendor, multi, avp_index);
                    vector_add(list_avp, (void*)avp_info);
                }
            }
            cmd = create_avp_in_cmd(name, code, isReq, id, list_avp);
            hashmap_put(avp_in_msg, name, cmd);
            //cmd
            hashmapi_put(cmd_map_name, code * 10 + isReq, name);
        }
    }
}
