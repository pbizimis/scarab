#ifndef __PERCEP_H_
#define __PERCEP_H_

#include "../bp/bp.h"
#include "../globals/global_types.h"
#include "../libs/hash_lib.h"

/************ PUT IN ARCH DEF ****************/
#define HISTORY_LENGTH 90
#define PERCEPTRON_TABLE_LENGTH 1024
#define THETA ((int)(1.93 * HISTORY_LENGTH + 14))
/************ PUT IN ARCH DEF ****************/

struct PerceptronBranchPredictor {
  uns8       global_history[HISTORY_LENGTH];
  Hash_Table perceptron_table;
  int32      history_length;
  int32      perceptron_table_length;
  int32      theta;
};

struct PerceptronBranchMetadata {
  uns8  global_history_copy[HISTORY_LENGTH];
  int32 y_out;
};

void   bp_perceptron_init(void);
uns8   bp_perceptron_pred(Op*);
int32  calculate_perceptron(struct PerceptronBranchPredictor* perc_state,
                            const int32*                      weigths);
void   bp_perceptron_update(Op*);
int32* get_weights(struct PerceptronBranchPredictor* perc_state, Addr addr);

void bp_perceptron_timestamp(Op* op);
void bp_perceptron_recover(Recovery_Info* info);
void bp_perceptron_spec_update(Op* op);
void bp_perceptron_retire(Op* op);
uns8 bp_perceptron_full(uns proc_id);

#endif
