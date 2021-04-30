#ifndef __NetworkDelay_HPP__
#define __NetworkDelay_HPP__


#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/message_bag.hpp>

#include <limits>
#include <assert.h>
#include <string>

#include "../data_structures/packet_format.hpp"
#include <random>

using namespace cadmium;
using namespace std;

#define MAX_QUEUE     100  

//Port definition
struct NetworkDelay_defs {
    struct out : public out_port<Packet_t> { };
    struct in : public in_port<Packet_t> { };
};


template<typename TIME> class NetworkDelay {
public:
    // ports definition
    using input_ports = std::tuple<typename NetworkDelay_defs::in>;
    using output_ports = std::tuple<typename NetworkDelay_defs::out>;
    
    struct PacketDelayPair {
    PacketDelayPair() {}
    PacketDelayPair(Packet_t i_packet, TIME i_delay)
        :packet(i_packet), delay(i_delay) {}

    Packet_t packet;
    TIME delay;
    };

    // state definition
    struct state_type {
        TIME sigma;
        bool active;
        Packet_t currentPacket;
        TIME totalDelayAhead;
        TIME delay;
        int queueIndex;
        PacketDelayPair packetDelayPair[MAX_QUEUE] = {};
        const TIME networkDelay = TIME("00:00:00:050");
    };
    state_type state;

    // default constructor without parameters
    NetworkDelay() noexcept {
        state.sigma = std::numeric_limits<TIME>::infinity();
        state.active = false;
        state.currentPacket = Packet_t(0,0);
        state.totalDelayAhead = TIME("00:00:00:000");
        state.delay = TIME("00:00:00:000");
        state.queueIndex = 0;
    }


    void internal_transition() 
    {
        if (state.active)
        {
            if (state.queueIndex <= 0) //If there is no packet in the queue, then go back to passive state.
            {
                state.active = false;
                state.sigma = std::numeric_limits<TIME>::infinity();
            }
            else //If queue is not empty
            {
                state.currentPacket = state.packetDelayPair[0].packet;//FIFO, take the first packet in the queue
                
                state.sigma = state.packetDelayPair[0].delay;
                
                for (int i = 1; i < state.queueIndex;i++)
                {
                    state.packetDelayPair[i-1] = state.packetDelayPair[i]; //shift to left, second one become first the one to be sent out
                }
                
                state.queueIndex--;
            }
        }
        else
        {
            assert(false&&"NetworkDelay: internal transition only have active state");
        }
    }

/*
External transition
Passive state: there is no packets in NetworkDelay module, store coming packet,change state to be active and start delay timeout.
Active state: there is at least one packet in the delay process.
If in active state, calculate total delay ahead, then calculate the delay for coming packet, store delay and packet in a queue
*/
    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {

        if (get_messages<typename NetworkDelay_defs::in>(mbs).size() > 1)
            assert(false && "one message per time uniti");

        vector<Packet_t> packet_port_in;
        packet_port_in = get_messages<typename NetworkDelay_defs::in>(mbs);
        
        
        if (!state.active) //If passive state
        {
            state.currentPacket = packet_port_in[0];
                            
            state.sigma = state.networkDelay; //start delay process with fixed network delat
            
            state.active = true; //change state to active
        }
        else //If active state
        {
            state.sigma = state.sigma - e;//this is remaining delay for the current packet
            state.totalDelayAhead = TIME("00:00:00");
            for (int i=0;i<state.queueIndex;i++) //loop through the packets already in the queue
            {
                state.totalDelayAhead = state.totalDelayAhead + state.packetDelayPair[i].delay;
            }
            state.totalDelayAhead += state.sigma;
            
            if (state.totalDelayAhead > state.networkDelay)
                assert(false && "total delay cannot be bigger than network delay");
            
            state.delay = state.networkDelay - state.totalDelayAhead;
            state.packetDelayPair[state.queueIndex] = PacketDelayPair(packet_port_in[0],state.delay);
            state.queueIndex++;
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

        get_messages<typename NetworkDelay_defs::out>(bags).push_back(state.currentPacket);
        
        return bags;
    }

    // time_advance function
    TIME time_advance() const {
        return state.sigma;
    }
    
    friend std::ostringstream& operator<<(std::ostringstream& os, const typename NetworkDelay<TIME>::state_type& i) {
        os << "current PacketSequence is: " << i.currentPacket.sequence << " & size is: " << i.currentPacket.size;
        return os;
    }
};
#endif // __NetworkDelay_HPP__
