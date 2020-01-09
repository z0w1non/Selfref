#ifndef BUILTIN_H
#define BUILTIN_H

#include "data.h"

data inc(data);
data dec(data);
data add(data);
data sub(data);
data mul(data);
data div_(data);
data mod(data);

data if_(data);
data call(data);
data bindsym(data);
data lambda(data);
data function(data);

data and_(data);
data or_(data);
data not_(data);

data length(data);
data setcar(data);
data setcdr(data);

data list(data);
data zipfirst(data);
data ziprest(data);
data zip(data);

data pushsym(data);
data popsym(data);

data tocharcode(data);
data fromcharcode(data);

#endif
