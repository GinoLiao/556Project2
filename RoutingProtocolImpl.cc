#include "RoutingProtocolImpl.h"
#include "Node.h"
#include "global.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <sstream>

RoutingProtocolImpl::RoutingProtocolImpl(Node *n) : RoutingProtocol(n) {
  sys = n;
  // add your own code

}

RoutingProtocolImpl::~RoutingProtocolImpl() {
  // add your own code (if needed)
}



                ////////////////////////////////////////////
                //                                        //
                //                PING PONG               //
                //                                        //
                ////////////////////////////////////////////
void RoutingProtocolImpl::init(unsigned short num_ports, unsigned short router_id, eProtocolType protocol_type) {
  //printf("Num of ports: %d, router ID: %d \n", num_ports, router_id);
  NumPorts=num_ports;
  RouterID = router_id;
  ProtocolType=protocol_type;
  //printf("initialization starts\n");
  InitPortStatus(NumPorts,RouterID);
  InitRoutingTable();
  MakePortStatus(NumPorts,RouterID);
  MakeForwardingTable();
  
  //printf("setting alarms\n");

  // static eAlarmType port_status_alarm_type = ALARM_PORT_STATUS;
  // (void) port_status_alarm_type;
  //SetPortStatusAlarm(this, 10000, &port_status_alarm_type);//every 10 sec
  //sys->set_alarm(this, 10000, &port_status_alarm_type);
  
 
  
  //setting alarms

  static eAlarmType alarm_frd = ALARM_FORWARDING;
  (void) alarm_frd;
  sys->set_alarm(this,30000,&alarm_frd);//every 30 sec
  
  static eAlarmType port_check_alarm_type = ALARM_PORT_CHECK;
  (void) port_check_alarm_type;
  //SetPortCheckAlarm(this, 1000, &port_check_alarm_type);//every 1 sec
  sys->set_alarm(this, 1000, &port_check_alarm_type);
  
  static eAlarmType alarm_frd_chk = ALARM_FORWARD_CHECK;
  (void) alarm_frd_chk;
  sys->set_alarm(this, 1000, &alarm_frd_chk);//every 1 sec


  
  //printf("set alarms done.\n");
}
void RoutingProtocolImpl::InitPortStatus(unsigned short num_ports, unsigned short router_id){
  
    //loop over all other nodes, initialize port status 
    portStatus = new PORT_STATUS();
  PORT_STATUS *cur = portStatus;
    //printf("init port status.\n");
  for(unsigned short i=0; i < num_ports;i++){
    //printf("Num of ports: %d, router ID: %d \n", num_ports, router_id);
        cur->id = 0;
    cur->port_num = i;
        cur->timestamp = sys->time();
        cur->TxDelay = INFINITY_COST; //set to inifinity
        cur->next = new PORT_STATUS();
    cur = cur->next;
    }    
}

void RoutingProtocolImpl::MakePortStatus(unsigned short num_ports, unsigned short router_id){
    eAlarmType alarm_type = ALARM_PORT_STATUS;
    (void) alarm_type;
    handle_alarm(&alarm_type);
}

void RoutingProtocolImpl::handle_alarm(void *data) {
    eAlarmType convertedData = *(eAlarmType*) data;
  //printf("handle alarm starts\n");
  //printf("handle type %d \n",convertedData);
    if(convertedData==ALARM_PORT_STATUS){
    //printf("handle port status alarm.\n");
        HndAlm_PrtStat(NumPorts, RouterID); 
    static eAlarmType port_status_alrm_type = ALARM_PORT_STATUS;
    (void) port_status_alrm_type;
    //printf("setting port update alarm. router: %d\n", RouterID);
    sys->set_alarm(this, 10000, &port_status_alrm_type);
    }
    else if(convertedData == ALARM_FORWARDING){
        HndAlm_frd(); 
    static eAlarmType alrm_frd = ALARM_FORWARDING;
    (void) alrm_frd;
    sys->set_alarm(this,30000,&alrm_frd);
    }
    else if(convertedData == ALARM_PORT_CHECK){
    //printf("handle port check alarm.\n");
    HndAlm_PrtChk();
    static eAlarmType port_check_alrm_type = ALARM_PORT_CHECK;
    (void) port_check_alrm_type;
    sys->set_alarm(this, 1000, &port_check_alrm_type);
    }  
    else if(convertedData == ALARM_FORWARD_CHECK){
        HndAlm_FrdChk();
    static eAlarmType alrm_frd_chk = ALARM_FORWARD_CHECK;
    (void) alrm_frd_chk;
    sys->set_alarm(this, 1000, &alrm_frd_chk);
    }  
    else{
        //printf("This alarm type cannot be handled! \n");
    }       
  
  }

void RoutingProtocolImpl::HndAlm_PrtStat(unsigned short num_ports, unsigned short router_id){
  //printf("Handle alarm of port status.\n");
  //printf("Num of ports: %d, router ID: %d \n", num_ports, router_id);
  for(unsigned short i=0;i<num_ports;i++){
    //printf("num ports: %d \n", num_ports);
    unsigned short msgsize = 12;
    char* buffer = (char*) malloc(msgsize);
    //cout<<"buffer: "<<strlen(buffer)<<endl;
    //cout<<"pkt size "<<pkt_d->size<<endl;
    ePacketType pkt_type = PING;
    //cout<<"pkt_detail type: "<<pkt_type<<endl;
    memcpy(&buffer[0], &pkt_type, 1);
    //cout<< "pkt type: "<<(int)buffer[0]<<endl;
    unsigned short msgsizeNet = htons(msgsize);
    //cout<<"size: "<<msgsize<<endl;
    //cout<<"msgsizeNet: "<< msgsizeNet<<endl;
    //cout<<"msgsizeHost: "<< ntohs(msgsizeNet)<<endl;
    memcpy(&buffer[2], &msgsizeNet, 2);
    unsigned short srcID = router_id;
    unsigned short srcIDNet = htons(srcID);
    //cout<<"src id "<<srcID<<endl;
    //cout<<"src id Net "<<srcIDNet<<endl;
    memcpy(&buffer[4], &srcIDNet, 2);
    unsigned int time = sys->time();
    //cout<<"time:"<<time<<endl;
    //char* msgPayload = (char *) malloc(4);
    //memcpy(&msgPayload,&time,4);
    memcpy(&buffer[8], &time, 4);
    //printf("! %x %d\n", buffer+8, (unsigned int&)buffer[8]);
    //cout<<"msgPayload "<<(unsigned int *)(msgPayload)<<endl;
    //cout<<"buffer: "<<strlen(buffer)<<endl;
    //cout<<"buffer Net: "<<strlen(buffer)<<endl;
    //cout<<"before sys send"<<endl;
    //cout<<"msg size: "<<msgsize<<endl;
    //cout<< "pkt type: "<<(int)buffer[0]<<endl;
    PktDetail *pkt= new PktDetail();
    pkt->packet_type = (unsigned short)PING;
    pkt->size = msgsize;
    pkt->src_id = router_id;
    pkt->payload = (char *)malloc(4);
    memcpy(&pkt->payload, &time,4);
    //printf("pkt type %d\n",*(unsigned int*)&pkt->packet_type);
    //printf("pkt src id %d\n",*(unsigned int*)&pkt->src_id);
    //printf("pkt size %d\n",*(unsigned int*)&pkt->size);
    //printf("payload time %d\n",*(unsigned int*)&pkt->payload);
    SendMsg(pkt,i);
    //sys->send(i,buffer,msgsize);
  }
  }

void RoutingProtocolImpl::HndAlm_PrtChk(){
//printf("Handle alarm port check \n");
  int currentTime = sys->time();
  //int currentTime = 1000;
  int lateTime = 15000;
  for(PORT_STATUS *cur=portStatus;cur->next != NULL;cur=cur->next){
  //printf("time after last update:%d \n", currentTime-cur->timestamp);
    if(  (currentTime-cur->timestamp)  >lateTime){
      if(cur==portStatus){            //ie the current first element 
        PORT_STATUS *deleteElt=portStatus;
        portStatus=portStatus->next;
        free(deleteElt);
      }
      else{                           //the element is not first
        PORT_STATUS *deleteElt=cur;
        cur=cur->next;
        free(deleteElt);
      }
    }
  }    
}

void RoutingProtocolImpl::recv(unsigned short port, void *packet, unsigned short size) {
  // add your own code
  
  
  //printf("\n receive starts\n");
  PktDetail *pkt=new PktDetail();
  //pkt->src_id=0;
  get_pkt_detail(packet, pkt, size);
  // char *buffer=(char *) packet;
  // char *pktType = (char *)malloc(1);
  // memcpy(&pktType,&buffer[0],1);
  // printf("\n packet type in recv: %d \n", *(int *)pktType);
  
  // printf("packet type in recv: %d \n", pkt->packet_type);
  // printf("size in recv: %d \n", size);
  // printf("src id in recv: %d \n", pkt->src_id);
  // printf("dest id in recv: %d \n", pkt->dest_id);
  // printf("payload in recv: %d \n", *(int *)&pkt->payload);
  if(pkt->packet_type == DATA) {
         send_data(port, pkt,(char *)packet, size);
  }
  else if(pkt->packet_type== PING){
        send_pong(port, (char *)packet, size);
  }
  else if(pkt->packet_type==PONG){
        update_port_status(port, pkt, size);
  }
  else if(pkt->packet_type==DV){
        updt_DV_RtTbl(port, pkt, size);
  }
  else if(pkt->packet_type==LS){
        updt_LS_RtTbl(port, pkt, size);
  }
  else{
        printf("This packet type cannot be handled! \n");
  } 
   
   
}

void RoutingProtocolImpl::update_port_status(unsigned short port, PktDetail *pkt, unsigned short size){
  //printf("in update port status \n");
    unsigned short destID = pkt->dest_id;
  //printf("dest id %d \n",destID);
    bool foundport = false;
    unsigned int timeDifference = (sys->time())  -  *(unsigned int*)&pkt->payload;
  //printf("systime %d  pkttime  %d   delay %d \n",sys->time(),*(unsigned int*)&pkt->payload,timeDifference);
  
    for (PORT_STATUS *cur = portStatus;cur->next != NULL;cur=cur->next){
      //printf("port num %d, port %d \n", cur->port_num, port);
            if(cur->port_num==port){
        //printf("found port\n");
              cur->timestamp=sys->time();
              cur->id=destID;
              cur->TxDelay=timeDifference;
              foundport=true;
            }
    }
    if(!foundport){
      //PORT_STATUS nextEntry = portStatus->next;
      PORT_STATUS *newEntry = new PORT_STATUS();
      
      newEntry->timestamp=sys->time();
      newEntry->id=destID;
      newEntry->port_num=port;
      newEntry->TxDelay=timeDifference;
      newEntry->next = portStatus;

      portStatus=newEntry;

    }

  }

void RoutingProtocolImpl::send_pong(unsigned short port, char *buffer, unsigned short size){
       //char *buffer= (char *) packet;
       // char type = *(char *)buffer;
     //printf("in send pong\n");
        unsigned short src_id = ntohs(*(unsigned short *) (buffer + 4));
    //printf("src id %d\n", src_id);
        //unsigned short dest_id = ntohs(*(unsigned short *) (buffer + sizeof(int) + sizeof(unsigned short)));
       // *(char *)buffer=2;
        ePacketType PongPktType = PONG;
        memcpy(&buffer[0],&PongPktType,1);
        //memcpy(buffer, PONG, 1);
        unsigned short srcIDNet = htons(RouterID);
    memcpy(&buffer[4],&srcIDNet,2);
        unsigned short destIDNet = htons(src_id);
    memcpy(&buffer[6],&destIDNet,2);
    //printf("pong pkt type: %d\n", (int) PongPktType);
    //printf("src ID: %d \n",RouterID);
    //printf("dest ID: %d\n",src_id);
        sys->send(port, buffer, size);
      }

                ////////////////////////////////////////////
                //                                        //
                //            HELPER FUNCTIONS            //
                //                                        //
                ////////////////////////////////////////////
void RoutingProtocolImpl::SendMsg(PktDetail* pkt_d, unsigned short portNum){
    //cout<<"in send msg"<<endl;
    char* buffer = (char*) malloc(pkt_d->size);
    ePacketType pkt_type = (ePacketType)pkt_d->packet_type;
    //cout<<"pkt_detail type "<<pkt_type<<endl;
    memcpy(&buffer[0], &pkt_type, 1);
    unsigned short msgsize = pkt_d->size;
    unsigned short msgsizeNet = htons(msgsize);
    //cout<<"msgsize: "<< msgsize<<endl;
    memcpy(&buffer[2], &msgsizeNet, 2);
    unsigned short srcID = pkt_d->src_id;
    unsigned short srcIDNet = htons(srcID);
    //cout<<"src id "<<pkt_d->src_id<<endl;
    memcpy(&buffer[4], &srcIDNet, 2);
    unsigned short destID = pkt_d->dest_id;
    unsigned short destIDNet = htons(destID);
    //cout<<"dest id "<<pkt_d->dest_id<<endl;
    memcpy(&buffer[6], &destIDNet, 2);
    char* msgPayload = (char *) malloc(msgsize-8);
    memcpy(&msgPayload,&(pkt_d->payload),4);
    //printf("msgPayload %d\n",*(int*)&msgPayload);
    //cout<<"payload "<<(unsigned int *)(pkt_d->payload)<<endl;
    for(int i=0; i<msgsize-8;i=i+4){
        unsigned int msgPayloadTemp;
        memcpy(&msgPayloadTemp, &msgPayload+i, 4);
        unsigned int msgPayloadTempNet = htonl(msgPayloadTemp);
        memcpy(&buffer[8+i], &msgPayloadTempNet, 4);
        //printf("msgPayload temp %d\n",(msgPayloadTemp));
    }
    //cout<<"buffer: "<<strlen(buffer)<<endl;
    //HTONS_message(buffer,msgsize);
    //cout<<"buffer Net: "<<strlen(buffer)<<endl;
    //cout<<"before sys send"<<endl;
    //cout<<"msg size: "<<msgsize<<endl;
    sys->send(portNum,buffer,msgsize);

  }

    void RoutingProtocolImpl::SendAllNeighbors(PktDetail* pkt_d){
      for(PORT_STATUS *pcur = portStatus;pcur!=NULL;pcur=pcur->next){
        SendMsg(pkt_d,pcur->port_num);
      }
  }

  void RoutingProtocolImpl::get_pkt_detail(void *pkt, PktDetail *pkt_d, unsigned short size){
      //printf("%d %d\n", size, 1[(unsigned short*)pkt]);
        //printf("get pkt detail");
        //char *pkt_copy={0};
        char *pkt_copy = (char *) pkt;
        //*pkt_copy = ntohs(*(char*) pkt);
        //*pkt_copy = *(char*) pkt;
        //NTOHS_message(pkt_copy,size);
        memcpy(&pkt_d->packet_type,&pkt_copy[0],1);//packetType
        //printf("pkt_d->packet_type: %d\n", pkt_d->packet_type);
        //memcpy(&size,&pkt_copy[2],2);
        unsigned short sizeNet = *(unsigned short*)(pkt_copy+2);
        //memcpy(&sizeNet,&pkt_copy[2],2);//size
        pkt_d->size = ntohs(sizeNet);
        //printf("pkt_d->size: %d\n", pkt_d->size);
        unsigned short srcIDNet = *(unsigned short*)(pkt_copy+4);
        pkt_d->src_id = ntohs(srcIDNet);//srcID
        //printf("pkt_d->src id: %d\n", pkt_d->src_id);
        unsigned short destIDNet = *(unsigned short*)(pkt_copy+6);
        pkt_d->dest_id = ntohs(destIDNet);//DestID
        //printf("pkt_d->dest id: %d\n", pkt_d->dest_id);
        //unsigned int time = *(unsigned int*)(pkt_copy+8);
        //printf("%x %d\n", pkt_copy+8, time);
        //printf("time: %d\n", time);
        //while(1);
        pkt_d->payload = (char *)malloc(size-8);
        for(int i=0; i<size-8;i+=4){
            unsigned int payloadTempNet;
            memcpy(&payloadTempNet,&pkt_copy[8+i],4);
            unsigned int payloadTempHost = ntohl(payloadTempNet);
            //printf("payload temp host %d\n", payloadTempHost);
            memcpy(&pkt_d->payload+i,&payloadTempHost,4);//payload
        }
        //printf("pkt_d->payload: %d\n", *(int *)&pkt_d->payload);
        /*pkt->packet_type = get_pkt_type(*packet);
        pkt->src_id = get_src_id(*packet);
        pkt->dest_id = get_dest_id(*packet);
        pkt->size = size;*/
        //get payload
        //return pkt;
  }


                ////////////////////////////////////////////
                //                                        //
                //      ROUTING PROTOCOL PARENT CODE      //
                //                                        //
                ////////////////////////////////////////////
void RoutingProtocolImpl::InitRoutingTable(){
  if (ProtocolType==P_DV){                      //Protocol for DV
    InitRoutingTable_DV();
  }
  else{
    InitRoutingTable_LS();
  }
}
void RoutingProtocolImpl::send_data(unsigned short port, PktDetail *pkt, char* buffer, unsigned short size){
  if (ProtocolType==P_DV){                      //Protocol for DV
    send_data_DV(port,pkt,buffer,size);
  }
  else{
    send_data_LS(port,pkt,buffer,size);
  }
}
void RoutingProtocolImpl::HndAlm_frd(){
  if (ProtocolType==P_DV){                      //Protocol for DV
    HndAlm_frd_DV();
  }
  else{
    HndAlm_frd_LS();
  }
}
void RoutingProtocolImpl::HndAlm_FrdChk(){
  if (ProtocolType==P_DV){                      //Protocol for DV
    HndAlm_FrdChk_DV();
  }
  else{
    HndAlm_FrdChk_LS();
  }
}
void RoutingProtocolImpl::HndAlm_FrdChk_DV(){
  int currentTime = sys->time();
  int lateTime = 30000;
  for(ROUT_TBL_DV *cur=routTblDV;cur->next != NULL;cur=cur->next){
    if(  (currentTime-cur->timestamp)  >lateTime){
      if(cur==routTblDV){            //ie the current first element 
        ROUT_TBL_DV *deleteElt=routTblDV;
        routTblDV=routTblDV->next;
        free(deleteElt);
      }
      else{                           //the element is not first
        ROUT_TBL_DV *deleteElt=cur;
        cur=cur->next;
        free(deleteElt);
      }
    }
  }    
}
void RoutingProtocolImpl::HndAlm_FrdChk_LS(){
  int currentTime = sys->time();
  int lateTime = 30000;
  for(ROUT_TBL_LS *cur=routTblLS;cur->next != NULL;cur=cur->next){
    if(  (currentTime-cur->timestamp)  >lateTime){
      if(cur==routTblLS){            //ie the current first element 
        ROUT_TBL_LS *deleteElt=routTblLS;
        routTblLS=routTblLS->next;
        free(deleteElt);
      }
      else{                           //the element is not first
        ROUT_TBL_LS *deleteElt=cur;
        cur=cur->next;
        free(deleteElt);
      }
    }
  }  
}

void RoutingProtocolImpl::MakeForwardingTable(){}





                    //////////////////////////////////////
                    //                                  //
                    //    ###               ####        //
                    //    ###             ##    ##      //
                    //    ###              ##           //
                    //    ###                ##         //
                    //    ###                  ##       //
                    //    ########        ##    ##      //
                    //    ########          ####        //
                    //                                  // 
                    //////////////////////////////////////
void RoutingProtocolImpl::InitRoutingTable_LS(){}
void RoutingProtocolImpl::findMinInFrontier(){}
void RoutingProtocolImpl::flood_LS(){}
void RoutingProtocolImpl::send_data_LS(unsigned short port, PktDetail *pkt, char* buffer, unsigned short size){}
void RoutingProtocolImpl::HndAlm_frd_LS(){}
void RoutingProtocolImpl::updt_LS_RtTbl(unsigned short port, PktDetail *pkt, unsigned short size){}
void RoutingProtocolImpl::addEntriesToLSRoutingTable(PktDetail *pkt){}
void RoutingProtocolImpl::removeFromFrontier(unsigned short remove_id){}
void RoutingProtocolImpl::addToFrontier(PktDetail *pkt){}
void RoutingProtocolImpl::forwardPacket(PktDetail *pkt){}

                    //////////////////////////////////////
                    //                                  //
                    //    ########      ####    ####    //
                    //    ###    ##      ##      ##     //
                    //    ###     ##      #      #      //
                    //    ###      ##     ##    ##      //
                    //    ###     ##       #    #       //
                    //    ###    ##        ##  ##       //
                    //    ########           ##         //
                    //                                  // 
                    //////////////////////////////////////
void RoutingProtocolImpl::InitRoutingTable_DV(){}
void RoutingProtocolImpl::send_data_DV(unsigned short port, PktDetail *pkt, char* buffer, unsigned short size){}
void RoutingProtocolImpl::HndAlm_frd_DV(){}
void RoutingProtocolImpl::updt_DV_RtTbl(unsigned short port, PktDetail *pkt, unsigned short size){}