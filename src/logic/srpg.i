%module srpg
%{
#include "vec.h"
%}

%include "vec.h"
%template(vec2) srpg::vec<2, float>;
%template(intvec2) srpg::vec<2, int>;
