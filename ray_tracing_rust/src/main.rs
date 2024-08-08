mod rt_weekend;
use rt_weekend::object::*;
use rt_weekend::vec3::*;
use rt_weekend::*;

use std::ops::Deref;
use std::time::{Duration, Instant};
fn main() {
    let mut scenes = Scenes::new();
    //test_scenes(&mut scenes);
    //many_balls_scene(&mut scenes);
    cornell_box(&mut scenes);

    let mut cam = camera::Camera::new(
        400, 
        (1.0),
        //Vec3::new(13.,2.,3.),
        vec3(278,278,-800),
        vec3(278,278,0),
        vec3(0,1,0),
        40.);

    cam.render_parallel(&scenes,1000,16);
    cam.write_file();
    
    /* 
    let mut dfdx = dfdx::init();
    loop{
        let mut c = String::new();
        std::io::stdin().read_line(&mut c).ok();
        match my_parse(&c){
            Some(B) => {
                let out = dfdx.switch(B);
                println!("out : {}",out);
            },
            _ => {println!("none");break;},
        } 
    }*/
    let inta = 123;
}


/*

fn test0(ref i:i32){参照を束縛できるよ！！！}


T -> &A を定義しておけば、&T -> &A も一緒に定義される
自動型変換はしてくれないので、自分でas_refを呼んでね
fn as_ref(&self)->&T
    a.as_ref()  ==   &a


デリファレンスだが、基本的には別の型へ変換した参照を返す
*演算子で呼んだ時だけ、deref()+*で参照が戻される
暗黙的な型変換(型強制)ではderef()メソッドが単体で呼ばれて参照が渡される
&a -> &*a　のような変則的な暗黙型変換もする(要は、&a -> a.deref() をしてる)
fn deref(&self)->&T
    *a          == *(a.deref())
    &*a         ==  a.deref()

//exsamples

    let a = Myi32{i:123};
    let b:&i32 = a.deref();
    let b:&i32 = (&*a);
    let b:i32 = *a;
    let b:i32 = *(a.deref());
    let b:&i32 = &a;
    

struct Myi32{i:i32,}
impl Deref for Myi32{
    type Target = i32;
    fn deref(&self) -> &Self::Target {
        &self.i
    }
}
*/


fn test_scenes(scenes:&mut Scenes){
    let material_grand= lambertian(0.8,0.8,0.0);
    let material_center= lambertian(0.7,0.3,0.3);
    let material_left= metal(0.8,0.8,0.8);
    let material_right= metal(0.7,0.6,0.2);
    let material_glass= dielectric(1.5);

    let material_light= light(4.0,4.0,4.0);
       
    let s0 = sphere(0.,0.,-1.,0.5,material_glass);
    let s4 = sphere(-0.5,0.2,-2.3,0.5,material_center);
    let s1 = sphere(-1.,0.,-1.,0.5,material_center);
    let s2 = sphere(1.,0.,-1.,0.5, material_light);
    let s5 = sphere(0.,1.0,-1.,0.5, material_light);
    let s3 = sphere(0.,-100.5,-1.,100.,material_grand);

    let red = lambertian(1.0, 0.2, 0.2);
    let quad = square(vec3(-3,-2,5), vec3(0,0,-4), vec3(0,4,0), material_light);

    scenes  .add(s0)
            .add(s1)   
            .add(s2)   
            .add(s3)   
            .add(s4)
            .add(s5)
            .add(quad);
}

fn many_balls_scene(scenes:&mut Scenes){
    let grand_material = lambertian(0.5, 0.5, 0.5);
    scenes.add(sphere(0., -1000., 0., 1000., grand_material));
    
    for a in -11..11{
        for b in -11..11{
            let choose_mat:f32 = rand::random();
            let center 
                = Vec3::new((a as f32)+0.9*rand::random::<f32>(),0.2,(b as f32)+0.9*rand::random::<f32>());

            if(choose_mat < 0.4){//deffuse
                let c = random_unit_vec()*random_unit_vec();
                let mat = lambertian(c[0], c[1], c[2]);
                scenes.add(sphere(center[0], center[1], center[2], 0.2, mat));

            }else if(choose_mat < 0.7){//metal
                let mut c = random_unit_vec();
                for i in 0..3{if c[i]<0.5 {c[i] += 0.5;}}

                let mat = metal(c[0],c[1],c[2]);
                scenes.add(sphere(center[0], center[1], center[2], 0.2, mat));

            }else{//glass
                let mat = dielectric(1.5);
                scenes.add(sphere(center[0], center[1], center[2], 0.2, mat));
            }
        }
    }

    let mat = dielectric(1.5);
    scenes.add(sphere(0., 1., 0., 1., mat));

    let mat = lambertian(0.4, 0.2, 0.1);
    scenes.add(sphere(-4., 1., 0., 1., mat));

    let mat = metal(0.7, 0.6, 0.5);
    scenes.add(sphere(4., 1., 0., 1., mat));
}

fn cornell_box(scenes:&mut Scenes){
    let red = lambertian(0.65, 0.05, 0.05);
    let white = lambertian(0.73, 0.73, 0.73);
    let green = lambertian(0.12, 0.45, 0.15);
    let light = light(32.0,32.0,32.0);
    
    let glass= dielectric(1.5);
    let metal = metal(0.9,0.9,0.9);

    let s0 = square(vec3(555,0,-555), vec3(0,555,0), vec3(0,0,5550), green);
    let s1 = square(vec3(0,0,-555), vec3(0,555,0), vec3(0,0,5550), red);
    let s2 = square(vec3(343,554,332), vec3(-130,0,0), vec3(0,0,-105), light);
    let s3 = square(vec3(0,0,-555), vec3(555,0,0), vec3(0,0,5550), white);//grand
    let s4 = square(vec3(555,555,-555), vec3(-555,0,0), vec3(0,0,5550), white);//cell
    let s5 = square(vec3(0,0,555), vec3(555,0,0), vec3(0,555,0), white);

    let ball = sphere(230.0, 150.0, 130.0, 100.0, glass);

    let cuboid0 = cuboid(vec3(0,0,0), vec3(165,330,165), metal);
    let rotate_y_cuboid0 = Rotate_y::new(cuboid0,15.0);
    let translate_cuboid0 = Translate::new(rotate_y_cuboid0,vec3(265,0,295));


    let yellow = lambertian(0.8, 0.8, 0.15);
    let cuboid1 = cuboid(vec3(0,0,0), vec3(165,400,165), yellow);
    let rotate_y_cuboid1 = Rotate_y::new(cuboid1,-20.0);
    let translate_cuboid1 = Translate::new(rotate_y_cuboid1,vec3(100,0,295));

    scenes  .add(s0)
            .add(s1) 
            .add(s2) 
            .add(s3) 
            .add(s4)
            .add(s5)
            .add(ball)
            .add(translate_cuboid0)
            .add(translate_cuboid1); 
}