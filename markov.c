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
  int order;
  int n_elements;
  int n_states;  // n_elements ** order

  char **elements;
  char **states;          // (index, state)
  float **probabilities;  // (index, probability)
} t_markov;

void print(t_markov *x) {
  post("[markov ]");
  post("[markov] f_path=%s", x->f_path);
  post("[markov] order=%d, n_elements=%d, n_states=%d", x->order, x->n_elements,
       x->n_states);

  post("[markov] matrix:");
  if (x->elements == NULL)
    post("WARNING: t_markov.elements is NULL");
  else
    for (int i = 0; i < x->n_elements; ++i)
      if (x->elements[i] == NULL)
        post("WARNING: t_markov.elements[%d] is NULL", i);
      else
        post("[markov ] element: %s", x->elements[i]);

  if (x->states == NULL)
    post("WARNING: t_markov.states is NULL");
  else
    for (int i = 0; i < x->n_states; ++i)
      if (x->states[i] == NULL)
        post("WARNING: t_markov.states[%d] is NULL", i);
      else
        post("[markov ] state: %s", x->states[i]);

  if (x->probabilities == NULL)
    post("WARNING: t_markov.probabilities is NULL");
  else
    for (int i = 0; i < x->n_states; ++i)
      if (x->probabilities[i] == NULL)
        post("WARNING: t_markov.probabilities[%d] is NULL", i);
      else
        for (int j = 0; j < x->n_elements; ++j)
          post("[markov ] probability (%s -> %s): %f", x->states[i],
               x->elements[j], x->probabilities[i][j]);
}

int csv_to_pm(t_markov *x, const char *f_path) {
  FILE *file = fopen(f_path, "r");
  if (file == NULL) {
    post("Error opening file %s. %s", f_path, strerror(errno));
    return 1;
  }

  x->elements = (char **)malloc(x->n_elements * sizeof(char *));
  x->states = (char **)malloc(x->n_states * sizeof(char *));
  x->probabilities = (float **)malloc(x->n_states * sizeof(float *));
  for (int i = 0; i < x->n_states; i++)
    x->probabilities[i] = (float *)malloc(x->n_elements * sizeof(float));

  if (x->elements == NULL || x->states == NULL || x->probabilities == NULL) {
    post("Error allocating memory for t_markov");
    return 1;
  }

  char line[MAX_LINE_SIZE];
  int line_i = 0;

  while (fgets(line, sizeof(line), file)) {
    char *token = strtok(line, ",;\n");
    int col_i = 0;

    while (token != NULL) {
      post("(%d, %d) %s", line_i, col_i, token);
      if (line_i == 0 && col_i > 0)  // Element
        x->elements[col_i - 1] = strdup(token);
      else {
        if (col_i == 0)  // State
          x->states[line_i - 1] = strdup(token);
        else  // Probability
          if (x->probabilities[line_i - 1] != NULL)
            x->probabilities[line_i - 1][col_i - 1] = atof(token);
      }

      token = strtok(NULL, ",;\n");
      ++col_i;
    }

    ++line_i;
  }

  fclose(file);
  return 0;
}

void on_bang(t_markov *x) { print(x); }

void *init(const t_symbol *t_f_path_sym, const t_floatarg t_order,
           const t_floatarg t_n_elements) {
  t_markov *x = (t_markov *)pd_new(markov_class);

  x->f_path = t_f_path_sym->s_name;
  x->order = t_order;
  x->n_elements = t_n_elements;
  x->n_states = pow(t_n_elements, t_order);

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
                           A_DEFSYMBOL,  // Absolute path
                           A_DEFFLOAT,   // Order
                           A_DEFFLOAT,   // Number of elements
                           0);

  class_addbang(markov_class, (t_method)on_bang);

  class_sethelpsymbol(markov_class, gensym("markov"));
}