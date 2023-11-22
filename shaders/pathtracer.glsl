#version 430

#define PI 3.1415

struct Sphere {
  vec3 center;
  float radius;
  int material; 
};

struct Material {
  vec4 albedo;
  vec4 emission;
};

layout(local_size_x = 8, local_size_y = 8) in;

layout(rgba32f, binding = 0) uniform image2D image;

layout(std430, binding = 1) readonly buffer sphere_buffer {
  Sphere spheres[];
};

layout(std140, binding = 2) readonly buffer material_buffer {
  Material materials[];
};

uniform int u_frames;
uniform int u_samples;
uniform int u_max_bounce;
uniform float u_time;
uniform bool u_reset_flag;
uniform vec3 u_background;

uniform vec3 u_camera_position;
uniform float u_camera_fov;

uniform vec3 u_camera_forward;
uniform vec3 u_camera_up;
uniform vec3 u_camera_right;

vec2 frag_coord;
float aspect_ratio;
float rng = u_time; // random number generator state

struct Ray {
  vec3 origin;
  vec3 direction;
};

struct HitInfo {
  float t;
  vec3 point;
  vec3 normal;
};

float PHI = 1.61803398874989484820459;  // Φ = Golden Ratio

float random_float(in vec2 xy, in float seed){
  return fract(tan(distance(xy*PHI, xy)*seed)*xy.x);
}

float rand() {
  rng += 0.1;
  return random_float(frag_coord, rng);
}

vec3 random_in_sphere()
{
    float z = rand() * 2.0 - 1.0;
    float a = rand() * 2.0 * PI;
    float r = sqrt(1.0f - z * z);
    float x = r * cos(a);
    float y = r * sin(a);
    return vec3(x, y, z);
}

vec3 cosine_weighted(vec3 normal) {
    return normalize(normal + random_in_sphere()); 
}

Ray camera_ray(vec2 xy)
{
  vec3 forward = u_camera_forward;
  vec3 right = u_camera_right;
  vec3 up = u_camera_up;

  float half_width = tan(u_camera_fov / 2);
  float half_height = half_width * aspect_ratio;

  float width = 2.0 * half_width;
  float height = 2.0 * half_height;

  vec3 target = u_camera_position + forward;

  vec3 view_point = target + (right * width * xy.x) + (up * height * xy.y);

  return Ray(u_camera_position, normalize(view_point - u_camera_position));
}

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

int find_collision(Ray ray, inout HitInfo closest_hit)
{
  float min_t = 0.01;
  float max_t = 10000.0;
  int closest_sphere = -1;

  for (int i = 0; i < spheres.length(); i++)
  {
    HitInfo hit;
    if (intersect(ray, spheres[i], min_t, max_t, hit))
    {
      if (hit.t < max_t)
      {
        closest_hit = hit;
        max_t = hit.t;
        closest_sphere = i;
      }
    }
  }

  return closest_sphere;
}

vec3 trace_path(Ray ray) 
{
  vec3 radiance = vec3(0.0);
  vec3 throughput = vec3(1.0);

  for (int bounce = 0; bounce < u_max_bounce; bounce++)
  {
    HitInfo hit;

    int i = find_collision(ray, hit);

    if (i == -1) {
      radiance += u_background * throughput;
      break;
    }


    Sphere sphere = spheres[i];
    Material material = materials[sphere.material];

    vec3 point = hit.point + hit.normal * 0.001;
    vec3 wi = cosine_weighted(hit.normal);
    ray = Ray(point, wi);

    throughput *= material.albedo.rgb; 
    radiance += material.emission.rgb * throughput;
  }

  return radiance;
}

void main() 
{
  ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);

  frag_coord = vec2(pixel_coords);
  vec2 resolution = vec2(imageSize(image));

  aspect_ratio = resolution.y / resolution.x;

  vec3 previous = imageLoad(image, pixel_coords).rgb;

  if (u_reset_flag)
  {
    previous = vec3(0);
  }
  
  vec2 xy = (frag_coord.xy - 0.5 * resolution.xy) / resolution.y;
  //vec2 xy = (frag_coord.xy - 0.5 * resolution.xy);

  Ray ray = camera_ray(xy);  

#if 0
  int tmp = int(u_frames) % int(1000);
  vec3 col = vec3(uv.x, uv.y, float(tmp) / float(1000));
  vec4 pixel = vec4(col, 1);

#else

  vec3 color = vec3(0);

  for (int s = 0; s < u_samples; s++)
  {
    color += trace_path(ray);
  }

  color /= float(u_samples);

  vec3 color_sum = previous * float(u_frames);
  vec3 final_color = (color + color_sum) / (u_frames + 1);
  vec4 pixel = vec4(final_color, 1);
#endif

#if 0
  pixel = vec4(
    rand(),
    rand(),
    rand(),
    1.0
  );
#endif

  imageStore(image, pixel_coords, pixel);
}