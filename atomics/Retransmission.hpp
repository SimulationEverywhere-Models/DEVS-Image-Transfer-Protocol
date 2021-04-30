#ifndef __RETRANSMISSION_HPP__
#define __RETRANSMISSION_HPP__

#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/message_bag.hpp>

#include <limits>
#include <assert.h>
#include <string>
#include <iostream>

#include "../data_structures/packet_format.hpp"

using namespace cadmium;
using namespace std;

//Port definition
struct Retransmission_defs {
    struct sizeChange : public out_port<int> { };//size to generator
    struct packetIn : public in_port<Packet_t> { };//packet from forwarder
    struct resend : public out_port<Packet_t> { };// packet (to be resent) to PacketForwarder
    struct ackIn : public in_port<Packet_t> { }; // ack (still packet) from the network
};


template<typename TIME> class Retransmission{
    public:
      // ports definition
      using input_ports = std::tuple<typename Retransmission_defs::packetIn,typename Retransmission_defs::ackIn>;
      using output_ports = std::tuple<typename Retransmission_defs::sizeChange, typename Retransmission_defs::resend>;

      // state definition
      struct state_type {
          TIME sigma;
          bool active;
          Packet_t packetBuffer[10] = {};
          int packetIndex;
          Packet_t ackBuffer[10] = {};
          int ackIndex;
          bool resending;
          Packet_t currentPacket;
          Packet_t resendBuffer[10] = {};
          int resendIndex;
          double chunkLossLimit;
          int dataLoss;
          int dataTotal;
          bool resent;
          int size;
      };
      state_type state;

      // default constructor without parameters
      Retransmission() noexcept {
          state.sigma = std::numeric_limits<TIME>::infinity();
          state.active = false;
          state.packetIndex = 0;
          state.ackIndex = 0;
          state.resending = false;
          state.currentPacket = Packet_t(10,20);
          state.resendIndex = 0;
          state.chunkLossLimit = 0.2;
          state.dataLoss = 0;
          state.dataTotal = 0;
          state.size =10;
          state.resent = false;
      }

/*
After timeout, compare packets sent with ack received, determine lost packets.
Calculate data loss, compare with acceptable limit, decides if lost packets need to be resent or not.
If lost packets need to be resent, send it one by one to PacketForwarder, send {0,0} packet to indicate "done".
After each cycle, go back to passive state and reset all state variables.
*/
      void internal_transition() 
      {
        if (!state.resent && state.active)
        {
            if (!state.resending)
            {
                state.resending = true;
                state.dataLoss = 0;
                state.dataTotal = 0;
                for (int i=0;i<state.packetIndex;i++) //loop to store lost packets and calculate data loss
                {
                    state.dataTotal = state.dataTotal + state.packetBuffer[i].size;
                    bool received = false;
                    for (int j=0;j<state.ackIndex;j++)
                    {
                        if (state.ackBuffer[j].sequence == state.packetBuffer[i].sequence)
                        {    
                            received = true;
                            break;
                        }
                    }
                    if (!received)
                    {
                        state.resendBuffer[state.resendIndex] = state.packetBuffer[i];
                        state.resendIndex++;
                        state.dataLoss += state.packetBuffer[i].size;
                    }
                }
                
            }
            
            state.ackIndex = 0;
            state.packetIndex = 0;
            
            double lossRatio = (double)state.dataLoss/(double)state.dataTotal;
            
            if (lossRatio <= state.chunkLossLimit) //If loss is acceptable, do not rensend any packets
            {
                state.currentPacket = Packet_t{0,0};//send {0,0} to PacketForwarder to signal resend "done"
                state.resent = true;

                if (lossRatio <= state.chunkLossLimit*0.5) //If loss is low, double the packet size
                {
                    state.size = 2*state.packetBuffer[1].size;
                }             
            }
            else //resend is needed
            {
                if (lossRatio >= state.chunkLossLimit*2) //If loss is high, half the packet size
                {
                    state.size = state.packetBuffer[1].size/2;
                }
                else
                {
                    state.size = state.packetBuffer[1].size; //no change of packet size
                }  
                if ((state.resendIndex - 1) < 0) //During retransmission, check if last lost packet has been sent
                {                                //If all sent, send {0,0} to PacketForwarder
                    state.resent = true;
                    state.currentPacket = Packet_t{0,0};
                }
                else // If not all lost packets are resent, take the next lost packet to be resent
                {
                    state.currentPacket = state.resendBuffer[state.resendIndex-1];
                    state.resendIndex--;
                }
            }
            
            
            state.sigma = TIME("00:00:00:005"); //5 ms for sending packet to PacketForwarder
        }
        else //Reaches here if all resent, reset state variables
        {
            state.active = false;
            state.packetIndex = 0;
            state.ackIndex = 0;
            state.resending = false;
            state.currentPacket = Packet_t{0,0};
            state.resendIndex = 0;
            state.resent = false;
            state.sigma = std::numeric_limits<TIME>::infinity();
        }
      }


/*
External transition function.
Start with passive state, receives packets sent from PacketForwarder.
Packets are sent with a cycle of 10 (send 10 packets out one by one without Ack).
After 10 packets sent and buffered in Retransmission Controller, wait for Ack with 600 ms timeout.
After timeout, go to internal transition
*/
      void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {

          if ((get_messages<typename Retransmission_defs::packetIn>(mbs).size()+ get_messages<typename Retransmission_defs::ackIn>(mbs).size())> 1)
               assert(false && "one message per time uniti");
			
		  vector<Packet_t> packetIn, ackIn;
          packetIn = get_messages<typename Retransmission_defs::packetIn>(mbs);
          ackIn = get_messages<typename Retransmission_defs::ackIn>(mbs);
          
          if (!state.active) //if passive
          {
             state.sigma = std::numeric_limits<TIME>::infinity();
             
             if (packetIn.size()) //if received packetIn signal
             {
                if (packetIn[0].sequence == 1) //reset counter if sequence 1 received
                {
                    state.packetIndex = 0;
                    state.ackIndex = 0;
                }
                   
                state.packetBuffer[state.packetIndex] = packetIn[0]; //place coming packet into buffer
				
                if (state.packetIndex >= 9) // if buffer full , size is 10
                {
                    state.resent = false;
                    state.active = true; //active state to resend packets
                    state.sigma = TIME("00:00:00:600"); // 600 ms for timeout starting from last packet received
                }

                state.packetIndex++;
                
             }
             else if (ackIn.size()) //Ack could come during the transmission of a group of 10 packets
             {
                state.ackBuffer[state.ackIndex] = ackIn[0];
                state.ackIndex++;
             }
             
          }
          else // if active
          {
            if (packetIn.size()) //should not happen during retransmission
            {
                assert(false&&"should not receive packets during timeout,index");
            }
            else if (ackIn.size()) //during timeout period, wait for ACK
            {
                state.ackBuffer[state.ackIndex] = ackIn[0];
                state.ackIndex++;
                state.sigma = state.sigma - e;//continue the timeout
            }
          }
          
      }

      // confluence transition
      void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
          internal_transition();
          external_transition(TIME(), std::move(mbs));
      }

      // output function
      typename make_message_bags<output_ports>::type output() const {
          typename make_message_bags<output_ports>::type bags;
          
          
          if(state.active && state.resending)
          {
            get_messages<typename Retransmission_defs::resend>(bags).push_back(state.currentPacket);
            get_messages<typename Retransmission_defs::sizeChange>(bags).push_back(state.size);
          }
          return bags;
      }

      // time_advance function
      TIME time_advance() const {
          return state.sigma;
      }

      friend std::ostringstream& operator<<(std::ostringstream& os, const typename Retransmission<TIME>::state_type& i) {
          os << "current index is: " << i.packetIndex<< "|| dataLoss is: " << i.dataLoss;
          return os;
      }
};
#endif // __PACKETGENERATOR_HPP__
