#include "RoutingProtocolImpl.h"

RoutingProtocolImpl::RoutingProtocolImpl(Node *n) : RoutingProtocol(n) {
  sys = n;
  // add your own code
  
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
}

struct RoutingTable{
	int id;
	int Destination;	//destination id 
	int NextHop;		//next hop id
	int Distance;
	int timestamp;
	struct RoutingTable *next;	
}


RoutingProtocolImpl::~RoutingProtocolImpl() {
  // add your own code (if needed)
}

void RoutingProtocolImpl::init(unsigned short num_ports, unsigned short router_id, eProtocolType protocol_type) {
  // add your own code
}

void RoutingProtocolImpl::handle_alarm(void *data) {
  // add your own code
}

struct pkt_detail{
	ePacketType packet_type;
	unsigned short src_id;
	unsigned short dest_id
	char* payload;
	struct pkt_detail *next;
};

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

// add more of your own code
