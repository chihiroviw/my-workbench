#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "GL/glpng.h"
#include "objread.h"

//#define MAIN

static char material_fname[BUF_SIZE]="";

int is_new_groupname(int ngroup, t_group *group, char *group_name)
// Returns the index of the group which has group_name
// If there is no existent group_name, ngroup will be returned
{
  int i;

  for(i=0;i<ngroup;i++){
    if(strncmp(group[i].group_name, group_name, BUF_SIZE)==0){
      return i;
    }
  }
  return ngroup;
}

void read_obj(const char *fname, t_obj *obj)
{
  FILE *fp;
  char s[BUF_SIZE],dummy[BUF_SIZE];
  int ngroup,i,f,j,k;
  int nmaterial,vflag,nflag,tflag;
  t_material *material;
  char fname_header[BUF_SIZE];
  char tmps[BUF_SIZE];
  int nvertex,nvnormal,nvtexture;
  t_group **group=&obj->group;

  // count the number of groups, number of vertecies, vertex normals, vertex textures
  if((fp=fopen(fname,"r"))==NULL){
    fprintf(stderr,"** error : can't read %s **\n",fname);
    exit(1);
  }
  strncpy(fname_header,fname,BUF_SIZE);
  i=strlen(fname)-1;
  //  printf("i=%d fname=%s\n",i,fname);
  while(i>=0 && fname_header[i]!='/'){
    fname_header[i]='\0';
    i--;
  }
  //  printf("fname_header=%s\n",fname_header);
  ngroup=nvertex=nvnormal=nvtexture=0;
  while(fgets(s,BUF_SIZE,fp)!=NULL){
    if(strncmp(s,"mtllib ",7)==0){
      sscanf(s+7,"%s",tmps);
      //      sscanf(s+7,"%s",material_fname);
      sprintf(material_fname,"%s%s",fname_header,tmps);
      printf("material_fname = %s\n",material_fname);
    } else if((s[0]=='g' || s[0]=='o') && s[1]==' '){
      ngroup++;
    } else if(s[0]=='v' && s[1]==' '){
      nvertex++;
    } else if(s[0]=='v' && s[1]=='n' && s[2]==' '){
      nvnormal++;
    } else if(s[0]=='v' && s[1]=='t' && s[2]==' '){
      nvtexture++;
    }
  }
  fclose(fp);
  // no 'g' line
  if(ngroup==0){
    ngroup++;
  }
  if(ngroup>0 && (obj->group=(t_group *)malloc(sizeof(t_group)*ngroup))==NULL){
    fprintf(stderr,"** can't malloc group **\n");
    exit(1);
  }
  obj->ngroup=ngroup;
  for(i=0;i<ngroup;i++){
    obj->group[i].nfacegroup=0;
    obj->group[i].group_name[0]='\0';
    obj->group[i].nv=obj->group[i].nvn=obj->group[i].nvt=0;
  }
  if(nvertex>0 && (obj->v=(t_vertex *)malloc(sizeof(t_vertex)*(nvertex+1)))==NULL){
    fprintf(stderr,"** error : can't malloc vertex **\n");
    exit(1);
  }
  obj->nvertex=nvertex;
  for(i=0;i<nvertex;i++) for(j=0;j<3;j++) obj->v[i].p[j] = obj->v[i].c[j] = 0.0;
  if(nvnormal>0 && (obj->vn=(t_vnormal *)malloc(sizeof(t_vnormal)*(nvnormal+1)))==NULL){
    fprintf(stderr,"** error : can't malloc vnormal **\n");
    exit(1);
  }
  obj->nvnormal=nvnormal;
  for(i=0;i<nvnormal;i++) for(j=0;j<3;j++) obj->vn[i].n[j] = 0.0;
  if(nvtexture>0 && (obj->vt=(t_vtexture *)malloc(sizeof(t_vtexture)*(nvtexture+1)))==NULL){
    fprintf(stderr,"** error : can't malloc vtexture **\n");
    exit(1);
  }
  obj->nvtexture=nvtexture;
  for(i=0;i<nvtexture;i++) for(j=0;j<2;j++) obj->vt[i].t[j] = 0.0;
  printf("ngroup=%d nvertex=%d nvnormal=%d nvtexture=%d\n",
	 ngroup,nvertex,nvnormal,nvtexture);


  // count the number of face groups
  if((fp=fopen(fname,"r"))==NULL){
    fprintf(stderr,"** error : can't read %s **\n",fname);
    exit(1);
  }
  ngroup=0;
  while(fgets(s,BUF_SIZE,fp)!=NULL){
    if((s[0]=='g' || s[0]=='o') && s[1]==' '){
      sscanf(s+2,"%s",(*group)[ngroup].group_name);
      (*group)[ngroup].nfacegroup=0;
      ngroup++;
    } else if(s[0]=='v' && s[1]==' '){
      // no 'g' line
      if(ngroup==0){
	(*group)[ngroup].nfacegroup=0;
	ngroup++;
      }
    } else if(sscanf(s,"usemtl Optimized %s Material",dummy)==1 ||
       sscanf(s,"usemtl %s",dummy)==1){
      (*group)[ngroup-1].nfacegroup++;
      //      printf("ngroup=%d nfacegroup=%d\n",ngroup,(*group)[ngroup-1].nfacegroup);
    }
  }
  for(i=0;i<ngroup;i++){
    // no 'usemtl' line
    if((*group)[i].nfacegroup==0){
      (*group)[i].nfacegroup++;
    }
    printf("group=%d nfacegroup=%d\n",i,(*group)[i].nfacegroup);
  }
  fclose(fp);
  for(i=0;i<ngroup;i++){
    if(((*group)[i].fg=(t_facegroup *)malloc
	(sizeof(t_facegroup)*(*group)[i].nfacegroup))==NULL){
      fprintf(stderr,"** can't malloc fg for group %d **\n",i);
      exit(1);
    }
  }

  // cound the number of faces for each facegroup
  if((fp=fopen(fname,"r"))==NULL){
    fprintf(stderr,"** error : can't read %s **\n",fname);
    exit(1);
  }
  ngroup=0;
  while(fgets(s,BUF_SIZE,fp)!=NULL){
    if((s[0]=='g' || s[0]=='o') && s[1]==' '){
      (*group)[ngroup].nfacegroup=0;
      ngroup++;
    }
    if(sscanf(s,"usemtl Optimized %s Material",dummy)==1 ||
       sscanf(s,"usemtl %s",dummy)==1){
      // no 'g' line
      if(ngroup==0){
	(*group)[ngroup].nfacegroup=0;
	ngroup++;
      }
      (*group)[ngroup-1].nfacegroup++;
      (*group)[ngroup-1].fg[(*group)[ngroup-1].nfacegroup-1].nface=0;
    }
    if(s[0]=='f' && s[1]==' '){
      // no 'g' line
      if(ngroup==0){
	(*group)[ngroup].nfacegroup=0;
	ngroup++;
      }
      // no 'usemtl' line
      if((*group)[ngroup-1].nfacegroup==0){
	(*group)[ngroup-1].nfacegroup++;
	(*group)[ngroup-1].fg[(*group)[ngroup-1].nfacegroup-1].nface=0;
      }
      (*group)[ngroup-1].fg[(*group)[ngroup-1].nfacegroup-1].nface++;
    }
  }
  for(i=0;i<ngroup;i++){
    for(f=0;f<(*group)[i].nfacegroup;f++){
      if(((*group)[i].fg[f].f=(t_face *)malloc(sizeof(t_face)*(*group)[i].fg[f].nface))==NULL){
	fprintf(stderr,"** error : can't malloc f for g=%d fg=%d **\n",i,f);
	exit(1);
      }
      printf("group=%d facegroup=%d nfaces=%d\n",
	     i,f,(*group)[i].fg[f].nface);
    }
  }
  fclose(fp);
  
  // read material data and count the number of materials
  if(material_fname[0]!='\0'){
    if((fp=fopen(material_fname,"r"))==NULL){
      fprintf(stderr,"** error : can't read %s **\n",material_fname);
      exit(1);
    }
    nmaterial=0;
    while(fgets(s,BUF_SIZE,fp)!=NULL){
      if(sscanf(s,"newmtl Optimized %s Material",dummy)==1 ||
	 sscanf(s,"newmtl %s",dummy)==1){
	nmaterial++;
      }
    }
    fclose(fp);
    //    printf("nmaterial=%d\n",nmaterial);
    if((material=(t_material *)malloc(sizeof(t_material)*nmaterial))==NULL){
      fprintf(stderr,"** error : can't malloc material **\n");
      exit(1);
    }
  } else {
    nmaterial=0;
    material=NULL;
  }

  // read material data
  //  printf("reading material file\n");
  if(nmaterial>0){
    if((fp=fopen(material_fname,"r"))==NULL){
      fprintf(stderr,"** error : can't read %s **\n",material_fname);
      exit(1);
    }
    nmaterial=0;
    while(fgets(s,BUF_SIZE,fp)!=NULL){
      if(sscanf(s,"newmtl Optimized %s Material",material[nmaterial].material_name)==1 ||
	 sscanf(s,"newmtl %s",material[nmaterial].material_name)==1){
	for(i=0;i<4;i++){
	  material[nmaterial].Kd[i]=1.0;
	  material[nmaterial].Ks[i]=0.0;
	  material[nmaterial].Ka[i]=0.0;
	  material[nmaterial].Ke[i]=0.0;
	}
	material[nmaterial].Ns=0.0;
	material[nmaterial].texture_fname[0]='\0';
	material[nmaterial].txbuf = NULL;
	material[nmaterial].tex_id = 0;
	material[nmaterial].tex_width=0;
	material[nmaterial].tex_height=0;
	material[nmaterial].tex_channel=0;
	material[nmaterial].alpha_test = 1;
	nmaterial++;
      }
      if(s[0]=='K' && s[1]=='d' && s[2]==' '){
	sscanf(s+3,"%f %f %f",
	       &material[nmaterial-1].Kd[0],
	       &material[nmaterial-1].Kd[1],
	       &material[nmaterial-1].Kd[2]);
	//      printf("material=%d Kd=%f %f %f\n",nmaterial-1,material[nmaterial-1].Kd[0],material[nmaterial-1].Kd[1],material[nmaterial-1].Kd[2]);
      }
      if(s[0]=='K' && s[1]=='s' && s[2]==' '){
	sscanf(s+3,"%f %f %f",
	       &material[nmaterial-1].Ks[0],
	       &material[nmaterial-1].Ks[1],
	       &material[nmaterial-1].Ks[2]);
	//      printf("material=%d Ks=%f %f %f\n",nmaterial-1,material[nmaterial-1].Ks[0],material[nmaterial-1].Ks[1],material[nmaterial-1].Ks[2]);
      }
      if(s[0]=='K' && s[1]=='a' && s[2]==' '){
	sscanf(s+3,"%f %f %f",
	       &material[nmaterial-1].Ka[0],
	       &material[nmaterial-1].Ka[1],
	       &material[nmaterial-1].Ka[2]);
	//      printf("material=%d Ka=%f %f %f\n",nmaterial-1,material[nmaterial-1].Ka[0],material[nmaterial-1].Ka[1],material[nmaterial-1].Ka[2]);
      }
      if(s[0]=='K' && s[1]=='e' && s[2]==' '){
	sscanf(s+3,"%f %f %f",
	       &material[nmaterial-1].Ke[0],
	       &material[nmaterial-1].Ke[1],
	       &material[nmaterial-1].Ke[2]);
	//      printf("material=%d Ke=%f %f %f\n",nmaterial-1,material[nmaterial-1].Ke[0],material[nmaterial-1].Ke[1],material[nmaterial-1].Ke[2]);
      }
      if(s[0]=='N' && s[1]=='s' && s[2]==' '){
	sscanf(s+3,"%f",&material[nmaterial-1].Ns);
	//	printf("material=%d Ns=%f\n",nmaterial-1,material[nmaterial-1].Ns);
      }
      if(sscanf(s,"map_Kd %s",tmps)==1){
	int *width = &material[nmaterial-1].tex_width;
	int *height = &material[nmaterial-1].tex_height;
	int *channel = &material[nmaterial-1].tex_channel;
	unsigned char **txbuf = &material[nmaterial-1].txbuf;
	int *alpha_test = &material[nmaterial-1].alpha_test;
	sprintf(material[nmaterial-1].texture_fname,"%s%s",fname_header,tmps);
	i=strlen(material[nmaterial-1].texture_fname)-1;
	while(i>=0 && material[nmaterial-1].texture_fname[i]!='.'){
	  i--;
	}
	if(strcmp(material[nmaterial-1].texture_fname+i,".data")==0){// .data format
	  FILE *tfp;
	  if(sscanf(s+strlen(tmps)+7," # %d %d",width,height)!=2){
	    fprintf(stderr,"** error : width and height of texture file should be specified **\n");
	    exit(1);
	  }
	  if((tfp = fopen(material[nmaterial-1].texture_fname,"rb"))==NULL){
	    fprintf(stderr,"** can't open texture file %s **\n",
		    material[nmaterial-1].texture_fname);
	    exit(1);
	  }
	  if((*txbuf = (unsigned char *)malloc(*width * *height * 4))==NULL){
	    fprintf(stderr,"** error: can't malloc txbuf in reading texture **\n");
	    exit(1);
	  }
	  fread(*txbuf, *width * *height * 4, 1, tfp);
	  for(j=3; j<*width * *height * 4; j+=4){
	    if((*txbuf)[j] != 0 && (*txbuf)[j] != 255){
	      *alpha_test = 0;
	    }
	  }
	  *channel=4;
	  //{int x,y;for(y=0; y<*height;y++) for(x=0;x<*width;x++){ j=(y*(*height)+x)*4;printf("pixel[%d,%d]=%d %d %d %d\n",x,y,(*txbuf)[j],(*txbuf)[j+1],(*txbuf)[j+2],(*txbuf)[j+3]);}}
	  fclose(tfp);
	} else if(strcmp(material[nmaterial-1].texture_fname+i,".png")==0){// .png format
	  pngRawInfo pnginfo;
	  if(pngLoadRaw(material[nmaterial-1].texture_fname,&pnginfo)){
	    *width=pnginfo.Width;
	    *height=pnginfo.Height;
	    *channel=pnginfo.Components;
	    if(pnginfo.Data==NULL){
	      fprintf(stderr,"** error : png color format should not be palette **\n");
	      exit(1);
	    }
	    if((*txbuf = (unsigned char *)malloc(*width * *height * 4))==NULL){
	      fprintf(stderr,"** error: can't malloc txbuf in reading texture **\n");
	      exit(1);
	    } else {
	      int x,y,ry,c;
	      for(y=0; y<*height;y++){
		ry=*height - y - 1;
		for(x=0;x<*width;x++){
		  (*txbuf)[(ry*(*height)+x)*4+3]=255;
		  for(c=0;c<*channel;c++){
		    (*txbuf)[(ry*(*height)+x)*4+c]=pnginfo.Data[(y*(*height)+x)*(*channel)+c];
		  }
		}
	      }
	    }
#if 0	    
	    if(*channel==3){
	      fprintf(stderr,"** error : channel=3 of png is not supported now **\n");
	      fprintf(stderr,"**         please add alpha channel to %s **\n",
		      material[nmaterial-1].texture_fname);
	      exit(1);
	    }
#endif
	    for(j=3; j<*width * *height * 4; j+=4){
	      if((*txbuf)[j] != 0 && (*txbuf)[j] != 255){
		*alpha_test = 0;
	      }
	    }
	    //{int x,y;for(y=0; y<*height;y++) for(x=0;x<*width;x++){ j=(y*(*height)+x)*4;printf("pixel[%d,%d]=%d %d %d %d\n",x,y,(*txbuf)[j],(*txbuf)[j+1],(*txbuf)[j+2],(*txbuf)[j+3]);}}
	  } else {
	    fprintf(stderr,"** error : can' load texture file %s **\n",
		    material[nmaterial-1].texture_fname);
	    exit(1);
	  }
	} else {
	  fprintf(stderr,"** error : file format of %s is not supported **\n",
		  material[nmaterial-1].texture_fname);
	  fprintf(stderr,"**         only .png and .data(raw data) are supported **\n");
	  exit(1);
	}
      }
    }
    fclose(fp);
  }

  // initialize dummy material
  if(material==NULL){
    if((material=(t_material *)malloc(sizeof(t_material)*1))==NULL){
      fprintf(stderr,"** error : can't malloc material **\n");
      exit(1);
    }
    material[0].material_name[0]='\0';
    material[0].texture_fname[0]='\0';
    material[0].txbuf=NULL;
    material[0].tex_id=0;
    material[0].tex_width=0;
    material[0].tex_height=0;
    material[0].tex_channel=0;
    for(i=0;i<4;i++){
      material[0].Kd[i]=0.0f;
      material[0].Ks[i]=0.0f;
      material[0].Ka[i]=0.0f;
      material[0].Ke[i]=0.0f;
    }
    material[0].Ns=0.0f;
  }

  // read vertex and face data
  //  printf("reading vertex and face data\n");
  if((fp=fopen(fname,"r"))==NULL){
    fprintf(stderr,"** error : can't read %s **\n",fname);
    exit(1);
  }
  ngroup=nvertex=nvnormal=nvtexture=0;
  while(fgets(s,BUF_SIZE,fp)!=NULL){
    int nfg,nfc;
    if((s[0]=='g' || s[0]=='o') && s[1]==' '){
      (*group)[ngroup].nfacegroup=0;
      vflag=nflag=tflag=0;
      ngroup++;
    }
    if(s[0]=='v' && s[1]==' '){
      // no 'g' line
      if(ngroup==0){
	(*group)[ngroup].nfacegroup=0;
	vflag=nflag=tflag=0;
	ngroup++;
      }
      nvertex++;
      (*group)[ngroup-1].nv++;
      sscanf(s+2,"%f %f %f",
      	     &obj->v[nvertex].p[0],&obj->v[nvertex].p[1],&obj->v[nvertex].p[2]);
      vflag=1;
    }
    if(s[0]=='v' && s[1]=='n' && s[2]==' '){
      nvnormal++;
      (*group)[ngroup-1].nvn++;
      sscanf(s+3,"%f %f %f",
	     &obj->vn[nvnormal].n[0],&obj->vn[nvnormal].n[1],&obj->vn[nvnormal].n[2]);
      nflag=1;
    }
    if(s[0]=='v' && s[1]=='t' && s[2]==' '){
      nvtexture++;
      (*group)[ngroup-1].nvt++;
      sscanf(s+3,"%f %f",
	     &obj->vt[nvtexture].t[0],&obj->vt[nvtexture].t[1]);
      tflag=1;
    }
    if(sscanf(s,"usemtl Optimized %s Material",
	      (*group)[ngroup-1].fg[(*group)[ngroup-1].nfacegroup].material.material_name)==1 ||
       sscanf(s,"usemtl %s",
	      (*group)[ngroup-1].fg[(*group)[ngroup-1].nfacegroup].material.material_name)==1){
      (*group)[ngroup-1].nfacegroup++;
      (*group)[ngroup-1].fg[(*group)[ngroup-1].nfacegroup-1].nface=0;
    }
    if(s[0]=='f' && s[1]==' '){
      t_face *face;
      // no 'usemtl' line
      if((*group)[ngroup-1].nfacegroup==0){
	(*group)[ngroup-1].nfacegroup++;
	(*group)[ngroup-1].fg[(*group)[ngroup-1].nfacegroup-1].nface=0;
      }
      i=(*group)[ngroup-1].fg[(*group)[ngroup-1].nfacegroup-1].nface;
      face=(*group)[ngroup-1].fg[(*group)[ngroup-1].nfacegroup-1].f;
      if(sscanf(s+2,"%d/%d/%d %d/%d/%d %d/%d/%d",
		&face[i].p[0],&face[i].t[0],&face[i].n[0],
		&face[i].p[1],&face[i].t[1],&face[i].n[1],
		&face[i].p[2],&face[i].t[2],&face[i].n[2])==9){
      } else if(sscanf(s+2,"%d//%d %d//%d %d//%d",
		       &face[i].p[0],&face[i].n[0],
		       &face[i].p[1],&face[i].n[1],
		       &face[i].p[2],&face[i].n[2])==6){
	face[i].t[0] = face[i].t[1] = face[i].t[2] = 0;
      } else if(sscanf(s+2,"%d/%d %d/%d %d/%d",
		       &face[i].p[0],&face[i].t[0],
		       &face[i].p[1],&face[i].t[1],
		       &face[i].p[2],&face[i].t[2])==6){
	face[i].n[0] = face[i].n[1] = face[i].n[2] = 0;
      } else if(sscanf(s+2,"%d %d %d",
		       &face[i].p[0], &face[i].p[1], &face[i].p[2])==3){
	face[i].n[0] = face[i].n[1] = face[i].n[2] = 0.0;
	face[i].t[0] = face[i].t[1] = face[i].t[2] = 0;
      }
      (*group)[ngroup-1].fg[(*group)[ngroup-1].nfacegroup-1].nface++;
    }
  }
  fclose(fp);

  // assign Kd and Ks for each material
  //  printf("assign Kd and Ks\n");
  for(i=0; i<ngroup; i++){
    for(f=0; f<(*group)[i].nfacegroup; f++){
      for(j=0; j<nmaterial; j++){
	if(strncmp((*group)[i].fg[f].material.material_name,
		   material[j].material_name,BUF_SIZE)==0){
	  (*group)[i].fg[f].material = material[j];
	}
      }
      if(nmaterial==0) (*group)[i].fg[f].material = material[0];
      printf("group=%d facegroup=%d material=%s texture_fname=%s(%d,%d)\n",
	     i,f,(*group)[i].fg[f].material.material_name,(*group)[i].fg[f].material.texture_fname,
	     (*group)[i].fg[f].material.tex_width,(*group)[i].fg[f].material.tex_height);
    }
  }
}

void read_obj2(const char *fname, t_obj *obj)
{
  FILE *fp;
  char s[BUF_SIZE],dummy[BUF_SIZE];
  int ngroup,i,f,j,k;
  int nmaterial,vflag,nflag,tflag;
  t_material *material;
  char fname_header[BUF_SIZE];
  char tmps[BUF_SIZE];
  int nvertex,nvnormal,nvtexture;
  t_group **group=&obj->group;
  char vtmp[4][BUF_SIZE];
  int vcount,nfacegroup;
  char group_name[BUF_SIZE];

  // count the number of groups, number of vertecies, vertex normals, vertex textures
  if((fp=fopen(fname,"r"))==NULL){
    fprintf(stderr,"** error : can't read %s **\n",fname);
    exit(1);
  }
  strncpy(fname_header,fname,BUF_SIZE);
  i=strlen(fname)-1;
  //  printf("i=%d fname=%s\n",i,fname);
  while(i>=0 && fname_header[i]!='/'){
    fname_header[i]='\0';
    i--;
  }
  //  printf("fname_header=%s\n",fname_header);
  ngroup=nvertex=nvnormal=nvtexture=0;
  while(fgets(s,BUF_SIZE,fp)!=NULL){
    if(strncmp(s,"mtllib ",7)==0){
      sscanf(s+7,"%s",tmps);
      //      sscanf(s+7,"%s",material_fname);
      sprintf(material_fname,"%s%s",fname_header,tmps);
      printf("material_fname = %s\n",material_fname);
    } else if((s[0]=='g' || s[0]=='o') && s[1]==' '){
      sscanf(s+2,"%s",group_name);
      if(is_new_groupname(ngroup,obj->group,group_name)==ngroup){
	ngroup++;
	if(ngroup==1){
	  if((obj->group=(t_group *)malloc(sizeof(t_group)))==NULL){
	    fprintf(stderr,"** can't malloc first group **\n");
	    exit(1);
	  }
	} else {
	  if((obj->group=(t_group *)realloc(obj->group,sizeof(t_group)*ngroup))==NULL){
	    fprintf(stderr,"** can't realloc group **\n");
	    exit(1);
	  }
	}
	strncpy(obj->group[ngroup-1].group_name,group_name,BUF_SIZE);
      } else {
	//	printf("Same as previous group: %s",s+2);
      }
    } else if(s[0]=='v' && s[1]==' '){
      nvertex++;
    } else if(s[0]=='v' && s[1]=='n' && s[2]==' '){
      nvnormal++;
    } else if(s[0]=='v' && s[1]=='t' && s[2]==' '){
      nvtexture++;
    }
  }
  fclose(fp);
  // no 'g' line
  if(ngroup==0){
    ngroup++;
    printf("  incremented if no group at all, ngroup=%d\n",ngroup);
    if((obj->group=(t_group *)malloc(sizeof(t_group)))==NULL){
      fprintf(stderr,"** can't malloc first group **\n");
      exit(1);
    }
    sprintf(obj->group[0].group_name,"DefaultGroup");
  }
  //  printf("allocated groups, ngroup=%d\n",ngroup);
  /*
  if(ngroup>0 && (obj->group=(t_group *)malloc(sizeof(t_group)*ngroup))==NULL){
    fprintf(stderr,"** can't malloc group **\n");
    exit(1);
  }
  */
  obj->ngroup=ngroup;
  for(i=0;i<ngroup;i++){
    obj->group[i].nfacegroup=0;
    //    obj->group[i].group_name[0]='\0';
    obj->group[i].nv=obj->group[i].nvn=obj->group[i].nvt=0;
  }
  if(nvertex>0 && (obj->v=(t_vertex *)malloc(sizeof(t_vertex)*(nvertex+1)))==NULL){
    fprintf(stderr,"** error : can't malloc vertex **\n");
    exit(1);
  }
  obj->nvertex=nvertex;
  for(i=0;i<nvertex;i++) for(j=0;j<3;j++) obj->v[i].p[j] = obj->v[i].c[j] = 0.0;
  if(nvnormal>0 && (obj->vn=(t_vnormal *)malloc(sizeof(t_vnormal)*(nvnormal+1)))==NULL){
    fprintf(stderr,"** error : can't malloc vnormal **\n");
    exit(1);
  }
  obj->nvnormal=nvnormal;
  for(i=0;i<nvnormal;i++) for(j=0;j<3;j++) obj->vn[i].n[j] = 0.0;
  if(nvtexture>0 && (obj->vt=(t_vtexture *)malloc(sizeof(t_vtexture)*(nvtexture+1)))==NULL){
    fprintf(stderr,"** error : can't malloc vtexture **\n");
    exit(1);
  }
  obj->nvtexture=nvtexture;
  for(i=0;i<nvtexture;i++) for(j=0;j<2;j++) obj->vt[i].t[j] = 0.0;
  printf("ngroup=%d nvertex=%d nvnormal=%d nvtexture=%d\n",
	 ngroup,nvertex,nvnormal,nvtexture);


  // count the number of face groups
  if((fp=fopen(fname,"r"))==NULL){
    fprintf(stderr,"** error : can't read %s **\n",fname);
    exit(1);
  }
  //  printf("before counting number of face groups, ngroup=%d\n",ngroup);
  ngroup=0;
  while(fgets(s,BUF_SIZE,fp)!=NULL){
    if((s[0]=='g' || s[0]=='o') && s[1]==' '){
      sscanf(s+2,"%s",group_name);
      ngroup=is_new_groupname(obj->ngroup,obj->group,group_name);
      //      sscanf(s+2,"%s",(*group)[ngroup].group_name);
      //      (*group)[ngroup].nfacegroup=0;
      //      ngroup++;
      //      printf("  incremented if g or o, ngroup=%d\n",ngroup);
    } else if(s[0]=='v' && s[1]==' '){
      // no 'g' line
      /*      
      if(ngroup==0){
	(*group)[ngroup].nfacegroup=0;
	ngroup++;
	printf("  incremented if no group before v, ngroup=%d\n",ngroup);
      }
      */
    } else if(sscanf(s,"usemtl Optimized %s Material",dummy)==1 ||
       sscanf(s,"usemtl %s",dummy)==1){
      (*group)[ngroup].nfacegroup++;
      //      (*group)[ngroup-1].nfacegroup++;
      //      printf("ngroup=%d nfacegroup=%d\n",ngroup,(*group)[ngroup-1].nfacegroup);
    }
  }
  ngroup=obj->ngroup;
  for(i=0;i<ngroup;i++){
    // no 'usemtl' line
    if((*group)[i].nfacegroup==0){
      (*group)[i].nfacegroup++;
    }
    printf("group=%d nfacegroup=%d group_name=%s\n",i,(*group)[i].nfacegroup,(*group)[i].group_name);
  }
  fclose(fp);
  for(i=0;i<ngroup;i++){
    if(((*group)[i].fg=(t_facegroup *)malloc
	(sizeof(t_facegroup)*(*group)[i].nfacegroup))==NULL){
      fprintf(stderr,"** can't malloc fg for group %d **\n",i);
      exit(1);
    }
    for(j=0;j<(*group)[i].nfacegroup;j++){
      (*group)[i].fg[j].nface=0;
      sprintf((*group)[i].fg[j].material.material_name,"DefaultMaterial");
    }
  }

  // cound the number of faces for each facegroup
  if((fp=fopen(fname,"r"))==NULL){
    fprintf(stderr,"** error : can't read %s **\n",fname);
    exit(1);
  }
  //  printf("before reading number of faces, ngroup=%d\n",ngroup);
  ngroup=0;
  nfacegroup=0;
  while(fgets(s,BUF_SIZE,fp)!=NULL){
    if((s[0]=='g' || s[0]=='o') && s[1]==' '){
      sscanf(s+2,"%s",group_name);
      ngroup=is_new_groupname(obj->ngroup,obj->group,group_name);
      nfacegroup=0;
      //      (*group)[ngroup].nfacegroup=0;
      //      ngroup++;
      //      printf("  incremented if g or o, ngroup=%d\n",ngroup);
    }
    if(sscanf(s,"usemtl Optimized %s Material",dummy)==1 ||
       sscanf(s,"usemtl %s",dummy)==1){
      // no 'g' line
      /*
      if(ngroup==0){
	(*group)[ngroup].nfacegroup=0;
	ngroup++;
	printf("  incremented if no group before usemtl, ngroup=%d\n",ngroup);
      }
      (*group)[ngroup-1].nfacegroup++;
      (*group)[ngroup-1].fg[(*group)[ngroup-1].nfacegroup-1].nface=0;
      (*group)[ngroup].nfacegroup++;
      (*group)[ngroup].fg[(*group)[ngroup].nfacegroup-1].nface=0;
      */
      nfacegroup++;
    }
    if(s[0]=='f' && s[1]==' '){
      // no 'g' line
      /*
      if(ngroup==0){
	(*group)[ngroup].nfacegroup=0;
	ngroup++;
	printf("  incremented if no group before f, ngroup=%d\n",ngroup);
      }
      */
      // no 'usemtl' line
      /*
      if((*group)[ngroup-1].nfacegroup==0){
	(*group)[ngroup-1].nfacegroup++;
	(*group)[ngroup-1].fg[(*group)[ngroup-1].nfacegroup-1].nface=0;
      }
      */
      if(nfacegroup==0) nfacegroup++;
      /*
      if((*group)[ngroup].nfacegroup==0){
	(*group)[ngroup].nfacegroup++;
	(*group)[ngroup].fg[(*group)[ngroup].nfacegroup-1].nface=0;
      }
      */
      vcount=0;
      vcount=sscanf(s,"f %s %s %s %s",vtmp[0],vtmp[1],vtmp[2],vtmp[3]);
      //      printf("ngroup=%d nface=%d vcount=%d\n",ngroup,(*group)[ngroup-1].fg[(*group)[ngroup-1].nfacegroup-1].nface,vcount);
      if(vcount==4){// 4-point surface
	//	(*group)[ngroup-1].fg[(*group)[ngroup-1].nfacegroup-1].nface += 2;
	(*group)[ngroup].fg[nfacegroup-1].nface += 2;
      } else if(vcount==3){// 3-point surface
	//	(*group)[ngroup-1].fg[(*group)[ngroup-1].nfacegroup-1].nface++;
	(*group)[ngroup].fg[nfacegroup-1].nface++;
      } else {
	fprintf(stderr,"** error : number of points per surface cannot be recognized **\n");
	exit(1);
      }
    }
  }
  ngroup=obj->ngroup;
  for(i=0;i<ngroup;i++){
    for(f=0;f<(*group)[i].nfacegroup;f++){
      if(((*group)[i].fg[f].f=(t_face *)malloc(sizeof(t_face)*(*group)[i].fg[f].nface))==NULL){
	fprintf(stderr,"** error : can't malloc f for g=%d fg=%d **\n",i,f);
	exit(1);
      }
      printf("group=%d facegroup=%d nfaces=%d\n",
	     i,f,(*group)[i].fg[f].nface);
    }
  }
  fclose(fp);
  
  // read material data and count the number of materials
  if(material_fname[0]!='\0'){
    if((fp=fopen(material_fname,"r"))==NULL){
      fprintf(stderr,"** error : can't read %s **\n",material_fname);
      exit(1);
    }
    nmaterial=0;
    while(fgets(s,BUF_SIZE,fp)!=NULL){
      if(sscanf(s,"newmtl Optimized %s Material",dummy)==1 ||
	 sscanf(s,"newmtl %s",dummy)==1){
	nmaterial++;
      }
    }
    fclose(fp);
    //    printf("nmaterial=%d\n",nmaterial);
    if((material=(t_material *)malloc(sizeof(t_material)*nmaterial))==NULL){
      fprintf(stderr,"** error : can't malloc material **\n");
      exit(1);
    }
  } else {
    nmaterial=0;
    material=NULL;
  }

  // read material data
  //  printf("reading material file, nmaterial=%d\n",nmaterial);
  if(nmaterial>0){
    if((fp=fopen(material_fname,"r"))==NULL){
      fprintf(stderr,"** error : can't read %s **\n",material_fname);
      exit(1);
    }
    nmaterial=0;
    while(fgets(s,BUF_SIZE,fp)!=NULL){
      if(sscanf(s,"newmtl Optimized %s Material",material[nmaterial].material_name)==1 ||
	 sscanf(s,"newmtl %s",material[nmaterial].material_name)==1){
	for(i=0;i<4;i++){
	  material[nmaterial].Kd[i]=1.0;
	  material[nmaterial].Ks[i]=0.0;
	  material[nmaterial].Ka[i]=0.0;
	  material[nmaterial].Ke[i]=0.0;
	}
	material[nmaterial].Ns=0.0;
	material[nmaterial].texture_fname[0]='\0';
	material[nmaterial].txbuf = NULL;
	material[nmaterial].tex_id = 0;
	material[nmaterial].tex_width=0;
	material[nmaterial].tex_height=0;
	material[nmaterial].tex_channel=0;
	material[nmaterial].alpha_test = 1;
	nmaterial++;
      }
      if(s[0]=='K' && s[1]=='d' && s[2]==' '){
	sscanf(s+3,"%f %f %f",
	       &material[nmaterial-1].Kd[0],
	       &material[nmaterial-1].Kd[1],
	       &material[nmaterial-1].Kd[2]);
	//      printf("material=%d Kd=%f %f %f\n",nmaterial-1,material[nmaterial-1].Kd[0],material[nmaterial-1].Kd[1],material[nmaterial-1].Kd[2]);
      }
      if(s[0]=='K' && s[1]=='s' && s[2]==' '){
	sscanf(s+3,"%f %f %f",
	       &material[nmaterial-1].Ks[0],
	       &material[nmaterial-1].Ks[1],
	       &material[nmaterial-1].Ks[2]);
	//      printf("material=%d Ks=%f %f %f\n",nmaterial-1,material[nmaterial-1].Ks[0],material[nmaterial-1].Ks[1],material[nmaterial-1].Ks[2]);
      }
      if(s[0]=='K' && s[1]=='a' && s[2]==' '){
	sscanf(s+3,"%f %f %f",
	       &material[nmaterial-1].Ka[0],
	       &material[nmaterial-1].Ka[1],
	       &material[nmaterial-1].Ka[2]);
	//      printf("material=%d Ka=%f %f %f\n",nmaterial-1,material[nmaterial-1].Ka[0],material[nmaterial-1].Ka[1],material[nmaterial-1].Ka[2]);
      }
      if(s[0]=='K' && s[1]=='e' && s[2]==' '){
	sscanf(s+3,"%f %f %f",
	       &material[nmaterial-1].Ke[0],
	       &material[nmaterial-1].Ke[1],
	       &material[nmaterial-1].Ke[2]);
	//      printf("material=%d Ke=%f %f %f\n",nmaterial-1,material[nmaterial-1].Ke[0],material[nmaterial-1].Ke[1],material[nmaterial-1].Ke[2]);
      }
      if(s[0]=='N' && s[1]=='s' && s[2]==' '){
	sscanf(s+3,"%f",&material[nmaterial-1].Ns);
	//	printf("material=%d Ns=%f\n",nmaterial-1,material[nmaterial-1].Ns);
      }
      if(sscanf(s,"map_Kd %s",tmps)==1){
	int *width = &material[nmaterial-1].tex_width;
	int *height = &material[nmaterial-1].tex_height;
	int *channel = &material[nmaterial-1].tex_channel;
	unsigned char **txbuf = &material[nmaterial-1].txbuf;
	int *alpha_test = &material[nmaterial-1].alpha_test;
	sprintf(material[nmaterial-1].texture_fname,"%s%s",fname_header,tmps);
	i=strlen(material[nmaterial-1].texture_fname)-1;
	while(i>=0 && material[nmaterial-1].texture_fname[i]!='.'){
	  i--;
	}
	if(strcmp(material[nmaterial-1].texture_fname+i,".data")==0){// .data format
	  FILE *tfp;
	  if(sscanf(s+strlen(tmps)+7," # %d %d",width,height)!=2){
	    fprintf(stderr,"** error : width and height of texture file should be specified **\n");
	    exit(1);
	  }
	  if((tfp = fopen(material[nmaterial-1].texture_fname,"rb"))==NULL){
	    fprintf(stderr,"** can't open texture file %s **\n",
		    material[nmaterial-1].texture_fname);
	    exit(1);
	  }
	  if((*txbuf = (unsigned char *)malloc(*width * *height * 4))==NULL){
	    fprintf(stderr,"** error: can't malloc txbuf in reading texture **\n");
	    exit(1);
	  }
	  fread(*txbuf, *width * *height * 4, 1, tfp);
	  for(j=3; j<*width * *height * 4; j+=4){
	    if((*txbuf)[j] != 0 && (*txbuf)[j] != 255){
	      *alpha_test = 0;
	    }
	  }
	  *channel=4;
	  //{int x,y;for(y=0; y<*height;y++) for(x=0;x<*width;x++){ j=(y*(*height)+x)*4;printf("pixel[%d,%d]=%d %d %d %d\n",x,y,(*txbuf)[j],(*txbuf)[j+1],(*txbuf)[j+2],(*txbuf)[j+3]);}}
	  fclose(tfp);
	} else if(strcmp(material[nmaterial-1].texture_fname+i,".png")==0){// .png format
	  pngRawInfo pnginfo;
	  if(pngLoadRaw(material[nmaterial-1].texture_fname,&pnginfo)){
	    *width=pnginfo.Width;
	    *height=pnginfo.Height;
	    *channel=pnginfo.Components;
	    if(pnginfo.Data==NULL){
	      fprintf(stderr,"** error : png color format should not be palette **\n");
	      exit(1);
	    }
	    if((*txbuf = (unsigned char *)malloc(*width * *height * 4))==NULL){
	      fprintf(stderr,"** error: can't malloc txbuf in reading texture **\n");
	      exit(1);
	    } else {
	      int x,y,ry,c;
	      for(y=0; y<*height;y++){
		ry=*height - y - 1;
		for(x=0;x<*width;x++){
		  (*txbuf)[(ry*(*height)+x)*4+3]=255;
		  for(c=0;c<*channel;c++){
		    (*txbuf)[(ry*(*height)+x)*4+c]=pnginfo.Data[(y*(*height)+x)*(*channel)+c];
		  }
		}
	      }
	    }
#if 0	    
	    if(*channel==3){
	      fprintf(stderr,"** error : channel=3 of png is not supported now **\n");
	      fprintf(stderr,"**         please add alpha channel to %s **\n",
		      material[nmaterial-1].texture_fname);
	      exit(1);
	    }
#endif
	    for(j=3; j<*width * *height * 4; j+=4){
	      if((*txbuf)[j] != 0 && (*txbuf)[j] != 255){
		*alpha_test = 0;
	      }
	    }
	    //{int x,y;for(y=0; y<*height;y++) for(x=0;x<*width;x++){ j=(y*(*height)+x)*4;printf("pixel[%d,%d]=%d %d %d %d\n",x,y,(*txbuf)[j],(*txbuf)[j+1],(*txbuf)[j+2],(*txbuf)[j+3]);}}
	  } else {
	    fprintf(stderr,"** error : can' load texture file %s **\n",
		    material[nmaterial-1].texture_fname);
	    exit(1);
	  }
	} else {
	  fprintf(stderr,"** error : file format of %s is not supported **\n",
		  material[nmaterial-1].texture_fname);
	  fprintf(stderr,"**         only .png and .data(raw data) are supported **\n");
	  exit(1);
	}
      }
    }
    fclose(fp);
  }

  // initialize dummy material
  if(material==NULL){
    if((material=(t_material *)malloc(sizeof(t_material)*1))==NULL){
      fprintf(stderr,"** error : can't malloc material **\n");
      exit(1);
    }
    material[0].material_name[0]='\0';
    material[0].texture_fname[0]='\0';
    material[0].txbuf=NULL;
    material[0].tex_id=0;
    material[0].tex_width=0;
    material[0].tex_height=0;
    material[0].tex_channel=0;
    for(i=0;i<4;i++){
      material[0].Kd[i]=0.0f;
      material[0].Ks[i]=0.0f;
      material[0].Ka[i]=0.0f;
      material[0].Ke[i]=0.0f;
    }
    material[0].Ns=0.0f;
  }

  // read vertex and face data
  //  printf("reading vertex and face data\n");
  if((fp=fopen(fname,"r"))==NULL){
    fprintf(stderr,"** error : can't read %s **\n",fname);
    exit(1);
  }
  //  printf("before reading vertex data, ngroup=%d\n",ngroup);
  for(i=0;i<ngroup;i++){
    for(j=0;j<(*group)[i].nfacegroup;j++){
      (*group)[i].fg[j].nface=0;
    }
  }
  ngroup=nvertex=nvnormal=nvtexture=0;
  nfacegroup=0;
  //  nface=0;
  //  i=nface;
  while(fgets(s,BUF_SIZE,fp)!=NULL){
    int nfg,nfc;
    if((s[0]=='g' || s[0]=='o') && s[1]==' '){
      sscanf(s+2,"%s",group_name);
      ngroup=is_new_groupname(obj->ngroup,obj->group,group_name);
      //      (*group)[ngroup].nfacegroup=0;
      vflag=nflag=tflag=0;
      //      ngroup++;
      //      printf("  incremented by g or o, ngroup=%d\n",ngroup);
    }
    if(s[0]=='v' && s[1]==' '){
      // no 'g' line
      /*
      if(ngroup==0){
	(*group)[ngroup].nfacegroup=0;
	vflag=nflag=tflag=0;
	ngroup++;
	printf("  incremented before v, ngroup=%d\n",ngroup);
      }
      */
      nvertex++;
      //      (*group)[ngroup-1].nv++;
      (*group)[ngroup].nv++;
      sscanf(s+2,"%f %f %f",
      	     &obj->v[nvertex].p[0],&obj->v[nvertex].p[1],&obj->v[nvertex].p[2]);
      vflag=1;
    }
    if(s[0]=='v' && s[1]=='n' && s[2]==' '){
      nvnormal++;
      //      (*group)[ngroup-1].nvn++;
      (*group)[ngroup].nvn++;
      sscanf(s+3,"%f %f %f",
	     &obj->vn[nvnormal].n[0],&obj->vn[nvnormal].n[1],&obj->vn[nvnormal].n[2]);
      nflag=1;
    }
    if(s[0]=='v' && s[1]=='t' && s[2]==' '){
      nvtexture++;
      //      (*group)[ngroup-1].nvt++;
      (*group)[ngroup].nvt++;
      sscanf(s+3,"%f %f",
	     &obj->vt[nvtexture].t[0],&obj->vt[nvtexture].t[1]);
      tflag=1;
    }
    /*
    if(sscanf(s,"usemtl Optimized %s Material",
	      (*group)[ngroup-1].fg[(*group)[ngroup-1].nfacegroup].material.material_name)==1 ||
       sscanf(s,"usemtl %s",
	      (*group)[ngroup-1].fg[(*group)[ngroup-1].nfacegroup].material.material_name)==1){
      (*group)[ngroup-1].nfacegroup++;
      (*group)[ngroup-1].fg[(*group)[ngroup-1].nfacegroup-1].nface=0;
    }
    */
    if(sscanf(s,"usemtl Optimized %s Material",
	      (*group)[ngroup].fg[nfacegroup].material.material_name)==1 ||
       sscanf(s,"usemtl %s",
	      (*group)[ngroup].fg[nfacegroup].material.material_name)==1){
      printf("ngroup=%d nfacegroup=%d usemtl is found: %s\n",ngroup,nfacegroup,(*group)[ngroup].fg[nfacegroup].material.material_name);
      if(nfacegroup!=0){
	nfacegroup++;
	(*group)[ngroup].fg[nfacegroup-1].nface=0;
      }
    }
    if(s[0]=='f' && s[1]==' '){
      t_face *face;
      int ptmp[4],ttmp[4],ntmp[4],vperv;
      // no 'usemtl' line
      /*
      if((*group)[ngroup-1].nfacegroup==0){
	(*group)[ngroup-1].nfacegroup++;
	(*group)[ngroup-1].fg[(*group)[ngroup-1].nfacegroup-1].nface=0;
      }
      i=(*group)[ngroup-1].fg[(*group)[ngroup-1].nfacegroup-1].nface;
      face=(*group)[ngroup-1].fg[(*group)[ngroup-1].nfacegroup-1].f;
      */
      if(nfacegroup==0){
	nfacegroup++;
	(*group)[ngroup].fg[nfacegroup-1].nface=0;
      }
      face=(*group)[ngroup].fg[nfacegroup-1].f;
      i=(*group)[ngroup].fg[nfacegroup-1].nface;

      vcount=0;
      vcount=sscanf(s,"f %s %s %s %s",vtmp[0],vtmp[1],vtmp[2],vtmp[3]);
      //      if(vcount==3) printf("f %s %s %s, nfacegroup=%d, face=%d\n",vtmp[0],vtmp[1],vtmp[2],nfacegroup,i);
      //      if(vcount==4) printf("f %s %s %s %s, nfacegroup=%d, face=%d\n",vtmp[0],vtmp[1],vtmp[2],vtmp[3],nfacegroup,i);
      if(vcount<3 || vcount>4){
	fprintf(stderr,"** error : number of points per surface is out of range **\n");
	exit(1);
      }
      for(j=0;j<vcount;j++){
	if(sscanf(vtmp[j],"%d/%d/%d",&ptmp[j],&ttmp[j],&ntmp[j])==3){
	  vperv=3;
	  if(j<3){
	    face[i].p[j] = ptmp[j];
	    face[i].t[j] = ttmp[j];
	    face[i].n[j] = ntmp[j];
	  }
	} else if(sscanf(vtmp[j],"%d//%d",&ptmp[j],&ntmp[j])==2){
	  vperv=2;
	  if(j<3){
	    face[i].p[j] = ptmp[j];
	    face[i].t[j] = 0;
	    face[i].n[j] = ntmp[j];
	  }
	} else if(sscanf(vtmp[j],"%d/%d",&ptmp[j],&ntmp[j])==2){
	  vperv=-2;
	  if(j<3){
	    face[i].p[j] = ptmp[j];
	    face[i].t[j] = ttmp[j];
	    face[i].n[j] = 0;
	  }
	} else if(sscanf(vtmp[j],"%d",&ptmp[j],&ntmp[j])==1){
	  vperv=1;
	  if(j<3){
	    face[i].p[j] = ptmp[j];
	    face[i].t[j] = 0;
	    face[i].n[j] = 0;
	  }
	}
      }
      //      printf("vcount=%d, ngroup=%d nfacegroup=%d nface=%d\n",vcount,ngroup,nfacegroup-1,(*group)[ngroup].fg[nfacegroup-1].nface);
      if(vcount==4){ 
	// next triangle
	/*
	(*group)[ngroup-1].fg[(*group)[ngroup-1].nfacegroup-1].nface++;
	i=(*group)[ngroup-1].fg[(*group)[ngroup-1].nfacegroup-1].nface;
	*/
	(*group)[ngroup].fg[nfacegroup-1].nface++;
	i=(*group)[ngroup].fg[nfacegroup-1].nface;

	for(j=0;j<3;j++){
	  int jnext = j>=2 ? 0:j+2;
	  if(vperv==3){
	    face[i].p[j] = ptmp[jnext];
	    face[i].t[j] = ttmp[jnext];
	    face[i].n[j] = ntmp[jnext];
	  } else if(vperv==2){
	    face[i].p[j] = ptmp[jnext];
	    face[i].t[j] = 0;
	    face[i].n[j] = ntmp[jnext];
	  } else if(vperv==-2){
	    face[i].p[j] = ptmp[jnext];
	    face[i].t[j] = ttmp[jnext];
	    face[i].n[j] = 0;
	  } else if(vperv==1){
	    face[i].p[j] = ptmp[jnext];
	    face[i].t[j] = 0;
	    face[i].n[j] = 0;
	  }
	}
	//	printf("only vcount=4, ngroup=%d nfacegroup=%d nface=%d\n",ngroup,nfacegroup-1,(*group)[ngroup].fg[nfacegroup-1].nface);
      }
      //      (*group)[ngroup-1].fg[(*group)[ngroup-1].nfacegroup-1].nface++;
      (*group)[ngroup].fg[nfacegroup-1].nface++;
      i=(*group)[ngroup].fg[nfacegroup-1].nface;
      /*
      if(sscanf(s+2,"%d/%d/%d %d/%d/%d %d/%d/%d",
		&face[i].p[0],&face[i].t[0],&face[i].n[0],
		&face[i].p[1],&face[i].t[1],&face[i].n[1],
		&face[i].p[2],&face[i].t[2],&face[i].n[2])==9){
      } else if(sscanf(s+2,"%d//%d %d//%d %d//%d",
		       &face[i].p[0],&face[i].n[0],
		       &face[i].p[1],&face[i].n[1],
		       &face[i].p[2],&face[i].n[2])==6){
	face[i].t[0] = face[i].t[1] = face[i].t[2] = 0;
      } else if(sscanf(s+2,"%d/%d %d/%d %d/%d",
		       &face[i].p[0],&face[i].t[0],
		       &face[i].p[1],&face[i].t[1],
		       &face[i].p[2],&face[i].t[2])==6){
	face[i].n[0] = face[i].n[1] = face[i].n[2] = 0;
      } else if(sscanf(s+2,"%d %d %d",
		       &face[i].p[0], &face[i].p[1], &face[i].p[2])==3){
	face[i].n[0] = face[i].n[1] = face[i].n[2] = 0.0;
	face[i].t[0] = face[i].t[1] = face[i].t[2] = 0;
      }
      (*group)[ngroup-1].fg[(*group)[ngroup-1].nfacegroup-1].nface++;
      */
    }
  }
  //  printf("after reading vertex data, ngroup=%d\n",ngroup);
  ngroup=obj->ngroup;
  fclose(fp);

  // assign Kd and Ks for each material
  //  printf("assign Kd and Ks\n");
  for(i=0; i<ngroup; i++){
    for(f=0; f<(*group)[i].nfacegroup; f++){
      for(j=0; j<nmaterial; j++){
	if(strncmp((*group)[i].fg[f].material.material_name,
		   material[j].material_name,BUF_SIZE)==0){
	  (*group)[i].fg[f].material = material[j];
	}
      }
      if(nmaterial==0) (*group)[i].fg[f].material = material[0];
      printf("group=%d facegroup=%d material=%s texture_fname=%s(%d,%d)\n",
	     i,f,(*group)[i].fg[f].material.material_name,(*group)[i].fg[f].material.texture_fname,
	     (*group)[i].fg[f].material.tex_width,(*group)[i].fg[f].material.tex_height);
    }
  }
}

void print_obj(t_obj obj)
{
  int ngroup=obj.ngroup;
  int nvertex=obj.nvertex;
  int nvnormal=obj.nvnormal;
  int nvtexture=obj.nvtexture;
  t_group *group=obj.group;
  int g,i,f;
  printf("ngroup=%d nvertex=%d nvnormal=%d nvtexture=%d\n",ngroup,nvertex,nvnormal,nvtexture);
  for(i=1;i<=nvertex;i++){
    printf("v[%d]=%f %f %f c=%f %f %f\n",i,obj.v[i].p[0],obj.v[i].p[1],obj.v[i].p[2],
	   obj.v[i].c[0],obj.v[i].c[1],obj.v[i].c[2]);
  }
  for(i=1;i<=nvnormal;i++){
    printf("vn[%d]=%f %f %f\n",i,obj.vn[i].n[0],obj.vn[i].n[1],obj.vn[i].n[2]);
  }
  for(i=1;i<=nvtexture;i++){
    printf("vt[%d]=%f %f\n",i,obj.vt[i].t[0],obj.vt[i].t[1]);
  }
  for(g=0;g<ngroup;g++){
    printf("g=%d group_name=%s nv=%d nvn=%d nvt=%d\n",
	   g,group[g].group_name,group[g].nv,group[g].nvn,group[g].nvt);
    for(i=0;i<group[g].nfacegroup;i++){
      printf("g=%d fg=%d material_name=%s texture_fname=%s alpha_test=%d\n",
	     g,i,group[g].fg[i].material.material_name,group[g].fg[i].material.texture_fname,group[g].fg[i].material.alpha_test);
      printf("         Kd=%f %f %f Ks=%f %f %f Ns=%f\n",
	     group[g].fg[i].material.Kd[0],group[g].fg[i].material.Kd[1],group[g].fg[i].material.Kd[2],
	     group[g].fg[i].material.Ks[0],group[g].fg[i].material.Ks[1],group[g].fg[i].material.Ks[2],
	     group[g].fg[i].material.Ns);
      printf("         Ka=%f %f %f Ke=%f %f %f\n",
	     group[g].fg[i].material.Ka[0],group[g].fg[i].material.Ka[1],group[g].fg[i].material.Ka[2],
	     group[g].fg[i].material.Ke[0],group[g].fg[i].material.Ke[1],group[g].fg[i].material.Ke[2]);

      for(f=0;f<group[g].fg[i].nface;f++){
	printf("         f[%d]=%d/%d/%d %d/%d/%d %d/%d/%d\n",
	       f,group[g].fg[i].f[f].p[0],group[g].fg[i].f[f].t[0],group[g].fg[i].f[f].n[0],
	       group[g].fg[i].f[f].p[1],group[g].fg[i].f[f].t[1],group[g].fg[i].f[f].n[1],
	       group[g].fg[i].f[f].p[2],group[g].fg[i].f[f].t[2],group[g].fg[i].f[f].n[2]);
      }
    }
  }
}

void write_obj(t_obj obj, const char *ofname, float scale, float sx, float sy, float sz)
{
  int ngroup,nvertex,nvnormal,nvtexture;
  t_group *group=obj.group;
  FILE *fp;
  int g,v,fg,f;
  if((fp=fopen(ofname,"w"))==NULL){
    fprintf(stderr,"** error : can't open %s for output **\n",ofname);
    exit(1);
  }
  nvertex=nvnormal=nvtexture=0;
  ngroup=obj.ngroup;
  fprintf(fp,"mtllib %s\n",material_fname);
  for(g=0;g<ngroup;g++){
    if(ngroup==1) fprintf(fp,"g %s\n",group[g].group_name);
    for(v=1; v<=group[g].nv; v++){
      nvertex++;
      fprintf(fp,"v %f %f %f\n",
	      obj.v[nvertex].p[0] * scale + sx,
	      obj.v[nvertex].p[1] * scale + sy,
	      obj.v[nvertex].p[2] * scale + sz);
    }
    for(v=1; v<=group[g].nvn; v++){
      nvnormal++;
      fprintf(fp,"vn %f %f %f\n",
	      obj.vn[nvnormal].n[0],
	      obj.vn[nvnormal].n[1],
	      obj.vn[nvnormal].n[2]);
    }
    for(v=1; v<=group[g].nvt; v++){
      nvtexture++;
      fprintf(fp,"vt %f %f\n",
	      obj.vt[nvtexture].t[0],
	      obj.vt[nvtexture].t[1]);
    }
    if(ngroup!=1) fprintf(fp,"g %s\n",group[g].group_name);
    for(fg=0; fg<group[g].nfacegroup; fg++){
      t_material *material=&group[g].fg[fg].material;
      fprintf(fp,"usemtl %s\n",material->material_name);
      for(f=0; f<group[g].fg[fg].nface; f++){
	t_face *tf=&group[g].fg[fg].f[f];
	fprintf(fp,"f %d/%d/%d %d/%d/%d %d/%d/%d\n",
		tf->p[0],tf->t[0],tf->n[0],
		tf->p[1],tf->t[1],tf->n[1],
		tf->p[2],tf->t[2],tf->n[2]);
      }
    }
  }
  fclose(fp);
}

static void gaiseki(float a[3], float b[3], float c[3])
{
  c[2]=a[0]*b[1]-a[1]*b[0];
  c[0]=a[1]*b[2]-a[2]*b[1];
  c[1]=a[2]*b[0]-a[0]*b[2];
}

static void normalize(float a[3], float b[3])
{
  int i;
  float size=0.0;
  for(i=0; i<3; i++) size += a[i]*a[i];
  size = 1.0f/sqrtf(size);
  for(i=0; i<3; i++) b[i] = a[i] * size;
}

void generate_normal(t_obj *obj)
{
  int ngroup=obj->ngroup;
  t_group *group=obj->group;
  int g,v,fg,f,j,xyz,nface;

  if(obj->nvnormal!=0) return;
  printf("generating normal vectors\n");
  
  // count total number of faces
  obj->nvnormal=0;
  for(g=0;g<ngroup;g++){
    for(fg=0; fg<group[g].nfacegroup; fg++){
      obj->nvnormal+=group[g].fg[fg].nface;
    }
  }
  if((obj->vn=(t_vnormal *)malloc(sizeof(t_vnormal)*(obj->nvnormal+1)))==NULL){
    fprintf(stderr,"** error : can't malloc vnormal **\n");
    exit(1);
  }
  ngroup=obj->ngroup;
  //  nface=0;
  for(g=0;g<ngroup;g++){
    group[g].nvn = 0;
    for(fg=0; fg<group[g].nfacegroup; fg++){
      t_material *material=&group[g].fg[fg].material;
      group[g].nvn += group[g].fg[fg].nface;
      for(f=0; f<group[g].fg[fg].nface; f++){
	t_face *tf=&group[g].fg[fg].f[f];
	float a[3],b[3],c[3];
	nface++;
	for(j=0; j<3; j++){
	  a[j] = obj->v[ tf->p[1] ].p[j] - obj->v[ tf->p[0] ].p[j];
	  b[j] = obj->v[ tf->p[2] ].p[j] - obj->v[ tf->p[0] ].p[j];
	}
	gaiseki(a,b,c);
	normalize(c,c);
	for(j=0; j<3; j++){
	  tf->n[j] = nface;
	  for(xyz=0; xyz<3; xyz++){
	    obj->vn[ tf->n[j] ].n[xyz] = c[xyz];
	  }
	}
      }
    }
  }
}

#ifdef MAIN
int main(int argc, char **argv)
{
  t_obj obj;

  if(argc<2){
    printf("usage :\n");
    printf("  print obj file\n");
    printf("    - %s obj_file_name\n",argv[0]);
    printf("  add normal to obj file\n");
    printf("    - %s read_obj_file_name write_obj_file_name\n",argv[0]);
    printf("    - mtl file is not generated\n");
    printf("  scale obj file\n");
    printf("    - %s read_obj_file_name write_obj_file_name scale\n",argv[0]);
    printf("    - mtl file is not generated\n");
    printf("  shift obj file\n");
    printf("    - %s read_obj_file_name write_obj_file_name shift_x shift_y shift_z\n",argv[0]);
    printf("    - mtl file is not generated\n");
    exit(1);
  }

  read_obj2(argv[1],&obj);

  if(argc==6){         // output shifted obj file
    float sx,sy,sz;
    sscanf(argv[3],"%f",&sx);
    sscanf(argv[4],"%f",&sy);
    sscanf(argv[5],"%f",&sz);
    write_obj(obj,argv[2],1.0f,sx,sy,sz);
  } else if(argc==4){  // output scaled obj file
    float scale;
    sscanf(argv[3],"%f",&scale);
    write_obj(obj,argv[2],scale,0.0f,0.0f,0.0f);
  } else if(argc==3){  // output with normal vector
    generate_normal(&obj);
    write_obj(obj,argv[2],1.0f,0.0f,0.0f,0.0f);
  } else if(argc==2){  // print obj file
    print_obj(obj);
  }
  
  return 0;
}
#endif
