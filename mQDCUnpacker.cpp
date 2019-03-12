/*
CRawmQDCUnpacker.cpp

Class to parse through data coming from mqdc 

Most of the program's structure borrowed from http://docs.nscl.msu.edu/daq/newsite/nscldaq-11.2/x5013.html


Gordon M.
Nov 2018
*/
#define HAVE_HLESS_IOSTREAM  //May not be necessary. Placed here for the sake of #include<Iostream.h> misbehaving
#include "mQDCUnpacker.h"
#include <string>
#include <stdexcept>
#include <iostream>

using namespace std;

//This is the main place where changes need to be made from one module to another; all else mostly name changes
//useful masks and shifts for mQDC:
static const uint16_t TYPE_MASK (0xc000);
static const uint16_t TYPE_HDR (0x4000);
static const uint16_t TYPE_DATA (0x0000);
static const uint16_t TYPE_TRAIL (0xc000);


//header-specific:
static const unsigned HDR_ID_SHIFT (0);
static const uint16_t HDR_ID_MASK (0x00ff);
static const unsigned HDR_COUNT_SHIFT (0);
static const uint16_t HDR_COUNT_MASK (0x0fff);

static const unsigned HDR_RES_SHIFT (12);
static const uint16_t HDR_RES_MASK (0xf000);

//data-specific:
static const unsigned DATA_CHANSHIFT (0);
static const uint16_t DATA_CHANMASK (0x001f);
static const uint16_t DATA_CONVMASK (0x0fff);

pair< uint16_t*, ParsedmQDCEvent> mQDCUnpacker::parse( uint16_t* begin,  uint16_t* end, int id) {

  ParsedmQDCEvent event;

  auto iter = begin;
  int bad_flag;
  unpackHeader(iter, event);
  if (iter > end){
    bad_flag =1;
  }
  iter += 2;

  int nWords = (event.s_count-1)*2; //count includes the eob 
  auto dataEnd = iter + nWords;

  if (event.s_id != id) {//If unexpected id, skip data; either bad event or bad stack
    bad_flag = 1;
    iter+=nWords;
    //Error testing
    //cout<<"Bad mQDC id: "<<event.s_id<<" Expected: "<<id<<endl;
  } else {
    iter = unpackData(iter, dataEnd, event);
  }
  if (iter>end || bad_flag || !isEOE(*(iter+1))){
    //Error testing
    //cout<<"mQDCUnpacker::parse() ";
    //cout<<"Unable to unpack event"<<endl;
  }
  iter +=2;

  return make_pair(iter, event);

}

bool mQDCUnpacker::isHeader(uint16_t word) {
  return ((word&TYPE_MASK) == TYPE_HDR);
}

void mQDCUnpacker::unpackHeader(uint16_t* word, ParsedmQDCEvent& event) {
  //Error testing
  /*if (!isHeader(*(word+1))) {
    cout<<"mQDCUnpacker::parseHeader() ";
    cout<<"Found non-header word when expecting header. ";
    cout<<"Word = ";
    cout<<*(word+1)<<endl;
  }*/

  event.s_res = (*word&HDR_RES_MASK) >> HDR_RES_SHIFT;
  event.s_count = (*word&HDR_COUNT_MASK) >> HDR_COUNT_SHIFT;
  word++;
  event.s_id = (*word&HDR_ID_MASK)>>HDR_ID_SHIFT;

}

bool mQDCUnpacker::isData(uint16_t word) {
  return ((word&TYPE_MASK) == TYPE_DATA );
}

void mQDCUnpacker::unpackDatum(uint16_t* word, ParsedmQDCEvent& event) {
  //Error testing
  /*if (!isData(*(word+1))) {
    cout<<"mQDCUnpacker::unpackDatum() ";
    cout<<"Found non-data word when expecting data: "<<*(word+1)<<endl;;
  }*/

  uint16_t data = *word&DATA_CONVMASK;
  ++word;
  int channel = (*word&DATA_CHANMASK) >> DATA_CHANSHIFT;

  auto chanData = make_pair(channel, data);
  event.s_data.push_back(chanData);
  
}

 uint16_t* mQDCUnpacker::unpackData( uint16_t* begin, uint16_t* end, ParsedmQDCEvent& event) {

  event.s_data.reserve(event.s_count+1); //memory allocation
  auto iter = begin;
  while (iter<end) {
    unpackDatum(iter, event);
    iter = iter+2;
  }

  return iter;

}

bool mQDCUnpacker::isEOE(uint16_t word) {
  return ((word&TYPE_MASK) == TYPE_TRAIL);
}


