
#ifndef STRUCT_H
#define STRUCT_H

// This header is used to define 
// all structs and functions
// used by the program
typedef struct {
  float x;
  float y;
  float z;
} vertex;

typedef vertex normal;

typedef struct {
  float u;
  float v;
} texture;

typedef struct{
  int vertex;
  int normal;
  int texture;
} Index;

typedef struct{
  int count;
  int *indexes;
} face;

typedef struct{
  int numOfObjVertex;
  int numOfObjNormal;
  int numOfObjTex; 
  int numOfObjFaces;
  int numOfTriangle;
  int indexLength;
  int indexCount;
  vertex* vertices;
  normal* normals;
  texture* textures;
  Index* indices;
  face* faces; 
  vertex min;
  vertex max;
} model;

typedef struct {
  char *file;
  char *outputFile;
  char *outputFileType;
  int verbose;
  int normals;
  int texture;
  int flatArray;  
} options;

//All global variables used
model OBJModel;

void printHelp( void );
int processArgs( int argc, char*argv[], options *o );
void generate( options *o);
void generateFile( const char* filename );
void cleanUp( void ); 
void init( void ); 
void count( char *line );
void processLine( const char *line );
void processVertex( const char *line);
void processNormal( const char *line);
void processTexture( const char *line);
void processFace( const char *line); 
int containsIndex( const Index *array, int arrayLength, const Index *index );

#endif
