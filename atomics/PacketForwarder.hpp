#ifndef __PACKETFORWARDER_HPP__
#define __PACKETFORWARDER_HPP__

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
struct PacketForwarder_defs {
    struct resendIn : public in_port<Packet_t> { };//resend from retransmission controller
    struct packetIn : public in_port<Packet_t> { };//packet from generator
    struct packetOut : public out_port<Packet_t> { };// packet to network
    struct nextPacket : public out_port<int> { }; // signal to generator
};


template<typename TIME> class PacketForwarder{
    public:
      // ports definition
      using input_ports = std::tuple<typename PacketForwarder_defs::packetIn,typename PacketForwarder_defs::resendIn>;
      using output_ports = std::tuple<typename PacketForwarder_defs::packetOut, typename PacketForwarder_defs::nextPacket>;

      // state definition
      struct state_type {
          TIME sigma;
          bool active;
          Packet_t currentPacket;
          int lastSeq;
          bool resending;
          bool sending;
          bool resendReceiving;
          Packet_t resendBuffer[10] = {};
          int resendIndex;
          int sequence;
          bool signalNextPacket;
          bool lastPacketSent;
      };
      state_type state;

      // default constructor without parameters
      PacketForwarder() noexcept {
          state.sigma = std::numeric_limits<TIME>::infinity();
          state.sending = false;
          state.currentPacket = Packet_t(0,0);
          state.lastSeq = 10;
          state.resending = false;
          state.resendReceiving = false;
          state.resendIndex = 0;
          state.sequence = 0;
          state.signalNextPacket = false;
          state.lastPacketSent = false;
      }

      // internal transition
      void internal_transition() 
      {
        if (state.resending && ! state.resendReceiving) //if in state resending
        {
            if (state.resendIndex <= 0) //If resend buffer is empty
            {
                state.resendIndex = 0;
                state.resending = false;
                state.signalNextPacket = true;
                state.currentPacket = Packet_t(0,0); //set packet size to 0 so the output function will not send any packet
                state.sigma = TIME("00:00:00:600"); // wait 600 ms to signal generator after retransmission done (
            }
            else
            {
                state.resendIndex--;
                state.currentPacket = state.resendBuffer[state.resendIndex];
                state.sigma = TIME("00:00:00:005"); //5ms forward time
            }
        }
        else if (state.sending)//for forwarding packets from generator
        {
            state.sending = false;
            state.sigma = std::numeric_limits<TIME>::infinity();
        }
        else //Not necessary, but to represent resending is done (could be merged with code above)
        {
            state.signalNextPacket = false;
            state.sigma = std::numeric_limits<TIME>::infinity();
        }
      }

      // external transition
      void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {

          if ((get_messages<typename PacketForwarder_defs::packetIn>(mbs).size()+ get_messages<typename PacketForwarder_defs::resendIn>(mbs).size())> 1)
               assert(false && "one message per time uniti");
			
		  vector<Packet_t> packetIn, resendIn;
          packetIn = get_messages<typename PacketForwarder_defs::packetIn>(mbs);
          resendIn = get_messages<typename PacketForwarder_defs::resendIn>(mbs);
          
          if (!state.sending)//If not in sending state (passive state)
          {
            if (packetIn.size())//packet coming from PacketGenerator
            {
                state.currentPacket = packetIn[0];
                state.sequence = state.currentPacket.sequence;
                state.signalNextPacket = false; // this is only true for after retransmision done.
                state.sending = true;//change to state sending
                state.sigma = TIME("00:00:00:005"); //5ms forward time
            }
            else if (resendIn.size()) //packet from the retransmission controller
            {
                state.sequence = 0;
                state.signalNextPacket = false;
                state.currentPacket = resendIn[0];
                if (state.currentPacket.size != 0) //check if {0,0} received, none-zero size indicates retransmission controller in progress
                {
                    state.resendReceiving = true; //still receiving from retransmission controller
                    state.resendBuffer[state.resendIndex] = state.currentPacket;
                    state.resendIndex++;
                    state.sigma = std::numeric_limits<TIME>::infinity();
                }
                else //if size is 0, retransmission from retransmission controller done
                {
                    if (state.resendReceiving)//If previously receving from retransmission controller
                        state.resending = true;//Now start resending packets
                    state.resendReceiving = false;
                    state.sigma = TIME("00:00:00:000");
                }
                
            }
            else
            {
                //should not happen
            }
          }
          else //sending state
          {
            //should not occur, it should synchronize with generator and retransmission controller 
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
          
          
          if (state.currentPacket.size != 0)
            get_messages<typename PacketForwarder_defs::packetOut>(bags).push_back(state.currentPacket);
          
          if ((state.sequence != state.lastSeq && !state.resending && !state.resendReceiving) || state.signalNextPacket)
            get_messages<typename PacketForwarder_defs::nextPacket>(bags).push_back((int)1);
          
          return bags;
      }

      // time_advance function
      TIME time_advance() const {
          return state.sigma;
      }

      friend std::ostringstream& operator<<(std::ostringstream& os, const typename PacketForwarder<TIME>::state_type& i) {
          os << "current size is: " << i.currentPacket.size<< "|| seq # is: " << i.currentPacket.sequence;
          return os;
      }
};
#endif // __PACKETGENERATOR_HPP__
