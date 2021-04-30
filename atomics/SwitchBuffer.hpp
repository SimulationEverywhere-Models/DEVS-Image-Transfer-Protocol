#ifndef __SwitchBuffer_HPP__
#define __SwitchBuffer_HPP__


#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/message_bag.hpp>

#include <limits>
#include <assert.h>
#include <string>

#include "../data_structures/packet_format.hpp"
#include <random>
#include <ctime>

using namespace cadmium;
using namespace std;

#define MAX_BUFFER_SIZE 40

//Port definition
struct SwitchBuffer_defs {
    struct out : public out_port<Packet_t> { };
    struct in : public in_port<Packet_t> { };
};

template<typename TIME> class SwitchBuffer {
public:
    // ports definition
    using input_ports = std::tuple<typename SwitchBuffer_defs::in>;
    using output_ports = std::tuple<typename SwitchBuffer_defs::out>;

    // state definition
    struct state_type {
        TIME sigma;
        bool active;
        Packet_t currentPacket;
        int currentBufferSize;
    };
    state_type state;

    // default constructor without parameters
    SwitchBuffer() noexcept {
        state.sigma = std::numeric_limits<TIME>::infinity();
        state.active = false;
        state.currentPacket = Packet_t(0,0);
        state.currentBufferSize = 20;
        //feed time as seed for random number generator
        srand ( time(NULL) );
    }

    // internal transition
    void internal_transition() {
        if (state.active) {
            state.active = false;
            state.sigma = std::numeric_limits<TIME>::infinity();
        }
    }

    // external transition
    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {

        if (get_messages<typename SwitchBuffer_defs::in>(mbs).size() > 1)
            assert(false && "one message per time uniti");

        vector<Packet_t> packet_port_in;
        packet_port_in = get_messages<typename SwitchBuffer_defs::in>(mbs);
        if (!state.active)
        {
            state.currentPacket = packet_port_in[0];
            
            // cast the random (0,1) to range (0,MAX_BUFFER_SIZE) 
            state.currentBufferSize = ((double)rand()/(double)RAND_MAX)*MAX_BUFFER_SIZE;
            
            if (state.currentBufferSize > state.currentPacket.size)
            {    // if packet fits into buffer, send it out
                state.active = true;
                state.sigma = TIME("00:00:00");
            }
            else
            {
                // do not send it
                state.sigma = std::numeric_limits<TIME>::infinity();
            }
        }
        else
        {
            assert(false&&"SwitchBuffer: should not receive packets in active state");    
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

        get_messages<typename SwitchBuffer_defs::out>(bags).push_back(state.currentPacket);
        
        return bags;
    }

    // time_advance function
    TIME time_advance() const {
        return state.sigma;
    }
    
    friend std::ostringstream& operator<<(std::ostringstream& os, const typename SwitchBuffer<TIME>::state_type& i) {
        os << "current PacketSequence is: " << i.currentPacket.sequence << " & size is: " << i.currentPacket.size<<" & BufferSize is: " << i.currentBufferSize;
        return os;
    }
};
#endif // __SwitchBuffer_HPP__
