#include "m_pd.h"

static t_class *polybang_class;

typedef struct _polybang {
  t_object x_obj;
  t_int init_count, current_count;
  t_int mod_A, mod_B;
  t_inlet *in_A, *in_B;
  t_outlet *out_A, *out_B, *out_sync, *out_count;
} t_polybang;

void print(t_polybang *x) {
  post("[polybang ] init_count=%d, current_count=%d", x->init_count,
       x->current_count);
  post("[polybang ] mod_A=%d, mod_B=%d", x->mod_A, x->mod_B);
}

void set_counts(t_polybang *x, t_floatarg init, t_floatarg curr) {
  x->init_count = init;
  x->current_count = curr;
}

void reset_counts(t_polybang *x) { set_counts(x, 0, 0); }

void set_mods(t_polybang *x, t_floatarg a, t_floatarg b) {
  x->mod_A = a <= 0 ? 1 : a;
  x->mod_B = b <= 0 ? 1 : b;
}

void on_bang(t_polybang *x) {
  print(x);

  t_int mod_A = x->mod_A;
  t_int mod_B = x->mod_B;
  t_int mod_sync = mod_A * mod_B;
  t_int n = x->current_count;

  if (n % mod_sync == 0) {
    outlet_bang(x->out_sync);
    x->current_count = 0;
  }
  if (n % mod_A == 0) outlet_bang(x->out_A);
  if (n % mod_B == 0) outlet_bang(x->out_B);

  ++x->current_count;
}

void onset_A(t_polybang *x, t_floatarg a) { set_mods(x, a, x->mod_B); }

void onset_B(t_polybang *x, t_floatarg b) { set_mods(x, x->mod_A, b); }

void on_reset(t_polybang *x) { reset_counts(x); }

void on_list(t_polybang *x, t_symbol *_, t_int argc, t_atom *argv) {
  if (argc == 2) {
    set_mods(x, atom_getfloat(argv), atom_getfloat(argv + 1));
    return;
  }

  pd_error(&x->x_obj, "[polybang ] Expected two arguments in list");
}

void *init(t_floatarg a, t_floatarg b) {
  t_polybang *x = (t_polybang *)pd_new(polybang_class);

  reset_counts(x);
  set_mods(x, a, b);

  x->in_A = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ratio_A"));
  x->in_B = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ratio_B"));

  x->out_A = outlet_new(&x->x_obj, &s_bang);
  x->out_B = outlet_new(&x->x_obj, &s_bang);
  x->out_sync = outlet_new(&x->x_obj, &s_bang);
  x->out_count = outlet_new(&x->x_obj, &s_float);

  print(x);

  return x;
}

void destroy(t_polybang *x) {
  inlet_free(x->in_A);
  inlet_free(x->in_B);

  outlet_free(x->out_A);
  outlet_free(x->out_B);
  outlet_free(x->out_sync);
  outlet_free(x->out_count);
}

void polybang_setup() {
  polybang_class = class_new(gensym("polybang"),  // Object name
                             (t_newmethod)init,   // Initializer
                             (t_method)destroy,   // Destructor
                             sizeof(t_polybang),  // Object size
                             CLASS_DEFAULT,       // Rectangle object
                             //  Creation arguments: float (A), float (B)
                             A_DEFFLOAT, A_DEFFLOAT, 0);

  class_addbang(polybang_class, (t_method)on_bang);
  class_addmethod(polybang_class, (t_method)onset_A, gensym("ratio_A"),
                  A_DEFFLOAT, 0);
  class_addmethod(polybang_class, (t_method)onset_B, gensym("ratio_B"),
                  A_DEFFLOAT, 0);
  class_addmethod(polybang_class, (t_method)on_reset, gensym("reset"), 0);
  class_addlist(polybang_class, (t_method)on_list);
  class_sethelpsymbol(polybang_class, gensym("polybang"));
}