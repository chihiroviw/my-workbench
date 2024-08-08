#include "Scene.h"
#include "vec3.h"
#include "uvec3.h"

#include <GL/freeglut.h>
#include <fstream>
#include <sstream>

#include <cmath> 

using namespace std;

// added by T. Narumi
#include "objread.c"
//static t_group *group;
//static int ngroup;
static t_obj Obj;
static const char *scene_fname_obj=NULL, *scene_fname_mtl=NULL;
static float scene_scale;
static char scene_command;
static int scene_iterations;
void Scene::set_output_fname(const char *fname_obj, const char *fname_mtl, float scale){
  scene_fname_obj = fname_obj;
  scene_fname_mtl = fname_mtl;
  scene_scale = scale;
}
void Scene::set_command_iterations(char com, int ite){
  scene_command = com;
  scene_iterations = ite;
}
void Scene::output_obj(void){
  FILE *fpo,*fpm;
  int nmaterial=0,ngroup=Obj.ngroup;
  t_group *group=Obj.group;
  int nvertex,nvnormal,nvtexture;
  nvertex=nvnormal=nvtexture=0;
  if(scene_fname_obj!=NULL){
    if((fpo=fopen(scene_fname_obj,"w"))==NULL){
      fprintf(stderr,"** error : can't open %s for output **\n",scene_fname_obj);
      exit(1);
    }
    if((fpm=fopen(scene_fname_mtl,"w"))==NULL){
      fprintf(stderr,"** error : can't open %s for output **\n",scene_fname_mtl);
      exit(1);
    }
    
    fprintf(fpo,"mtllib %s\n",scene_fname_mtl);
    for(int g=0;g<ngroup;g++){
      fprintf(fpo,"g %s\n",group[g].group_name);
      for(int v=1; v<=group[g].nv; v++){
	nvertex++;
	fprintf(fpo,"v %f %f %f\n",
		Obj.v[nvertex].p[0],
		Obj.v[nvertex].p[1],
		Obj.v[nvertex].p[2]);
      }
      for(int v=1; v<=group[g].nvn; v++){
	nvnormal++;
	fprintf(fpo,"vn %f %f %f\n",
		Obj.vn[nvnormal].n[0],
		Obj.vn[nvnormal].n[1],
		Obj.vn[nvnormal].n[2]);
      }
      for(int v=1; v<=group[g].nvt; v++){
	nvtexture++;
	fprintf(fpo,"vt %f %f\n",
		Obj.vt[nvtexture].t[0],
		Obj.vt[nvtexture].t[1]);
      }
      for(int fg=0; fg<group[g].nfacegroup; fg++){
	t_material *material=&group[g].fg[fg].material;
	for(int f=0; f<group[g].fg[fg].nface; f++){
	  t_face *tf=&group[g].fg[fg].f[f];
	  fprintf(fpo,"usemtl %s_%d\n",material->material_name,nmaterial);
	  fprintf(fpo,"f %d %d %d\n",tf->p[0],tf->p[1],tf->p[2]);
	  fprintf(fpm,"newmtl %s_%d\n",material->material_name,nmaterial);
	  fprintf(fpm,"Kd %f %f %f\n",radiosities[nmaterial].x,radiosities[nmaterial].y,radiosities[nmaterial].z);
	  fprintf(fpm,"Ka %f %f %f\n",material->Ka[0],material->Ka[1],material->Ka[2]);
	  fprintf(fpm,"illum 2\n\n");
	  nmaterial++;
	}
      }
    }
    fclose(fpo);
    fclose(fpm);
  }
}

void Scene::loadFromOBJFile(const char* filename) {
    vertices.clear();
    faces.clear();
    faceNormals.clear();
    faceEmissions.clear();
    faceReflectivities.clear();
    formFactors.clear();

    VtoF.clear();

    // added by T.Narumi
#if 1
    t_group *group;
    int ngroup;
    read_obj(filename,&Obj);
    ngroup=Obj.ngroup;
    group=Obj.group;
    // set vertex color 
    for(int g=0;g<ngroup;g++){
      for(int fg=0; fg<group[g].nfacegroup; fg++){
	t_vertex *tv=Obj.v;
	t_material *material=&group[g].fg[fg].material;
	for(int f=0; f<group[g].fg[fg].nface; f++){
	  t_face *tf=&group[g].fg[fg].f[f];
	  for(int xyz=0; xyz<3; xyz++){
	    tv[ tf->p[0] ].c[xyz] = material->Kd[xyz];
	    tv[ tf->p[1] ].c[xyz] = material->Kd[xyz];
	    tv[ tf->p[2] ].c[xyz] = material->Kd[xyz];
	  }
	}
      }
    }
    // copy vertex coordinate and color
    for(int i=1;i<=Obj.nvertex;i++){
      vec3 v3=vec3(Obj.v[i].p[0], Obj.v[i].p[1], Obj.v[i].p[2]);
      vec3 kd=vec3(Obj.v[i].c[0], Obj.v[i].c[1], Obj.v[i].c[2]);
      vertices.push_back(v3);
      vertexColors.push_back(kd);
    }
    // set vertex index of faces
    int fi=0;
    for(int g=0;g<ngroup;g++){
      for(int fg=0; fg<group[g].nfacegroup; fg++){
	//	t_vertex *tv=group[g].v;
	t_material *material=&group[g].fg[fg].material;
	for(int f=0; f<group[g].fg[fg].nface; f++){
	  t_face *tf=&group[g].fg[fg].f[f];
	  uvec3 fc=uvec3(tf->p[0]-1,tf->p[1]-1,tf->p[2]-1);
	  vec3 ke=vec3(material->Ke[0],material->Ke[1],material->Ke[2]);
	  vec3 kd=vec3(material->Kd[0],material->Kd[1],material->Kd[2]);
	  uvec3 code = encodeColor(fi++);
	  faces.push_back(fc);
	  areas.push_back(0); // added by T.Narumi
	  faceEmissions.push_back(ke);
	  faceReflectivities.push_back(kd);
	  faceColorCodes.push_back(code);
	}
      }
    }
    
#else
    ifstream fs(filename);

    char c;
    vec3 vertex;
    uvec3 face;
    int index[3];
    vec3 currentEmission;
    vec3 currentReflectivity; /*  this is the material "color" */
    long faceIndex = 0;

    std::string line;
    while (std::getline(fs, line)) {
        std::istringstream iss(line);
        iss >> c;

        string name;
        switch (c) {
        case 'g':
            /*  this marks the start a new object (i.e. a new material) */
            iss >> name;
            /*  assign proper colors according to the object */
            if (name.find("wall") != string::npos) {
                currentEmission = vec3(0,0,0);
                currentReflectivity = 0.7 * vec3(1, 0.25, 0.000000); /* red */
            } else if (name.find("door") != string::npos) {
                currentEmission = vec3(0,0,0);
                currentReflectivity = 0.5 * vec3(1,1,1); /* gray */
            } else if (name.find("window") != string::npos) {
                currentEmission = vec3(1,1,1);
                currentReflectivity = 0.6 * vec3(1, 1, 1);
            } else if (name.find("frame") != string::npos) {
                currentEmission = vec3(0,0,0);
                currentReflectivity = 0.1 * vec3(0.06, 0.06, 0.06); /* very dark */
            } else if (name.find("floor") != string::npos) {
                currentEmission = vec3(0,0,0);
                currentReflectivity = 0.3 * vec3(1,1,1);
            } else if (name.find("pole") != string::npos) {
                currentEmission = vec3(0,0,0);
                currentReflectivity = 0.5 * vec3(0.5, 0.5, 0.5);
            } else if (name.find("lamp") != string::npos) {
                currentEmission = 2 * vec3(1, 1, 1);
                currentReflectivity = vec3(1, 1, 1);
            } else if (name.find("Cube") != string::npos) {
                currentEmission = vec3(0,0,0);
                currentReflectivity = 0.7 * vec3(1,1,1);
            }
            break;
        case 'v':
            /* read a vertex */
            iss >> vertex.x;
            iss >> vertex.y;
            iss >> vertex.z;
            vertices.push_back(vertex);
            /* initial vertex colors for debugging */
            vertexColors.push_back(currentReflectivity);
            break;
        case 'f':
            /* read a triangle's vertex indices */
            iss >> index[0];
            iss >> index[1];
            iss >> index[2];
            /* NOTE: index in obj files starts from 1 */
            face = uvec3(index[0] - 1, index[1] -1, index[2] -1);
            faces.push_back(face);
	    areas.push_back(0); // added by T.Narumi
            faceEmissions.push_back(currentEmission );
            faceReflectivities.push_back(currentReflectivity);
            {
                uvec3 code = encodeColor(faceIndex);
                faceColorCodes.push_back(code);
            }
            faceIndex++;
            break;
        default:
            /* skip the line */
            break;
        }
    }

    fs.close();
#endif
    // added by T.Narumi for debugging
    /*
    for(int i=0;i<vertices.size();i++){
      printf("vertices[%d]=%f %f %f colors=%f %f %f\n",
	     i,vertices[i].x,vertices[i].y,vertices[i].z,
	     vertexColors[i].x,vertexColors[i].y,vertexColors[i].z);
    }
    for(int i=0;i<faces.size();i++){
      printf("faces[%d]=%d %d %d emission=%f %f %f\n",
	     i,faces[i].x,faces[i].y,faces[i].z,
	     faceEmissions[i].x,faceEmissions[i].y,faceEmissions[i].z);
      printf("  reflect=%f %f %f colorcode=%d %d %d\n",
	     faceReflectivities[i].x,faceReflectivities[i].y,faceReflectivities[i].z,
	     faceColorCodes[i].x,faceColorCodes[i].y,faceColorCodes[i].z);
    }
    */

    buildVertexToFaceMap();
    autoCalculateNormals();
    vec3 r(0.5, 0.5, 0.5);  /* initialize radiosities to 0.5 */
    radiosities = vector<vec3> (faces.size(), r);
    unshotradiosities = vector<vec3> (faces.size(), {0,0,0});// added by T.Narumi
}

void Scene::buildVertexToFaceMap() {
    VtoF = vector< vector<int> >(vertices.size());

    for (unsigned int i = 0; i < faces.size(); i++) {
        uvec3& face = faces[i];
        /* add this face to the list of faces for all three vertices */
        VtoF[face.x].push_back(i);
        VtoF[face.y].push_back(i);
        VtoF[face.z].push_back(i);
    }
}

void Scene::render() {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        for (unsigned int fi = 0; fi < faces.size(); fi++) {
        uvec3& face =  faces[fi];
        vec3& v1 = vertices[face.x];
        vec3& v2 = vertices[face.y];
        vec3& v3 = vertices[face.z];

        //vec3& c1 = radiosities[fi];
        //vec3& c2 = c1;
        //vec3& c3 = c1;
        vec3& c1 = vertexColors[face.x];
        vec3& c2 = vertexColors[face.y];
        vec3& c3 = vertexColors[face.z];

        glBegin(GL_TRIANGLES);
            glColor3f(c1.x, c1.y, c1.z);
            glVertex3f(v1.x, v1.y, v1.z);
            glColor3f(c2.x, c2.y, c2.z);
            glVertex3f(v2.x, v2.y, v2.z);
            glColor3f(c3.x, c3.y, c3.z);
            glVertex3f(v3.x, v3.y, v3.z);
        glEnd();
    }
}

void Scene::autoCalculateNormals () {
    faceNormals.clear();

    for (unsigned int i = 0; i < faces.size(); i++) {
        uvec3& face =  faces[i];
        vec3& v1 = vertices[face.x];
        vec3& v2 = vertices[face.y];
        vec3& v3 = vertices[face.z];

        vec3 normal = cross(v2 - v1, v3 - v2);
        normal.normalize();
        faceNormals.push_back(normal);
    }
}

void Scene::calculateFormFactors_one(int f) {
        int pixelCount[MAX_FACES] = {0};

        uvec3& face = faces[f];
        vec3 centroid = (vertices[face.x] + vertices[face.y] + vertices[face.z]) * 0.333333f;

        /* the new basis for the coordinate system around the centroid */
        /* n is the normal; d1, d2 are orthogonal unit vectors on the plane of the face */
        vec3& n = faceNormals[f];
        vec3 d1, d2;

        /* find vertex farthest from centroid */
        vec3 a = vertices[face.x] - centroid;
        vec3 b =  vertices[face.y] - centroid;
        vec3 c = vertices[face.z] - centroid;
        d1 = longestVector(a,b,c);
        d1.normalize();
        d2 =  cross(n, d1);

        /* half-length of the unit hemicube */
	//        float r = 0.5;
        float r = 0.1; // modified by T.Narumi

        float left, right, bottom, top;
        vec3& eye = centroid;
        vec3 lookat, up;

        for (int i = 1; i <= 5; i++) {
            if (i == 1) { /* setup view for face 1 (top) */
                left = -r; right = r; bottom = -r; top = r;
                lookat = centroid + n; up = d2;
            }
            else if (i == 2) { /* setup view for face 2 (left) */
                left = -r; right = r; bottom = 0; top = r;
                lookat = centroid + d1; up = n;
            }
            else if (i == 3) { /* setup view for face 3 (right) */
                left = -r; right = r; bottom = 0; top = r;
                lookat = centroid - d1; up = n;
            }
            else if (i == 4) { /* setup view for face 4 (back) */
                left = -r; right = r; bottom = 0; top = r;
                lookat = centroid - d2; up = n;
            }
            else if (i == 5) { /* setup view for face 5 (front) */
                left = -r; right = r; bottom = 0; top = r;
                lookat = centroid + d2; up = n;
            }

            /* setup viewport */
            unsigned int viewWidth = (right - left) / (2 * r) * HEMICUBE_RESOLUTION;
            unsigned int viewHeight = (top - bottom) / (2 * r) * HEMICUBE_RESOLUTION;
            glViewport(0, 0, viewWidth, viewHeight);

            /* setup the hemicube's side as a viewplane */
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glFrustum(left, right, bottom, top, r, 1000);
            gluLookAt(eye.x, eye.y, eye.z, lookat.x, lookat.y, lookat.z, up.x, up.y, up.z);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            /* rasterize entire scene onto this side of the hemicube */
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            renderColorCoded();

            GLubyte* buffer = new GLubyte[viewWidth * viewHeight * 3];
            /* get the contents from frame buffer */
            glReadPixels(0, 0, viewWidth, viewHeight, GL_RGB, GL_UNSIGNED_BYTE, buffer);

            /* count pixels for each face */
            for (int y = 0; y < viewHeight; y++) {
                for (int x = 0; x < viewWidth; x++) {
                    uvec3 color(buffer[(y*viewWidth+x)*3], buffer[(y*viewWidth+x)*3+1], buffer[(y*viewWidth+x)*3+2]);
                    unsigned int face = decodeColor(color);
                    if (face == -1) {
                        continue; /* empty pixel */
                    }
                    pixelCount[face]++;
                }
            }

            delete buffer;
        }
        /* end of for loop, pixels now counted on all 5 sides of hemicube */

        int totalPixels = 3 * HEMICUBE_RESOLUTION * HEMICUBE_RESOLUTION;

        /* compute form factors */
        for (unsigned int k = 0; k < faces.size(); k++) {
            if (k == f) {
                formFactors[f][k] = 0;
                continue;
            }
            float factor = pixelCount[k] / (float) totalPixels;
	    //	    printf("f=%d k=%d pixelCount=%d ff=%f\n",f,k,pixelCount[k],factor);
            formFactors[f][k] = factor;
        }
}

void Scene::calculateFormFactors() {
    formFactors.clear();
    formFactors = vector<vector<float> > (faces.size(), vector<float>(faces.size(), 0.0f));
    for (unsigned int f = 0; f < faces.size(); f++) {
      calculateFormFactors_one(f);
    }
}

void Scene::renderColorCoded() {
    for (unsigned int fi = 0; fi < faces.size(); fi++) {
        uvec3& face =  faces[fi];
        vec3& v1 = vertices[face.x];
        vec3& v2 = vertices[face.y];
        vec3& v3 = vertices[face.z];

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        uvec3& color = faceColorCodes[fi];
        glColor3ub(color.x, color.y, color.z);
        glBegin(GL_TRIANGLES);
            glVertex3f(v1.x, v1.y, v1.z);
            glVertex3f(v2.x, v2.y, v2.z);
            glVertex3f(v3.x, v3.y, v3.z);
        glEnd();
    }
}

uvec3 Scene::encodeColor (unsigned int f) {
    uvec3 code;
    f = f + 1; /* to take care of 0 based indexing of faces */
    code.x = f % 256;
    code.y = (f >> 8) % 256;
    code.z = (f >> 16) % 256;
    return code;
}

unsigned int Scene::decodeColor (uvec3 color) {
    return color.x + ((unsigned int)color.y << 8) + ((unsigned int)color.z << 16) - 1;
}

void Scene::calculateRadiosities() {
  for (int i = 0; i < faces.size(); i++) {
    radiosities[i] = faceEmissions[i];
    for (int j = 0; j < faces.size(); j++) {
      // radiosities[i].add(faceReflectivities[i] * formFactors[j][i] * radiosities[j]);
      radiosities[i].add(faceReflectivities[i] * formFactors[i][j] * radiosities[j]);// modified by T.Narumi
      //	    printf("  in calculateRadiosities: from patch-%d to patch-%d, ff=%f\n",i,j,formFactors[j][i]);
    }
    //	printf("radiosityes[%d]=(%f,%f,%f)\n",i,radiosities[i].x,radiosities[i].y,radiosities[i].z);
  }
}

// added by T.Narumi
void Scene::calculateGatherRadiosities() {
  static int ini=0;
  static std::vector<vec3> old_radiosities;
  if(ini==0){
    float sum;
    //    for(int i=0;i<faces.size();i++){sum=0;printf("ff[%d] ",i);for(int j=0;j<faces.size();j++){sum+=formFactors[i][j];printf("%f ",formFactors[i][j]);}printf("sum=%f\n",sum);}
    //    for(int i=0;i<faces.size();i++){sum=0;for(int j=0;j<faces.size();j++) sum+=formFactors[j][i];printf("sum[%d]=%f\n",i,sum);}
    for (int i = 0; i < faces.size(); i++){
      radiosities[i] = faceEmissions[i];
      old_radiosities.push_back({0,0,0});
    }
    ini=1;
  }
  for (int i = 0; i < faces.size(); i++) old_radiosities[i] = radiosities[i];
  for (int i = 0; i < faces.size(); i++) {
    radiosities[i] = faceEmissions[i];
    for (int j = 0; j < faces.size(); j++) {
      radiosities[i].add(faceReflectivities[i] * formFactors[i][j] * old_radiosities[j]);// modified by T.Narumi
      //	    printf("  in calculateRadiosities: from patch-%d to patch-%d, ff=%f\n",i,j,formFactors[j][i]);
    }
    //	printf("radiosityes[%d]=(%f,%f,%f)\n",i,radiosities[i].x,radiosities[i].y,radiosities[i].z);
  }
}
void Scene::calculateAreas() {
  for (int i = 0; i < faces.size(); i++){
    vec3 a = vertices[faces[i].y] - vertices[faces[i].x];
    vec3 b = vertices[faces[i].z] - vertices[faces[i].x];
    vec3 o;
    o.x = a.y * b.z - b.y * a.z;
    o.y = a.z * b.x - b.z * a.x;
    o.z = a.x * b.y - b.x * a.y;
    areas[i] = 0.5f * std::sqrt(o.x * o.x + o.y * o.y + o.z * o.z);
    //    printf("areas[%d]=%f\n",i,areas[i]);
  }
}
void Scene::initializeUnshotRadiosities(){
  for (int i = 0; i < faces.size(); i++) {
    unshotradiosities[i] = faceEmissions[i];
    radiosities[i] = faceEmissions[i];
  }
  // malloc formfactors
  formFactors.clear();
  formFactors = vector<vector<float> > (faces.size(), vector<float>(faces.size(), 0.0f));
}
void Scene::calculateProgressiveRadiosities() {
  float maxRadiosity = 0.0f;
  int max = 0;
  for (int i = 0; i < faces.size(); i++) {
    if(areas[i] * unshotradiosities[i].magnitude()
       > maxRadiosity){
      maxRadiosity = areas[i] * unshotradiosities[i].magnitude();
      max = i;
    }
  }
  //    printf("Max unshotradiosity[%d]=(%f,%f,%f)\n",max,unshotradiosities[max].x,unshotradiosities[max].y,unshotradiosities[max].z);
  calculateFormFactors_one(max);
  for (int j = 0; j < faces.size(); j++) {
    //vec3 inc = faceReflectivities[j] * formFactors[max][j] * unshotradiosities[max];
    vec3 inc = faceReflectivities[j] * formFactors[max][j] * areas[max] / areas[j] * unshotradiosities[max];
    radiosities[j].add(inc);
    unshotradiosities[j].add(inc);
    //      printf("  in calculateProgressiveRadiosities: from patch-%d to patch-%d, ff=%f, B=(%f,%f,%f) U=(%f,%f,%f)\n",j,max,formFactors[max][j],radiosities[j].x,radiosities[j].y,radiosities[j].z,unshotradiosities[j].x,unshotradiosities[j].y,unshotradiosities[j].z);
  }
  unshotradiosities[max] = {0, 0, 0};
}

void Scene::interpolateColors() {
    for (int vi = 0; vi < vertices.size(); vi++) {
        vector<int>& nearFaces = VtoF[vi];
        vec3 color(0,0,0);
        for (int fi = 0; fi < nearFaces.size(); fi++) {
            color.add(radiosities[nearFaces[fi]]);
        }
        color = color / (float) nearFaces.size();
        vertexColors[vi] = color;
    }
}
