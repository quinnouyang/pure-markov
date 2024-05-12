#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "m_pd.h"

#define MAX_LINE_SIZE 1024

static t_class *markov_class;

typedef struct _probability_matrix {
  int state_len;
  int n_elements;
  int n_states;  // n_elements ** state_len

  char *elements;
  char **states;          // (index, state)
  float **probabilities;  // (index, probability)
} probability_matrix;

typedef struct _markov {
  t_object x_obj;
  const char *f_path;

  probability_matrix m;
} t_markov;

int csv_to_pm(probability_matrix m, const char *f_path) {
  FILE *file = fopen(f_path, "r");
  if (file == NULL) {
    post("Error opening file %s. %s", f_path, strerror(errno));
    return 1;
  }

  // Allocate `probability_matrix`
  m.elements = (char *)malloc(m.n_elements * sizeof(char));
  m.states = (char **)malloc(m.n_states * sizeof(char *));
  m.probabilities = (float **)malloc(m.n_states * sizeof(float *));
  for (int i = 0; i < m.n_states; i++)
    m.probabilities[i] = (float *)malloc(m.n_elements * sizeof(float));

  char line[MAX_LINE_SIZE];
  int line_count = 0;

  while (fgets(line, sizeof(line), file)) {
    char *token = strtok(line, ",");
    int col_count = 0;

    while (token != NULL) {
      if (col_count == 0) {  // Skip first (empty/dummy) entry in first row
        token = strtok(NULL, ",");
        ++col_count;
        continue;
      }

      if (line_count == 0) {  // Elements
        if (col_count > 0) m.elements[col_count - 1] = *token;
      } else {
        if (col_count == 0)  // State
          m.states[line_count - 1] = strdup(token);
        else  // Probabilities
          m.probabilities[line_count - 1][col_count - 1] = atof(token);
      }

      token = strtok(NULL, ",");
      col_count++;
    }
    line_count++;
  }

  fclose(file);
  return 0;
}

void print(t_markov *x) {
  post("[markov] f_path=%s", x->f_path);
  post("[markov] state_len=%d, n_elements=%d, n_states=%d", x->m.state_len,
       x->m.n_elements, x->m.n_states);

  post("[markov] matrix:");
  for (int i = 0; i < x->m.n_states; i++) {
    post("[markov] %s", x->m.states[i]);
    for (int j = 0; j < x->m.n_elements; j++)
      post("[markov] %c: %f", x->m.elements[j], x->m.probabilities[i][j]);
  }
}

void on_bang(t_markov *x) { print(x); }

void *init(const t_symbol *t_f_path_sym, const t_floatarg t_state_len,
           const t_floatarg t_n_states) {
  t_markov *x = (t_markov *)pd_new(markov_class);

  x->f_path = t_f_path_sym->s_name;
  x->m.state_len = t_state_len;
  x->m.n_states = t_n_states;
  x->m.n_elements = pow(t_state_len, t_n_states);

  csv_to_pm(x->m, x->f_path);

  return x;
}

void destroy(t_markov *x) { (void)x; }

void markov_setup() {
  markov_class = class_new(gensym("markov"), (t_newmethod)init,
                           (t_method)destroy, sizeof(t_markov), CLASS_DEFAULT,
                           A_DEFSYMBOL, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);

  class_addbang(markov_class, (t_method)on_bang);

  class_sethelpsymbol(markov_class, gensym("markov"));
}