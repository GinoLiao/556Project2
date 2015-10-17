#include "RoutingProtocolImpl.h"
#include "Node.h"
#include "global.h"
#include <string.h>

RoutingProtocolImpl::RoutingProtocolImpl(Node *n) : RoutingProtocol(n) {
  sys = n;
  // add your own code

}

RoutingProtocolImpl::~RoutingProtocolImpl() {
  // add your own code (if needed)
}





void RoutingProtocolImpl::init(unsigned short num_ports, unsigned short router_id, eProtocolType protocol_type) {
  NumPorts=num_ports;
  RouterID = router_id;
  ProtocolType=protocol_type;
  InitPortStatus(NumPorts,RouterID);
  InitRoutingTable();
  MakePortStatus(NumPorts,RouterID);
  MakeForwardingTable();
  
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
    PORT_STATUS *cur = portStatus;
    for(unsigned short i=1; i <= num_ports;i++){
      if(i != router_id){ //if different port, put it to port status
        cur->id = i;
        cur->timestamp = sys->time();
        cur->TxDelay = INFINITY_COST; //set to inifinity
        cur = cur->next;
      }
    }
    
  }
  
  void RoutingProtocolImpl::InitRoutingTable(){}
  /*
    send ping to update port status with neighbors' delays
  */
  void RoutingProtocolImpl::MakePortStatus(unsigned short num_ports, unsigned short router_id){
  /*  for(unsigned short i=1;i<num_ports;i++){
      if(i != router_id){
        //make ping packet
        ping_pkt = (char *) malloc(65536);
        ping_pkt[strlen(ping_pkt)-1] = 0; //set its end
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
    //void *alarm_type;
    eAlarmType alarm_type = ALARM_PORT_STATUS;
    //memcpy(&alarm_type, &ALARM_PORT_STATUS, 16);
    handle_alarm((void *)alarm_type);
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

    if(convertedData==ALARM_PORT_STATUS){
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
  for(unsigned short i=1;i<num_ports;i++){
    if(i != router_id){ 
  //make ping packet
    char *ping_pkt;
        ping_pkt = (char *) malloc(65536);
        ping_pkt[strlen(ping_pkt)-1] = 0; //set its end
          //set packet type
        ePacketType PingPktType = PING;
        memcpy(&ping_pkt[0],&PingPktType,8);  
          //set size

        unsigned short PingPktSizeNet = 65535;
        memcpy(&ping_pkt[16],&PingPktSizeNet,16);
          //set srouce ID
        memcpy(&ping_pkt[32],&router_id,16);
          //destination ID unused in PING packet
          //set payload to time
    unsigned int time = sys->time();
        memcpy(&ping_pkt[64],&time,32);
        //send to all other ports to find neighbor
        sys->send(i, ping_pkt, PingPktSizeNet);
    }
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


  void RoutingProtocolImpl::send_data(unsigned short port, PktDetail *pkt, unsigned short size){}
  void RoutingProtocolImpl::send_pong(unsigned short port, PktDetail *pkt, unsigned short size){}
  void RoutingProtocolImpl::update_port_status(unsigned short port, PktDetail *pkt, unsigned short size){
    unsigned short idToRefresh = pkt->dest_id;
    for (PORT_STATUS *cur = portStatus;
           cur->next != NULL;
           cur=cur->next){
            if(cur->id==idToRefresh){
              cur->timestamp=sys->time();
            }
      }


  }
  void RoutingProtocolImpl::updt_DV_RtTbl(unsigned short port, PktDetail *pkt, unsigned short size){}
  void RoutingProtocolImpl::updt_LS_RtTbl(unsigned short port, PktDetail *pkt, unsigned short size){}
  void RoutingProtocolImpl::get_pkt_detail(void *pkt, PktDetail *pkt_d){

    pkt = NULL;
    /*pkt->packet_type = get_pkt_type(*packet);
    pkt->src_id = get_src_id(*packet);
    pkt->dest_id = get_dest_id(*packet);
    pkt->size = size;*/
    //get payload
    //return pkt;
  }




void RoutingProtocolImpl::recv(unsigned short port, void *packet, unsigned short size) {
  // add your own code
  PktDetail *pkt= {0};
  pkt->src_id=0;
  get_pkt_detail(packet, pkt);
  
  
  
  if(pkt->packet_type == DATA) {
         send_data(port, pkt, size);
  }
  else if(pkt->packet_type== PING){
        send_pong(port, pkt, size);
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






