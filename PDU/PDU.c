#include "PDU.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

PDU_Type detect_load_type(PDU* object){
    return load_is_ipv4;
}

long get_load_length_ipv4(const char* str){
    long toret=0;
    toret+=str[2];
    toret+=str[3]*256;
    return toret;
}

long get_load_length_ipv6(const char* str){
    long toret=0;
    toret+=str[4];
    toret+=str[5]*256;
    return toret+40;
}

PDU* new_PDU(PDU_Type type){
    if(type=type_ethernet_v2){
        PDU* toret=(PDU*)malloc(sizeof(PDU));
        if(toret==NULL){
            printf("Memory allocation failed.\n");
            return NULL;
        }
        toret->type=type;
        memset(toret,0,sizeof(PDU));
        return toret;
    }else{
        printf("802.3 frame is temporarily unsupported.\n");
        return NULL;
    }
}

long pack_PDU(PDU* object,const char* source_file,const char* src_mac,const char* dst_mac){
    FILE* f=fopen(source_file,"r");
    if(f==NULL){
        return -1;
    }
    char internet_layer_pdu[PDU_MAX_LOAD_SIZE];
    int load_size=0;
    load_size=fread(internet_layer_pdu,1,PDU_MAX_LOAD_SIZE,f);
    if(load_size==0){
        return -2;
    }

    object->load_type=detect_load_type(object);
    switch (object->load_type)
    {
    case load_is_ipv4:
        load_size=get_load_length_ipv6(internet_layer_pdu);
        break;
    case load_is_ipv6:
        load_size=get_load_length_ipv6(internet_layer_pdu);
        break;
    case load_is_arp:
    case load_is_rarp:
        load_size=28;
        break;
    default:
        break;
    }

    for(int i=0;i<17;i++){
        object->dst_mac[i]=dst_mac[i];
        object->src_mac[i]=src_mac[i];
    }
    
    //Now all ready, go packing PDU.
    memset(object->buffer,PDU_QIANDAO,7);
    object->buffer[7]=PDU_SOF;
    memcpy(object->buffer+8,internet_layer_pdu,load_size);
    return 0;
}