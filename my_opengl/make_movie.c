#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

cv::VideoWriter writer;
int Width, Height;
CvMat *Image;

//#define MAIN

extern "C"
void make_movie_initialize(const char *fname, int width, int height)
{
  int fourcc, fps=30;
  char c[4] = {'M', 'J', 'P', 'G'};

  fourcc = (c[0] & 255) + ((c[1] & 255) << 8) 
    + ((c[2] & 255) << 16) + ((c[3] & 255) << 24); 
  writer.open(fname, fourcc, fps, cv::Size(width, height));
  Image = cvCreateMat(height,width,CV_8UC3);
  Width = width;
  Height = height;
}

extern "C"
void make_movie_oneframe(unsigned char *rawdata)
{
  memcpy(Image->data.ptr,rawdata,Width*Height*3);
  cvCvtColor(Image,Image,CV_RGB2BGR);
  cvFlip(Image);
  writer << Image;
}

#ifdef MAIN
int main(int argc, char **argv)
{
  Width  = 1200;
  Height = 800;
  unsigned char *r = (unsigned char *)malloc(sizeof(unsigned char) * Width * Height * 3);
  int x, y, c, f;
  make_mp4_initialize("test.mov",Width,Height);
  for(f=0; f<300; f++){
    for(y=0; y<Height; y++){
      for(x=0; x<Width; x++){
	for(c=0; c<3; c++){
	  if(c==2) r[(y * Width + x) * 3 + c] = 255;
	  else     r[(y * Width + x) * 3 + c] = x + y + f;
	}
      }
    }
    make_mp4_oneframe(r);
  }
  return 0;
}
#endif
