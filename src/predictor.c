//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "NAME";
const char *studentID   = "PID";
const char *email       = "EMAIL";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//

// Gshare
uint32_t ghistory;
uint8_t  *gBHT;
// Tournament

uint32_t *local_history_table_t;
uint32_t pc_mask;

uint8_t *local_prediction_table_t;
uint32_t lpt_mask;

uint32_t *global_history_table_t;
uint32_t global_mask_t;
uint32_t global_history;

uint32_t *choose;

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //

  switch (bpType) {
    case STATIC:
      return;
    case TOURNAMENT:
      // Tournament Initialize
      pc_mask = (1 << pcIndexBits) - 1;
      lpt_mask = (1 << lhistoryBits) - 1;
      global_mask_t = (1 << ghistoryBits) - 1;

      global_history = 0;

      choose = (uint32_t *)malloc(sizeof(uint32_t) * (1 << ghistoryBits)); // TAKE to select global one
      memset(choose, WT, 1 << ghistoryBits);

      local_history_table_t = (uint32_t *)malloc(sizeof(uint32_t) * (1 << pcIndexBits));
      memset(local_history_table_t, 0,1 << pcIndexBits);

      local_prediction_table_t = (uint8_t *)malloc(sizeof(uint8_t) * (1 << lhistoryBits));
      memset(local_prediction_table_t, WN, 1 << lhistoryBits);

      global_history_table_t = (uint32_t *)malloc(sizeof(uint32_t) * (1 << ghistoryBits));
      memset(global_history_table_t, WN, 1 << ghistoryBits);
      // Tournament Initialize Done
      break;
    case GSHARE:
      // Gshare Initialize
      global_mask_t = (1 << ghistoryBits) - 1;

      ghistory = 0;
      gBHT = (uint8_t *)malloc(sizeof(uint8_t) * (1 << ghistoryBits));
      memset(gBHT, WN, 1 << ghistoryBits);
      break;
    case CUSTOM:
    default:
      break;
  }
  
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//

// gshare predictor
uint8_t gshare_predict(uint32_t pc) {
  uint32_t gIndex = global_mask_t & (ghistory ^ pc);
  uint8_t gprediction = gBHT[gIndex];
  return (gprediction >= WT) ? TAKEN : NOTTAKEN;
}

// tournament predictor 
uint8_t tournament_predict(uint32_t pc) {
  uint8_t lprediction = 
    local_prediction_table_t[local_history_table_t[pc & pc_mask] & lpt_mask] >= WT;

  uint8_t gprediction = 
    global_history_table_t[global_history] >= WT;
    
  return (choose[global_history] >= WT) ? gprediction : lprediction;  
  // return gprediction;
  // return lprediction;
}

uint8_t
make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  //

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      return gshare_predict(pc);
    case TOURNAMENT:
      return tournament_predict(pc);
    case CUSTOM:
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//

void gshare_train(uint32_t pc, uint8_t outcome) {
  uint32_t gIndex = global_mask_t & (ghistory ^ pc);
  if (outcome == TAKEN && gBHT[gIndex] != ST) {
    gBHT[gIndex] ++;
  }else if (outcome == NOTTAKEN && gBHT[gIndex] != SN) {
    gBHT[gIndex] --;
  }
  
  ghistory = ((ghistory << 1) | outcome) & global_mask_t;
}

void tournament_train(uint32_t pc, uint8_t outcome) {
  uint32_t lpidx = local_history_table_t[pc & pc_mask] & lpt_mask;
  uint8_t lprediction = local_prediction_table_t[lpidx];
  if (outcome == NOTTAKEN && lprediction != SN) local_prediction_table_t[lpidx]--; 
  if (outcome == TAKEN && lprediction != ST) local_prediction_table_t[lpidx]++; 
  local_history_table_t[pc & pc_mask] = (lpidx << 1) | outcome;


  uint8_t gprediction = global_history_table_t[global_history];
  if (outcome == NOTTAKEN && gprediction != SN) global_history_table_t[global_history]--;
  if (outcome == TAKEN && gprediction != ST) global_history_table_t[global_history]++;

  if (lprediction >> 1 != gprediction >> 1) {
    if (outcome == lprediction >> 1 && choose[global_history] != SN) choose[global_history]--;
    if (outcome == gprediction >> 1 && choose[global_history] != ST) choose[global_history]++;
  }

  global_history = (global_history << 1 | outcome) & global_mask_t;
}

void
train_predictor(uint32_t pc, uint8_t outcome)
{
  //
  //TODO: Implement Predictor training

  switch (bpType) {
    case STATIC:
      return;
    case GSHARE:
      gshare_train(pc, outcome);
      return;
    case TOURNAMENT:
      tournament_train(pc, outcome);
      return;
    case CUSTOM:
      return;
    default:
      break;
  }


  //
}
