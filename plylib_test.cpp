/*
Author: Maciej Halber
Date: 04/09/18
Description: Benchmarking the read and write capabilities of plylib by @cnr-isti-vclab
I use ply file io code found in: https://github.com/cnr-isti-vclab/vcglib/tree/master/wrap/ply
which I believe to be used in meshlab. Can someone confirm or deny?
Example adapred from: https://github.com/cnr-isti-vclab/vcglib/blob/master/wrap/io_trimesh/import_ply.h
Task is to get positions and vertex_indices from a ply file that describe
triangular mesh and write that mesh back to hard drive.
License: Public Domain

Compilation:
g++ -I<path_to_msh> -Iplylib/ -O2 -std=c++11 plylib/plylib.cpp plylib_test.cpp -o bin/plylib_test

Notes: 
- Poor docs, hard to use
- Requires additional datastructures that try to predict everything(?)
- Decently flexible, other that the above point
- Plylib does not have write support, writing is adapted as a separate function from:
  (https://github.com/cnr-isti-vclab/vcglib/blob/master/wrap/io_trimesh/export_ply.h)
*/

#include <thread>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include "plylib.h"

#define MSH_STD_INCLUDE_LIBC_HEADERS
#define MSH_STD_INCLUDE_HEADERS
#define MSH_STD_IMPLEMENTATION
#define MSH_ARGPARSE_IMPLEMENTATION
#include "msh/msh_std.h"
#include "msh/msh_argparse.h"

typedef struct options
{
  bool verbose;
  char* input_filename;
  char* output_filename;
} Opts;

typedef struct vec3f
{
  float x,y,z;
} Vec3f;

typedef struct tri
{
  int i1, i2, i3;
} Tri;


typedef struct triangle_mesh
{
  int n_verts;
  int n_faces;
  Vec3f* vertices;
  Tri* faces;
} TriMesh;

struct LoadPly_VertAux
{
    float p[3];
    float n[3];
    int flags;
    float q; // the confidence
    float intensity;
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
    unsigned char data[1];
    float radius;
    float u,v,w;
};

struct LoadPly_FaceAux
{
    unsigned char size;
    int v[512];
    int flags;
    float q;
    float texcoord[32];
    unsigned char ntexcoord;
    int texcoordind;
    float colors[32];
    unsigned char ncolors;

    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;

    unsigned char data[1];
};

void
read_ply( const char* filename, TriMesh* mesh, bool * is_binary )
{
  using namespace vcg::ply;
  PlyFile pf;
  pf.Open(filename, PlyFile::MODE_READ);
  pf.AddToRead("vertex", "x", T_FLOAT, T_FLOAT,offsetof(LoadPly_VertAux,p),0,0,0,0,0 );
  pf.AddToRead("vertex", "y", T_FLOAT, T_FLOAT,offsetof(LoadPly_VertAux,p)+sizeof(float),0,0,0,0,0 );
  pf.AddToRead("vertex", "z", T_FLOAT, T_FLOAT,offsetof(LoadPly_VertAux,p)+2*sizeof(float),0,0,0,0,0 );
  pf.AddToRead("face", "vertex_indices", T_INT, T_INT, offsetof(LoadPly_FaceAux,v), 1, 0, T_UCHAR, T_UCHAR, offsetof(LoadPly_FaceAux,size) );
  LoadPly_VertAux va;
  LoadPly_FaceAux fa;
  *is_binary = ( pf.GetFormat() != F_ASCII );
  for(int i=0;i<int(pf.elements.size());i++)
  {
    int n = pf.ElemNumber(i);
    if( !strcmp( pf.ElemName(i),"vertex" ) )
    {
      int j;
      pf.SetCurElement(i);
      mesh->n_verts = n;
      mesh->vertices = (Vec3f*)malloc(sizeof(Vec3f)*n);
      for(j=0;j<n;++j)
      {
        pf.Read( (void *)&(va) );
        mesh->vertices[j].x = va.p[0];
        mesh->vertices[j].y = va.p[1];
        mesh->vertices[j].z = va.p[2];
      }
    }
    else if( !strcmp( pf.ElemName(i),"face")  )
    {
      int j;
      pf.SetCurElement(i);
      mesh->n_faces = n;
      mesh->faces = (Tri*)malloc(sizeof(Tri)*n);
      for(j=0;j<n;++j)
      {   
        pf.Read( (void *)&(fa) );
        mesh->faces[j].i1 = fa.v[0];
        mesh->faces[j].i2 = fa.v[1];
        mesh->faces[j].i3 = fa.v[2];
      }
    }
  }
  pf.Destroy();
}

// This is extremly simplified version of code in https://github.com/cnr-isti-vclab/vcglib/blob/master/wrap/io_trimesh/export_ply.h
 static int Save( const TriMesh* mesh, const char * filename, bool binary )	// V1.0
{
    FILE * fpout;
    int i;
    const char * hbin = "binary_little_endian";
    const char * hasc = "ascii";
    const char * h;
    const char* open_format;
    bool multit = false;

    if(binary) { h=hbin; open_format="wb"; }
    else       { h=hasc; open_format="w"; }

    
    fpout = fopen(filename,open_format);
    if(fpout==NULL)	{
        return 0;
    }
    fprintf(fpout,
        "ply\n"
        "format %s 1.0\n"
        "comment VCGLIB generated\n"
        ,h
        );


    const char* vttp = "float";
    fprintf(fpout,"element vertex %d\n",mesh->n_verts);
    fprintf(fpout,"property %s x\n", vttp);
    fprintf(fpout,"property %s y\n", vttp);
    fprintf(fpout,"property %s z\n", vttp);

    fprintf(fpout,"element face %d\n", mesh->n_faces );
    fprintf(fpout,"property list uchar int vertex_indices\n" );

    fprintf(fpout, "end_header\n"	);

    int j;
    for( j = 0; j < mesh->n_verts; j++)
    {
        if( binary )
        {
            float t;

            t = mesh->vertices[j].x; fwrite(&t,sizeof(float),1,fpout);
            t = mesh->vertices[j].y; fwrite(&t,sizeof(float),1,fpout);
            t = mesh->vertices[j].z; fwrite(&t,sizeof(float),1,fpout);
        }
        else 	// ***** ASCII *****
        {
            fprintf(fpout, "%g %g %g\n" ,mesh->vertices[j].x, mesh->vertices[j].y, mesh->vertices[j].z);
        }
    }

    /*vcg::tri::*/

    char c = 3;
    int vv[3];
    for( j = 0; j < mesh->n_faces; j++)
    {
        if(binary)
        {
            vv[0]=mesh->faces[j].i1;
            vv[1]=mesh->faces[j].i2;
            vv[2]=mesh->faces[j].i3;
            fwrite(&c,1,1,fpout);
            fwrite(vv,sizeof(int),3,fpout);
        }
        else	// ***** ASCII *****
        {
            fprintf(fpout,"%d " ,c);
            fprintf(fpout,"%d %d %d \n", mesh->faces[j].i1, mesh->faces[j].i2, mesh->faces[j].i3);
        }
    }
    fclose(fpout);
    return 0;
}

void
write_ply( const char* filename, const TriMesh* mesh, bool is_binary )
{
  Save( mesh, filename, is_binary );
}

int parse_arguments( int argc, char**argv, Opts* opts)
{
  msh_argparse_t parser;
  opts->input_filename  = NULL;
  opts->output_filename = (char*)"test.ply";
  opts->verbose         = 0;

  msh_ap_init( &parser, "mshply test",
               "This program simply reads and writes an input ply file" );
  msh_ap_add_string_argument( &parser, "input_filename", NULL, "Name of a ply file to read",
                           &opts->input_filename, 1 );
  msh_ap_add_string_argument( &parser, "--output_filename", "-o", "Name of a ply file to write",
                          &opts->output_filename, 1 );
  msh_ap_add_bool_argument( &parser, "--verbose", "-v", "Print verbose information",
                        &opts->verbose, 0 );

  if( !msh_ap_parse(&parser, argc, argv) )
  {
    return 1;
  }
  return 0;
}

int
main( int argc, char** argv )
{
  uint64_t t1, t2;
  Opts opts = {0};
  TriMesh mesh = {0};

  int parse_err = parse_arguments( argc, argv, &opts );
  if( parse_err ) { return 1; }

  bool is_binary = false;

  msh_cprintf(opts.verbose, "Reading %s ...\n", opts.input_filename );
  t1 = msh_time_now();
  read_ply( opts.input_filename, &mesh, &is_binary );
  t2 = msh_time_now();
  float read_time = msh_time_diff_ms( t2, t1 );
  t1 = msh_time_now();
  write_ply( opts.output_filename, &mesh, is_binary );
  t2 = msh_time_now();
  float write_time = msh_time_diff_ms( t2, t1 );
  msh_cprintf( !opts.verbose, "%f %f\n", read_time, write_time );
  msh_cprintf( opts.verbose, "Reading done in %lf ms\n", read_time );
  msh_cprintf( opts.verbose, "Writing done in %lf ms\n", write_time );
  msh_cprintf( opts.verbose, "N. Verts : %d ;N. Faces: %d \n", 
               mesh.n_verts, mesh.n_faces );
  int test_idx = 1024;
  msh_cprintf( opts.verbose, "Vert no. %d : %f %f %f\n",
                              test_idx, 
                              mesh.vertices[test_idx].x,
                              mesh.vertices[test_idx].y,
                              mesh.vertices[test_idx].z );
  msh_cprintf( opts.verbose, "Face no. %d : %d %d %d\n", 
                              test_idx, 
                              mesh.faces[test_idx].i1,
                              mesh.faces[test_idx].i2,
                              mesh.faces[test_idx].i3 );

  return 0;
}