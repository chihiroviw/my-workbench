use super::ray::*;
use super::vec3::*;
use super::object::*;
use std::{fs::File, io::Write};
use std::f32::consts::PI;
use std::sync::Mutex;
use rand::prelude::*;
use rayon::prelude::*;

//return pixel position
struct PixelIter{
    x:usize,y:usize,
    w:usize,h:usize,
}
impl Iterator for PixelIter{
    type Item = (usize, usize);
    fn next(&mut self) -> Option<Self::Item> {
        let pos = (self.x,self.y);

        if(pos == (self.w-1, self.h-1)){return None;}

        self.x += 1;
        if(self.x == self.w){
            self.x = 0;
            self.y += 1;
        }
        Some(pos)
    }
}



//return ray vector and pixel position
struct RayIter{
    pixeliter:PixelIter,
    ray_du:Vec3,
    ray_dv:Vec3,
    ray_00:Vec3,
    cam_center:Vec3,
}
impl Iterator for RayIter {    
    type Item = (usize, usize, Ray);
    fn next(&mut self) -> Option<Self::Item> {
        let p = self.pixeliter.next();
        match p {
            Some((x,y)) => {
                let pixel_center 
                    = self.ray_00 + self.ray_du*x as f32 + self.ray_dv*y as f32;
                let ray_dir = pixel_center - self.cam_center;

                let ray = Ray::new(self.cam_center,ray_dir);
                Some((x,y,ray))
            },
            None=>{None}
        }
    } 
}


//camera
pub struct Camera{
    //frame buffer info0.5
    aspect_ratio:f32,
    image_height:i32,
    image_width:i32,
    vfov:f32,

    //viewport info
    focal_length:f32,
    viewport_height:f32,
    viewport_widht:f32,

    //ray generate info
    cam_center:Vec3,
    viewport_u:Vec3, //horizon
    viewport_v:Vec3, //v
    ray_du:Vec3,
    ray_dv:Vec3,
    ray_00:Vec3,
    viewport_upper_left:Vec3,


    //frame buffer
    buffer:Vec<u8>,
}


impl Camera{
    pub fn new( i_width:i32, 
                aspect_ratio:f32,
                lookfrom:Vec3,
                lookat:Vec3,
                vup:Vec3,
                v_fov:f32)->Camera{

        let mut i_height = (i_width as f32/aspect_ratio)as i32;
        if(i_height < 1){i_height=1;}


        let fl = (lookfrom-lookat).length();

        let theta = v_fov * PI/180.0; //degrees2rad
        let h:f32 = (theta/2.0).tan();
        let v_height:f32 = 2.0*h*fl;
        let v_width:f32 = v_height*(i_width as f32 / i_height as f32);


        //calc viewport and ray param
        let cc = lookfrom;

        let w = unit_vector(lookfrom - lookat);
        let u = unit_vector(cross(vup, w));
        let v = cross(w,u);

        let vu = u*v_width;
        let vv = -v*v_height;

        let r_du = vu/i_width as f32;
        let r_dv = vv/i_height as f32;

        let vul = cc-w*fl-vu/2.-vv/2.;
        let p00 = vul+r_du/2.+ r_dv/2.;

        Camera{
            aspect_ratio:aspect_ratio,
            image_width:i_width,
            image_height:i_height,
            vfov:v_fov,

            focal_length:fl,
            viewport_height:v_height,
            viewport_widht:v_width,

            cam_center:cc,
            viewport_u:vu,
            viewport_v:vv,
            ray_du:r_du,
            ray_dv:r_dv,
            viewport_upper_left:vul,
            ray_00:p00,
            
            buffer:vec![0;(i_width*i_height*3)as usize]
        }  
    }

    pub fn write_file(&self){
        let mut file = File::create("./image.ppm").unwrap();
        let header = format!("P3\n{} {}\n255\n",self.image_width,self.image_height);
        file.write(header.as_bytes());

        for i in 0..self.image_height*self.image_width{
            let color 
                = format!("{} {} {}\n",
                        self.buffer[i as usize *3],
                        self.buffer[i as usize *3+1],
                        self.buffer[i as usize *3+2]);
            file.write(color.as_bytes());
        }
    }

    fn pixles(&self)->PixelIter{ //iterator
        PixelIter{
            x:0,y:0,
            w:self.image_width as usize, h:self.image_height as usize,
        }
    }

    fn rays(&self)->RayIter{ //iterator
        RayIter { 
            pixeliter:self.pixles(),    
            ray_du:self.ray_du,
            ray_dv:self.ray_dv,
            ray_00:self.ray_00,
            cam_center:self.cam_center,
        }
    }

    fn linear_to_gamma(x:f32)->f32{
        x.sqrt()
    }

    fn draw_point(&mut self, x:usize, y:usize, c:Vec3){
        let pos = (y*(self.image_width as usize)+x)*3;
        self.buffer[pos+0]=(Camera::linear_to_gamma(c[0])*255.999) as u8;
        self.buffer[pos+1]=(Camera::linear_to_gamma(c[1])*255.999) as u8;
        self.buffer[pos+2]=(Camera::linear_to_gamma(c[2])*255.999) as u8;
    }

    fn pixel_sample_square(rdu:&Vec3,rdv:&Vec3)->Vec3{
        let mut rng = rand::thread_rng();
        let dx = *rdu*(rng.gen::<f32>()-0.5);
        let dy = *rdv*(rng.gen::<f32>()-0.5);
        dx+dy
    }

    fn get_random_ray(x:usize, y:usize, r00:&Vec3,rdu:&Vec3,rdv:&Vec3,cc:&Vec3)->Ray{
        let pixel_center 
            = *r00 + *rdu*x as f32 + *rdv*y as f32;
        let ray_dir 
            = pixel_center - *cc + Camera::pixel_sample_square(rdu,rdv);
        
        Ray::new(*cc,ray_dir)
    }

    //use surface info to calculate reflect ray color
    fn ray_color(ray:Ray, scenes:&Scenes, reflect_num:u32)->Vec3{

        let t_range = Interval{min:1e-4,max:f32::MAX};
        let mut attenuation_rate= Vec3::new(1.,1.,1.);
        let mut reflected_ray = ray;
        let mut color = Vec3::new(0.0,0.0,0.0); 

        for _ in 0..reflect_num{
            match scenes.intersection(&reflected_ray, t_range){
                Some(hs @ HitStatus{mat:Material::Light{..},..})=>{
                    color = attenuation(&hs, &reflected_ray);
                    break;
                },
                Some(hs)=>{
                    let rt = scatter(&hs, &reflected_ray);
                    let at = attenuation(&hs, &reflected_ray);
                    attenuation_rate = attenuation_rate*at;
                    reflected_ray = rt; 
                },
                None=>{break;},
            }
            if(attenuation_rate.length_squared()<1e-4){break;};
        }
        color*attenuation_rate
    }

    fn pixel_color_parallel(
        x:usize,y:usize,
        r00:Vec3,rdu:Vec3,rdv:Vec3,cc:Vec3,
        scenes:&Scenes,spp:u32,reflect_num:u32)->Vec3{

        let mut color = Vec3::new(0.,0.,0.);
        for _ in 0..spp{
            let ray = Camera::get_random_ray(x,y,&r00,&rdu,&rdv,&cc);
            color += Camera::ray_color(ray, &scenes,reflect_num);
        }
        color / spp as f32 

        /* 
        let mut vc:Vec<Vec3> = Vec::with_capacity(spp as usize);
        for _ in 0..spp{
            let ray = Camera::get_random_ray(x,y,&r00,&rdu,&rdv,&cc);
            let color = Camera::ray_color(ray, &scenes,reflect_num);
            vc.push(color);
        }
        vc.sort_by(|a, b|{
            a.length_squared().partial_cmp(&b.length_squared()).unwrap()
        }); 

        let mut color = vec3(0,0,0);
        for i in spp/4..spp*3/4{
            color += vc[i as usize];
        }
        color / ((spp/2) as f32)
        */
    }

    fn draw_point_prallel(x:usize, band:&mut [u8],c:Vec3){
        let pos = x*3;
        band[pos+0]=(Camera::linear_to_gamma(c[0])*255.999) as u8;
        band[pos+1]=(Camera::linear_to_gamma(c[1])*255.999) as u8;
        band[pos+2]=(Camera::linear_to_gamma(c[2])*255.999) as u8;       
    }
    
    pub fn render_parallel(&mut self, scenes:&Scenes, 
                            spp:u32, 
                            reflect_num:u32){
         
        let bands: Vec<(usize, &mut [u8])>
                = self.buffer
                .chunks_mut((self.image_width*3) as usize)
                .enumerate()
                .collect();
               
        let r00 = self.ray_00;
        let (rdu,rdv) = (self.ray_du,self.ray_dv);
        let cc =  self.cam_center;
        let h = self.image_height as f32;
        let w = self.image_width as usize;

        //rayonはメインスレッドが生きていることを保証できるから&scenesでおｋ
        //thread::spawnだとメインスレッドの死ぬ可能性を考えて、基本的に参照をクロージャに渡せない
        //Arc必須 
        let mut next_border = Mutex::new(1.0 as f32);
        let mut cp = Mutex::new(0.0 as f32);
        bands.into_par_iter()
            .for_each(|(y,band)|{
                for x in 0..w{ 
                    let c 
                        = Camera::pixel_color_parallel(x, y, r00, rdu, rdv, cc,scenes, spp, reflect_num);                 
                    Camera::draw_point_prallel(x, band, c);
                }

                let mut lcp = cp.lock().unwrap();
                let mut lnb = next_border.lock().unwrap();
                *lcp += 1.0;
                let complet_rate = *lcp/h*100.0;
                if(*lnb < complet_rate){
                    *lnb += 1.0;
                    println!("{:>02.0}/% completed.",complet_rate);
                }
            });  


        println!("rendered!\n");
    }
}
