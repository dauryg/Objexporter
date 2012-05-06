#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "objexporter.h"

int main( int argc, char *argv[] )
{
  // Initial Setup
  init();
  options o;
  atexit( cleanUp );

  o.file              = NULL;
  o.outputFile        = NULL;
  o.outputFileType    = NULL;
  o.verbose           = 0;
  o.normals           = 0;
  o.texture           = 0;
  o.flatArray         = 0;
  int failed          = processArgs( argc, argv, &o );

  if( failed )
  {
    return 1;
  }

  generate( &o );
  return 0;
}

void init()
{
  OBJModel.numOfObjVertex = 0;
  OBJModel.numOfObjNormal = 0;
  OBJModel.numOfObjTex    = 0;
  OBJModel.numOfTriangle  = 0;
  OBJModel.indexCount     = 0;
  OBJModel.indexLength    = 0;
  OBJModel.min.x          = 1000;
  OBJModel.min.y          = 1000;
  OBJModel.min.z          = 1000;
  OBJModel.max.x          = -1000;
  OBJModel.max.y          = -1000;
  OBJModel.max.z          = -1000;
  OBJModel.indices        = NULL;
  OBJModel.vertices       = NULL;
  OBJModel.normals        = NULL;
  OBJModel.textures       = NULL;
  OBJModel.faces          = NULL;
}

void printHelp()
{
  printf( "Usage: objexporter [option] SOURCEFILE DESTFILE\n" );
  printf( "Exports Wavefront OBJ file to a flat array.\n" );
  printf( "\n" );
  printf( "  -v               Verbose\n" );
  printf( "\n" );
  printf( "Examples:\n" );
  printf( "          objexporter model.obj model.h  generate the flat array's to model.h\n" );
  printf( "\n" );
}

void cleanUp( void )
{ 
  int i;
  for( i = 0; i < OBJModel.numOfTriangle; i++)
  {
    free( OBJModel.faces[i].indexes );
  }

  free( OBJModel.vertices );
  free( OBJModel.normals );
  free( OBJModel.textures );
  free( OBJModel.indices );
  free( OBJModel.faces );
}

void generate( options *o)
{
  FILE  *fileHandle   = fopen( o->file, "r" );
  char  line[ 256 ];

  if( !fileHandle )
  {
    perror( "Error opening the source file" );
  }
  else
  {
    assert( fileHandle ); 
    
    while( fgets( line, 256, fileHandle ) )
    {
      count( line );
    }

    rewind( fileHandle );

    OBJModel.vertices     = calloc( OBJModel.numOfObjVertex , sizeof( vertex ) );
    assert( OBJModel.vertices );

    if( OBJModel.numOfObjNormal > 0 )
    {
      OBJModel.normals      = calloc( OBJModel.numOfObjNormal , sizeof( normal ) );
      assert( OBJModel.normals );
    }

    if( OBJModel.numOfObjNormal > 0 )
    {
      OBJModel.textures     = calloc( OBJModel.numOfObjTex , sizeof( texture ) );
      assert( OBJModel.textures );
    }
    
    OBJModel.faces        = calloc( OBJModel.numOfObjFaces , sizeof( face ) );
    OBJModel.indexLength  = OBJModel.numOfObjFaces * 3;
    OBJModel.indices      = calloc( OBJModel.indexLength, sizeof( Index ) );

    assert( OBJModel.faces );
    assert( OBJModel.indices );

    while( fgets( line, 256, fileHandle ) )
    {
      processLine( line );
    } 

    generateFile( o->outputFile );
    
    if( o->verbose )
    { 
      printf( "Total Vertices: %i\n", OBJModel.numOfObjVertex );
      printf( "Total Normals: %i\n", OBJModel.numOfObjNormal );
      printf( "Total Textures: %i\n", OBJModel.numOfObjTex );
      printf( "Total Index: %i\n", OBJModel.indexCount );
    }

    fclose( fileHandle );
  }
}

void count( char *line )
{ 
  char temp[256];
  strcpy( temp, line );

  char *token   = strtok( temp, " " );

  if( strcmp( token, "v" ) == 0 )
  { 
    OBJModel.numOfObjVertex++;
  }
  else if( strcmp( token, "vn" ) == 0 )
  { 
    OBJModel.numOfObjNormal++;
  }
  else if( strcmp( token, "vt" ) == 0 )
  {
    OBJModel.numOfObjTex++;
  }
  else if( strcmp( token, "f" ) == 0 )
  { 
    OBJModel.numOfObjFaces++;
  }
}

int processArgs( int argc, char*argv[], options *o )
{
  assert( argv );
  assert( o );

  // setup default options
  o->outputFileType        = "c";
  
  if( argc < 3 )
  {
    printf( "objexporter: missing operand\n" );
    printf( "Try 'objexporter --help' for more information\n");
    return 1;
  }

  int i;
  for( i = 1; i < argc - 2; i++ )
  {
    char *arg = argv[i];

    if( strcmp( arg, "--help" ) == 0 )
    {
      printHelp();
      return 1;
    }
    else if( strcmp( arg, "-v" ) == 0 )
    {
      o->verbose = 1;
    }
  }

  o->file         = argv[ argc - 2 ];
  o->outputFile   = argv[ argc - 1 ];

  return 0;
}

void processLine( const char *line )
{
  char temp[256];
  strcpy( temp, line );

  char *token   = strtok( temp, " " );

  if( strcmp( token, "v" ) == 0 )
  { 
    processVertex( line );
  }
  else if( strcmp( token, "vn" ) == 0 )
  { 
    processNormal( line );
  }
  else if( strcmp( token, "vt" ) == 0 )
  {
    processTexture( line ); 
  }
  else if( strcmp( token, "f" ) == 0 )
  { 
    processFace( line );
  }
}

void processVertex( const char *line)
{
  static int i = 0;
  
  float x,z,y;
  
  sscanf( line, "%*s %f %f %f", &x, &y, &z );
  
  OBJModel.vertices[ i ].x  = x;
  OBJModel.vertices[ i ].y  = y;
  OBJModel.vertices[ i ].z  = z;

  //Min
  if( x < OBJModel.min.x )
  {
    OBJModel.min.x = x;
  }
  if( y < OBJModel.min.y )
  {
    OBJModel.min.y = y;
  }
  if( z < OBJModel.min.z )
  {
    OBJModel.min.z = z;
  }

  //Max
  if( x > OBJModel.max.x )
  {
    OBJModel.max.x = x;
  }
  if( y > OBJModel.max.y )
  {
    OBJModel.max.y = y;
  }
  if( z > OBJModel.max.z )
  {
    OBJModel.max.z = z;
  }
  i++;
}

void processNormal( const char *line)
{
  static int i = 0;

  float x,z,y;
  sscanf( line, "%*s %f %f %f", &x, &y, &z );
  
  OBJModel.normals[ i ].x  = x;
  OBJModel.normals[ i ].y  = y;
  OBJModel.normals[ i ].z  = z;

  i++;
}

void processTexture( const char *line)
{
  static int i = 0;

  float u,v;
  sscanf( line, "%*s %f %f", &u, &v );
  
  OBJModel.textures[ i ].u  = u;
  OBJModel.textures[ i ].v  = v;

  i++;
}

void processFace( const char *line)
{
  static int i = 0;

  //Determine how many tokens the line contains
  char templine[ 256 ];
  char *token;
  int numOfTokens;

  strcpy( templine , line );
  
  numOfTokens     = 0;
  token           = strtok( templine, " \r\n" );

  while( token != NULL )
  {
    numOfTokens++;
    token     = strtok( NULL, " \r\n" );
  }

  // subtract the line identifying token 'f'
  numOfTokens -= 1;

  int *indexes      = calloc( numOfTokens, sizeof( int ) );
  assert( indexes );

  OBJModel.faces[ i ].count       = numOfTokens;
  OBJModel.faces[ i ].indexes     = indexes;

  //Get each token, append to the main index array if
  //it doesn't exist and add to the face's index list
  strcpy( templine , line );
  token = strtok( templine, " \r\n" ); // skip the first token

  token           = strtok( NULL, " \r\n" );
  
  int element = 0; 
  while( token != NULL )
  {

    int vertex    = 0;
    int normal    = 0;
    int texture   = 0; 
    
    //index vertex/texture/normal
    //has texture and normal
    if( OBJModel.numOfObjTex > 0 && OBJModel.numOfObjNormal > 0 )
    {
      sscanf( token, "%d/%d/%d", &vertex, &texture, &normal );
    } 
    //index vertex/texture 
    //has texture but no normal
    else if( OBJModel.numOfObjTex > 0 && OBJModel.numOfObjNormal == 0 )
    {
      sscanf( token, "%d/%d", &vertex, &normal );
    }
    //index vertex//normal 
    //has normal but no texture
    else if( OBJModel.numOfObjNormal > 0 && OBJModel.numOfObjTex == 0 )
    {
      sscanf( token, "%d//%d", &vertex, &normal );
    }
    //index vertex
    //has only a vertex
    else
    {
      sscanf( token, "%d", &vertex );
    }
    
    Index index;
    index.vertex  = 0;
    index.normal  = 0;
    index.texture = 0;

    if( vertex < 0 )
    {
      index.vertex  = OBJModel.numOfObjVertex + vertex;
    }
    else
    {
      index.vertex  = vertex - 1;
    }

    if( OBJModel.numOfObjNormal )
    {
      if( normal < 0 )
      {
        index.normal  = OBJModel.numOfObjNormal + normal;
      }
      else
      {
        index.normal  = normal - 1;
      }
    }

    if( OBJModel.numOfObjTex )
    {
      if( texture < 0 )
      {
        index.texture = OBJModel.numOfObjTex + texture;
      }
      else
      {
        index.texture = texture - 1;
      }
    }
    
    int indexElement  = containsIndex( OBJModel.indices, OBJModel.indexLength, &index );

    if( indexElement > -1 )
    { 
      OBJModel.faces[ i ].indexes[ element ]     = indexElement;
    }
    else
    { 
      if( OBJModel.indexCount < OBJModel.indexLength )
      {
        OBJModel.indices[ OBJModel.indexCount ] = index;
        OBJModel.faces[ i ].indexes[ element ]  = OBJModel.indexCount;
      }
      else
      {
        int newLength         = OBJModel.indexLength * 2;
        Index *indices        = realloc( OBJModel.indices, sizeof( Index ) * newLength );

        if( !indices )
        {
          printf( "Error allocating memory" );
          abort();
        }

        assert( indices );
        OBJModel.indices                        = indices;
        OBJModel.indexLength                    = newLength;
        OBJModel.indices[ OBJModel.indexCount ] = index;
        OBJModel.faces[ i ].indexes[ element ]  = OBJModel.indexCount;
      }
      OBJModel.indexCount++;
    } 
    element++;
    token           = strtok( NULL, " \r\n" );
  }
  //printf( "Post processface index:%d\r\n", i );
  i++;
}

int containsIndex( const Index *array, int arrayLength, const Index *index )
{
  int i;
  for( i = 0; i < arrayLength; i++)
  {
    if( index->vertex == array[ i ].vertex &&
        index->normal == array[ i ].normal &&
        index->texture == array[ i ].texture )
    {
      return i;
    }
  }

  //not found
  return -1;
}

void generateFile( const char* filename )
{ 
  FILE *handle  = fopen( filename, "w" );
  int i;
  int j;



  /////////////// Vertex

  fprintf( handle, "float vertex[] = {\r\n" );
  //Vertices
  for( i = 0; i < OBJModel.indexCount; i++ )
  {
    Index index = OBJModel.indices[ i ]; 
    vertex v    = OBJModel.vertices[ index.vertex ];
    
    //tri
    if( i < OBJModel.indexCount - 1 )
    {
      fprintf( handle, "%f,%f,%f,\r\n", v.x, v.y, v.z);
    }
    else
    {
      fprintf( handle, "%f,%f,%f\r\n", v.x, v.y, v.z);
    }
  }
  fprintf( handle, "};\r\n" );



  /////////////// Texture

  if( OBJModel.numOfObjNormal > 0 )
  {
    fprintf( handle, "float texture[] = {\r\n" );
    //Texture
    for( i = 0; i < OBJModel.indexCount; i++ )
    {
      Index index = OBJModel.indices[ i ]; 
      texture t   = OBJModel.textures[ index.texture ];
      
      //tri
      if( i < OBJModel.indexCount - 1 )
      {
        fprintf( handle, "%f,%f,\r\n", t.u, t.v );
      }
      else
      {
        fprintf( handle, "%f,%f\r\n", t.u, t.v );
      }
    }
    fprintf( handle, "};\r\n" );
  }



  /////////////// Normal

  if( OBJModel.numOfObjNormal > 0 )
  {
    fprintf( handle, "float normal[] = {\r\n" );
    //Normals
    for( i = 0; i < OBJModel.indexCount; i++ )
    {
      Index index = OBJModel.indices[ i ]; 
      normal n    = OBJModel.normals[ index.normal ];
      
      //tri
      if( i < OBJModel.indexCount - 1 )
      {
        fprintf( handle, "%f,%f,%f,\r\n", n.x, n.y, n.z);
      }
      else
      {
        fprintf( handle, "%f,%f,%f\r\n", n.x, n.y, n.z);
      }
    }
    fprintf( handle, "};\r\n" );
  }



  /////////////// Index

  // Change name !!!!!!!
  fprintf( handle, "int index[] = {\r\n" );

  //Index
  int indexCount = 0;
  for( i = 0; i < OBJModel.numOfObjFaces; i++ )
  {
    face f = OBJModel.faces[ i ];
    
    for( j = 0; j < f.count; j++ )
    {
      int faceIndex = f.indexes[ j ];

      if( j > 2 )
      {
        fprintf( handle, "\r\n" );
        int indexA      = f.indexes[ 0 ];
        int indexB      = f.indexes[ j - 1 ];

        if( j < f.count - 1 )
        {
          fprintf( handle, "%d,%d,%d,\r\n", indexB, faceIndex, indexA );
          indexCount += 3;
        }
        else
        {
          if( i < OBJModel.numOfObjFaces - 1 )
          {
            fprintf( handle, "%d,%d,%d,\r\n", indexB, faceIndex, indexA );
            indexCount += 3;
          }
          else
          {
            fprintf( handle, "%d,%d,%d\r\n", indexB, faceIndex, indexA );
            indexCount += 3;
          }
        }
      }
      else
      {
        if( j < f.count - 1 )
        {
          fprintf( handle, "%d,", faceIndex );
          indexCount++;
        }
        else
        {
          if( i < OBJModel.numOfObjFaces - 2 )
          {
            fprintf( handle, "%d,", faceIndex );
            indexCount++;
          }
          else
          {
            fprintf( handle, "%d", faceIndex );
            indexCount++;
          }
          fprintf( handle, "\r\n" );
        }
      }
    }
  }
  fprintf( handle, "};\r\n" );


  fprintf( handle, "\r\n" );
  fprintf( handle, "float min[] = { %f,%f,%f };\r\n", OBJModel.min.x, OBJModel.min.y, OBJModel.min.z );
  fprintf( handle, "float max[] = { %f,%f,%f };\r\n", OBJModel.max.x, OBJModel.max.y, OBJModel.max.z);
  fprintf( handle, "int numOfVertex = %d;\r\n", OBJModel.indexCount * 3 );
  fprintf( handle, "int numOfNormal = %d;\r\n", OBJModel.indexCount * 3 );
  fprintf( handle, "int numOfTexture = %d;\r\n", OBJModel.indexCount * 2 );
  fprintf( handle, "int numOfIndex = %d;\r\n", indexCount );

  
/*
  //Texture
  for( i = 0; i < OBJModel.numOfObjFaces; i++ )
  {
    face f = OBJModel.faces[ i ];
    
    for( j = 0; j < f.count; j++ )
    {
      printf( "%d ", f.indexes[ j ] );
    }
    printf( "\r\n" );
  }

  //Normals
  for( i = 0; i < OBJModel.numOfObjFaces; i++ )
  {
    face f = OBJModel.faces[ i ];
    
    for( j = 0; j < f.count; j++ )
    {
      printf( "%d ", f.indexes[ j ] );
    }
    printf( "\r\n" );
  }
*/
  //printf( "Min < %f,%f,%f >\r\n", OBJModel.min.x, OBJModel.min.y, OBJModel.min.z );
  //printf( "Max < %f,%f,%f >\r\n", OBJModel.max.x, OBJModel.max.y, OBJModel.max.z);
}
