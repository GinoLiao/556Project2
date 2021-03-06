#ifndef ROUTINGPROTOCOLIMPL_H
#define ROUTINGPROTOCOLIMPL_H

#include "RoutingProtocol.h"

class RoutingProtocolImpl : public RoutingProtocol {
  public:
    RoutingProtocolImpl(Node *n);
    ~RoutingProtocolImpl();

    void init(unsigned short num_ports, unsigned short router_id, eProtocolType protocol_type);
    // As discussed in the assignment document, your RoutingProtocolImpl is
    // first initialized with the total number of ports on the router,
    // the router's ID, and the protocol type (P_DV or P_LS) that
    // should be used. See global.h for definitions of constants P_DV
    // and P_LS.

    void handle_alarm(void *data);
    // As discussed in the assignment document, when an alarm scheduled by your
    // RoutingProtoclImpl fires, your RoutingProtocolImpl's
    // handle_alarm() function will be called, with the original piece
    // of "data" memory supplied to set_alarm() provided. After you
    // handle an alarm, the memory pointed to by "data" is under your
    // ownership and you should free it if appropriate.

    void recv(unsigned short port, void *packet, unsigned short size);
    // When a packet is received, your recv() function will be called
    // with the port number on which the packet arrives from, the
    // pointer to the packet memory, and the size of the packet in
    // bytes. When you receive a packet, the packet memory is under
    // your ownership and you should free it if appropriate. When a
    // DATA packet is created at a router by the simulator, your
    // recv() function will be called for such DATA packet, but with a
    // special port number of SPECIAL_PORT (see global.h) to indicate
    // that the packet is generated locally and not received from 
    // a neighbor router.
    struct pkt_detail_impl{
        unsigned short packet_type;
        unsigned short src_id;
        unsigned short dest_id;
        unsigned short size;
        char* payload;
    };

    typedef struct pkt_detail_impl PktDetail;
    

    
    


 private:
    /*
        TYPE DEFINITIONS
    */
        struct PortStatus{
          int id;	//id the port goes to
		  int port_num;
          int timestamp;
          int TxDelay;
          struct PortStatus *next;
        };
        typedef struct PortStatus PORT_STATUS;

        //now: DV only
        struct RoutingTable_DV{
          int Destination;  //destination id 
          int NextHop;      //next hop id
          int Distance;
          int timestamp;
          struct RoutingTable_DV *next; 
        };

        typedef struct RoutingTable_DV ROUT_TBL_DV;


        //LS Data Structures
        struct RoutingTable_LS{
          int Destination;
          int NextHop;
          int Distance;
          int timestamp;
          unsigned int seqNum;
          struct RoutingTable_LS *next;   
        };
        typedef struct RoutingTable_LS ROUT_TBL_LS;

        struct Node_LS{
          int id;
          int Distance;
          struct Node_LS *next;
        };
        typedef struct Node_LS NODE_LS;


            //Alarm type
        enum eAlarmType {
          ALARM_PORT_STATUS,
          ALARM_FORWARDING,
          ALARM_PORT_CHECK,
          ALARM_FORWARD_CHECK
        };

  
    /*
        VARIABLE DEFINITIONS
    */
      Node *sys; // To store Node object; used to access GSR9999 interfaces
      PORT_STATUS *portStatus;
      ROUT_TBL_DV *routTblDV;
      //LS data structures
      ROUT_TBL_LS *routTblLS;
      NODE_LS *frontier;
      NODE_LS *neighbors;
      NODE_LS *minDistNode;
      NODE_LS *allKnownNodes;
      unsigned int seqNumSelf;
      
      //Router ID
      unsigned short RouterID;
      //Protocol Type
      eProtocolType ProtocolType;
      //number of ports
      unsigned short NumPorts;
    
    
    
    

    /*
    Function definitions
    */
      void InitPortStatus(unsigned short num_ports, unsigned short router_id);
      void InitRoutingTable();
      void MakePortStatus(unsigned short num_ports, unsigned short router_id);
      void MakeForwardingTable();
      void SetPortStatusAlarm(RoutingProtocol *r, unsigned int duration, void *data);
      void SetForwardingAlarm();
      //void SetPortCheckAlarm(RoutingProtocol *r, unsigned int duration, void *data);
      //void SetForwardCheckAlarm();
      void HndAlm_PrtStat(unsigned short num_ports, unsigned short router_id);
      void HndAlm_frd();
      void HndAlm_PrtChk();
      void HndAlm_FrdChk();
      void send_data(unsigned short port, PktDetail *pkt, char* buffer, unsigned short size);
      void send_pong(unsigned short port, char* packet, unsigned short size);
      void update_port_status(unsigned short port, PktDetail *pkt, unsigned short size);
      void get_pkt_detail(void *pkt, PktDetail *pkt_d, unsigned short size);
  	  void SendMsg(PktDetail* pkt_d, unsigned short portNum);
  	  void NTOHS_message(char* buffer, unsigned short size);
  	  void HTONS_message(char* buffer, unsigned short size);
  	  void BufferConvHelper(char* buffer, unsigned short size,bool isNTOHS);
  	  void SendAllNeighbors(PktDetail* pkt_d);

      
      
      unsigned short findPortNumber(unsigned short neighborID);
      bool isNeighbor(unsigned short routID);

      //LS functions
      void HndAlm_FrdChk_LS();
      void send_data_LS(unsigned short port, PktDetail *pkt, char* buffer, unsigned short size);
      void HndAlm_frd_LS();
      void InitRoutingTable_LS();
      void updt_LS_RtTbl(unsigned short port, PktDetail *pkt, unsigned short size);

      void addEntriesToLSRoutingTable(PktDetail *pkt);
      void removeFromFrontier(unsigned short remove_id);
      void addToFrontier(PktDetail *pkt);
      void forwardPacket(PktDetail *pkt);
      void findMinInFrontier();
      void flood_LS();



      //DV functions
      void HndAlm_FrdChk_DV();
      void send_data_DV(unsigned short port, PktDetail *pkt, char* buffer, unsigned short size);
      void InitRoutingTable_DV();
      void updt_DV_RtTbl(unsigned short port, PktDetail *pkt, unsigned short size);
      void HndAlm_frd_DV();



      
      
      
      
      
      
      
      

      

    
      
    
};

#endif

