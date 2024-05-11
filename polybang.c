#include "m_pd.h"

static t_class *polybang_class;

typedef struct _polybang {
  t_object x_obj;
  t_int init_count, current_count;
  t_int mod_A, mod_B;
} t_polybang;

void set_counts(t_polybang *x, t_floatarg init, t_floatarg curr) {
  x->init_count = init;
  x->current_count = curr;
}

void reset_counts(t_polybang *x) { set_counts(x, 0, 0); }

void set_mods(t_polybang *x, t_floatarg a, t_floatarg b) {
  x->mod_A = a <= 0 ? 1 : a;
  x->mod_B = b <= 0 ? 1 : b;
}

void *polybang_new(t_floatarg a, t_floatarg b) {
  t_polybang *x = (t_polybang *)pd_new(polybang_class);

  reset_counts(x);
  set_mods(x, a, b);

  post("Polybang initialized with A=%d, B=%d", x->mod_A, x->mod_B);

  return x;
}

void polybang_on_bang(t_polybang *x) { post("[polybang ] is ready to go!"); }

void polybang_setup() {
  polybang_class = class_new(gensym("polybang"),           // Object name
                             (t_newmethod)(polybang_new),  // Initializer
                             NULL,                         // Destructor
                             sizeof(t_polybang),           // Object size
                             CLASS_DEFAULT,                // Rectangle object
                             //  Creation arguments: float (A), float (B)
                             A_DEFFLOAT, A_DEFFLOAT,
                             0  //  No more arguments
  );

  class_addbang(polybang_class, (t_method)polybang_on_bang);
}