/*
Author: Maciej Halber
Date: 11/26/20
Description:  Bencharking of the read and write capabilities of micro_ply.h by Nick Klingensmith (@maluoi)
Task is to get positions and vertex_indices from a ply file that describe
triangular mesh and write that mesh back to storage.
License: Public Domain

Compilation:
GCC  : g++ -I<path_to_msh> -Imshply/ -O2 -std=c11 mshply_test.c -o bin/mshply_test
MSVC : 

Comments: micro_ply.h only supports ASCII .ply files. It also does not seem to support anyway of prespecifing list size.
Additionally, it seems to silently crash if binary files are passed in. 
*/

#define MSH_STD_INCLUDE_LIBC_HEADERS
#define MSH_STD_IMPLEMENTATION
#define MSH_ARGPARSE_IMPLEMENTATION
#define MICRO_PLY_IMPL
#include "msh/msh_std.h"
#include "msh/msh_argparse.h"
#include "micro_ply/micro_ply.h"
#include "base_test.h"

bool
read_ply( const char* filename, TriMesh* mesh, bool *is_binary )
{
	// Read the data from the input file
	void  *data;
	size_t size;
	FILE  *fp;
	if ((fp = fopen(filename, "rb")) == nullptr)
  {
		return false;
	}
	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	rewind(fp);
	data = malloc(size);
	fread (data, size, 1, fp);
	fclose(fp);

	// Parse the data using ply_read
	ply_file_t file;
	if (!ply_read(data, size, &file))
  {
		return false;
  }

	float     fzero = 0;
	ply_map_t map_verts[] = {
		{ PLY_PROP_POSITION_X,  ply_prop_decimal, sizeof(float), 0,  &fzero },
		{ PLY_PROP_POSITION_Y,  ply_prop_decimal, sizeof(float), 4,  &fzero },
		{ PLY_PROP_POSITION_Z,  ply_prop_decimal, sizeof(float), 8,  &fzero } };
  ply_convert(&file, PLY_ELEMENT_VERTICES, map_verts, msh_count_of(map_verts), sizeof(Vec3f), (void **)&mesh->vertices, &mesh->n_verts);

	uint32_t  izero = 0;
	ply_map_t map_inds[] = { { PLY_PROP_INDICES, ply_prop_uint, sizeof(uint32_t), 0, &izero } };
	ply_convert(&file, PLY_ELEMENT_FACES, map_inds, msh_count_of(map_inds), sizeof(uint32_t), (void **)&mesh->faces, &mesh->n_faces);
  mesh->n_faces /= 3;
  
	// You gotta free the memory manually!
	ply_free(&file);
	free(data);

  return true;
}

void
write_ply( const char* filename, TriMesh* mesh, bool is_binary )
{
}

int
main( int argc, char** argv )
{
  bool is_able_to_write_ply = false;
  return run_test("microply_test", is_able_to_write_ply, argc, argv );
}
