use std::clone;
use std::f32::consts::PI;


use super::vec3::*;
use super::ray::*;

pub struct HitStatus{
    pub point:Vec3,
    pub unit_normal:Vec3, //always point against the incident ray
    pub t:f32,
    pub front_face:bool,
    pub mat:Material,
}

pub trait Intersection{
    fn intersection(&self, ray:&Ray, t_range:Interval)->Option<HitStatus>;
}

#[derive(Clone,Copy)]
pub struct Interval{
    pub min:f32,
    pub max:f32,
}
impl Interval{
    pub fn contains(&self, x:f32)->bool{self.min<=x&&x<= self.max}
    pub fn surrounds(&self, x:f32)->bool{self.min<x&&x<self.max}
}


pub struct Scenes{
    objects:Vec<Box<dyn Intersection+Sync+Send>>,
}
impl Scenes {
    pub fn new()->Scenes{Scenes{objects:vec![]}}
    //pub fn add(&mut self, obj:impl Intersection+'static)->&mut Self{
    pub fn add<T:Intersection+Sync+Send+'static>(&mut self, obj:T)->&mut Self{
        self.objects.push(Box::new(obj));
        self
    }
}
impl Intersection for Scenes{
    fn intersection(&self, ray:&Ray, t_range:Interval)->Option<HitStatus> {
        let mut hit_status = None;
        let mut closest_so_far = t_range.max;

        for obj in &self.objects{
            if let Some(temp) = obj.intersection(ray, 
                    Interval {min: t_range.min, max:closest_so_far}){ 
                closest_so_far = temp.t;
                hit_status = Some(temp);
            }
        }
        hit_status
    }
}

//define material
#[derive(Clone, Copy)]
pub enum Material{
    Lambertian{albedo:Vec3},
    Metal{albedo:Vec3},
    Dielectric{ir:f32},
    Light{emit:Vec3},
}

fn reflect_metal(unit_v:Vec3,unit_n:Vec3)->Vec3{
    unit_v - unit_n*dot(unit_v,unit_n)*2.
}

fn refract_delectric(uv:Vec3, un:Vec3, etai_over_etat:f32)->Vec3{
    let cos_theta = fmin(dot(-uv,un),1.0);
    let r_out_perp = (uv+un*cos_theta)*etai_over_etat;
    let r_out_parallel 
        = -un*((1.0-r_out_perp.length_squared()).abs().sqrt());
    r_out_perp+r_out_parallel
}

pub fn scatter(hs:&HitStatus, ray_in:&Ray)->Ray{
    match hs.mat{
        Material::Lambertian{ref albedo}=>{
            let direction = hs.unit_normal+random_unit_vec();
            Ray::new(hs.point,direction)},
            
        Material::Metal{ref albedo}=>{
            let reflected 
                = reflect_metal(unit_vector(ray_in.d()),hs.unit_normal);
            Ray::new(hs.point,reflected)}, 

        Material::Dielectric {ir }=>{
            let refraction_ratio = if hs.front_face {1./ir}else{ir};//air=1.0
            let unit_d = unit_vector(ray_in.d());

            let cos_theta = fmin(dot(-unit_d,hs.unit_normal),1.0);
            let sin_theta = (1.0-cos_theta*cos_theta).sqrt();

            let mut d = Vec3::new(0.,0.,0.);
            if( 1.0<refraction_ratio*sin_theta ){ //cannot refract 
                d = reflect_metal(unit_d, hs.unit_normal)
            }else{
                d = refract_delectric(unit_d, hs.unit_normal, refraction_ratio);
            }
            Ray::new(hs.point,d)
        },

        Material::Light { ref emit }=>{
            Ray::new(Vec3::new(0.,0.,0.),
                        Vec3::new(0.,0.,0.))
        }
    }
}
pub fn attenuation(hs:&HitStatus, ray_in:&Ray)->Vec3{
    match hs.mat{
        Material::Lambertian{albedo}=>{albedo},
        Material::Metal{albedo} => {albedo}
        Material::Dielectric { ir }=>{Vec3::new(1., 1., 1.)}
        Material::Light { ref emit }=>{*emit}
    }
}



//define object

//sphere
pub struct Sphere{
    center:Vec3,
    radius:f32,
    material:Material,
}
impl Sphere {
    pub fn new(center:Vec3, radius:f32, material:Material)->Sphere{
        Sphere {center, radius, material}
    }
}
impl Intersection for Sphere{
    fn intersection(&self, ray:&Ray, t_range:Interval)->Option<HitStatus> {
        let oc = ray.o()-self.center;
        let a = ray.d().length_squared();
        let half_b = dot(oc,ray.d());
        let c = oc.length_squared() - self.radius*self.radius;

        let discriminant = half_b*half_b-a*c;
        if discriminant<0. {return None;}

        //find the nearest root that lies in the acceptable range
        let sqrt_d = discriminant.sqrt();

        // t0 <= t1
        let t0 = (-half_b-sqrt_d)/a;// -root
        let t1 = (-half_b+sqrt_d)/a;// +root
        let t = match (t_range.surrounds(t0),t_range.surrounds(t1)){
            (true, _) => {t0},
            (false, true) => {t1},
            (false, false) => {return None;},
        };

        let p = ray.at(t);
        let unit_outward_normal = (p-self.center)/self.radius;
        let ff = dot(unit_vector(ray.d()),unit_outward_normal)<0.;
        Some(HitStatus{
            point:p,
            unit_normal:if ff {unit_outward_normal}else{-unit_outward_normal},
            t,
            front_face:ff,
            mat:self.material})
    }
}

//square
pub struct Square{
    q:Vec3, u:Vec3, v:Vec3,
    unit_normal:Vec3,
    D:f32,
    w:Vec3,
    material:Material,
}
impl Square {
    pub fn new(q:Vec3, u:Vec3, v:Vec3, material:Material)->Square{
        let n = cross(u,v);
        let unit_normal = unit_vector(n);
        let D = dot(unit_normal,q);
        let w = n/dot(n,n);
        Square {q,u,v,unit_normal,D,w,material}
    }
}
impl Intersection for Square{
    fn intersection(&self, ray:&Ray, t_range:Interval)->Option<HitStatus> {
        //t = (D - d*P)/n*d   Ray(t) = P+t*d
        let nd = dot(self.unit_normal,ray.d());
        if(nd.abs() < 1e-8){return None};

        let t = (self.D-dot(self.unit_normal,ray.o()))/nd;
        if(!t_range.contains(t)){return None;}

        let p = ray.at(t);
        let plannar_hitpt_vector = p-self.q;
        let alpha = dot(self.w,cross(plannar_hitpt_vector,self.v));
        let beta = dot(self.w,cross(self.u,plannar_hitpt_vector));

        if(alpha<0. || 1.<alpha || beta<0. || 1.<beta){return None;}

        let ff = dot(ray.d(),self.unit_normal)<0.0;
        let un = if(ff){self.unit_normal}else{-self.unit_normal};
        Some(HitStatus{
            point:p,
            unit_normal:un,
            t,
            front_face:true,
            mat:self.material,
        })
    }
}


//rotate_y
pub struct Rotate_y{
    obj:Box<dyn Intersection+Sync+Send>,
    cos:f32,
    sin:f32,
}
impl Rotate_y{
    pub fn new<T:Intersection+Sync+Send+'static>(obj:T,angle:f32)->Rotate_y{
        Rotate_y{
            obj:Box::new(obj),
            cos:(angle*PI/180.0).cos(),
            sin:(angle*PI/180.0).sin(),
        }
    }
}
impl Intersection for Rotate_y{
    fn intersection(&self, ray:&Ray, t_range:Interval)->Option<HitStatus> {
        let mut o = ray.o();
        let mut d = ray.d();

        o[0] = self.cos*ray.o()[0] - self.sin*ray.o()[2];
        o[2] = self.sin*ray.o()[0] + self.cos*ray.o()[2];

        d[0] = self.cos*ray.d()[0] - self.sin*ray.d()[2];
        d[2] = self.sin*ray.d()[0] + self.cos*ray.d()[2];

        let rotated_ray = Ray::new(o,d);

        match self.obj.intersection(&rotated_ray, t_range){
            Some(mut hs)=>{
                let mut p = hs.point;
                p[0] = self.cos*hs.point[0] + self.sin*hs.point[2];
                p[2] = -self.sin*hs.point[0] + self.cos*hs.point[2];

                let mut n = hs.unit_normal;
                n[0] = self.cos*hs.unit_normal[0] + self.sin*hs.unit_normal[2];
                n[2] = -self.sin*hs.unit_normal[0] + self.cos*hs.unit_normal[2];

                hs.point = p;
                hs.unit_normal = n;

                Some(hs)
            },
            None=>{None}
        }
    }
}

//translate
pub struct Translate{
    obj:Box<dyn Intersection+Sync+Send>,
    v:Vec3,
}
impl Translate{
    pub fn new<T:Intersection+Sync+Send+'static>(obj:T,v:Vec3)->Translate{
        Translate{
            obj:Box::new(obj),
            v,
        }
    }
}
impl Intersection for Translate{
    fn intersection(&self, ray:&Ray, t_range:Interval)->Option<HitStatus> {
        let offset_ray = Ray::new(ray.o()-self.v,ray.d());


        match self.obj.intersection(&offset_ray, t_range){
            Some(mut hs)=>{
                hs.point += self.v;
                Some(hs)
            },
            None=>{None}
        }
    }
}

//utility
fn fmin(x:f32, y:f32)->f32{if x<y {x}else{y}}
fn fmax(x:f32, y:f32)->f32{if x<y {y}else{x}}

pub fn lambertian(r:f32,g:f32,b:f32)->Material{
    Material::Lambertian { albedo: Vec3::new(r,g,b) }
}
pub fn metal(r:f32,g:f32,b:f32)->Material{
    Material::Metal { albedo: Vec3::new(r,g,b) }
}
pub fn dielectric(ir:f32)->Material{
    Material::Dielectric { ir: ir }
}
pub fn light(r:f32,g:f32,b:f32)->Material{
    Material::Light{emit:Vec3::new(r,g,b)}
}

pub fn sphere(x:f32,y:f32,z:f32,r:f32,mat:Material)->Sphere{
    Sphere::new(Vec3::new(x,y,z),r,mat)
}
pub fn square(q:Vec3,u:Vec3,v:Vec3,mat:Material)->Square{
    Square::new(q,u,v,mat)
}
pub fn cuboid(a:Vec3, b:Vec3, mat:Material)->Scenes{
    let min = Vec3::new(fmin(a.x(),b.x()),fmin(a.y(),b.y()),fmin(a.z(),b.z()));
    let max = Vec3::new(fmax(a.x(),b.x()),fmax(a.y(),b.y()),fmax(a.z(),b.z()));

    let dx = Vec3::new(max.x()-min.x(),0.,0.);
    let dy = Vec3::new(0., max.y()-min.y(),0.);
    let dz = Vec3::new(0.,0.,max.z()-min.z());


    let mut cuboid = Scenes::new();
    cuboid.add(square(vec3(min.x(),min.y(),max.z()), dx, dy, mat));
    cuboid.add(square(vec3(max.x(),min.y(),max.z()), -dz, dy, mat));
    cuboid.add(square(vec3(max.x(),min.y(),min.z()), -dx, dy, mat));
    cuboid.add(square(vec3(min.x(),min.y(),min.z()), dz, dy, mat));
    cuboid.add(square(vec3(min.x(),max.y(),max.z()), dx, -dz, mat));
    cuboid.add(square(vec3(min.x(),min.y(),min.z()), dx, dz, mat));

    cuboid
}