#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <SFMT.h>

extern void sfmt_init_gen_rand(sfmt_t * sfmt, uint32_t seed);
extern double sfmt_genrand_real2(sfmt_t * sfmt);

typedef double Neuron, Weight;
typedef struct { Weight *w; int n_pre; int n_post; } Connection;
typedef struct { Neuron *z; int n; } Layer;
typedef struct { Layer *layer; Connection *connection; sfmt_t rng; int n; } Network;

double all_to_all ( Network *n, const int i, const int j ) { return 1.; }
double sigmoid ( double x ) { return 1. / ( 1. + exp ( - x ) ); }

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
  for ( int i = 0; i < layer -> n; i++) { layer -> z [ i ] = 0.; }

  if ( bias ) { layer -> z [ layer -> n ] = +1.; } // バイアス初期化
}

void deleteLayer ( Network *network, const int layer_id )
{
  Layer *layer = &network -> layer [ layer_id ];
  free ( layer -> z );
}

void createConnection ( Network *network, const int layer_id, double ( *func ) ( Network *, const int, const int ) )
{
  Connection *connection = &network -> connection [ layer_id ];

  const int n_pre = network -> layer [ layer_id ] . n + 1; // +1はバイアスの分
  const int n_post = ( layer_id == network -> n - 1 ) ? 1 : network -> layer [ layer_id + 1 ] . n;

  connection -> w = ( Weight * ) malloc ( n_pre * n_post * sizeof ( Weight ) );

  for ( int i = 0; i < n_post; i++ ) {
    for ( int j = 0; j < n_pre; j++ ) {
      connection -> w [ j + n_pre * i ] = func ( network, i, j );
    }
  }
  connection -> n_pre = n_pre;
  connection -> n_post = n_post;
}

void deleteConnection ( Network *network, const int layer_id )
{
  Connection *connection = &network -> connection [ layer_id ];
  free ( connection -> w );
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

int main ( void )
{
  sfmt_t rng;
  sfmt_init_gen_rand ( &rng, getpid ( ) );

  // 作る
  Network network;
  createNetwork ( &network, 3, rng );
  createLayer ( &network, 0, 2 );
  createLayer ( &network, 1, 2 );
  createLayer ( &network, 2, 1 );
  createConnection ( &network, 0, all_to_all );
  createConnection ( &network, 1, all_to_all );

  // 試す
  Neuron x [ ] = { 0., 1. };
  setInput ( &network, x );
  forwardPropagation ( &network, sigmoid );
  // dump ( &network );

  // 消す
  deleteConnection ( &network, 1 );
  deleteConnection ( &network, 0 );
  deleteLayer ( &network, 2 );
  deleteLayer ( &network, 1 );
  deleteLayer ( &network, 0 );
  deleteNetwork ( &network );
}
