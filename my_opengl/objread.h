#define BUF_SIZE 1024

typedef struct {
  float p[3];
  float c[3];
} t_vertex;

typedef struct {
  float n[3];
} t_vnormal;

typedef struct {
  float t[2];
} t_vtexture;

typedef struct {
  int p[3];
  int n[3];
  int t[3];
} t_face;

typedef struct {
  char material_name[BUF_SIZE];
  char texture_fname[BUF_SIZE];
  int tex_width, tex_height, tex_channel;
  unsigned char *txbuf;
  unsigned int tex_id;
  float Kd[4];
  float Ks[4];
  float Ka[4];
  float Ke[4];
  float Ns;
  int alpha_test;// 1 : if all the alpha values of the texture are 1 or 0.
                 // 0 : otherwise. Blending occurs.
} t_material;

typedef struct {
  int nface;
  t_face *f;
  t_material material;
} t_facegroup;

typedef struct {
  char group_name[BUF_SIZE];
  int nv, nvn, nvt;
  int nfacegroup;
  t_facegroup *fg;
} t_group;

typedef struct {
  int ngroup;
  t_group *group;
  int nvertex, nvnormal, nvtexture;
  t_vertex *v;
  t_vnormal *vn;
  t_vtexture *vt;
} t_obj;

void read_obj(const char *fname, t_obj *obj);
void print_obj(t_obj obj);
void write_obj(t_obj obj, const char *ofname, float scale, float sx, float sy, float sz);
void generate_normal(t_obj *obj);
