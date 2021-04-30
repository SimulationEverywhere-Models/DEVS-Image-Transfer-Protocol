#ifndef __RECEIVER_HPP__
#define __RECEIVER_HPP__


#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/message_bag.hpp>

#include <limits>
#include <assert.h>
#include <string>

#include "../data_structures/packet_format.hpp"

using namespace cadmium;
using namespace std;

//Port definition
struct Receiver_defs {
    struct out : public out_port<Packet_t> { };
    struct in : public in_port<Packet_t> { };
};

template<typename TIME> class Receiver {
public:
    // ports definition
    using input_ports = std::tuple<typename Receiver_defs::in>;
    using output_ports = std::tuple<typename Receiver_defs::out>;

    // state definition
    struct state_type {
        TIME sigma;
        bool active;
        int currentACK;
        int currentSize;
    };
    state_type state;

    // default constructor without parameters
    Receiver() noexcept {
        state.sigma = std::numeric_limits<TIME>::infinity();
        state.active = false;
        state.currentACK = 0;
        state.currentSize = 0;
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

        if (get_messages<typename Receiver_defs::in>(mbs).size() > 1)
            assert(false && "one message per time uniti");

        vector<Packet_t> packet_port_in;
        packet_port_in = get_messages<typename Receiver_defs::in>(mbs);
        if (state.active) {//should not happen, all atomic modules are synchronized, receiver only receives one packet a time
            state.currentACK = packet_port_in[0].sequence;
            state.currentSize = packet_port_in[0].size;
            state.sigma = TIME("00:00:00");
        }
        else {
            state.active = true;
            state.currentACK = packet_port_in[0].sequence;
            state.currentSize = packet_port_in[0].size;
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
        out_aux = Packet_t(state.currentSize, state.currentACK);
        get_messages<typename Receiver_defs::out>(bags).push_back(out_aux);
        return bags;
    }

    // time_advance function
    TIME time_advance() const {
        return state.sigma;
    }

    friend std::ostringstream& operator<<(std::ostringstream& os, const typename Receiver<TIME>::state_type& i) {
        os << "current ACK is: " << i.currentACK << " & size is: " << i.currentSize;
        return os;
    }
};
#endif // __RECEIVER_HPP__
