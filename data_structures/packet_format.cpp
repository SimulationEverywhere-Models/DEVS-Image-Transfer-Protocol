#include <math.h>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <string>

#include "packet_format.hpp"

/***************************************************/
/************* Output stream ************************/
/***************************************************/

ostream& operator<<(ostream& os, const Packet_t& msg) {
  os << msg.size << " " << msg.sequence;
  return os;
}

/***************************************************/
/************* Input stream ************************/
/***************************************************/

istream& operator>> (istream& is, Packet_t& msg) {
  is >> msg.size;
  is >> msg.sequence;
  return is;
}
