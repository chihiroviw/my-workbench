#include "mnist.h"

#define MNIST_DEBUG 0
#define MNIST_DEBUG2 0
#define MNIST_DEBUG3 0

static void mnist_read_image_file ( const char *filename, double ***data )
{
  FILE *file = fopen ( filename, "rb" );

  if ( file == NULL ) { fprintf ( stderr, "err fopen %s\n", filename ); exit ( 1 ); }
  
  { // get the magic number
    unsigned char buf [ 4 ];
    fread ( buf, sizeof ( unsigned char ), 4, file );
    int magic = buf [ 3 ] + 256 *( buf [ 2 ] + 256 *( buf [ 1 ] + 256 *( buf [ 0 ] )));
    if ( MNIST_DEBUG ) { printf ( "%d\n", magic ); }
    if ( MNIST_IMAGE_FILE_MAGIC != magic ) { fprintf ( stderr, "err magic\n" ); exit ( 1 ); }
  }

  int number_of_items;
  { // get the number of items
    unsigned char buf [ 4 ];
    fread ( buf, sizeof ( unsigned char ), 4, file );
    number_of_items = buf [ 3 ] + 256 *( buf [ 2 ] + 256 *( buf [ 1 ] + 256 *( buf [ 0 ] )));
    if ( MNIST_DEBUG ) { printf ( "%d\n", number_of_items ); }
  }

  { // skip 2 integers for row and column sizes
    unsigned char buf [ 8 ];
    fread ( buf, sizeof ( unsigned char ), 8, file );
  }

  { // read the data
    *data = ( double ** ) malloc ( number_of_items * sizeof ( double * ) );
    for ( int i = 0; i < number_of_items; i++ ) {
      (*data) [ i ] = ( double * ) malloc ( MNIST_IMAGE_SIZE * sizeof ( double ) );
    }

    for ( int i = 0; i < number_of_items; i++ ) {
      for ( int j = 0; j < MNIST_IMAGE_SIZE; j++ ) {
	unsigned char buf;
	fread ( &buf, sizeof ( unsigned char ), 1, file );
	(*data) [ i ][ j ] = buf / 255.0;
	if ( MNIST_DEBUG && MNIST_DEBUG2 ) { printf ( "%f\n", ( *data ) [ i ][ j ] ); }
      }
    }
  }
  
  fclose ( file );
}

static void mnist_read_label_file ( const char *filename, int **data )
{
  FILE *file = fopen ( filename, "rb" );

  if ( file == NULL ) { fprintf ( stderr, "err fopen %s\n", filename ); exit ( 1 ); }
  
  { // get the magic number
    unsigned char buf [ 4 ];
    fread ( buf, sizeof ( unsigned char ), 4, file );
    int magic = buf [ 3 ] + 256 *( buf [ 2 ] + 256 *( buf [ 1 ] + 256 *( buf [ 0 ] )));
    if ( MNIST_DEBUG ) { printf ( "%d\n", magic ); }
    if ( MNIST_LABEL_FILE_MAGIC != magic ) { fprintf ( stderr, "err magic\n" ); exit ( 1 ); }
  }

  int number_of_items;
  { // get the number of items
    unsigned char buf [ 4 ];
    fread ( buf, sizeof ( unsigned char ), 4, file );
    number_of_items = buf [ 3 ] + 256 *( buf [ 2 ] + 256 *( buf [ 1 ] + 256 *( buf [ 0 ] )));
    if ( MNIST_DEBUG ) { printf ( "%d\n", number_of_items ); }
  }

  { // read the data
    *data = ( int * ) malloc ( number_of_items * sizeof ( int ) );

    for ( int i = 0; i < number_of_items; i++ ) {
      unsigned char buf;
      fread ( &buf, sizeof ( unsigned char ), 1, file );
      (*data) [ i ] = buf;
      if ( MNIST_DEBUG && MNIST_DEBUG2 ) { printf ( "%d\n", ( *data ) [ i ] ); }
    }
  }
  
  fclose ( file );
}

void mnist_initialize ( double ***training_image, int **training_label, double ***test_image, int **test_label )
{
  mnist_read_image_file ( MNIST_TRAINING_IMAGE_FILE, training_image );
  mnist_read_label_file ( MNIST_TRAINING_LABEL_FILE, training_label );
  mnist_read_image_file ( MNIST_TEST_IMAGE_FILE, test_image );
  mnist_read_label_file ( MNIST_TEST_LABEL_FILE, test_label );
}

void mnist_finalize ( double **training_image, int *training_label, double **test_image, int *test_label )
{
  for ( int i = 0; i < MNIST_TRAINING_DATA_SIZE; i++ ) { free ( training_image [ i ] ); }
  free ( training_image );
  free ( training_label );
  
  for ( int i = 0; i < MNIST_TEST_DATA_SIZE; i++ ) { free ( test_image[ i ] ); }
  free ( test_image );
  free ( test_label );
}

void mnist_generate_png ( double **data, const int n, const char *filename )
{
  gdImagePtr im = gdImageCreate ( MNIST_IMAGE_ROW_SIZE, MNIST_IMAGE_COL_SIZE );

  const int n_grayscale = 256;
  int gray [ n_grayscale ];
  for ( int i = 0; i < n_grayscale; i++ ) { gray [ i ] = gdImageColorAllocate ( im, i, i, i ); }

  for ( int i = 0; i < MNIST_IMAGE_ROW_SIZE; i++ ) {
    for ( int j = 0; j < MNIST_IMAGE_COL_SIZE; j++ ) {
      int index = ( int ) ( ( n_grayscale - 1 ) * data [ n ][ j + MNIST_IMAGE_COL_SIZE * i ] );
      if ( MNIST_DEBUG && MNIST_DEBUG3 ) { printf ( "%d ", index ); }
      gdImageSetPixel ( im, j, i, gray [ index ] );
    }
  }
  
  {
    FILE *file = fopen ( filename, "wb" );
    gdImagePng ( im, file );
    fclose ( file );
  }

  gdImageDestroy ( im );
  
  return;
}

int local_main(void)
{
  double **training_image, **test_image;
  int *training_label, *test_label;

  mnist_initialize ( &training_image, &training_label, &test_image, &test_label );

  {  // Demo: generate 60 png files while printing corresponding labels
    for ( int i = 0; i < MNIST_TRAINING_DATA_SIZE; i += 1000 ) {
      char fn [ 1024 ];
      sprintf ( fn, "./png/%d.png", i );
      mnist_generate_png ( training_image, i, fn );
      printf ( "%d\n", training_label [ i ] );
    }
  }
  
  mnist_finalize ( training_image, training_label, test_image, test_label );

  return 0;
}

#if 0
int main(void)
{
  return local_main();
}
#endif
