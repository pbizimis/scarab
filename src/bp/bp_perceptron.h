#ifndef __PERCEP_H_
#define __PERCEP_H_

#include "../bp/bp.h"
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

void  bp_perceptron_init(void);
uns8  bp_perceptron_pred(Op*);
int32 calculate_perceptron(Op* op, int32* weigths);
void  bp_perceptron_update(Op*);

void bp_perceptron_timestamp(Op* op);
void bp_perceptron_recover(Recovery_Info* info);
void bp_perceptron_spec_update(Op* op);
void bp_perceptron_retire(Op* op);
uns8 bp_perceptron_full(uns proc_id);

#endif
