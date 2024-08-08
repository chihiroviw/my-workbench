use rand::prelude::*;

#[derive(Debug,Clone,Copy)]
pub struct Vec3{
    e:[f32;3],
}

impl Vec3{
    pub fn new(e0:f32,e1:f32,e2:f32)->Vec3{
        Vec3{e:[e0,e1,e2]}
    }
    pub fn x(&self)->f32 {self.e[0]}
    pub fn y(&self)->f32 {self.e[1]}
    pub fn z(&self)->f32 {self.e[2]}
    pub fn length(&self)->f32{self.length_squared().sqrt()}
    pub fn length_squared(&self)->f32{
        self.x()*self.x()+self.y()*self.y()+self.z()*self.z()
    }
}

impl std::ops::Index<usize> for Vec3{ // == *v.index(1)
    type Output = f32;
    fn index(&self, index: usize) -> &Self::Output {
        &(self.e[index])
    }
}

impl std::ops::IndexMut<usize> for Vec3{ // == *v.index(1)
    fn index_mut(&mut self, index: usize) -> &mut Self::Output {
       &mut self.e[index] 
    }
}

impl std::ops::Neg for Vec3{
    type Output = Vec3;
    #[inline]
    fn neg(self)->Self::Output{
        Vec3{e:[
            -self.x(),
            -self.y(),
            -self.z()
        ]}
    }
}

impl std::ops::Add<Vec3> for Vec3{
    type Output = Vec3;
    #[inline]
    fn add(self, rhs:Vec3)->Self::Output{
        Vec3{e:[
            self.x()+rhs.x(),
            self.y()+rhs.y(),
            self.z()+rhs.z()
        ]}
    }
}

impl std::ops::AddAssign<Vec3> for Vec3{
    #[inline]
    fn add_assign(&mut self, rhs: Vec3) {
        self.e[0] += rhs.e[0];
        self.e[1] += rhs.e[1];
        self.e[2] += rhs.e[2];
    }
}

impl std::ops::Sub<Vec3> for Vec3{
    type Output = Vec3;
    #[inline]
    fn sub(self, rhs:Vec3)->Self::Output{
        Vec3{e:[
            self.x()-rhs.x(),
            self.y()-rhs.y(),
            self.z()-rhs.z()
        ]}
    }
}

impl std::ops::SubAssign<Vec3> for Vec3{
    #[inline]
    fn sub_assign(&mut self, rhs: Vec3) {
        self.e[0] -= rhs.e[0];
        self.e[1] -= rhs.e[1];
        self.e[2] -= rhs.e[2];
    }
}

impl std::ops::Mul<f32> for Vec3{
    type Output = Vec3;
    #[inline]
    fn mul(self, rhs:f32)->Self::Output{
        Vec3{e:[
            self.x()*rhs,
            self.y()*rhs,
            self.z()*rhs
        ]}
    }
}

impl std::ops::Mul<Vec3> for Vec3{
    type Output = Vec3;
    #[inline]
    fn mul(self, rhs:Vec3)->Self::Output{
        Vec3{e:[
            self[0]*rhs[0],
            self[1]*rhs[1],
            self[2]*rhs[2]
        ]}
    }
}

impl std::ops::MulAssign<f32> for Vec3{
    #[inline]
    fn mul_assign(&mut self, rhs: f32) {
        self.e[0] *= rhs;
        self.e[1] *= rhs;
        self.e[2] *= rhs;
    }
}

impl std::ops::Div<f32> for Vec3{
    type Output = Vec3;
    #[inline]
    fn div(self, rhs:f32)->Self::Output{
        Vec3{e:[
            self.x()/rhs,
            self.y()/rhs,
            self.z()/rhs
        ]}
    }
}
/*
impl std::ops::Div<f32> for &Vec3{
    type Output = Vec3;
    fn div(self, rhs:f32)->Vec3{
        Vec3{e:[
            self.x()/rhs,
            self.y()/rhs,
            self.z()/rhs
        ]}
    }
}*/
impl std::ops::DivAssign<f32> for Vec3{
    #[inline]
    fn div_assign(&mut self, rhs: f32) {
        self.e[0] /= rhs;
        self.e[1] /= rhs;
        self.e[2] /= rhs;
    }
}

#[inline]
pub fn dot(u:Vec3, v:Vec3)->f32{
    u.x()*v.x() + u.y()*v.y() + u.z()*v.z()
}

#[inline]
pub fn cross(u:Vec3, v:Vec3)->Vec3{
    Vec3::new(
        u.e[1]*v.e[2]-u.e[2]*v.e[1],
        u.e[2]*v.e[0]-u.e[0]*v.e[2],
        u.e[0]*v.e[1]-u.e[1]*v.e[0])
}

#[inline]
pub fn unit_vector(v:Vec3)->Vec3{
    v/v.length()
}

pub fn random_vec()->Vec3{
    let mut rng = rand::thread_rng();
    Vec3::new(rng.gen_range(-1.0..1.0),
                rng.gen_range(-1.0..1.0),
                rng.gen_range(-1.0..1.0)) 
}

pub fn random_unit_vec()->Vec3{
    let v3 = random_vec();
    unit_vector(v3) 
}

pub fn random_on_hemisphere(normal:Vec3)->Vec3{
    let on_unit_sphere = random_unit_vec();
    if(0. < dot(on_unit_sphere,normal)){
        on_unit_sphere
    }else{
        -on_unit_sphere
    }
}

pub fn vec3<T:Into<f64>>(a:T,b:T,c:T)->Vec3{

    Vec3::new(a.into() as f32,b.into() as f32,c.into() as f32)
}