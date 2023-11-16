
struct Sphere {
    vec3 center;
    float radius;
};

struct Ray {
    vec3 origin;
    vec3 direction;
};

struct HitInfo {
    float t;
    vec3 point;
    vec3 normal;
};

bool intersect(Ray ray, Sphere sphere, float min_t, float max_t, inout HitInfo info) 
{
    vec3 m = ray.origin - sphere.center;
    float b = dot(m, ray.direction);
    float c = dot(m, m) - sphere.radius * sphere.radius;
    
    if( 0.0 < c && 0.0 < b ) {
        return false;
    }
    
    float discr = b * b - c;
    if (discr < 0.0 ) {
        return false;
    }
    
    float t = -b - sqrt(discr);
    if (t < 0.0) {
        t = -b + sqrt(discr);
    }
    
    if (min_t < t && t < max_t) {
        info.t = t;
        info.point = ray.origin + ray.direction * t;
        info.normal = (info.point - sphere.center) / sphere.radius;
        return true;
    } else {
        return false;
    }
}

