#include "bp_perceptron.h"
#include <stdlib.h>

struct PerceptronBranchPredictor pbp;

void bp_perceptron_init() {
  pbp.history_length          = HISTORY_LENGTH;
  pbp.perceptron_table_length = PERCEPTRON_TABLE_LENGTH;
  pbp.theta                   = THETA;

  pbp.global_history_register = 0;

  init_hash_table(&pbp.perceptron_table, "Perceptron Table",
                  pbp.perceptron_table_length,
                  sizeof(int32) * (pbp.history_length + 1));
}

uns8 bp_perceptron_pred(Op* op) {
  Flag   new_entry;
  Addr   branch_address = 0;
  int32* weights = hash_table_access_create(&pbp.perceptron_table,
                                            branch_address, &new_entry);
  if(new_entry) {
    weights[0] = 0;
    for(int i = 1; i < pbp.history_length + 1; i++) {
      // could be changed to random values for faster learning
      weights[i] = 0;
    }
  }

  int32 taken = calculate_perceptron(weights);


  return taken;
}

int32 calculate_perceptron(int32* weights) {
  int32 result = 0;
  result += weights[0];
  for(int i = 1; i < pbp.history_length + 1; i++) {
    if((pbp.global_history_register >> (i - 1)) & 0x01) {
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
  int32* weights = 0;
  Flag   t = 0;
  int32  y_out = 0;
  Flag   y = (y_out >= 0) ? 1 : 0;

  if(y != t || abs(y_out) <= pbp.theta) {
    int32 t_val = (t == 1) ? 1 : -1;

    weights[0] += t_val;

    for(int i = 1; i < pbp.history_length + 1; i++) {
      int32 x_i = ((pbp.global_history_register >> (i - 1)) & 0x01) ? 1 : -1;
      weights[i] += t_val * x_i;
    }
  }

  // add new branch target to history
  pbp.global_history_register = (pbp.global_history_register << 1) | (t & 0x1);
  // mask to correct length
  pbp.global_history_register &= (1 << pbp.history_length) - 1;
}

void bp_perceptron_timestamp(Op* op) {}
void bp_perceptron_recover(Recovery_Info* info) {}
void bp_perceptron_spec_update(Op* op) {}
void bp_perceptron_retire(Op* op) {}
uns8 bp_perceptron_full(uns proc_id) {
  return 0;
}
