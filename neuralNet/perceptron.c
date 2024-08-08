#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <SFMT.h>

extern void sfmt_init_gen_rand(sfmt_t * sfmt, uint32_t seed);
extern double sfmt_genrand_real2(sfmt_t * sfmt);

typedef double Neuron, Weight;
typedef struct { Weight *w; Weight *dw; int n_pre; int n_post; } Connection;
typedef struct { Neuron *z; int n; } Layer;
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

double updateByPerceptronRule ( Network *network, Neuron z [ ], double ( *diff_activation ) ( double ) )
{
  const double Eta = 0.1;

  double error = 0.;
  {
    Layer *l = &network -> layer [ network -> n - 1 ];
    for ( int j = 0; j < l -> n; j++ ) {
      error += 0.5 * ( ( l -> z [ j ] - z [ j ] ) * ( l -> z [ j ] - z [ j ] ) );
    }
  }

  Layer *output_layer = &network -> layer [ network -> n - 1 ];
  Layer *hidden_layer = &network -> layer [ network -> n - 2 ];
  Connection *c = &network -> connection [ network -> n - 2 ];
  for ( int i = 0; i < c -> n_post; i++ ) {
    for ( int j = 0; j < c -> n_pre; j++ ) {
      double o = output_layer -> z [ i ];
      double d = ( z [ i ]  - o ) * diff_activation ( o );
      c -> dw [ j + c -> n_pre * i ] += Eta * d * ( hidden_layer -> z [ j ] );
    }
  }

  return error;
}

void initializeDW ( Network *network )
{
  Connection *c = &network -> connection [ network -> n - 2 ];
  for ( int i = 0; i < c -> n_post; i++ ) {
    for ( int j = 0; j < c -> n_pre; j++ ) {
      c -> dw [ j + c -> n_pre * i ] = 0.;
    }
  }
}

void updateW ( Network *network )
{
  Connection *c = &network -> connection [ network -> n - 2 ];
  for ( int i = 0; i < c -> n_post; i++ ) {
    for ( int j = 0; j < c -> n_pre; j++ ) {
      c -> w [ j + c -> n_pre * i ] += c -> dw [ j + c -> n_pre * i ];
    }
  }
}

void print_layer(Layer *layer, int n, int bias){
    Layer *current_layer = layer+n;

    printf("Layer:%d(%d)  ",n,(bias?current_layer->n+1:current_layer->n));
    for(int i=0; i<current_layer->n; i++){
        printf("%2.2f ",(current_layer->z)[i]);
    }

    if(bias)printf("%2.2f ",(current_layer->z)[current_layer->n]);
    printf("\n");
}

void print_connection(Connection *c){
    for(int i=0; i<c->n_post; i++){
        printf("              ");

        for(int j=0; j<c->n_pre; j++){
            printf("%2.2f␣",c->w[j+c->n_pre*i]);
        }

        printf("\n");
    }
}

void dump(Network *network){
    printf("\nnumber of layers: %d\n",network->n);
    printf("-------------------\n");

    for(int i=0; i<(network->n)-1; i++){
        print_layer((network->layer),i,1);
        printf("\n");
        print_connection((network->connection)+i);
        printf("-------------------\n");
    }

    print_layer(network->layer,(network->n)-1,0);
    printf("-------------------\n");
}

int main ( void )
{
  sfmt_t rng;
  sfmt_init_gen_rand ( &rng, getpid ( ) );

  Network network;
  createNetwork ( &network, 3, rng );
  createLayer ( &network, 0, 2 );
  createLayer ( &network, 1, 128 );
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
      error += updateByPerceptronRule ( &network, z [ k ], diff_sigmoid );
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
