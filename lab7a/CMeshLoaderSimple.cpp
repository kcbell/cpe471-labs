#include "CMeshLoaderSimple.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <math.h>
#include <iomanip>
#include <string>
#include <streambuf>

class SColor
{

public:

    float Red, Green, Blue;

    SColor()
        : Red(0.7f), Green(0.7f), Blue(0.7f)
    {}

};

class SVector3
{

public:

	float X, Y, Z;

	SVector3()
		: X(0), Y(0), Z(0)
	{}

	SVector3(float in)
		: X(in), Y(in), Z(in)
	{}

	SVector3(float in_x, float in_y, float in_z)
		: X(in_x), Y(in_y), Z(in_z)
	{}

	SVector3 crossProduct(SVector3 const & v) const
	{
		return SVector3(Y*v.Z - v.Y*Z, v.X*Z - X*v.Z, X*v.Y - v.X*Y);
	}

	float dotProduct(SVector3 const & v) const
	{
		return X*v.X + Y*v.Y + Z*v.Z;
	}

	float length() const
	{
		return sqrtf(X*X + Y*Y + Z*Z);
	}

	SVector3 operator + (SVector3 const & v) const
	{
		return SVector3(X+v.X, Y+v.Y, Z+v.Z);
	}

	SVector3 & operator += (SVector3 const & v)
	{
		X += v.X;
		Y += v.Y;
		Z += v.Z;

		return * this;
	}

	SVector3 operator - (SVector3 const & v) const
	{
		return SVector3(X-v.X, Y-v.Y, Z-v.Z);
	}

	SVector3 & operator -= (SVector3 const & v)
	{
		X -= v.X;
		Y -= v.Y;
		Z -= v.Z;

		return * this;
	}

	SVector3 operator * (SVector3 const & v) const
	{
		return SVector3(X*v.X, Y*v.Y, Z*v.Z);
	}

	SVector3 & operator *= (SVector3 const & v)
	{
		X *= v.X;
		Y *= v.Y;
		Z *= v.Z;

		return * this;
	}

	SVector3 operator / (SVector3 const & v) const
	{
		return SVector3(X/v.X, Y/v.Y, Z/v.Z);
	}

	SVector3 & operator /= (SVector3 const & v)
	{
		X /= v.X;
		Y /= v.Y;
		Z /= v.Z;

		return * this;
	}

	SVector3 operator * (float const s) const
	{
		return SVector3(X*s, Y*s, Z*s);
	}

	SVector3 & operator *= (float const s)
	{
		X *= s;
		Y *= s;
		Z *= s;

		return * this;
	}

	SVector3 operator / (float const s) const
	{
		return SVector3(X/s, Y/s, Z/s);
	}

	SVector3 & operator /= (float const s)
	{
		X /= s;
		Y /= s;
		Z /= s;

		return * this;
	}

};

class SVertex
{

public:

    SVector3 Position;
    SColor Color;

};

class CMesh
{

    friend class CVertexBufferObject;
    friend class CMeshLoader;

    struct STriangle
    {
        unsigned int VertexIndex1, VertexIndex2, VertexIndex3;
		SColor Color;
    };

    std::vector<SVertex> Vertices;
    std::vector<STriangle> Triangles;

    CMesh();

public:

    ~CMesh();

	void centerMeshByAverage(SVector3 const & CenterLocation);
	void centerMeshByExtents(SVector3 const & CenterLocation);

	void resizeMesh(SVector3 const & Scale);

};

bool CMeshLoader::loadVertexBufferObjectFromMesh(std::string const & fileName, int & TriangleCount, GLuint & PositionBufferHandle, GLuint & ColorBufferHandle)
{
	CMesh * Mesh = loadASCIIMesh(fileName);
	if (! Mesh)
		return false;

    //ADD code to resize the mesh and to reposition it at the origin
    Mesh

    //ADD code to create a VBO for the mesh
	
	return true;
}

CMesh * const CMeshLoader::loadASCIIMesh(std::string const & fileName)
{
	std::ifstream File;
	File.open(fileName.c_str());

	if (! File.is_open())
	{
		std::cerr << "Unable to open mesh file: " << fileName << std::endl;
		return 0;
	}

	CMesh * Mesh = new CMesh();

	while (File)
	{
		std::string ReadString;
		std::getline(File, ReadString);

		std::stringstream Stream(ReadString);

		std::string Label;
		Stream >> Label;

		if (Label.find("#") != std::string::npos)
		{
			// Comment, skip
			continue;
		}

		if ("Vertex" == Label)
		{
			int Index;
			Stream >> Index; // We don't care, throw it away

			SVector3 Position;
			Stream >> Position.X;
			Stream >> Position.Y;
			Stream >> Position.Z;

			SVertex Vertex;
			Vertex.Position = Position;

			Mesh->Vertices.push_back(Vertex);
		}
		else if ("Face" == Label)
		{
			int Index;
			Stream >> Index; // We don't care, throw it away

			int Vertex1, Vertex2, Vertex3;
			Stream >> Vertex1;
			Stream >> Vertex2;
			Stream >> Vertex3;

			CMesh::STriangle Triangle;
			Triangle.VertexIndex1 = Vertex1 - 1;
			Triangle.VertexIndex2 = Vertex2 - 1;
			Triangle.VertexIndex3 = Vertex3 - 1;

			size_t Location;
			if ((Location = ReadString.find("{")) != std::string::npos) // there is a color
			{
				Location = ReadString.find("rgb=(");
				Location += 5; // rgb=( is 5 characters

				ReadString = ReadString.substr(Location);
				std::stringstream Stream(ReadString);
				float Color;
				Stream >> Color;
				Triangle.Color.Red = Color;
				Stream >> Color;
				Triangle.Color.Green = Color;
				Stream >> Color;
				Triangle.Color.Blue = Color;
			}

			Mesh->Triangles.push_back(Triangle);
		}
		else if ("" == Label)
		{
			// Just a new line, carry on...
		}
		else if ("Corner" == Label)
		{
			// We're not doing any normal calculations... (oops!)
		}
		else
		{
			std::cerr << "While parsing ASCII mesh: Expected 'Vertex' or 'Face' label, found '" << Label << "'." << std::endl;
		}
	}

	if (! Mesh->Triangles.size() || ! Mesh->Vertices.size())
	{
		delete Mesh;
		return 0;
	}

	return Mesh;
}



CMesh::CMesh()
{}

CMesh::~CMesh()
{}

void CMesh::centerMeshByAverage(SVector3 const & CenterLocation)
{
//ADD this or the below function
}

void CMesh::centerMeshByExtents(SVector3 const & CenterLocation)
{
    if (Vertices.size() < 2)
        return;

    SVector3 Min, Max;
    {
        std::vector<SVertex>::const_iterator it = Vertices.begin();
        Min = it->Position;
        Max = it->Position;
        for (; it != Vertices.end(); ++ it)
        {
            if (Min.X > it->Position.X)
                Min.X = it->Position.X;
            if (Min.Y > it->Position.Y)
                Min.Y = it->Position.Y;
            if (Min.Z > it->Position.Z)
                Min.Z = it->Position.Z;

            if (Max.X < it->Position.X)
                Max.X = it->Position.X;
            if (Max.Y < it->Position.Y)
                Max.Y = it->Position.Y;
            if (Max.Z < it->Position.Z)
                Max.Z = it->Position.Z;
        }
    }

    SVector3 Center = (Max + Min) / 2;

    SVector3 VertexOffset = CenterLocation - Center;
    for (std::vector<SVertex>::iterator it = Vertices.begin(); it != Vertices.end(); ++ it)
        it->Position += VertexOffset;
}

void CMesh::resizeMesh(SVector3 const & Scale)
{
    if (Vertices.size() < 2)
        return;

    SVector3 Min, Max;
    {
        std::vector<SVertex>::const_iterator it = Vertices.begin();
        Min = it->Position;
        Max = it->Position;
        for (; it != Vertices.end(); ++ it)
        {
            if (Min.X > it->Position.X)
                Min.X = it->Position.X;
            if (Min.Y > it->Position.Y)
                Min.Y = it->Position.Y;
            if (Min.Z > it->Position.Z)
                Min.Z = it->Position.Z;

            if (Max.X < it->Position.X)
                Max.X = it->Position.X;
            if (Max.Y < it->Position.Y)
                Max.Y = it->Position.Y;
            if (Max.Z < it->Position.Z)
                Max.Z = it->Position.Z;
        }
    }

    SVector3 Extent = (Max - Min);
    SVector3 Resize = Scale / std::max(Extent.X, std::max(Extent.Y, Extent.Z));
    for (std::vector<SVertex>::iterator it = Vertices.begin(); it != Vertices.end(); ++ it)
        it->Position *= Resize;
}
