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

//RNG from code by Moroz Mykhailo (https://www.shadertoy.com/view/wltcRS)
//internal RNG state 
uvec4 seed;

void init_rand(vec2 p, int frame)
{
  seed = uvec4(p, uint(frame), uint(p.x) + uint(p.y));
}

void pcg4d(inout uvec4 v)
{
  v = v * 1664525u + 1013904223u;
  v.x += v.y * v.w; v.y += v.z * v.x; v.z += v.x * v.y; v.w += v.y * v.z;
  v = v ^ (v >> 16u);
  v.x += v.y * v.w; v.y += v.z * v.x; v.z += v.x * v.y; v.w += v.y * v.z;
}

float rand()
{
  pcg4d(seed); 
  return float(seed.x) / float(0xffffffffu);
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
  float min_t = 0.0001;
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
    vec3 albedo = material.albedo.rgb;
    vec3 emission = material.emission.rgb;
    float smoothness = material.albedo.w;

    vec3 point = hit.point + hit.normal * 0.001;

    vec3 diffuse = cosine_weighted(hit.normal);
    vec3 specular = reflect(ray.direction, hit.normal);

    ray.origin = point;
    ray.direction = mix(diffuse, specular, smoothness);

    float cos_theta = dot(hit.normal, -ray.direction);

    vec3 brdf = albedo / PI;
    float pdf = cos_theta / PI;

    throughput *= (brdf * cos_theta / pdf); 
    radiance += material.emission.rgb * throughput;
  }

  return radiance;
}

void main() 
{
  ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);

  frag_coord = vec2(pixel_coords);
  vec2 resolution = vec2(imageSize(image));

  init_rand(frag_coord, u_frames);

  aspect_ratio = resolution.y / resolution.x;

  vec3 previous;

  if (u_reset_flag) {
    previous = vec3(0);
  } else {
    previous = imageLoad(image, pixel_coords).rgb;
  }
  
  vec2 xy = (frag_coord / resolution) * 2.0 - 1.0;

  Ray ray = camera_ray(xy);  

  vec3 color = vec3(0);

  for (int s = 0; s < u_samples; s++)
  {
    color += trace_path(ray);
  }

  color /= float(u_samples);

  vec3 color_sum = previous * float(u_frames);
  vec3 final_color = (color + color_sum) / (u_frames + 1);
  vec4 pixel = vec4(final_color, 1);


  imageStore(image, pixel_coords, pixel);
}