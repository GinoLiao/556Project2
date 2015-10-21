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

  static eAlarmType port_status_alarm_type = ALARM_PORT_STATUS;
  (void) port_status_alarm_type;
  //SetPortStatusAlarm(this, 10000, &port_status_alarm_type);//every 10 sec
  //sys->set_alarm(this, 10000, &port_status_alarm_type);
  
  SetForwardingAlarm();//every 30 sec
  
  static eAlarmType port_check_alarm_type = ALARM_PORT_CHECK;
  (void) port_check_alarm_type;
  //SetPortCheckAlarm(this, 1000, &port_check_alarm_type);//every 1 sec
  sys->set_alarm(this, 1000, &port_check_alarm_type);
  
  SetForwardCheckAlarm();//every 1 sec
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
		eAlarmType alarm_type = ALARM_PORT_STATUS;
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
	//printf("handle alarm starts\n");
	//printf("handle type %d \n",convertedData);
    if(convertedData==ALARM_PORT_STATUS){
		printf("handle port status alarm.\n");
        HndAlm_PrtStat(NumPorts, RouterID);	
		static eAlarmType port_status_alrm_type = ALARM_PORT_STATUS;
		(void) port_status_alrm_type;
		printf("setting port update alarm. router: %d\n", RouterID);
		sys->set_alarm(this, 10000, &port_status_alrm_type);
    }
    else if(convertedData == ALARM_FORWARDING){
        HndAlm_frd(); 
    }
    else if(convertedData == ALARM_PORT_CHECK){
		printf("handle port check alarm.\n");
		HndAlm_PrtChk();
		static eAlarmType port_check_alrm_type = ALARM_PORT_CHECK;
		(void) port_check_alrm_type;
		sys->set_alarm(this, 1000, &port_check_alrm_type);
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
	//printf("Num of ports: %d, router ID: %d \n", num_ports, router_id);
	for(unsigned short i=0;i<num_ports;i++){
/*		PktDetail *pkt= new PktDetail();
		ePacketType pkt_type = PING;
		//printf("before casting packet type\n");
		pkt->packet_type = (unsigned short)pkt_type;
		//printf("packet type set\n");
		pkt->src_id = router_id;
		//printf("before setting size.\n");
		pkt->size = 12;
		unsigned int time = sys->time();
		//printf("before setting payload to time.\n");
		pkt->payload = (char *) malloc(4);
		memcpy(&(pkt->payload),&time,4);
		//cout<<"time: "<<time<<endl;
		//cout<<"time pointer: "<<timeptr<<endl;
		//cout<<"ping payload time "<<(unsigned int *)(pkt->payload)<<endl;
		//printf("ready to send msg.\n");
		SendMsg(pkt,i);
		//printf("send done\n");*/
		printf("num ports: %d \n", num_ports);
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
		sys->send(i,buffer,msgsize);
	}
  }
  
  void RoutingProtocolImpl::HndAlm_frd(){}
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
  void RoutingProtocolImpl::HndAlm_FrdChk(){}


  




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
         send_data(port, pkt, size);
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




  void RoutingProtocolImpl::send_data(unsigned short port, PktDetail *pkt, unsigned short size){}

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
  
  void RoutingProtocolImpl::updt_DV_RtTbl(unsigned short port, PktDetail *pkt, unsigned short size){}
  void RoutingProtocolImpl::updt_LS_RtTbl(unsigned short port, PktDetail *pkt, unsigned short size){}
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
		memcpy(&pkt_d->payload,&pkt_copy[8],size-8);//payload
		//printf("pkt_d->payload: %d\n", *(int *)&pkt_d->payload);
		
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
	//cout<<"buffer: "<<strlen(buffer)<<endl;
	//cout<<"pkt size "<<pkt_d->size<<endl;
    unsigned short type =   (unsigned short) (pkt_d->packet_type);
	cout<<"pkt_detail type "<<pkt_d->packet_type<<endl;
    memcpy(&buffer[0], &type, 2);
	cout<< "pkt type: "<<(int)buffer[0]<<endl;
    unsigned short msgsize = pkt_d->size;
	//cout<<"msgsize: "<< msgsize<<endl;
    memcpy(&buffer[2], &msgsize, 2);
    unsigned short srcID = pkt_d->src_id;
    //cout<<"src id "<<pkt_d->src_id<<endl;
	memcpy(&buffer[4], &srcID, 2);
    unsigned short destID = pkt_d->dest_id;
	//cout<<"dest id "<<pkt_d->dest_id<<endl;
    memcpy(&buffer[6], &destID, 2);
	char* msgPayload = (char *) malloc(msgsize-8);
    memcpy(&msgPayload,&(pkt_d->payload),4);
	//cout<<"payload "<<(unsigned int *)(pkt_d->payload)<<endl;
    memcpy(&buffer[8], &msgPayload, msgsize-8);
	//cout<<"msgPayload "<<(unsigned int *)(msgPayload)<<endl;
    //cout<<"buffer: "<<strlen(buffer)<<endl;
	HTONS_message(buffer,msgsize);
	//cout<<"buffer Net: "<<strlen(buffer)<<endl;
	//cout<<"before sys send"<<endl;
	//cout<<"msg size: "<<msgsize<<endl;
	cout<< "pkt type: "<<(int)buffer[0]<<endl;
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
    for(unsigned short i=2;i<size;i+=2){
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



