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

// Tournament

uint32_t *local_history_table_t;
uint32_t pc_mask;

uint8_t *local_prediction_table_t;
uint32_t lpt_mask;

uint8_t *global_prediction_table_t;
uint32_t global_mask_t;
uint32_t global_history;

uint8_t *choose;

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
  pc_mask = (1 << pcIndexBits) - 1;
  lpt_mask = (1 << lhistoryBits) - 1;
  global_mask_t = (1 << ghistoryBits) - 1;

  // Tournament Initialize

  global_history = 0;

  choose = (uint8_t *)malloc(sizeof(uint8_t) * (1 << ghistoryBits)); // TAKE to select global one
  memset(choose, WT, 1 << ghistoryBits);

  local_history_table_t = (uint32_t *)malloc(sizeof(uint32_t) * (1 << pcIndexBits));
  memset(local_history_table_t, 0, 1 << pcIndexBits * sizeof(uint32_t));

  local_prediction_table_t = (uint8_t *)malloc(sizeof(uint8_t) * (1 << lhistoryBits));
  memset(local_prediction_table_t, WN, 1 << lhistoryBits);

  global_prediction_table_t = (uint8_t *)malloc(sizeof(uint32_t) * (1 << ghistoryBits));
  memset(global_prediction_table_t, WN, 1 << ghistoryBits);
  // Tournament Initialize Done
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//

// tournament predictor 
uint8_t tournament_predict(uint32_t pc) {
  uint8_t lprediction = 
    local_prediction_table_t[local_history_table_t[pc & pc_mask] & lpt_mask] >= WT;

  uint8_t gprediction = 
    global_prediction_table_t[global_history] >= WT;
    
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

void tournament_train(uint32_t pc, uint8_t outcome) {
  uint32_t lpidx = local_history_table_t[pc & pc_mask] & lpt_mask;
  uint8_t lprediction = local_prediction_table_t[lpidx];
  if (outcome == NOTTAKEN && lprediction != SN) local_prediction_table_t[lpidx]--; 
  if (outcome == TAKEN && lprediction != ST) local_prediction_table_t[lpidx]++; 
  local_history_table_t[pc & pc_mask] = (lpidx << 1) | outcome;


  uint8_t gprediction = global_prediction_table_t[global_history];
  if (outcome == NOTTAKEN && gprediction != SN) global_prediction_table_t[global_history]--;
  if (outcome == TAKEN && gprediction != ST) global_prediction_table_t[global_history]++;

  if (lprediction >> 1 != gprediction >> 1) {
    // printf("choose before: %d ", choose[global_history]);
    if (outcome == lprediction >> 1 && choose[global_history] != SN) choose[global_history]--;
    if (outcome == gprediction >> 1 && choose[global_history] != ST) choose[global_history]++;
    // printf("lpre: %d, gpre: %d, choose: %d\n", lprediction, gprediction, choose[global_history]);
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
    case TOURNAMENT:
      tournament_train(pc, outcome);
      return;
    case CUSTOM:
    default:
      break;
  }


  //
}
