#ifndef __TYPES_H__
#define __TYPES_H__

/* Color struct */
typedef struct color_struct {
   double r;
   double g;
   double b;
   double f; // "filter" or "alpha"
} color_t;

typedef struct vertex_struct {
   int x;
   int y;
   int z;
} vertex_t;

typedef struct bary_struct {
   double alpha;
   double beta;
   double gamma;
} bary_t;

#endif
