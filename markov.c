#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "m_pd.h"

#define DELIMITERS ",;\n"
#define MAX_LINE_SIZE 1024

static t_class *markov_class;

typedef struct _markov {
  t_object x_obj;
  const char *csv_path;

  // Probability matrix
  int order;
  int n_states;
  int n_grams;  // n_states ** order

  char **states;
  char **grams;           // (index, state)
  float **probabilities;  // (index, probability)
} t_markov;

void print(t_markov *x) {
  post("[markov ]");
  post("[markov ] csv_path=%s", x->csv_path);
  post("[markov ] order=%i, n_states=%i, n_grams=%i", x->order, x->n_states,
       x->n_grams);

  post("[markov ] probability matrix:");
  if (x->states == NULL)
    post("WARNING: t_markov.states is NULL");
  else
    for (int i = 0; i < x->n_states; ++i)
      post("[markov ] states[%i]: %s", i,
           x->states[i] != NULL ? x->states[i] : "NULL");

  if (x->grams == NULL)
    post("WARNING: t_markov.grams is NULL");
  else
    for (int i = 0; i < x->n_grams; ++i)
      post("[markov ] grams[%i]: %s", i,
           x->grams[i] != NULL ? x->grams[i] : "NULL");

  if (x->probabilities == NULL)
    post("WARNING: t_markov.probabilities is NULL");
  else if (x->states != NULL && x->grams != NULL)
    for (int i = 0; i < x->n_grams; ++i)
      for (int j = 0; j < x->n_states; ++j)
        post("[markov ] probabilities[%i][%i] (%s -> %s): %f", i, j,
             x->grams[i], x->states[j],
             x->probabilities[i] != NULL ? x->probabilities[i][j] : -1);
}

int csv_to_pm(t_markov *x, const char *csv_path) {
  FILE *file = fopen(csv_path, "r");
  if (file == NULL) {
    post("Error opening file %s. %s", csv_path, strerror(errno));
    return 1;
  }

  x->states = (char **)malloc(x->n_states * sizeof(char *));
  x->grams = (char **)malloc(x->n_grams * sizeof(char *));
  x->probabilities = (float **)malloc(x->n_grams * sizeof(float *));
  for (int i = 0; i < x->n_grams; ++i)
    x->probabilities[i] = (float *)malloc(x->n_states * sizeof(float));

  if (x->states == NULL || x->grams == NULL || x->probabilities == NULL) {
    post("Error allocating memory for t_markov");
    return 1;
  }

  char line[MAX_LINE_SIZE];
  int line_i = 0;

  while (fgets(line, sizeof(line), file)) {
    char *token = strtok(line, DELIMITERS);
    int col_i = 0;

    while (token != NULL) {
      if (line_i == 0) {  // State
        if (col_i > 0) x->states[col_i - 1] = strdup(token);
      } else {
        if (col_i == 0)  // Gram
          x->grams[line_i - 1] = strdup(token);
        else  // Probability
          x->probabilities[line_i - 1][col_i - 1] = atof(token);
      }

      token = strtok(NULL, DELIMITERS);
      ++col_i;
    }

    ++line_i;
  }

  fclose(file);
  return 0;
}

void on_bang(t_markov *x) { print(x); }

void *init(const t_symbol *t_sym, const t_floatarg t_fl1,
           const t_floatarg t_fl2) {
  t_markov *x = (t_markov *)pd_new(markov_class);

  x->csv_path = t_sym->s_name;
  x->order = t_fl1;
  x->n_states = t_fl2;
  x->n_grams = pow(x->n_states, x->order);

  csv_to_pm(x, x->csv_path);

  return x;
}

void destroy(t_markov *x) {
  if (x->states != NULL)
    for (int i = 0; i < x->n_states; ++i) free(x->states[i]);
  free(x->states);

  if (x->grams != NULL)
    for (int i = 0; i < x->n_grams; ++i) free(x->grams[i]);
  free(x->grams);

  if (x->probabilities != NULL)
    for (int i = 0; i < x->n_grams; ++i) free(x->probabilities[i]);
  free(x->probabilities);

  post("Destroyed t_markov");
}

void markov_setup() {
  markov_class = class_new(gensym("markov"), (t_newmethod)init,
                           (t_method)destroy, sizeof(t_markov), CLASS_DEFAULT,
                           A_DEFSYMBOL,  // Absolute path
                           A_DEFFLOAT,   // Order
                           A_DEFFLOAT,   // Number of states
                           0);

  class_addbang(markov_class, (t_method)on_bang);

  class_sethelpsymbol(markov_class, gensym("markov"));
}