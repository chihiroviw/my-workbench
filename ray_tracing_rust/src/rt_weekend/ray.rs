use super::vec3::Vec3;

pub struct Ray{
    origin:Vec3,
    direction:Vec3,
}

impl Ray{
    pub fn new(o:Vec3, d:Vec3)->Ray{
        Ray{origin:o, direction:d}
    }
    pub fn o(&self)->Vec3{self.origin}
    pub fn d(&self)->Vec3{self.direction}
    pub fn at(&self,t:f32)->Vec3{
        self.origin+self.direction*t
    }
}

