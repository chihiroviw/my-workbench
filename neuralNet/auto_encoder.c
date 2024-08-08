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

double updateByBackPropagationPartial 
    ( Network *network, Neuron z [ ], double ( *diff_activation ) (double ) ){
    const double Eta = 0.1;

    double error = 0.;
    {
    Layer *l = &network -> layer [ network -> n - 1 ];
    for ( int j = 0; j < l -> n; j++ ) {
        error += 0.5 * ( ( l -> z [ j ] - z [ j ] ) * ( l -> z [ j ] - z [ j ] ) );
    }
    }

    //
    {
    int i= network->n-1;
    Layer *l = &network->layer[i];
    for(int j=0; j<l->n; j++){
        Neuron o = l->z[j];
        l->delta[j] = z[j]-o;
    }
    }

    for(int i=network->n-2; network->n-3<=i; i--){
        Layer *l = &network->layer[i];
        Connection *c = &network->connection[i];
        Layer *l_next = &network->layer[i+1];

        for(int j=0; j<c->n_post; j++){
            for(int k=0; k<c->n_pre; k++){
                c->dw[k+c->n_pre*j] += Eta *(l_next->delta[j]) *diff_activation(l_next->z[j]) *(l->z[k]);
            }
        }

        for(int k=0; k<c->n_pre; k++) l->delta[k]=0.0;

        for(int j=0; j<c->n_pre; j++){
            for(int k=0; k<c->n_post; k++){
                l->delta[j] += l_next->delta[k] *diff_activation(l_next->z[k]) *c->w[j+c->n_pre*k];
            }
        }

    }

    return error;
}

void copyConnection 
    ( Network *network_src, int layer_id_src, Network *network_dst, int layer_id_dst ){

    Connection *c_s = &network_src->connection[layer_id_src];
    Connection *c_d = &network_dst->connection[layer_id_dst];

    for(int i=0; i<c_s->n_post*c_s->n_pre; i++){
        c_d->w[i]=c_s->w[i];
    }

    return ;
}

void copyConnectionWithTranspose ( Network *network_src, int layer_id_src, Network *
    network_dst, int layer_id_dst ){
    Connection *c_s = &network_src->connection[layer_id_src];
    Connection *c_d = &network_dst->connection[layer_id_dst];

    for(int i=0; i<c_s->n_post; i++){
        for(int j=0; j<c_s->n_pre-1; j++){
            c_d->w[i + j*(c_d->n_pre)]= c_s->w[j + i*(c_s->n_pre)];
        }
    }

    return ;
}

void deleteAllNetwork(Network *network){
    int layer_num = network->n;
    int connection_num = layer_num-1;

    for(int i=0; i<connection_num; i++) deleteConnection(network,i);

    for(int i=0; i<layer_num; i++) deleteLayer(network,i);

    deleteNetwork(network);

    return ;
}

void createAllNetwork(Network *network, int *layer_element_count, int num_of_layers, sfmt_t rng){
    createNetwork(network,num_of_layers,rng);

    for(int i=0; i<num_of_layers; i++) createLayer(network,i,layer_element_count[i]);

    for(int i=0; i<num_of_layers-1; i++){
        if(i==num_of_layers-2)createConnection(network,i,uniform_random);
        else createConnection(network,i,sparse_random);}

    return ;
}


int main ( void ){

    sfmt_t rng;
    sfmt_init_gen_rand ( &rng, getpid ( ) );

    double **training_image, **test_image;
    int *training_label, *test_label;
    mnist_initialize ( &training_image, &training_label, &test_image, &test_label );


    //create network1
    Network network1;
    int le[16];
    le[0] = MNIST_IMAGE_SIZE;
    le[1] = 64;
    le[2] = MNIST_IMAGE_SIZE;
    createAllNetwork(&network1,le,3,rng);
    copyConnectionWithTranspose ( &network1, 0, &network1, 1 ); //tied weight

    // Training
    double error = 1.0; // arbitrary large number
    int MINI_BATCH_SIZE=50;
    int EPOCH=10;

    for(int e=0; e<EPOCH; e++){
        for(int i=0; i<MNIST_TRAINING_DATA_SIZE/MINI_BATCH_SIZE; i++){
            error = 0.;
            initializeDW ( &network1 );
            copyConnectionWithTranspose ( &network1, 0, &network1, 1 );

            for(int j=0; j<MINI_BATCH_SIZE; j++){
                int k=(int)(MNIST_TRAINING_DATA_SIZE*sfmt_genrand_real2(&rng));
                setInput(&network1, training_image[k]);
                forwardPropagation(&network1,sigmoid);
                error += updateByBackPropagationPartial(&network1, network1.layer[0].z, diff_sigmoid);
            }

            updateW ( &network1 );
        }

        printf("epoch:%d\n",e+1);
    }


    //create network2
    Network network2;
    le[0] = MNIST_IMAGE_SIZE;
    le[1] = 64 ,le[2] = 32, le[3] = 64;
    createAllNetwork(&network2,le,4,rng);
    copyConnection ( &network1, 0, &network2, 0 );
    copyConnectionWithTranspose ( &network2, 1, &network2, 2 ); // tied weight
    //delete network1
    deleteAllNetwork(&network1);

    //training
    for(int e=0; e<EPOCH; e++){
        for(int i=0; i<MNIST_TRAINING_DATA_SIZE/MINI_BATCH_SIZE; i++){
            error = 0.;
            initializeDW ( &network2 );
            copyConnectionWithTranspose ( &network2, 1, &network2, 2 );

            for(int j=0; j<MINI_BATCH_SIZE; j++){
                int k=(int)(MNIST_TRAINING_DATA_SIZE*sfmt_genrand_real2(&rng));
                setInput(&network2, training_image[k]);
                forwardPropagation(&network2,sigmoid);
                error += updateByBackPropagationPartial(&network2, network2.layer[1].z, diff_sigmoid);
            }

            updateW ( &network2 );
        }

        printf("epoch:%d\n",e+1);
    }


    //create network last
    Network network_last;
    le[0] = MNIST_IMAGE_SIZE;
    le[1] = 64 ,le[2] = 32 ,le[3]=MNIST_LABEL_SIZE;
    createAllNetwork(&network_last,le,4,rng);
    copyConnection ( &network2, 0, &network_last, 0 );
    copyConnection ( &network2, 1, &network_last, 1 );
    //delete network2
    deleteAllNetwork(&network2);

    //training
    EPOCH = 8;
    MINI_BATCH_SIZE = 20;
    //
    for(int e=0; e<EPOCH; e++){
        for(int i=0; i<MNIST_TRAINING_DATA_SIZE/MINI_BATCH_SIZE; i++){
            error = 0.;
            initializeDW ( &network_last );

            for(int j=0; j<MINI_BATCH_SIZE; j++){
                int k=(int)(MNIST_TRAINING_DATA_SIZE*sfmt_genrand_real2(&rng));
                double z[MNIST_LABEL_SIZE] = {0.};
                z[training_label[k]] = 1.;

                setInput(&network_last, training_image[k]);
                forwardPropagation(&network_last,sigmoid);
                error += updateByBackPropagation(&network_last, z, diff_sigmoid);
            }

            updateW ( &network_last );
        }

        printf("epoch:%d\n",e+1);
    }



    // Evaluatation
    {
        Layer *output_layer = &network_last . layer [ network_last . n - 1 ];
        const int n = output_layer -> n;
        int correct = 0;
        for ( int i = 0; i < MNIST_TEST_DATA_SIZE; i++ ) {
            setInput ( &network_last, test_image [ i ] );
            forwardPropagation ( &network_last, sigmoid );
            int maxj = 0;
            double maxz = 0.;

            for ( int j = 0; j < n; j++ ) {
                if ( output_layer -> z [ j ] > maxz ) { maxz = output_layer -> z [ j ]; maxj = j; }
            }

            correct += ( maxj == test_label [ i ] );
        }

        fprintf ( stderr, "success_rate = %f\n", ( double ) correct / MNIST_TEST_DATA_SIZE );
    }


    mnist_finalize ( training_image, training_label, test_image, test_label );

    return 0;
}


