/* This files stores common datastructures and functions required to run test using each of the benchmarks */

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
  int32_t i1, i2, i3;
} Tri;

typedef struct triangle_mesh
{
  int32_t n_verts;
  int32_t n_faces;
  Vec3f* vertices;
  Tri* faces;
} TriMesh;


bool read_ply( const char* filename, TriMesh* mesh, bool *is_binary );
void write_ply( const char* filename, TriMesh* mesh, bool is_binary );

int32_t 
parse_arguments( const char* program_name, int argc, char**argv, Opts* opts )
{
  msh_argparse_t parser;
  opts->input_filename  = NULL;
  opts->output_filename = NULL;
  opts->verbose         = 0;

  msh_ap_init( &parser, program_name,
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

int32_t
run_test(const char* program_name, bool is_able_to_write_ply, int argc, char** argv )
{
  uint64_t t1, t2;
  Opts opts = {0};
  TriMesh mesh = {0};

  int parse_err = parse_arguments( program_name, argc, argv, &opts );
  if( parse_err ) { return 1; }

  if (!is_able_to_write_ply)
  {
    opts.output_filename = NULL;
  }

  bool is_binary = false;
  msh_cprintf( opts.verbose, "Reading %s ...\n", opts.input_filename );
  t1 = msh_time_now();
  read_ply( opts.input_filename, &mesh, &is_binary );
  t2 = msh_time_now();
  double read_time = msh_time_diff_ms( t2, t1 );

  double write_time = -1.0f;
  if( opts.output_filename )
  {
    t1 = msh_time_now();
    write_ply( opts.output_filename, &mesh, is_binary ); 
    t2 = msh_time_now();
    write_time = msh_time_diff_ms( t2, t1 );
  }
  
  msh_cprintf( !opts.verbose, "%f %f\n", read_time, write_time );

  msh_cprintf( opts.verbose, "Reading done in %lf ms\n", read_time );
  msh_cprintf( opts.verbose && opts.output_filename, "Writing done in %lf ms\n", write_time );
  msh_cprintf( opts.verbose, "N. Verts : %d; N. Faces: %d\n", mesh.n_verts, mesh.n_faces );

  return 0;
}