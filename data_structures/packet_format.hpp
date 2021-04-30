#ifndef BOOST_SIMULATION_MESSAGE_HPP
#define BOOST_SIMULATION_MESSAGE_HPP

#include <assert.h>
#include <iostream>
#include <string>

using namespace std;

/*******************************************/
/**************** Packet_t ****************/
/*******************************************/
struct Packet_t {
    Packet_t() {}
    Packet_t(int i_size, int i_seq)
        :size(i_size), sequence(i_seq) {}

    int   size;
    int   sequence;
};

istream& operator>> (istream& is, Packet_t& msg);

ostream& operator<<(ostream& os, const Packet_t& msg);


#endif // BOOST_SIMULATION_MESSAGE_HPP
