#ifndef __PERCEP_H_
#define __PERCEP_H_

#include "../globals/global_types.h"
#include "../libs/hash_lib.h"

/************ PUT IN ARCH DEF ****************/
#define HISTORY_LENGTH 64
#define PERCEPTRON_TABLE_LENGTH 1024
#define THETA 127  // value from paper
/************ PUT IN ARCH DEF ****************/

struct PerceptronBranchPredictor {
  uns64      global_history_register;
  Hash_Table perceptron_table;
  int32      history_length;
  int32      perceptron_table_length;
  int32      theta;
};

void init_branch_predictor(void);
void clean_branch_predictor(void);
Flag get_prediction(Addr branch_address);
int32 calculate_perceptron(int32* weigths);
void train_perceptron(int32* weights, Flag t, int32 y_out);

#endif
