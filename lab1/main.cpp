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

int main(void)
{
   int IMG_WIDTH = 640;
   int IMG_HEIGHT = 480;
   triangle_t t;

   // Read a mesh from file
   CMesh* mesh;
   mesh = CMeshLoader::loadASCIIMesh(argv[1]);

   // make a 640x480 image (allocates buffer on the heap)
   Image img(640, 480);

   rasterizeMesh(img, mesh);

   // write the targa file to disk
   img.WriteTga((char *)"awesome.tga", true);
   // true to scale to max color, false to clamp to 1.0
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
