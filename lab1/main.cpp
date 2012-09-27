/*
   CPE 471 Lab 1
   Base code for Rasterizer
*/

#include <stdio.h>
#include <stdlib.h>
#include "Image.h"
#include "types.h"
#include <string>
#include <iostream>
using namespace std;

void rasterizeTriangle(Image &img, triangle_t t);
int det(int a, int b, int c, int d);
bary_t computeBary(int x, int y, triangle_t t, double area);
color_t computeColor(triangle_t t, bary_t p);
triangle_t transform(triangle_t t, int w, int h);
vertex_t transformVertex(vertex_t v, int w, int h);

int main(void)
{
   int WIDTH = 960;
   int HEIGHT = 600;
   triangle_t t;

   cin >> t.a.x;
   cin >> t.a.y;
   cin >> t.b.x;
   cin >> t.b.y;
   cin >> t.c.x;
   cin >> t.c.y;

   t.aC.r = 1.0;
   t.aC.g = 0.0;
   t.aC.b = 0.0;
   t.aC.f = 1.0;
   
   t.bC.r = 0.0;
   t.bC.g = 1.0;
   t.bC.b = 0.0;
   t.bC.f = 1.0;
   
   t.cC.r = 0.0;
   t.cC.g = 0.0;
   t.cC.b = 1.0;
   t.cC.f = 1.0;

   // make a image (allocates buffer on the heap)
   Image img(WIDTH, HEIGHT);
   
   rasterizeTriangle(img, transform(t, WIDTH, HEIGHT));

   // write the targa file to disk
   img.WriteTga((char *)"awesome.tga", true);
   // true to scale to max color, false to clamp to 1.0
}

triangle_t transform(triangle_t t, int w, int h) {
  //printf("(%d, %d), (%d, %d), (%d, %d)\n", t.a.x, t.a.y, t.b.x, t.b.y, t.c.x, t.c.y);
  t.a = transformVertex(t.a, w, h);
  t.b = transformVertex(t.b, w, h);
  t.c = transformVertex(t.c, w, h);
  //printf("(%d, %d), (%d, %d), (%d, %d)\n", t.a.x, t.a.y, t.b.x, t.b.y, t.c.x, t.c.y);
  return t;
}

vertex_t transformVertex(vertex_t v, int w, int h) {
   double l = -w/(double)h;
   double r = w/(double)h;
   double b = -1.0;
   double t = 1.0;
   
   // c * Xw + d = Xp
   // e * Yw + f = Yp

   // c * l + d = 0
   // c * r + d = w-1
   // ----------------
   // c(l-r) = -(w-1)
   double c = -(w-1) / (l-r);
   double d = (w-1) - (c*r);

   // e * b + f = 0
   // e * t + f = h-1
   double e = -(h-1) / (b-t);
   double f = (h-1) - (e*t);

   v.x = (int)(c * v.x + d);
   v.y = (int)(e * v.y + f);

   return v;
}

void rasterizeTriangle(Image &img, triangle_t t)
{
   // Calculate bounding box
   int min_x = img.width();
   int max_x = 0;
   int min_y = img.height();
   int max_y = 0;

   min_x = min(min_x, min(t.a.x, min(t.b.x, t.c.x)));
   min_y = min(min_y, min(t.a.y, min(t.b.y, t.c.y)));
   max_x = max(max_x, max(t.a.x, max(t.b.x, t.c.x)));
   max_y = max(max_y, max(t.a.y, max(t.b.y, t.c.y)));
   
   // Calculate area
   double area = 0.5 * det(t.c.x - t.a.x, t.b.x - t.a.x, t.c.y - t.a.y, t.b.y - t.a.y);

   // For each pixel in bounding box
   for (int y = min_y; y <= max_y; y++)
      for (int x = min_x; x <= max_x; x++)
      {
         // Calculate barycentric coordinates
         bary_t bary = computeBary(x, y, t, area);
         // If coords are in bounds
         if (bary.alpha >= -0.0001 && bary.alpha <= 1.0 &&
             bary.beta  >= -0.0001 && bary.beta  <= 1.0 &&
             bary.gamma >= -0.0001 && bary.gamma <= 1.0)
         {
            // Color the pixel
            color_t color = computeColor(t, bary);
            img.pixel(x, y, color);
         }
      }
}

int det(int a, int b, int c, int d) {
   return (a * d) - (b * c);
}

bary_t computeBary(int x, int y, triangle_t t, double area) {
   bary_t ret;
   ret.beta = -0.5 * det(t.a.x - t.c.x, x - t.c.x, t.a.y - t.c.y, y - t.c.y) / area;
   ret.gamma = -0.5 * det(t.b.x - t.a.x, x - t.a.x, t.b.y - t.a.y, y - t.a.y) / area;
   ret.alpha = 1 - ret.beta - ret.gamma;
   return ret;
}

color_t computeColor(triangle_t t, bary_t p) {
   color_t color;
   color_t a = t.aC;
   color_t b = t.bC;
   color_t c = t.cC;
   color.r = (a.r * p.alpha) + (b.r * p.beta) + (c.r * p.gamma);
   color.g = (a.g * p.alpha) + (b.g * p.beta) + (c.g * p.gamma);
   color.b = (a.b * p.alpha) + (b.b * p.beta) + (c.b * p.gamma);
   color.f = (a.f * p.alpha) + (b.f * p.beta) + (c.f * p.gamma);

   return color;
}
