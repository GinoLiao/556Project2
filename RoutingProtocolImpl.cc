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

RoutingProtocolImpl::RoutingProtocolImpl(Node *n) : RoutingProtocol(n) {
  sys = n;
  // add your own code

}

RoutingProtocolImpl::~RoutingProtocolImpl() {
  // add your own code (if needed)
}





void RoutingProtocolImpl::init(unsigned short num_ports, unsigned short router_id, eProtocolType protocol_type) {
	printf("Num of ports: %d, router ID: %d \n", num_ports, router_id);
  NumPorts=num_ports;
  RouterID = router_id;
  ProtocolType=protocol_type;
  printf("initialization starts\n");
  InitPortStatus(NumPorts,RouterID);
  InitRoutingTable();
  MakePortStatus(NumPorts,RouterID);
  MakeForwardingTable();
  
  printf("setting alarms\n");
  eAlarmType port_status_alarm_type = ALARM_PORT_STATUS;
  (void) port_status_alarm_type;
  SetPortStatusAlarm(this, 10000, &port_status_alarm_type);//every 10 sec
  
  SetForwardingAlarm();//every 30 sec
  
  eAlarmType port_check_alarm_type = ALARM_PORT_CHECK;
  (void) port_check_alarm_type;
  SetPortCheckAlarm(this, 1000, &port_check_alarm_type);//every 1 sec
  
  SetForwardCheckAlarm();//every 1 sec

}

  void RoutingProtocolImpl::InitPortStatus(unsigned short num_ports, unsigned short router_id){
    /*PktDetail test;
    test.packet_type=1;
    printf ("test.packet_type:%d\n", test.packet_type);*/
	
    //loop over all other nodes, initialize port status 
    portStatus = new PORT_STATUS();
	PORT_STATUS *cur = portStatus;
    printf("init port status.\n");
	for(unsigned short i=1; i <= num_ports;i++){
		printf("Num of ports: %d, router ID: %d \n", num_ports, router_id);
       	cur->id = 0;
		cur->port_num = i;
        cur->timestamp = sys->time();
        cur->TxDelay = INFINITY_COST;	//set to inifinity
        cur->next = new PORT_STATUS();
		cur = cur->next;
    }    
  }
  
  void RoutingProtocolImpl::InitRoutingTable(){
    

    if (ProtocolType==P_DV){                      //Protocol for DV
      /*
      routTblDV = new ROUT_TBL_DV();              //Init the first element
      
      ROUT_TBL_DV *cur = routTblDV;

      cur->Destination = RouterID;                //adds the first element
      cur->NextHop=RouterID;
      cur->Distance = 0;
      cur->timestamp = sys->time();             
      cur->next=new ROUT_TBL_DV();
      cur = cur->next;

      for(PORT_STATUS *curPortStat = portStatus;curPortStat!=NULL;curPortStat=curPortStat->next){
        cur->Destination = curPortStat->id;
        
        if(curPortStat->TxDelay==INFINITY_COST){           //i.e. if the node is not a neighbor.
          cur->NextHop=0;
          cur->Distance = 0;
        }
        else{                                              //i.e. if the node is a neighbor
          cur->NextHop=curPortStat->id;
          cur->Distance = curPortStat->TxDelay;
        }

        cur->next=new ROUT_TBL_DV();                       // addes the next element at iter. the cur
        cur = cur->next;
      }
      */
      
    }
    else{

    }
  }
  /*
  	send ping to update port status with neighbors' delays
  */
  void RoutingProtocolImpl::MakePortStatus(unsigned short num_ports, unsigned short router_id){
  /*  for(unsigned short i=1;i<num_ports;i++){
      if(i != router_id){
        //make ping packet
        ping_pkt = (char *) malloc(65536);
        ping_pkt[strlen(ping_pkt)-1] = 0;	//set its end
          //set packet type
        ePacketType PingPktType = PING;
        memcpy(&ping_pkt[0],&PingPktType,8);	
          //set size

        unsigned short PingPktSizeNet = 65536;
        memcpy(&ping_pkt[16],&PingPktSizeNet,16);
          //set srouce ID
        memcpy(&ping_pkt[32],&router_id,16);
        	//set destination IF
        memcpy(&ping_pkt[48],&i,16);
        	//set payload to time
        memcpy(&ping_pkt[48],&sys->time(),16);
        //send to all other ports to find neighbor
        sys->send(i, ping_pkt, PingPktSizeNet);
	  }
  }*/
		printf("make port status\n");
		//void *alarm_type;
		eAlarmType alarm_type = ALARM_PORT_STATUS;
		//memcpy(&alarm_type, &ALARM_PORT_STATUS, 16);
		printf("alarm type: %d\n", alarm_type);
		(void) alarm_type;
		handle_alarm(&alarm_type);
      }

  void RoutingProtocolImpl::MakeForwardingTable(){}
  void RoutingProtocolImpl::SetPortStatusAlarm(RoutingProtocol *r, unsigned int duration, void *data){
	sys->set_alarm(r, duration, data);
	  
  }
  void RoutingProtocolImpl::SetForwardingAlarm(){}
  void RoutingProtocolImpl::SetPortCheckAlarm(RoutingProtocol *r, unsigned int duration, void *data){
	sys->set_alarm(r, duration, data);  
  }
  void RoutingProtocolImpl::SetForwardCheckAlarm(){}

void RoutingProtocolImpl::handle_alarm(void *data) {
    eAlarmType convertedData = *(eAlarmType*) data;
	
	printf("handle alarm starts\n");
    if(convertedData==ALARM_PORT_STATUS){
		printf("handle port status alarm.\n");
        HndAlm_PrtStat(NumPorts, RouterID);
    }
    else if(convertedData == ALARM_FORWARDING){
        HndAlm_frd(); 
    }
    else if(convertedData == ALARM_PORT_CHECK){
        HndAlm_PrtChk();
    }  
    else if(convertedData == ALARM_FORWARD_CHECK){
        HndAlm_FrdChk();
    }  
    else{
        printf("This alarm type cannot be handled! \n");
    }       

    }

  void RoutingProtocolImpl::HndAlm_PrtStat(unsigned short num_ports, unsigned short router_id){
	printf("Handle alarm of port status.\n");
	printf("Num of ports: %d, router ID: %d \n", num_ports, router_id);
	for(unsigned short i=1;i<=num_ports;i++){
		printf("make ping packet now\n");
		//make ping packet
/*		char *ping_pkt = (char *) malloc(12);
		printf("seg 1?\n");
          //set packet type
        ePacketType PingPktType = PING;
		//unsigned short PingPktTypeNet = htons(PingPktType);
        memcpy(&ping_pkt[0],&PingPktType,1);	
          //set size
		printf("seg2?\n");
		unsigned short PingPktSizeHost = 12;
        unsigned short PingPktSizeNet = htons(PingPktSizeHost);
        memcpy(&ping_pkt[2],&PingPktSizeHost,2);
		printf("size host: %d, size net: %d, i: %d\n",PingPktSizeHost, PingPktSizeNet,i);
		printf("seg3?\n");
          //set srouce ID
		//unsigned short router_id_Net = htons(router_id);
        memcpy(&ping_pkt[4],&router_id,2);
        	//destination ID unused in PING packet
        	//set payload to time
		printf("seg4?\n");
		unsigned int time = sys->time();
		//unsigned int timeNet = htonl(time);
        memcpy(&ping_pkt[8],&time,4);
        //send to all other ports to find neighbor
		printf("ping packet: %s", ping_pkt);
		printf("seg5?\n");
		HTONS_message(ping_pkt,PingPktSizeHost);
        sys->send(i, ping_pkt, PingPktSizeHost);
		printf("send?\n");*/
		PktDetail *pkt={0};
		ePacketType pkt_type = PING;
		printf("before casting packet type\n");
		pkt->packet_type = (unsigned short)pkt_type;
		printf("packet type set\n");
		pkt->src_id = router_id;
		pkt->size = 12;
		unsigned int time = sys->time();
		pkt->payload = (char *)time;
		printf("ready to send\n");
		SendMsg(pkt,i);
		printf("send done\n");
	}
  }
  
  void RoutingProtocolImpl::HndAlm_frd(){}
  void RoutingProtocolImpl::HndAlm_PrtChk(){
    int currentTime = sys->time();
    //int currentTime = 1000;
    
    if(currentTime-portStatus->timestamp>15000){       //delete first element
      PORT_STATUS *deleteElt = portStatus;
      portStatus =portStatus->next;
      free(deleteElt);
    }
                                            //iterate through the list
    if(portStatus->next==NULL){return;}
    else{
      PORT_STATUS *lag = portStatus;
      for (PORT_STATUS *cur = portStatus->next;
           cur->next != NULL;
           cur=cur->next){
            if(currentTime-cur->timestamp>15000){
              lag->next=cur->next;
              free(cur);
            }
      }
    }
    
  }
  void RoutingProtocolImpl::HndAlm_FrdChk(){}


  




void RoutingProtocolImpl::recv(unsigned short port, void *packet, unsigned short size) {
  // add your own code
  PktDetail *pkt={0};
  pkt->src_id=0;
  get_pkt_detail(packet, pkt, size);
  char *buffer=(char *) packet;
  
  printf("receive starts\n");
  if(pkt->packet_type == DATA) {
         send_data(port, pkt, size);
  }
  else if(pkt->packet_type== PING){
        send_pong(port, buffer, size);
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




  void RoutingProtocolImpl::send_data(unsigned short port, PktDetail *pkt, unsigned short size){}

  void RoutingProtocolImpl::send_pong(unsigned short port, char *buffer, unsigned short size){
       //char *buffer= (char *) packet;
       // char type = *(char *)buffer;
        unsigned short src_id = ntohs(*(unsigned short *) (buffer + sizeof(int)));
        //unsigned short dest_id = ntohs(*(unsigned short *) (buffer + sizeof(int) + sizeof(unsigned short)));
       // *(char *)buffer=2;
        ePacketType PongPktType = PONG;
        memcpy(buffer,&PongPktType,1);
        //memcpy(buffer, PONG, 1);
        *(unsigned short *) (buffer + sizeof(int))=htons(RouterID);
        *(unsigned short *) (buffer + sizeof(int)+sizeof(unsigned short))=htons(src_id);
        sys->send(port, buffer, size);
      }

void RoutingProtocolImpl::update_port_status(unsigned short port, PktDetail *pkt, unsigned short size){
    unsigned short destID = pkt->dest_id;
    bool foundport = false;
    unsigned int timeDifference = (sys->time())  -  *(unsigned int*)(pkt->payload);
    for (PORT_STATUS *cur = portStatus;cur->next != NULL;cur=cur->next){
            if(cur->port_num==port){
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
  
  void RoutingProtocolImpl::updt_DV_RtTbl(unsigned short port, PktDetail *pkt, unsigned short size){}
  void RoutingProtocolImpl::updt_LS_RtTbl(unsigned short port, PktDetail *pkt, unsigned short size){}
  void RoutingProtocolImpl::get_pkt_detail(void *pkt, PktDetail *pkt_d, unsigned short size){
  char *pkt_copy={0};
    //pkt_copy = (char *) malloc(size);
  *pkt_copy = ntohs(*(char*) pkt);
    memcpy(&pkt_d->packet_type,&pkt_copy[0],1);//packetType
    //memcpy(&size,&pkt_copy[2],2);
    memcpy(&pkt_d->size,&pkt_copy[2],2);//size
    memcpy(&pkt_d->src_id,&pkt_copy[4],2);//srcID
    memcpy(&pkt_d->dest_id,&pkt_copy[6],2);//DestID
    memcpy(&pkt_d->payload,&pkt_copy[8],size-8);//payload
    
    
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
    char* buffer = (char*) malloc(pkt_d->size);
    
    unsigned short type =   (unsigned short) (pkt_d->packet_type);
    memcpy(&buffer[0], &type, 2);
    unsigned short msgsize = pkt_d->size;
    memcpy(&buffer[2], &msgsize, 2);
    unsigned short srcID = pkt_d->src_id;
    memcpy(&buffer[4], &srcID, 2);
    unsigned short destID = pkt_d->dest_id;
    memcpy(&buffer[6], &destID, 2);
    char* msgPayload = pkt_d->payload;
    memcpy(&buffer[8], &msgPayload, msgsize-8);
    HTONS_message(buffer,msgsize);
    sys->send(portNum,buffer,msgsize);


  }
  
  void RoutingProtocolImpl::NTOHS_message(char* buffer, unsigned short size){
    BufferConvHelper(buffer,size,true);
  }
  void RoutingProtocolImpl::HTONS_message(char* buffer, unsigned short size){
    BufferConvHelper(buffer,size,false);
  }
  void RoutingProtocolImpl::BufferConvHelper(char* buffer, unsigned short size,bool isNTOHS){
    char *newBuffer = (char*)malloc(size);
    for(unsigned short i=0;i<size;i+=2){
      unsigned short temp = (unsigned short)buffer[i];
      if(isNTOHS){
        temp=ntohs(temp);
      }
      else{
        temp=htons(temp);
      }
      memcpy(&newBuffer[i], &temp, 2);
    }
    char* freeMem = buffer;
    buffer=newBuffer;
    free(freeMem);
  }



