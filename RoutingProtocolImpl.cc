#include "RoutingProtocolImpl.h"

RoutingProtocolImpl::RoutingProtocolImpl(Node *n) : RoutingProtocol(n) {
  sys = n;
  // add your own code
	//Alarm type
    enum eAlarmType {
    	ALARM_PORT_STATUS,
    	ALARM_FORWARDING,
    	ALARM_PORT_CHECK,
    	ALARM_FORWARD_CHECK
    };
    //Router ID
    unsigned short RouterID;
    //Protocol Type
    eProtocolType ProtocolType;
    //number of ports
      unsigned short NumPorts;

}

struct PortStatus{
	int id;
	int timestamp;
	int TxDelay;
	struct PortStatus *next;
};

//now: DV only
struct RoutingTable_DV{
	int id;
	int Destination;	//destination id 
	int NextHop;		//next hop id
	int Distance;
	int timestamp;
	struct RoutingTable_DV *next;	
};
//incomplete LS implementation
struct RoutingTable_LS{
	struct RoutingTable_LS *next;    
};





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
	switch ( (*eAlarmType)* data) {
   	 case ALARM_PORT_STATUS:
     		 HndAlm_PrtStat();
     	 break;
   	 case ALARM_FORWARDING:
     		 HndAlm_frd():
         break;
         case ALARM_PORT_CHECK
        	 HndAlm_PrtChk();
         break;
         case ALARM_FORWARD_CHECK
        	 HndAlm_FrdChk();
         break;
   	 default:
     		 printf("This alarm type cannot be handled! \n");
     	 break;
    }
}
void RoutingProtocolImpl::HndAlm_PrtStat(){}
void RoutingProtocolImpl::HndAlm_frd(){}
void RoutingProtocolImpl::HndAlm_PrtChk(){}
void RoutingProtocolImpl::HndAlm_FrdChk(){}



void RoutingProtocolImpl::recv(unsigned short port, void *packet, unsigned short size) {
  // add your own code
  pkt_detail pkt;
  pkt->packet_type = get_pkt_type(*packet);
  pkt->src_id = get_src_id(*packet);
  pkt->dest_id = get_dest_id(*packet);
  
  
   switch (pkt->packet_type) {
    case DATA:
     	 send_data(port, pkt, size);
      break;
    case PING:
     	 send_pong(port, pkt, size):
	 break;
	 case PONG
		 update_port_status(port, pkt, size);
	 break;
	 case DV
		 updt_DV_RtTbl(port, pkt, size);
	 break;
	 case LS
		 updt_LS_RtTbl(port, pkt, size);
	 break;
    default:
     	 printf("This packet type cannot be handled! \n");
      break;
	}
  
}


  void RoutingProtocolImpl::send_data(unsigned short port, pkt_detail pkt, unsigned short size){}
  void RoutingProtocolImpl::send_pong(unsigned short port, pkt_detail pkt, unsigned short size){}
  void RoutingProtocolImpl::update_port_status(unsigned short port, pkt_detail pkt, unsigned short size){}
  void RoutingProtocolImpl::updt_DV_RtTbl(unsigned short port, pkt_detail pkt, unsigned short size){}
  void RoutingProtocolImpl::updt_LS_RtTbl(unsigned short port, pkt_detail pkt, unsigned short size){}











