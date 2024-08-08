#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <SFMT.h>
#include "mnist.h"

extern void sfmt_init_gen_rand(sfmt_t * sfmt, uint32_t seed);
extern double sfmt_genrand_real2(sfmt_t * sfmt);

typedef double Neuron, Delta, Weight;
typedef struct { Weight *w; Weight *dw; int n_pre; int n_post; } Connection;
typedef struct { Neuron *z; Delta *delta; int n; } Layer;
typedef struct { Layer *layer; Connection *connection; sfmt_t rng; int n; } Network;

double all_to_all ( Network *n, const int i, const int j ) { return 1.; }
double uniform_random ( Network *n, const int i, const int j ) { return 1. - 2. * sfmt_genrand_real2 ( &n -> rng ); }
double sparse_random ( Network *n, const int i, const int j )
{
  return ( sfmt_genrand_real2 ( &n -> rng ) < 0.5 ) ? uniform_random ( n, i, j ) : 0.;
}
double sigmoid ( double x ) { return 1. / ( 1. + exp ( - x ) ); }
double diff_sigmoid ( double z ) { return z * ( 1. - z ); } // f'(x) = f(x) ( 1 - f(x) ) = z ( 1 - z )

void createNetwork ( Network *network, const int number_of_layers, const sfmt_t rng )
{
  network -> layer = ( Layer * ) malloc ( number_of_layers * sizeof ( Layer ) );
  network -> connection = ( Connection * ) malloc ( number_of_layers * sizeof ( Connection ) );
  network -> n = number_of_layers;
  network -> rng = rng;
}

void deleteNetwork ( Network *network )
{
  free ( network -> layer );
  free ( network -> connection );
}

void createLayer ( Network *network, const int layer_id, const int number_of_neurons )
{
  Layer *layer = &network -> layer [ layer_id ];

  layer -> n = number_of_neurons;

  int bias = ( layer_id < network -> n - 1 ) ? 1 : 0; // 出力層以外はバイアスを用意

  layer -> z = ( Neuron * ) malloc ( ( number_of_neurons + bias ) * sizeof ( Neuron ) );
  for ( int i = 0; i < layer -> n; i++) { layer -> z [ i ] = 0.; } // 初期化
  if ( bias ) { layer -> z [ layer -> n ] = +1.; } // バイアス初期化

  // Deltaを追加
  layer -> delta = ( Delta * ) malloc ( ( number_of_neurons + bias ) * sizeof ( Delta ) );
  for ( int i = 0; i < layer -> n; i++) { layer -> delta [ i ] = 0.; }
  if ( bias ) { layer -> delta [ layer -> n ] = 0.; } // バイアス初期化

}

void deleteLayer ( Network *network, const int layer_id )
{
  Layer *layer = &network -> layer [ layer_id ];
  free ( layer -> z );
  free ( layer -> delta );
}

void createConnection ( Network *network, const int layer_id, double ( *func ) ( Network *, const int, const int ) )
{
  Connection *connection = &network -> connection [ layer_id ];

  const int n_pre = network -> layer [ layer_id ] . n + 1; // +1 for bias
  const int n_post = ( layer_id == network -> n - 1 ) ? 1 : network -> layer [ layer_id + 1 ] . n;

  connection -> w = ( Weight * ) malloc ( n_pre * n_post * sizeof ( Weight ) );
  for ( int i = 0; i < n_post; i++ ) {
    for ( int j = 0; j < n_pre; j++ ) {
      connection -> w [ j + n_pre * i ] = func ( network, i, j );
    }
  }

  connection -> dw = ( Weight * ) malloc ( n_pre * n_post * sizeof ( Weight ) );
  for ( int i = 0; i < n_post; i++ ) {
    for ( int j = 0; j < n_pre; j++ ) {
      connection -> dw [ j + n_pre * i ] = 0.;
    }
  }

  connection -> n_pre = n_pre;
  connection -> n_post = n_post;
}

void deleteConnection ( Network *network, const int layer_id )
{
  Connection *connection = &network -> connection [ layer_id ];
  free ( connection -> w );
  free ( connection -> dw );
}

void setInput ( Network *network, Neuron x [ ] )
{
  Layer *input_layer = &network -> layer [ 0 ];
  for ( int i = 0; i < input_layer -> n; i++ ) {
    input_layer -> z [ i ] = x [ i ];
  }
}

void forwardPropagation ( Network *network, double ( *activation ) ( double ) )
{
  for ( int i = 0; i < network -> n - 1; i++ ) {
    Layer *l_pre = &network -> layer [ i ];
    Layer *l_post = &network -> layer [ i + 1 ];
    Connection *c = &network -> connection [ i ];
    for ( int j = 0; j < c -> n_post; j++ ) {
      Neuron u = 0.;
      for ( int k = 0; k < c -> n_pre; k++ ) {
	u += ( c -> w [ k + c -> n_pre * j ] ) * ( l_pre -> z [ k ] );
      }
      l_post -> z [ j ] = activation ( u );
    }
  }
}

double updateByBackPropagation ( Network *network, Neuron z [ ], double ( *diff_activation ) ( double ) )
{
  const double Eta = 0.1;

  double error = 0.;
  {
    Layer *l = &network -> layer [ network -> n - 1 ];
    for ( int j = 0; j < l -> n; j++ ) {
      error += 0.5 * ( ( l -> z [ j ] - z [ j ] ) * ( l -> z [ j ] - z [ j ] ) );
    }
  }

    {
        int i= network->n-1;
        Layer *l = &network->layer[i];
        for(int j=0; j<l->n; j++){
            Neuron o = l->z[j];
            l->delta[j] = z[j]-o;
        }
    }

    for(int i= network->n-2; 0<=i; i--){
        Layer *l = &network->layer[i];
        Connection *c = &network->connection[i];
        Layer *l_next = &network->layer[i+1];

        for(int j=0; j<c->n_post; j++){
            for(int k=0; k<c->n_pre; k++){
                c->dw[k+c->n_pre*j] += Eta
                *(l_next->delta[j])
                *diff_activation(l_next->z[j])
                *(l->z[k]);
            }
        }

        for(int k=0; k<c->n_pre; k++) l->delta[k]=0.0;

        for(int j=0; j<c->n_pre; j++){
            for(int k=0; k<c->n_post; k++){
                l->delta[j] += l_next->delta[k]
                *diff_activation(l_next->z[k])
                *c->w[j+c->n_pre*k];
            }
        }
    }

    return error;
}

void initializeDW ( Network *network )
{
  for ( int layer_id = 0; layer_id < network -> n - 1; layer_id++ ) {
    Connection *c = &network -> connection [ layer_id ];
    for ( int i = 0; i < c -> n_post; i++ ) {
      for ( int j = 0; j < c -> n_pre; j++ ) {
	c -> dw [ j + c -> n_pre * i ] = 0.;
      }
    }
  }
}

void updateW ( Network *network )
{
  for ( int layer_id = 0; layer_id < network -> n - 1; layer_id++ ) {
    Connection *c = &network -> connection [ layer_id ];
    for ( int i = 0; i < c -> n_post; i++ ) {
      for ( int j = 0; j < c -> n_pre; j++ ) {
	c -> w [ j + c -> n_pre * i ] += c -> dw [ j + c -> n_pre * i ];
      }
    }
  }
}

int main ( void ){

    sfmt_t rng;
    sfmt_init_gen_rand ( &rng, getpid ( ) );

    double **training_image, **test_image;
    int *training_label, *test_label;
    mnist_initialize ( &training_image, &training_label, &test_image, &test_label );

    Network network;
    createNetwork ( &network, 3, rng );
    createLayer ( &network, 0, MNIST_IMAGE_SIZE );
    createLayer ( &network, 1, 32 );
    //createLayer ( &network, 2, 32 );
    createLayer ( &network, 2, MNIST_LABEL_SIZE );

    createConnection ( &network, 0, sparse_random );
    //createConnection ( &network, 1, sparse_random );
    createConnection ( &network, 1, uniform_random );

    // Training
    double error = 1.0; // arbitrary large number
    int MINI_BATCH_SIZE=20;
    int EPOCH=8;

    for(int e=0; e<EPOCH; e++){
        for(int i=0; i<MNIST_TRAINING_DATA_SIZE/MINI_BATCH_SIZE; i++){

            error = 0.;
            initializeDW ( &network );

            for(int j=0; j<MINI_BATCH_SIZE; j++){
                int k=(int)(MNIST_TRAINING_DATA_SIZE*sfmt_genrand_real2(&rng));
                double z[MNIST_LABEL_SIZE] = {0.};
                z[training_label[k]] = 1.;

                setInput(&network, training_image[k]);
                forwardPropagation(&network,sigmoid);
                error += updateByBackPropagation(&network, z, diff_sigmoid);
            }

            updateW ( &network );
            //printf ( "%d %f\n", i, error );
        }

        printf("epoch:%d\n",e+1);
    }

    //dump(&network);


    // Evaluatation
    {
        Layer *output_layer = &network . layer [ network . n - 1 ];
        const int n = output_layer -> n;
        int correct = 0;
        for ( int i = 0; i < MNIST_TEST_DATA_SIZE; i++ ) {
            setInput ( &network, test_image [ i ] );
            forwardPropagation ( &network, sigmoid );
            int maxj = 0;
            double maxz = 0.;

            for ( int j = 0; j < n; j++ ) {
                if ( output_layer -> z [ j ] > maxz ) { maxz = output_layer -> z [ j ]; maxj = j; }
            }

            correct += ( maxj == test_label [ i ] );
        }

        fprintf ( stderr, "success_rate = %f\n", ( double ) correct / MNIST_TEST_DATA_SIZE );
    }

    deleteConnection ( &network, 1 );
    deleteConnection ( &network, 0 );
    deleteLayer ( &network, 2 );
    deleteLayer ( &network, 1 );
    deleteLayer ( &network, 0 );
    deleteNetwork ( &network );

    mnist_finalize ( training_image, training_label, test_image, test_label );

    return 0;
}

/*
int main ( void )
{
  sfmt_t rng;
  sfmt_init_gen_rand ( &rng, getpid ( ) );

  Network network;
  createNetwork ( &network, 3, rng );
  createLayer ( &network, 0, 2 );
  createLayer ( &network, 1, 2 );
  createLayer ( &network, 2, 1 );
  createConnection ( &network, 0, sparse_random );
  createConnection ( &network, 1, uniform_random );

  Neuron x [ 4 ][ 2 ] = { { 0., 0. }, { 0., 1. }, { 1., 0. }, { 1., 1. } };
  Neuron z [ 4 ][ 1 ] = { { 0. } , { 1. } , { 1. } , { 0.} };
  const int number_of_training_data = 4;

  // Training
  double error = 1.0; // arbitrary large number
  const double Epsilon = 0.001; // tolerant error rate
  int i = 0;
  while ( error > Epsilon ) {
    error = 0.;
    initializeDW ( &network );
    for ( int j = 0; j < number_of_training_data; j++ ) {
      //int k = ( int ) ( number_of_training_data * sfmt_genrand_real2 ( &rng ) );
      int k = j;
      setInput ( &network, x [ k ] );
      forwardPropagation ( &network, sigmoid );
      error += updateByBackPropagation ( &network, z [ k ], diff_sigmoid );
    }
    updateW ( &network );
    printf ( "%d %f\n", i, error );
    i++;
  }
  fprintf ( stderr, "# of epochs = %d\n", i );

  // Test
  Layer *output_layer = &network . layer [ network. n - 1 ];
  const int n = output_layer -> n;
  for ( int i = 0; i < number_of_training_data; i++ ) {
    setInput ( &network, x [ i ] );
    forwardPropagation ( &network, sigmoid );
    //dump ( &network );
    for ( int j = 0; j < n; j++ ) {
      fprintf ( stderr, "%f%s", output_layer -> z [ j ], ( j == n - 1 ) ? "\n" : " " );
    }
  }

  deleteConnection ( &network, 1 );
  deleteConnection ( &network, 0 );
  deleteLayer ( &network, 2 );
  deleteLayer ( &network, 1 );
  deleteLayer ( &network, 0 );
  deleteNetwork ( &network );
}
*/
