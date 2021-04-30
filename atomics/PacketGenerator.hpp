#ifndef __PACKETGENERATOR_HPP__
#define __PACKETGENERATOR_HPP__

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
struct PacketGenerator_defs {
    struct nextPacket : public in_port<int> { };//1 for next packet, 0 for stop
    struct startSignal : public in_port<int> { };//# for total data at the begining, 0 for non-start
    struct sizeChange : public in_port<int> { };// 0 for not changing, otherwize change the size to the integer
    struct packetDone : public out_port<Packet_t> { };
};

template<typename TIME> class PacketGenerator{
    public:
      // ports definition
      using input_ports = std::tuple<typename PacketGenerator_defs::nextPacket,typename PacketGenerator_defs::startSignal,typename PacketGenerator_defs::sizeChange>;
      using output_ports = std::tuple<typename PacketGenerator_defs::packetDone>;

      // state definition
      struct state_type {
          TIME sigma;
          bool active;
          bool sending;
          int current_sequence;
          int current_size;
          int totalGeneratedSize;
          int totalDataSize;
          int packetSize;
          int sequence;
      };
      state_type state;

      // default constructor without parameters
      PacketGenerator() noexcept {
          state.sigma = std::numeric_limits<TIME>::infinity();
          state.active = false;
          state.sending = false;
          state.current_sequence = 0;
          state.current_size = 0;
          state.totalGeneratedSize = 0;
          state.totalDataSize = 0;//default value
          state.packetSize = 10;//default value
          state.sequence = 1;
      }

      // internal transition
      void internal_transition() {
          if (state.active) //in active state until all data sent
          {
            if(state.sending)
            {
              if(state.totalGeneratedSize < state.totalDataSize)
              {
                if((state.totalGeneratedSize + state.packetSize) > state.totalDataSize)
                  state.packetSize = state.totalDataSize - state.totalGeneratedSize;
                  
                state.current_size = state.packetSize;
				state.current_sequence = state.sequence;
                state.sequence++;
                
                if (state.sequence >= 11)//packet sequence from 1 to 10
                    state.sequence = 1;
                    
				state.totalGeneratedSize += state.packetSize;
              }else // all data sent
              {
                state.active = false;
              }
            }
          }
          state.sigma = std::numeric_limits<TIME>::infinity();
          state.sending = false;
      }

      // external transition
      void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
			
		  vector<int> start_in, next_packet_in, size_change_in;
          start_in = get_messages<typename PacketGenerator_defs::startSignal>(mbs);
          next_packet_in = get_messages<typename PacketGenerator_defs::nextPacket>(mbs);
          size_change_in = get_messages<typename PacketGenerator_defs::sizeChange>(mbs);
		  //std::cout<<start_in.size()<< " "<<next_packet_in.size()<< " "<<size_change_in.size();
		  
		  if(!state.active){//passive state		  
            if(start_in.size()){//if it is start signal
              state.active = true;//once start, state set to active
              state.totalDataSize = start_in[0];//read total amount of data to be sent from start signal
              state.sending = true;//start sending

              if(state.packetSize > state.totalDataSize)//check current packet size, it should be less than total data to be sent
                assert(false && "illegal packet size");

              state.current_sequence = state.sequence;
              state.current_size = state.packetSize;
              state.sequence++;
			  state.totalGeneratedSize += state.packetSize;//keep track of total generated data

              state.sigma = TIME("00:00:00");
            }
          }else{//once start, it is in active state, it might receive sizeChange signal from retransmission controller
            if(size_change_in.size() && size_change_in[0]!=0){
              state.packetSize = size_change_in[0];
            }
			
			if(next_packet_in.size()){//wait for nextPacket signal from PacketForwarder
              state.sending = true;
            }
			state.sigma = TIME("00:00:00");
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
          Packet_t out_aux;
          out_aux = Packet_t(state.current_size, state.current_sequence);
          if(state.active && state.sending)
            get_messages<typename PacketGenerator_defs::packetDone>(bags).push_back(out_aux);
          return bags;
      }

      // time_advance function
      TIME time_advance() const {
          return state.sigma;
      }

      friend std::ostringstream& operator<<(std::ostringstream& os, const typename PacketGenerator<TIME>::state_type& i) {
          os << "current size is: " << i.current_size<< "|| seq # is: " << i.current_sequence << " || totalGeneratedSize is: " << i.totalGeneratedSize;
          return os;
      }
};
#endif // __PACKETGENERATOR_HPP__
