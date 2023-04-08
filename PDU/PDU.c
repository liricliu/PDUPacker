#include "PDU.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

PDU_Type detect_load_type(PDU* object){
    if((object->buffer)[20]==0x08&&(object->buffer)[21]==0x00)
        return load_is_ipv4;
    if((object->buffer)[20]==0x86&&(object->buffer)[21]==0xDD)
        return load_is_ipv6;
    if((object->buffer)[20]==0x08&&(object->buffer)[21]==0x06)
        return load_is_arp;
    if((object->buffer)[20]==0x08&&(object->buffer)[21]==0x35)
        return load_is_rarp;
    return load_is_any;
}

long get_load_length_ipv4(const unsigned char* str){
    long toret=0;
    toret+=str[2];
    toret+=str[3]*256;
    return toret;
}

long get_load_length_ipv6(const unsigned char* str){
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

long pack_PDU(PDU* object,const char* source_file,const char* src_mac,const char* dst_mac,PDU_Load_Type loadtype){
    FILE* f=fopen(source_file,"rb");
    if(f==NULL){
        return -1;
    }
    char internet_layer_pdu[PDU_MAX_LOAD_SIZE];
    int load_size=0;
    load_size=fread(internet_layer_pdu,1,PDU_MAX_LOAD_SIZE,f);
    if(load_size==0){
        return -2;
    }

    object->load_type=loadtype;
    switch (object->load_type)
    {
    case load_is_ipv4:
        load_size=get_load_length_ipv4(internet_layer_pdu);
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
    if(load_size>1500){
        return -3;
    }
    object->load_length=load_size;
    for(int i=0;i<17;i++){
        object->dst_mac[i]=dst_mac[i];
        object->src_mac[i]=src_mac[i];
    }
    
    //Now all ready, go packing PDU.
    memset(object->buffer,PDU_QIANDAO,7);
    object->buffer[7]=PDU_SOF;
    //DST MAC
    sscanf(dst_mac, "%hhX:%hhX:%hhX:%hhX:%hhX:%hhX", &(object->buffer)[8],&(object->buffer)[9],&(object->buffer)[10],&(object->buffer)[11],&(object->buffer)[12], &(object->buffer)[13]);
    //SRC MAC
    sscanf(src_mac, "%hhX:%hhX:%hhX:%hhX:%hhX:%hhX", &(object->buffer)[14],&(object->buffer)[15],&(object->buffer)[16],&(object->buffer)[17],&(object->buffer)[18], &(object->buffer)[19]);
    
    //Protocal
    if(loadtype==load_is_any){
        (object->buffer)[20]=(load_size+18)&0xff;
        (object->buffer)[21]=(load_size+18)>>8;
    }
    if(loadtype==load_is_ipv4){
        (object->buffer)[20]=0x08;
        (object->buffer)[21]=0x00;
    }
    if(loadtype==load_is_ipv6){
        (object->buffer)[20]=0x86;
        (object->buffer)[21]=0xDD;
    }
    if(loadtype==load_is_arp){
        (object->buffer)[20]=0x08;
        (object->buffer)[21]=0x06;
    }
    if(loadtype==load_is_rarp){
        (object->buffer)[20]=0x08;
        (object->buffer)[21]=0x35;
    }

    memcpy(object->buffer+22,internet_layer_pdu,load_size);

    //CRC
    if(load_size<PDU_MIN_LOAD_SIZE){
        (object->buffer)[PDU_MIN_LOAD_SIZE+22]=0xff;
        (object->buffer)[PDU_MIN_LOAD_SIZE+23]=0xff;
        (object->buffer)[PDU_MIN_LOAD_SIZE+24]=0xff;
        (object->buffer)[PDU_MIN_LOAD_SIZE+25]=0xff;
    }else{
        (object->buffer)[load_size+22]=0xff;
        (object->buffer)[load_size+23]=0xff;
        (object->buffer)[load_size+24]=0xff;
        (object->buffer)[load_size+25]=0xff;
    }
    return 0;
}

void parse_PDU(PDU* object){
    object->load_type=detect_load_type(object);
    switch(object->load_type){
    case load_is_ipv4:
        object->load_length=get_load_length_ipv4(object->buffer+22);
        break;
    case load_is_ipv6:
        object->load_length=get_load_length_ipv6(object->buffer+22);
        break;
    case load_is_arp:
    case load_is_rarp:
        object->load_length=28;
        break;
    default:
        object->load_length=((object->buffer)[20])+((object->buffer)[21]<<8)-18;
        break;
    }
    sprintf(object->dst_mac,"%02X:%02X:%02X:%02X:%02X:%02X",(object->buffer)[8],(object->buffer)[9],(object->buffer)[10],(object->buffer)[11],(object->buffer)[12],(object->buffer)[13]);
    sprintf(object->src_mac,"%02X:%02X:%02X:%02X:%02X:%02X",(object->buffer)[14],(object->buffer)[15],(object->buffer)[16],(object->buffer)[17],(object->buffer)[18],(object->buffer)[19]);
}
