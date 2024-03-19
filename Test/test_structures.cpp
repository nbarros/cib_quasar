/*
 * test_structures.cpp
 *
 *  Created on: Mar 18, 2024
 *      Author: Nuno Barros
 */

#include <iostream>
#include <cstdio>
#include <bitset>

    const std::string dump_binary(uint32_t val)
    {
      return std::bitset<32>(val).to_string();
    }

//  typedef struct laser_state_t
//  {
//    // lsb
//    uint8_t ext_shutter_closed    : 1;
//    uint8_t laser_shutter_closed  : 1;
//    uint8_t qswitch_en            : 1;
//    uint8_t laser_started         : 1;
//    uint8_t padding               : 4;
//    // msb
//  } laser_state_t;
//
//  typedef union laser_state_u
//  {
//    laser_state_t state;
//    uint8_t       value;
//    uint8_t get_byte() {return value;}
//  } laser_state_u;

    // note that a bool has the size of 1 byte, like a char
    typedef struct laser_state_t
    {
      // lsb
      bool ext_shutter_closed    : 1;
      bool laser_shutter_closed  : 1;
      bool qswitch_en            : 1;
      bool laser_started         : 1;
      uint8_t padding               : 4;
      // msb
      laser_state_t() :
        ext_shutter_closed(0x0),
        laser_shutter_closed(0x0),
        qswitch_en(0x0),
        laser_started(0x0),
        padding(0x0) {}
    } laser_state_t;

    typedef struct laser_state_u
    {
      laser_state_t state;
      laser_state_u(laser_state_t s) : state(s) {state.padding = 0x0;}
      laser_state_u(uint8_t s) : state(*reinterpret_cast<laser_state_t*>(&s)) {state.padding = 0x0;}
      uint8_t get_byte() {return *reinterpret_cast<uint8_t*>(&state);}
      bool    is_set(uint16_t bit) {return (get_byte() & ((0x1U << bit) & 0xFF));}
      bool    bits_set(uint8_t mask) {return (get_byte() & mask);}
    } laser_state_u;

  int main(int argc, char **argv)
  {
    printf("Size of struct : %lu\n",sizeof(laser_state_t));
    printf("Size of union  : %lu\n",sizeof(laser_state_u));

    laser_state_t st;
    st.ext_shutter_closed   = 0x0;
    st.laser_shutter_closed = 0x1;
    st.qswitch_en = 0x1;
    st.laser_started = 0x1;

    printf("Word struct: %X\n",*reinterpret_cast<uint8_t*>(&st));
    laser_state_u l(st);
    printf("Union: 0x%X\n",l.get_byte());
    printf("Union fcn: 0x%X\n",l.get_byte());
    printf("Binary : [%s]\n",dump_binary(l.get_byte()).c_str());

    laser_state_u ll(0xA);
    printf("Union: 0x%X\n",ll.get_byte());
    printf("Union fcn: 0x%X\n",ll.get_byte());
    printf("Binary : [%s]\n",dump_binary(ll.get_byte()).c_str());



    return 0;
  }


