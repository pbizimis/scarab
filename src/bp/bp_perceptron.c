#include "bp_perceptron.h"
#include <stdlib.h>
#include "../debug/debug_macros.h"
#include "../debug/debug_print.h"
#include "core.param.h"

#define DEBUG(proc_id, args...) _DEBUG(proc_id, DEBUG_BP, ##args)

struct PerceptronBranchPredictor* perceptron_states;
static uns                        test_counter = 0;

void bp_perceptron_init() {
  test_counter++;
  perceptron_states = (struct PerceptronBranchPredictor*)malloc(
    NUM_CORES * sizeof(struct PerceptronBranchPredictor));
  if(perceptron_states == NULL) {
    // handle malloc fail
  }

  for(int i = 0; i < NUM_CORES; i++) {
    perceptron_states[i].history_length          = HISTORY_LENGTH;
    perceptron_states[i].perceptron_table_length = PERCEPTRON_TABLE_LENGTH;
    perceptron_states[i].theta                   = THETA;
    perceptron_states[i].global_history_register = 0;

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
  DEBUG(op->proc_id, "\nPERCEPTRON PRED\n");
  Flag   new_entry;
  Addr   branch_address = 0;
  int32* weights        = hash_table_access_create(
    &perceptron_states[op->proc_id].perceptron_table, branch_address,
    &new_entry);
  if(new_entry) {
    weights[0] = 0;
    for(int i = 1; i < perceptron_states[op->proc_id].history_length + 1; i++) {
      // could be changed to random values for faster learning
      weights[i] = 0;
    }
  }

  int32 taken = calculate_perceptron(op, weights);


  return taken;
}

int32 calculate_perceptron(Op* op, int32* weights) {
  int32 result = 0;
  result += weights[0];
  for(int i = 1; i < perceptron_states[op->proc_id].history_length + 1; i++) {
    if((perceptron_states[op->proc_id].global_history_register >> (i - 1)) &
       0x01) {
      result += weights[i];
    } else {
      result -= weights[i];
    }
  }

  return result;

  /*
    if(result >= 0)
      return 1;
    else
      return 0;
      */
}

void bp_perceptron_update(Op* op) {
  DEBUG(op->proc_id, "\nPERCEPTRON UPDATE\n");
  DEBUG(op->proc_id, "\nPROC_ID %d\n", op->proc_id);
  DEBUG(op->proc_id, "\nNUM_CORES %d\n", NUM_CORES);
  DEBUG(op->proc_id, "\nINIT: %d\n", test_counter);
  DEBUG(op->proc_id, "\n %lld \n", op->inst_info->addr);


  int32* weights = 0;
  Flag   t       = 0;
  int32  y_out   = 0;
  Flag   y       = (y_out >= 0) ? 1 : 0;

  if(y != t || abs(y_out) <= perceptron_states[op->proc_id].theta) {
    int32 t_val = (t == 1) ? 1 : -1;

    weights[0] += t_val;

    for(int i = 1; i < perceptron_states[op->proc_id].history_length + 1; i++) {
      int32 x_i = ((perceptron_states[op->proc_id].global_history_register >>
                    (i - 1)) &
                   0x01) ?
                    1 :
                    -1;
      weights[i] += t_val * x_i;
    }
  }

  // add new branch target to history
  perceptron_states[op->proc_id].global_history_register =
    (perceptron_states[op->proc_id].global_history_register << 1) | (t & 0x1);
  // mask to correct length
  perceptron_states[op->proc_id].global_history_register &=
    (1 << perceptron_states[op->proc_id].history_length) - 1;
}

void bp_perceptron_timestamp(Op* op) {}
void bp_perceptron_recover(Recovery_Info* info) {}
void bp_perceptron_spec_update(Op* op) {}
void bp_perceptron_retire(Op* op) {}
uns8 bp_perceptron_full(uns proc_id) {
  return 0;
}
