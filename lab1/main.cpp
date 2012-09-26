/*
   CPE 471 Lab 1
   Base code for Rasterizer
*/

#include <stdio.h>
#include <stdlib.h>
#include "Image.h"
#include "CMeshLoaderSimple.h"
#include "types.h"
#include <string>
#include <iostream>
using namespace std;

void rasterizeTriangle(Image &img, STriangle t);
int det(int a, int b, int c, int d);
bary_t computeBary(int x, int y, STriangle t, double area);
SColor computeColor(STriangle t, bary_t p, CMesh* mesh);
void rasterizeMesh(Image &img, CMesh* mesh);

int main(int argc, char** argv)
{
   int IMG_WIDTH = 640;
   int IMG_HEIGHT = 480;

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

void rasterizeMesh(Image &img, CMesh* mesh)
{
   
}

void rasterizeTriangle(Image &img, STriangle t)
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

bary_t computeBary(int x, int y, STriangle t, double area, CMesh* mesh)
{
   bary_t ret;
   ret.beta = -0.5 * det(mesh->Vertices.at(VertexIndex1).X - mesh->Vertices.at(VertexIndex3).X,
         x - mesh->Vertices.at(VertexIndex3).X,
         mesh->Vertices.at(VertexIndex1).Y - mesh->Vertices.at(VertexIndex3).Y,
         y - mesh->Vertices.at(VertexIndex3).Y) / area;
   ret.gamma = -0.5 * det(mesh->Vertices.at(VertexIndex2).X - mesh->Vertices.at(VertexIndex1).X,
         x - mesh->Vertices.at(VertexIndex1).X,
         mesh->Vertices.at(VertexIndex2).Y - mesh->Vertices.at(VertexIndex1).Y,
         y - mesh->Vertices.at(VertexIndex1).Y) / area;
   ret.alpha = 1 - ret.beta - ret.gamma;
   return ret;
}

SColor computeColor(STriangle t, bary_t p, CMesh * mesh)
{
   SColor color;
   SColor a = mesh->Vertices.at(VertexIndex1).Color;
   SColor b = mesh->Vertices.at(VertexIndex2).Color;
   SColor c = mesh->Vertices.at(VertexIndex3).Color;
   color.Red = (a.Red * p.alpha) + (b.Red * p.beta) + (c.Red * p.gamma);
   color.Green = (a.Green * p.alpha) + (b.Green * p.beta) + (c.Green * p.gamma);
   color.Blue = (a.Blue * p.alpha) + (b.Blue * p.beta) + (c.Blue * p.gamma);
   //color.f = (a.f * p.alpha) + (b.f * p.beta) + (c.f * p.gamma);

   return color;
}
