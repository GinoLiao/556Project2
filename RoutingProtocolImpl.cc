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
                //         IMPLEMENTED FUNCTIONS          //
                //                                        //
                ////////////////////////////////////////////
void RoutingProtocolImpl::init(unsigned short num_ports, unsigned short router_id, eProtocolType protocol_type) {
  printf("Num of ports: %d, router ID: %d \n", num_ports, router_id);
  NumPorts=num_ports;
  RouterID = router_id;
  ProtocolType=protocol_type;
  printf("initialization starts\n");
  InitPortStatus(NumPorts,RouterID);
  InitRoutingTable();
  MakePortStatus(NumPorts,RouterID);
  
  
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
}

//handle alarm
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
         send_data(port, pkt,(char*)packet, size);
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
                ////////////////////////////////////////////
                //                                        //
                //               PING PONG                //
                //                                        //
                ////////////////////////////////////////////
//ZPINGPONG
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
  /*
    send ping to update port status with neighbors' delays
  */
  void RoutingProtocolImpl::MakePortStatus(unsigned short num_ports, unsigned short router_id){
    eAlarmType alarm_type = ALARM_PORT_STATUS;
    (void) alarm_type;
    handle_alarm(&alarm_type);
      }
void RoutingProtocolImpl::HndAlm_PrtStat(unsigned short num_ports, unsigned short router_id){
  printf("Handle alarm of port status.\n");
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
    printf("payload time %d\n",*(unsigned int*)&pkt->payload);
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

                ////////////////////////////////////////////
                //                                        //
                //           HELPER FUNCTIONS             //
                //                                        //
                ////////////////////////////////////////////
//ZHELP
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
      printf("payload temp host %d\n", payloadTempHost);
      memcpy(&pkt_d->payload+i,&payloadTempHost,4);//payload
    }
    printf("pkt_d->payload: %d\n", *(int *)&pkt_d->payload);
    /*pkt->packet_type = get_pkt_type(*packet);
    pkt->src_id = get_src_id(*packet);
    pkt->dest_id = get_dest_id(*packet);
    pkt->size = size;*/
    //get payload
    //return pkt;
  }
//test Github desktop

  void RoutingProtocolImpl::SendAllNeighbors(PktDetail* pkt_d){
    for(PORT_STATUS *pcur = portStatus;pcur!=NULL;pcur=pcur->next){
      SendMsg(pkt_d,pcur->port_num);
    }
  }
  
  void RoutingProtocolImpl::SendMsg(PktDetail* pkt_d, unsigned short portNum){
  cout<<"in send msg"<<endl;
    char* buffer = (char*) malloc(pkt_d->size);
  ePacketType pkt_type = (ePacketType)pkt_d->packet_type;
  cout<<"pkt_detail type "<<pkt_type<<endl;
    memcpy(&buffer[0], &pkt_type, 1);
    unsigned short msgsize = pkt_d->size;
  unsigned short msgsizeNet = htons(msgsize);
  cout<<"msgsize: "<< msgsize<<endl;
    memcpy(&buffer[2], &msgsizeNet, 2);
    unsigned short srcID = pkt_d->src_id;
  unsigned short srcIDNet = htons(srcID);
    cout<<"src id "<<pkt_d->src_id<<endl;
  memcpy(&buffer[4], &srcIDNet, 2);
    unsigned short destID = pkt_d->dest_id;
  unsigned short destIDNet = htons(destID);
  cout<<"dest id "<<pkt_d->dest_id<<endl;
    memcpy(&buffer[6], &destIDNet, 2);
  char* msgPayload = (char *) malloc(msgsize-8);
    memcpy(&msgPayload,&(pkt_d->payload),4);
  printf("msgPayload %d\n",*(int*)&msgPayload);
  //cout<<"payload "<<(unsigned int *)(pkt_d->payload)<<endl;
  for(int i=0; i<msgsize-8;i=i+4){
    unsigned int msgPayloadTemp;
    memcpy(&msgPayloadTemp, &msgPayload+i, 4);
    unsigned int msgPayloadTempNet = htonl(msgPayloadTemp);
    memcpy(&buffer[8+i], &msgPayloadTempNet, 4);
    printf("msgPayload temp %d\n",(msgPayloadTemp));
  }
    //cout<<"buffer: "<<strlen(buffer)<<endl;
  //HTONS_message(buffer,msgsize);
  //cout<<"buffer Net: "<<strlen(buffer)<<endl;
  //cout<<"before sys send"<<endl;
  //cout<<"msg size: "<<msgsize<<endl;
    sys->send(portNum,buffer,msgsize);

  }
  unsigned short RoutingProtocolImpl::findPortNumber(unsigned short neighborID){
    for (PORT_STATUS *cur = portStatus;cur->next != NULL;cur=cur->next){
        if(cur->id==neighborID){
          return cur-> port_num;
        }
    }
    return 0;
  }
  bool RoutingProtocolImpl::isNeighbor(unsigned short routID){
    for (PORT_STATUS *cur = portStatus;cur->next != NULL;cur=cur->next){
        if(cur->id==routID){
          return true;
        }
    }
    return false;
  }


                ////////////////////////////////////////////
                //                                        //
                //      ROUTING PROTOCOL PARENT CODE      //
                //                                        //
                ////////////////////////////////////////////
//ZPR

void RoutingProtocolImpl::InitRoutingTable(){
  if (ProtocolType==P_DV){                      //Protocol for DV
    //InitRoutingTable_DV();
  }
  else{
    //InitRoutingTable_LS();
  }
}

void RoutingProtocolImpl::send_data(unsigned short port, PktDetail *pkt, char* buffer, unsigned short size){
  if (ProtocolType==P_DV){                      //Protocol for DV
    //send_data_DV(port,pkt,buffer,size);
  }
  else{
    //send_data_LS(port,pkt,buffer,size);
  }
}

void RoutingProtocolImpl::HndAlm_frd(){
  if (ProtocolType==P_DV){                      //Protocol for DV
    //HndAlm_frd_DV();
  }
  else{
    //HndAlm_frd_LS();
  }
}

void RoutingProtocolImpl::HndAlm_FrdChk(){
  if (ProtocolType==P_DV){                      //Protocol for DV
    //HndAlm_FrdChk_DV();
  }
  else{
    //HndAlm_FrdChk_LS();
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
//ZLS
  void RoutingProtocolImpl::InitRoutingTable_LS(){
  //make a rounting table with cost to neighbors finite and non-neighbors infinite
  //set total number of nodes to itself+neighbors to S.  
  //make a link list of frontier nodes.
  
  routTblLS=NULL;
  frontier=NULL;
  for(PORT_STATUS *cur=portStatus;cur!=NULL;cur=cur->next){
    if(cur->TxDelay!=INFINITY_COST){
      ROUT_TBL_LS *addRout = new ROUT_TBL_LS();
      addRout->Destination=cur->id;
      addRout->NextHop=cur->port_num;
      addRout->timestamp=sys->time();
      addRout->Distance=cur->TxDelay;
      addRout->seqNum=0;
      
      NODE_LS *addfrontier = new NODE_LS();
      addfrontier->id=cur->id;
      addfrontier->Distance=cur->TxDelay;

      //config the add elts

      addRout->next=routTblLS;
      routTblLS=addRout;

      addfrontier->next=frontier;
      frontier->next=addfrontier;
    }
  }
  minDistNode=NULL;
  //find first minimum router to wait for.
  findMinInFrontier();

  allKnownNodes = new NODE_LS();
  allKnownNodes->id=RouterID;
  allKnownNodes->Distance=0;
  allKnownNodes->next=NULL;

  seqNumSelf=1;
  neighbors=NULL;
  flood_LS();

}
void RoutingProtocolImpl::findMinInFrontier(){
  minDistNode->id=0;
  minDistNode->Distance=INFINITY_COST;
  for(NODE_LS *fcur=frontier;fcur!=NULL;fcur=fcur->next){
    if(fcur->Distance<minDistNode->Distance){
      minDistNode->id=fcur->id;
      minDistNode->Distance=fcur->Distance;
    }
  }
}
void RoutingProtocolImpl::flood_LS(){
  unsigned short size = 12;

  //Deletes all neighbors
  NODE_LS *neighborNode = neighbors;
  while(neighborNode!=NULL){
    NODE_LS *delNode = neighborNode;
    neighborNode=neighborNode->next;
    free(delNode);
  }
  neighbors=NULL;

  //adds all neighbors back from portStatus
  for(PORT_STATUS *cur=portStatus;cur!=NULL;cur=cur->next){
    if(cur->TxDelay!=INFINITY_COST){
      size+=4;

      NODE_LS *addNeighbor = new NODE_LS();
      addNeighbor->id=cur->id;
      addNeighbor->Distance=cur->TxDelay;
      addNeighbor->next=neighbors;
      neighbors=addNeighbor; 
    }
  }

  //configures the payload for the flood message
  char *sendPayload = (char*) malloc(size-12); 
  NODE_LS *cur_neighbor = neighbors;
  memcpy(&sendPayload[0],&seqNumSelf,4);
  for(int i=4;i<(size-12+4);i+=4){
    memcpy(&sendPayload[i],&(cur_neighbor->id),2);
    memcpy(&sendPayload[i+2],&(cur_neighbor->Distance),2);
  }
  PktDetail *floodMsg = new PktDetail();
  floodMsg->packet_type=LS;
  floodMsg->src_id=RouterID;
  floodMsg->dest_id=0;
  floodMsg->size=size;
  floodMsg->payload=sendPayload;
  SendAllNeighbors(floodMsg);
  free(floodMsg);

  //increment seqNumSelf 
  seqNumSelf+=1;
}
void RoutingProtocolImpl::send_data_LS(unsigned short port, PktDetail *pkt, char* buffer, unsigned short size){
  for(ROUT_TBL_LS *rcur = routTblLS;rcur!=NULL;rcur=rcur->next){
    if(rcur->Destination==pkt->dest_id){
      sys->send(rcur->NextHop,buffer,size);
      return;
    }
  }
}
void RoutingProtocolImpl::HndAlm_frd_LS(){
  //changed it to flood regardless of if the neighbors change, flood anyways
  flood_LS();
  /*
  //checks if the distance from its neighbors change
  for(PORT_STATUS *PEntry=portStatus;PEntry!=NULL;PEntry=PEntry->next){
    for(NODE_LS *NEntry=neighbors;NEntry!=NULL;NEntry=NEntry->next){
      if(  (PEntry->id==NEntry->id) && (PEntry->TxDelay != NEntry->Distance)  ){
        //if so, flood
        flood_LS();
      }

    }
  }*/
}


void RoutingProtocolImpl::updt_LS_RtTbl(unsigned short port, PktDetail *pkt, unsigned short size){
  unsigned int SequenceNum = (unsigned int) (pkt->payload[0]);
  bool isSeqNumGood=false;
  for(ROUT_TBL_LS *REntry=routTblLS;REntry->next != NULL;REntry=REntry->next){
    if(  (REntry->Destination==pkt->src_id) && (REntry->seqNum < SequenceNum)  ){
      isSeqNumGood=true;
    }
  }
  if(isSeqNumGood){
    //if the sequenc number of this message is okay
    addEntriesToLSRoutingTable(pkt);
    if(pkt->src_id==minDistNode->id){
      //if node is the minimum node
      removeFromFrontier(minDistNode->id);
      addToFrontier(pkt);   // potential problem, I added nodes in only as long as they are not in the allknownnodes
                            // might need to exclude existing nodes in frontier as well.
      forwardPacket(pkt);
    }
  }
}
void RoutingProtocolImpl::addEntriesToLSRoutingTable(PktDetail *pkt){
  if(pkt->src_id==minDistNode->id){
    //if the packet came from the minimum node
    
    
    //unsigned short minID=0;
    //unsigned short minDistance=INFINITY_COST;
    for(int i=4;i<pkt->size-12+4;pkt->size+=4){
      unsigned short id= (unsigned short)(pkt->payload[i]);
      unsigned short distance= (unsigned short)(pkt->payload[i+2]);
      ROUT_TBL_LS *knownNode=new ROUT_TBL_LS();
      
      //find min in the payload
      //if(distance<minDistance){
      //  minID=id;
      //  minDistance=distance;
      //}
      for(ROUT_TBL_LS *REntry=routTblLS;REntry->next != NULL;REntry=REntry->next){
        if(REntry->Destination==pkt->src_id){
          knownNode->Destination=REntry->Destination;
          knownNode->Distance=REntry->Distance;
          knownNode->NextHop=REntry->NextHop;
        }
      }

      bool inRoutTable=false;
 
      for(ROUT_TBL_LS *REntry=routTblLS;REntry->next != NULL;REntry=REntry->next){
        if(REntry->Destination==id){
          //this node already has an entry in the routing table
          inRoutTable=true;
          if((distance+knownNode->Distance)<REntry->Distance){
            //i.e. this new path is better than the previous path.
            REntry->Destination=id;
            REntry->NextHop=knownNode->NextHop;
            REntry->Distance=knownNode->Distance+distance;
            REntry->timestamp=sys->time();
          }
          else{
            //refresh the timestamp;
            REntry->timestamp=sys->time();
          }
        }
      }
      free(knownNode);
      if(!inRoutTable){
        //make a new routing entry for this node with the same port as the minimum node
        ROUT_TBL_LS *addRout = new ROUT_TBL_LS();
        addRout->Destination=id;  //destination id 
        addRout->NextHop=findPortNumber(minDistNode->id);      //next hop id
        addRout->Distance=knownNode->Distance+distance;
        addRout->timestamp=sys->time();
        addRout->next=routTblLS;

        routTblLS=addRout;
      }
    }

    //adds the min node in payload to the allKnownNodes
    NODE_LS *addKnownNode = new NODE_LS();
    addKnownNode->id=minDistNode->id;//might be wrong
    addKnownNode->Distance=minDistNode->Distance;
    addKnownNode->next=allKnownNodes;

    allKnownNodes=addKnownNode;

  }
}
void RoutingProtocolImpl::removeFromFrontier(unsigned short remove_id){
  
  if(frontier->id==remove_id){
    NODE_LS *delElm = frontier;
    frontier=frontier->next;
    free(delElm);
    return;
  }
  NODE_LS *lag=frontier;
  NODE_LS *fcur=frontier->next;
  while(fcur->next!=NULL){
    if(fcur->id==remove_id){
      NODE_LS *delElm = fcur;
      lag->next=fcur->next;
      fcur=fcur->next->next;
      free(delElm);
      return;
    }
    else{
      lag=fcur;
      fcur=fcur->next;
    }
  }
  if(fcur->id==remove_id){
    //special case for the last element
    lag->next=fcur->next;
    free(fcur);
  }
}
void RoutingProtocolImpl::addToFrontier(PktDetail *pkt){
  for(int i=4;i<pkt->size-12+4;pkt->size+=4){
    unsigned short id= (unsigned short)(pkt->payload[i]);
    unsigned short distance= (unsigned short)(pkt->payload[i+2]);
    bool isKnownNode=false;
    for(NODE_LS *knownNode = allKnownNodes;knownNode!=NULL;knownNode=knownNode->next){
      if(id==(knownNode->id)){
        //this node is a known node
        isKnownNode=true;
      }
    }
    if(!isKnownNode){
      //add it to the frontier
      NODE_LS *addfrontier=new NODE_LS();
      addfrontier->id=id;
      addfrontier->Distance=distance;
      addfrontier->next=frontier;
      frontier=addfrontier;
    }
  }
}
void RoutingProtocolImpl::forwardPacket(PktDetail *pkt){
  for(PORT_STATUS *PEntry;PEntry!=NULL;PEntry=PEntry->next){
    if(PEntry->id !=pkt->src_id){
      SendMsg(pkt,PEntry->port_num);
    }
  }
}



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
//ZDV

  void RoutingProtocolImpl::InitRoutingTable_DV(){
    //routTblDV = new ROUT_TBL_DV();              //Init the first element
      
      
      /*
      cur->Destination = RouterID;                //adds the first element
      cur->NextHop=RouterID;
      cur->Distance = 0;
      cur->timestamp = sys->time();             
      cur->next=new ROUT_TBL_DV();
      cur = cur->next;*/

      //ROUT_TBL_DV *cur = routTblDV;
      routTblDV=NULL;
      for(PORT_STATUS *curPortStat = portStatus;curPortStat!=NULL;curPortStat=curPortStat->next){
        if(curPortStat->TxDelay!=INFINITY_COST){           //i.e. if the node is a neighbor
          ROUT_TBL_DV *addREntry = new ROUT_TBL_DV();

          addREntry->Destination = curPortStat->id;
          addREntry->NextHop=curPortStat->id;
          addREntry->Distance = curPortStat->TxDelay;
          addREntry->next=routTblDV;                       // addes the next element at iter. the cur
          routTblDV = addREntry;
        }
      }
  }


  void RoutingProtocolImpl::send_data_DV(unsigned short port, PktDetail *pkt, char* buffer, unsigned short size){
  for(ROUT_TBL_DV *rcur = routTblDV;rcur!=NULL;rcur=rcur->next){
        if(rcur->Destination==pkt->dest_id){
          sys->send(rcur->NextHop,buffer,size);
          return;
        }
      }
  }
  void RoutingProtocolImpl::HndAlm_frd_DV(){
    for(PORT_STATUS *pcur = portStatus;pcur!=NULL;pcur=pcur->next){
      if(pcur->TxDelay!=INFINITY_COST){                 
      //i.e. the node is a neighbor
        for(ROUT_TBL_DV *rcur = routTblDV;rcur!=NULL;rcur=rcur->next){      
        //loop through all the entries in the routing table
          if(  (rcur->Destination==pcur->id)   &&   (rcur->Distance != pcur->TxDelay)  ){
          //if this entry is for a neighbor and it's cost has changed
            unsigned int costDiff = (pcur->TxDelay)-(rcur->Distance); 
            
            

            if(costDiff<0){                             // sends the new cost of this neighbor to all neighbors
              PktDetail *sendPkt = new PktDetail();
              sendPkt->packet_type=DV;
              sendPkt->src_id=RouterID;
              sendPkt->size=8+4;
              char* msgPayload=(char*)malloc(4);
              memcpy(&msgPayload[0],&(pcur->id),2);
              memcpy(&msgPayload[2],&(pcur->TxDelay),2);
              sendPkt->payload=msgPayload;  
              SendAllNeighbors(sendPkt);
              free(sendPkt);
            }

            //update all of the routing entries that goes through this changed neighbor
            for(ROUT_TBL_DV *rcur2 = routTblDV;rcur2!=NULL;rcur2=rcur2->next){
              if(rcur2->NextHop==pcur->id){                 
              //if we use this node to get to this destination
                rcur2->timestamp=sys->time();
                rcur2->Distance= (rcur2->Distance)-costDiff;

                if(costDiff<0){
                  // if costDiff is negative, then send a DV update packet.
                  PktDetail *sendPkt=new PktDetail();
                  sendPkt->packet_type=DV;
                  sendPkt->src_id=RouterID;
                  sendPkt->size=8+4;
                  char* msgPayload=(char*)malloc(4);
                  memcpy(&msgPayload[0],&(rcur2->NextHop),2);
                  memcpy(&msgPayload[2],&(rcur2->Distance),2);
                  sendPkt->payload=msgPayload;  
                  SendAllNeighbors(sendPkt);
                  free(sendPkt);
                }
              }
            }
          }
        }
      }
    }
  }

  void RoutingProtocolImpl::updt_DV_RtTbl(unsigned short port, PktDetail *pkt, unsigned short size){
    unsigned short sourceID = pkt->src_id;
    unsigned short destNodeID = (unsigned short) ((pkt->payload)[0]);
    unsigned short distance = (unsigned short) ((pkt->payload)[2]);

    //check if we can access the router that this message is from.
    for(ROUT_TBL_DV *rcur = routTblDV;rcur!=NULL;rcur=rcur->next){
      if(rcur->Destination==sourceID){

        ROUT_TBL_DV *proxyEntry = rcur;           //the node we will use to go to the new destination
        unsigned short potentialDistance = (proxyEntry->Distance) + distance;
        
        //bool foundDestInRoutTbl = false;
        // check to see if this rout was better then the one we already have on our routingTable
        for(ROUT_TBL_DV *rcur2 = routTblDV;rcur2!=NULL;rcur=rcur2->next){
          if(  (rcur2->Destination==destNodeID) ){
            //foundDestInRoutTbl = true;
            ROUT_TBL_DV *existingEntry = rcur2;

            if(existingEntry->Distance<= potentialDistance){
            }
            else{
              //want to re-rout to this new rout
              existingEntry->NextHop=(proxyEntry->NextHop);                        //to do: incomplete
              existingEntry->Distance=potentialDistance;
              existingEntry->timestamp=sys->time();
            }
            
          }
        }
        ////////////////////////////////////////////
        //
        //        POTENTIAL ERROR
        //
        ////////////////////////////////////////////
        if(isNeighbor(destNodeID)){                                                 // re-discovering neighbors
          for (PORT_STATUS *cur = portStatus;cur->next != NULL;cur=cur->next){
            if (cur->id==destNodeID && cur->TxDelay<potentialDistance){
              ROUT_TBL_DV *newEntry = new ROUT_TBL_DV();
              newEntry->Destination=cur->id;  //destination id 
              newEntry->NextHop=cur->port_num;      //next hop id
              newEntry->Distance=cur->TxDelay;
              newEntry->timestamp=sys->time();
              newEntry->next = routTblDV; 

              routTblDV=newEntry;
            }
          }
        }
      }
    }
  }
 
  


  
  
