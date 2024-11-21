#include "bp_perceptron.h"
#include <stdlib.h>

struct PerceptronBranchPredictor pbp;

void init_branch_predictor() {
  pbp.history_length          = HISTORY_LENGTH;
  pbp.perceptron_table_length = PERCEPTRON_TABLE_LENGTH;
  pbp.theta                   = THETA;

  pbp.global_history_register = 0;

  init_hash_table(&pbp.perceptron_table, "Perceptron Table",
                  pbp.perceptron_table_length,
                  sizeof(int32) * (pbp.history_length + 1));
}

void clean_branch_predictor() {
  hash_table_clear(&pbp.perceptron_table);
}

Flag get_prediction(Addr branch_address) {
  Flag   new_entry;
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

void train_perceptron(int32* weights, Flag t, int32 y_out) {
  Flag y = (y_out >= 0) ? 1 : 0;

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
