#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "m_pd.h"

#define MAX_LINE_SIZE 1024

static t_class *markov_class;

typedef struct _markov {
  t_object x_obj;
  const char *f_path;

  // Probability matrix
  int state_len;
  int n_elements;
  int n_states;  // n_elements ** state_len

  char **elements;
  char **states;          // (index, state)
  float **probabilities;  // (index, probability)
} t_markov;

void print(t_markov *x) {
  post("[markov ]");
  post("[markov] f_path=%s", x->f_path);
  post("[markov] state_len=%d, n_elements=%d, n_states=%d", x->state_len,
       x->n_elements, x->n_states);

  post("[markov] matrix:");
  if (x->elements == NULL)
    post("WARNING: t_markov.elements is NULL");
  else {
    for (int i = 0; i < x->n_elements; ++i) {
      if (x->elements[i] == NULL)
        post("WARNING: t_markov.elements[%d] is NULL", i);
      else
        post("[markov ] element: %s", x->elements[i]);
    }
  }

  if (x->states == NULL)
    post("WARNING: t_markov.states is NULL");
  else {
    for (int i = 0; i < x->n_states; ++i) {
      if (x->states[i] == NULL)
        post("WARNING: t_markov.states[%d] is NULL", i);
      else
        post("[markov ] element: %s", x->states[i]);
    }
  }
}

int csv_to_pm(t_markov *x, const char *f_path) {
  FILE *file = fopen(f_path, "r");
  if (file == NULL) {
    post("Error opening file %s. %s", f_path, strerror(errno));
    return 1;
  }

  // char line[MAX_LINE_SIZE];
  // int line_i = 0;
  (void)x;

  x->elements = (char **)malloc(x->n_elements * sizeof(char *));
  x->states = (char **)malloc(x->n_states * sizeof(char *));
  x->probabilities = (float **)malloc(x->n_states * sizeof(float *));

  // for (int i = 0; i < x->n_states; i++)
  //   x->probabilities[i] = (float *)malloc(x->n_elements * sizeof(float));

  // while (fgets(line, sizeof(line), file)) {
  //   char *token = strtok(line, ",");
  //   int col_i = 0;

  //   while (token != NULL) {
  //     // Skip first (empty/dummy) entry in first row
  //     if (line_i == 0 && col_i == 0) {
  //       token = strtok(NULL, ",");
  //       ++col_i;
  //       continue;
  //     }

  //     if (line_i == 0)  // Elements
  //       x->elements[col_i] = *token;
  //     else {
  //       if (col_i == 0)  // State
  //         x->states[line_i] = strdup(token);
  //       else {  // Probabilities
  //         if (x->probabilities[line_i] != NULL)
  //           x->probabilities[line_i][col_i] = atof(token);
  //       }
  //     }

  //     token = strtok(NULL, ",");
  //     col_i++;
  //   }

  //   line_i++;
  // }

  fclose(file);
  return 0;
}

void on_bang(t_markov *x) { print(x); }

void *init(const t_symbol *t_f_path_sym, const t_floatarg t_state_len,
           const t_floatarg t_n_states) {
  t_markov *x = (t_markov *)pd_new(markov_class);

  x->f_path = t_f_path_sym->s_name;
  x->state_len = t_state_len;
  x->n_states = t_n_states;
  x->n_elements = pow(t_state_len, t_n_states);

  csv_to_pm(x, x->f_path);

  return x;
}

void destroy(t_markov *x) {
  if (x->elements != NULL)
    for (int i = 0; i < x->n_elements; i++) free(x->elements[i]);
  free(x->elements);

  if (x->states != NULL)
    for (int i = 0; i < x->n_states; i++) free(x->states[i]);
  free(x->states);

  if (x->probabilities != NULL)
    for (int i = 0; i < x->n_states; i++) free(x->probabilities[i]);
  free(x->probabilities);

  post("Destroyed t_markov");
}

void markov_setup() {
  markov_class = class_new(gensym("markov"), (t_newmethod)init,
                           (t_method)destroy, sizeof(t_markov), CLASS_DEFAULT,
                           A_DEFSYMBOL, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);

  class_addbang(markov_class, (t_method)on_bang);

  class_sethelpsymbol(markov_class, gensym("markov"));
}