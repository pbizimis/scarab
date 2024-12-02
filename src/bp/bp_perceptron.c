#include "bp_perceptron.h"
#include <stdint.h>
#include <stdlib.h>
#include "../debug/debug_macros.h"
#include "../debug/debug_print.h"
#include "core.param.h"
#include "globals/assert.h"

#define DEBUG(proc_id, args...) _DEBUG(proc_id, DEBUG_BP, ##args)

struct PerceptronBranchPredictor* perceptron_states;

int32* get_weights(struct PerceptronBranchPredictor* perc_state, Addr addr) {
  Flag   new_entry;
  int32* weights = hash_table_access_create(&perc_state->perceptron_table, addr,
                                            &new_entry);
  if(new_entry) {
    weights[0] = 0;
    for(int i = 1; i < perc_state->history_length + 1; i++) {
      weights[i] = 0;
    }
  }
  return weights;
}

void bp_perceptron_init() {
  perceptron_states = (struct PerceptronBranchPredictor*)malloc(
    NUM_CORES * sizeof(struct PerceptronBranchPredictor));
  if(perceptron_states == NULL) {
    // handle malloc fail
  }

  for(int i = 0; i < NUM_CORES; i++) {
    perceptron_states[i].history_length          = HISTORY_LENGTH;
    perceptron_states[i].perceptron_table_length = PERCEPTRON_TABLE_LENGTH;
    perceptron_states[i].theta                   = THETA;
    for(int j = 0; j < perceptron_states[i].history_length; j++)
      perceptron_states[i].global_history[j] = 0;

    init_hash_table(&perceptron_states[i].perceptron_table, "Perceptron Table",
                    perceptron_states[i].perceptron_table_length,
                    sizeof(int32) * (perceptron_states[i].history_length + 1));
  }
}

/*
void bp_perceptron_cleanup() {
  free(perceptron_states);
}
*/


uns8 bp_perceptron_pred(Op* op) {
  struct PerceptronBranchPredictor* perc_state =
    &perceptron_states[op->proc_id];
  const Addr   addr    = op->oracle_info.pred_addr;
  const int32* weights = get_weights(perc_state, addr);

  int32 y_out = calculate_perceptron(perc_state, weights);

  struct PerceptronBranchMetadata* metadata = (struct PerceptronBranchMetadata*)
                                                op->recovery_info.branch_id;
  memcpy(metadata->global_history_copy, perc_state->global_history,
         perc_state->history_length);
  metadata->y_out = y_out;


  if(y_out >= 0)
    return 1;
  else
    return 0;
}

int32 calculate_perceptron(struct PerceptronBranchPredictor* perc_state,
                           const int32*                      weights) {
  int32 result = 0;
  result += weights[0];
  for(int i = 0; i < perc_state->history_length; i++) {
    if(perc_state->global_history[i]) {
      result += weights[i + 1];
    } else {
      result -= weights[i + 1];
    }
  }

  return result;
}

void bp_perceptron_update(Op* op) {
  struct PerceptronBranchPredictor* perc_state =
    &perceptron_states[op->proc_id];
  const Addr addr = op->oracle_info.pred_addr;

  struct PerceptronBranchMetadata* metadata = (struct PerceptronBranchMetadata*)
                                                op->recovery_info.branch_id;
  uns8*  history_copy = metadata->global_history_copy;
  int32  y_out        = metadata->y_out;
  int32* weights      = get_weights(perc_state, addr);

  Flag t = op->oracle_info.dir;
  Flag y = (y_out >= 0) ? 1 : 0;
  if(y != t)
    DEBUG(op->proc_id, "BRANCH MISPREDICTED\n");
  else
    DEBUG(op->proc_id, "BRANCH PREDICTED\n");

  // use history from point of prediction (not global history) for training
  if(y != t || abs(y_out) <= perc_state->theta) {
    int32 t_val = (t == 1) ? 1 : -1;

    weights[0] += t_val;

    double factor = 1;
    for(int i = 0; i < perc_state->history_length; i++) {
      if(i > 30)
        factor = 0.9;
      int32 x_i = history_copy[i] ? 1 : -1;
      weights[i + 1] += t_val * x_i * factor;
      if(weights[i + 1] > perc_state->theta)
        weights[i + 1] = perc_state->theta;
      if(weights[i + 1] < (perc_state->theta * -1))
        weights[i + 1] = perc_state->theta * -1;
    }
  }

  free(metadata);

  // add new branch target to global history
  uns8 t_new = t;
  uns8 t_old;
  for(int i = 0; i < perc_state->history_length; i++) {
    t_old                         = perc_state->global_history[i];
    perc_state->global_history[i] = t_new;
    t_new                         = t_old;
  }
}

void bp_perceptron_timestamp(Op* op) {
  struct PerceptronBranchMetadata* metadata = (struct PerceptronBranchMetadata*)
    malloc(sizeof(struct PerceptronBranchMetadata));
  ASSERT(op->proc_id, sizeof(uintptr_t) <= sizeof(int64));
  op->recovery_info.branch_id = (uintptr_t)metadata;
}
void bp_perceptron_recover(Recovery_Info* info) {}
void bp_perceptron_spec_update(Op* op) {}
void bp_perceptron_retire(Op* op) {}
uns8 bp_perceptron_full(uns proc_id) {
  return 0;
}
