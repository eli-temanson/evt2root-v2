/*
Parameters.cpp

Method for computing new parameters for SpecTcl from raw data; specifically
for the Resoneut setup

Gordon M.
Nov. 2018
*/
#include "Parameters.h"
#include "CRawRNUnpacker.h"
#include <BufferDecoder.h>
#include </usr/opt/daq/11.0/include/DataFormat.h>
#include <CRingBufferDecoder.h>
#include <TCLAnalyzer.h>
#include <stdio.h>
#include <string>
#include <cstdint>
#include <stdexcept>
#include <iostream>
using namespace std;

Parameters::Parameters() :
  tdc1_values("tdc1", 65535,0.0, 65534.0, "channels", 32, 0),
  tdc2_values("tdc2", 65535,0.0, 65534.0, "channels", 32, 0),
  tdc2_diff("tdc2_diff", 4096,0.0, 4095.0, "channels", 12, 0),
  adc1_values("adc1", 4096, 0.0, 4095.0, "channels", 32, 0),
  adc2_values("adc2", 4096, 0.0, 4095.0, "channels", 32, 0),
  adc3_values("adc3", 4096, 0.0, 4095.0, "channels", 32, 0),
  adc4_values("adc4", 4096, 0.0, 4095.0, "channels", 32, 0),
  adc5_values("adc5", 4096, 0.0, 4095.0, "channels", 32, 0),

  qdc1_values("qdc1", 4096, 0.0, 4095.0, "channels", 32, 0),
  qdc2_values("qdc2", 4096, 0.0, 4095.0, "channels", 32, 0),
  qdc3_values("qdc3", 4096, 0.0, 4095.0, "channels", 32, 0),
  // sum_pos("sum_pos", 4096, 0.0, 4095.0, "channels", 12, 0),
  neut_pos_sum("neut_pos_sum", 4096, 0.0, 4095.0, "channels", 12, 0),
  neut_max("neut_max", 4096, 0.0, 4095.0, "channels", 12, 0),
  gate_slope("gate_slope", 4096, 0.0, 4095.0, "channels", 12, 0),
  ratio_neut("ratio_neut", 4096, 0.0, 4095.0, "channels", 12, 0),
  e_s1("e_s1", 4096,0.0, 4096.0, "channels"),
  e_s2("e_s2", 4096,0.0, 4096.00, "channels")
{
}

Parameters::~Parameters() {
}

Bool_t Parameters::operator() (const Address_t pEvent,
                              CEvent& rEvent,
			      CAnalyzer& rAnalyzer,
                              CBufferDecoder& rDecoder)
{
  int i,j,k;
  int temp[4];
 
  for (i=0;i<12;i++){
    /* loop over crystals */
    // sum_pos[i]=0;
    for (j=0;j<4;j++){
      if (i<8){
	if (qdc2_values[j+4*i].isValid() && qdc2_values[j+4*i]>0 && qdc2_values[j+4*i]<4000){
	  temp[j] = qdc2_values[j+4*i];
	  // sum_pos[i]= sum_pos[i]+ temp[j];
	}
	else{
	  temp[j]=0;
	  //  sum_pos[i]=0;
	}
      } else {
   
	if (qdc1_values[j+4*(i-8)].isValid() && qdc1_values[j+4*(i-8)]>0 && qdc1_values[j+4*(i-8)]<4000){
	  temp[j] = qdc1_values[j+4*(i-8)]; 
	  //  sum_pos[i]= sum_pos[i]+ temp[j];
	}

	else{
	  temp[j]=0;
	}
      }
    }
    neut_pos_sum[i] = 0;
    neut_max[i] = 0;
    for (j=0;j<4;j++){
      neut_pos_sum[i] = neut_pos_sum[i] + temp[j];
      if (temp[j]>neut_max[i]){
	neut_max[i] = temp[j];
      }
    }
    //calculate the slope of the two gates for each crystal
    if (qdc3_values[i].isValid() && qdc3_values[i+16].isValid()) {
      gate_slope[i] = qdc3_values[i]/qdc3_values[i+16];
    }
    if (qdc3_values[i].isValid() && qdc3_values[i+16].isValid()) {
      ratio_neut[i] = qdc3_values[i+16]/qdc3_values[i];
    }
  } // loop over crystals for charges...
  
  int first_hit = 4096;
  //int first_hit[12] = {4000,4000,4000,4000,4000,4000,4000,4000,4000,4000,4000,4000};
  //int add_hit[12] = {1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000};
  // loop over crystals to find first hit;
  for (j=0;j<12;j++){
    if (tdc2_values[j+16].isValid() && (first_hit>tdc2_values[j+16])){
      first_hit=tdc2_values[j+16];
    }
  }
  //  double E_S1=60.0, E_S2=60.0;

  //  loop over crystals to subtract first hit from all times;
  int si_demax=4000;
   int si_emax=4000;
   // for (k=0; k<16; k++){
     /*if (adc1_values[k].isValid() && si_demax>adc1_values[k]){
	si_demax=adc1_values[k];
      }
 if (adc2_values[k].isValid() &&  si_emax>adc2_values[k]){
     si_emax=adc2_values[k];
      }
      }*/
   for (k=0; k<16; k++){
 if (adc1_values[k].isValid() && si_demax>adc1_values[k] && adc2_values[k].isValid() &&  si_emax>adc2_values[k]){

	si_demax=adc1_values[k];
        si_emax=adc2_values[k];
      }

      }
 e_s1=si_demax;
   e_s2=si_emax;
   

  
  for (j=0;j<12;j++){
    
    if (tdc2_values[j+16].isValid()){
      tdc2_diff[j]=tdc2_values[j+16]-first_hit+1000;
    }
  }  
  return kfTRUE;
}
