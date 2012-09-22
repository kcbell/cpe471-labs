/*
   CPE 471 Lab 1
   Base code for Rasterizer
   Example code using B. Somers' image code - writes out a sample tga
*/

#include <stdio.h>
#include <stdlib.h>
#include "Image.h"
#include "types.h"
#include <string>
#include <iostream>
using namespace std;

int main(void)
{
   int IMG_WIDTH = 640;
   int IMG_HEIGHT = 480;
   
   // Get input, one vertex per line
   string input;
   vertex_t vertices[3];
   int min_x = IMG_WIDTH;
   int max_x = 0;
   int min_y = IMG_HEIGHT;
   int max_y = 0;
   for (int coord = 0; coord < 6; coord++)
   {
      if (coord % 2 == 0)
      {
         cin >> vertices[coord/2].x;
         min_x = min(min_x, vertices[coord/2].x);
         max_x = max(max_x, vertices[coord/2].x);
      }
      else
      {
         cin >> vertices[coord/2].y;
         min_y = min(min_y, vertices[coord/2].y);
         max_y = max(max_y, vertices[coord/2].y);
      }
   }
   
   // make a blue
   color_t blue;
   blue.r = 0.1;
   blue.g = 0.1;
   blue.b = 0.9;

   // make a 640x480 image (allocates buffer on the heap)
   Image img(640, 480);

   //Render bounding box as blue
   for (int y = min_y; y <= max_y; y++)
      for (int x = min_x; x <= max_x; x++)
      {
         img.pixel(x, y, blue);
      }

   // write the targa file to disk
   img.WriteTga((char *)"awesome.tga", true);
   // true to scale to max color, false to clamp to 1.0
}

int det(int a, int b, int c, int d) {
   return (a * d) - (b * c);
}

bary_t computeBary(int x, int y, triangle_t t) {
   bary_t ret;
   double area = 0.5 * det(t.c.x - t.a.x, t.b.x - t.a.x, t.c.y - t.a.y, t.b.y - t.a.y);
   ret.beta = 0.5 * det(t.a.x - t.c.x, x - t.c.x, t.a.y - t.c.y, y - t.c.y) / area;
   ret.gamma = 0.5 * det(t.b.x - t.a.x, x - t.a.x, t.b.y - t.a.y, y - t.a.y) / area;
   ret.alpha = 1 - ret.beta - ret.gamma
   return ret;
}

color_t computeColor(color_t a, color_t b, color_t c, bary_t p) {
   color_t color;
   color.r = (a.r * p.alpha) + (b.r * p.beta) + (c.r * p.gamma);
   color.g = (a.g * p.alpha) + (b.g * p.beta) + (c.g * p.gamma);
   color.b = (a.b * p.alpha) + (b.b * p.beta) + (c.b * p.gamma);
   color.f = (a.f * p.alpha) + (b.f * p.beta) + (c.f * p.gamma);

   return color;
}
