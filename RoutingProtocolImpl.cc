#include "RoutingProtocolImpl.h"

RoutingProtocolImpl::RoutingProtocolImpl(Node *n) : RoutingProtocol(n) {
  sys = n;
  // add your own code

}





void RoutingProtocolImpl::init(unsigned short num_ports, unsigned short router_id, eProtocolType protocol_type) {
  NumPorts=num_ports;
  RouterID = router_id;
  ProtocolType=protocol_type;
  InitPortStatus();
  InitRoutingTable();
  MakePortStatus();
  MakeForwardingTable();
  SetPortStatusAlarm();//every 10 sec
  SetForwardingAlarm();//every 30 sec
  SetPortCheckAlarm();//every 1 sec
  SetForwardCheckAlarm();//every 1 sec
}
void RoutingProtocolImpl::InitPortStatus(){}
void RoutingProtocolImpl::InitRoutingTable(){}
void RoutingProtocolImpl::MakePortStatus(){}
void RoutingProtocolImpl::MakeForwardingTable(){}
void RoutingProtocolImpl::SetPortStatusAlarm(){}
void RoutingProtocolImpl::SetForwardingAlarm(){}
void RoutingProtocolImpl::SetPortCheckAlarm(){}
void RoutingProtocolImpl::SetForwardCheckAlarm(){}

void RoutingProtocolImpl::handle_alarm(void *data) {
    eAlarmType convertedData = *(eAlarmType*) data;

    if(convertedData==ALARM_PORT_STATUS){
        HndAlm_PrtStat();
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

void RoutingProtocolImpl::HndAlm_PrtStat(){}
void RoutingProtocolImpl::HndAlm_frd(){}
void RoutingProtocolImpl::HndAlm_PrtChk(){}
void RoutingProtocolImpl::HndAlm_FrdChk(){}







void RoutingProtocolImpl::recv(unsigned short port, void *packet, unsigned short size) {
  // add your own code
  pkt_detail pkt;
  get_pkt_detail(packet, pkt);
  
  
  
  if(pkt.packet_type == DATA) {
         send_data(port, pkt, size);
  }
  else if(pkt.packet_type== PING){
        send_pong(port, pkt, size);
  }
  else if(pkt.packet_type==PONG){
        update_port_status(port, pkt, size);
  }
  else if(pkt.packet_type==DV){
        updt_DV_RtTbl(port, pkt, size);
  }
  else if(pkt.packet_type==LS){
        updt_LS_RtTbl(port, pkt, size);
  }
  else{
        printf("This packet type cannot be handled! \n");
  } 
   
   
}


  void RoutingProtocolImpl::send_data(unsigned short port, pkt_detail pkt, unsigned short size){}
  void RoutingProtocolImpl::send_pong(unsigned short port, pkt_detail pkt, unsigned short size){}
  void RoutingProtocolImpl::update_port_status(unsigned short port, pkt_detail pkt, unsigned short size){}
  void RoutingProtocolImpl::updt_DV_RtTbl(unsigned short port, pkt_detail pkt, unsigned short size){}
  void RoutingProtocolImpl::updt_LS_RtTbl(unsigned short port, pkt_detail pkt, unsigned short size){}
  void RoutingProtocolImpl::get_pkt_detail(void *pkt, pkt_detail pkt_d){
    pkt = NULL;
    /*pkt->packet_type = get_pkt_type(*packet);
    pkt->src_id = get_src_id(*packet);
    pkt->dest_id = get_dest_id(*packet);
    pkt->size = size;*/
    //get payload
    //return pkt;
  }



