#version 430

#define PI        3.14159265359
#define EPSILON   0.005
#define INF       1e5
#define NO_HIT    -1
#define INVALID   4294967295 // uint max

struct Sphere {
  vec3 center;
  float radius;
  int material; 
};

struct Material {
  vec4 albedo;
  vec3 emission;
  uint type;
};

struct Mesh {
  uint start;
  uint size;
  int material;
};

struct Node {
  vec4 min;
  vec4 max;
  uint left;
  uint right;
  uint offset;
  uint count;
};

layout(local_size_x = 8, local_size_y = 8) in;

layout(rgba32f, binding = 0) uniform image2D image;

layout(std140, binding = 1) readonly buffer sphere_buffer {
  Sphere spheres[];
};

layout(std140, binding = 2) readonly buffer material_buffer {
  Material materials[];
};

layout(std140, binding = 3) readonly buffer mesh_buffer {
  Mesh meshes[];
};

layout(std430, binding = 4) readonly buffer vertex_buffer {
  vec4 vertices[];  // w encodes material id
};

layout(std430, binding = 5) readonly buffer kd_tree {
  Node nodes[];
};

uniform int u_frames;
uniform uint u_samples;
uniform uint u_max_bounce;
uniform float u_time;
uniform vec3 u_background;
uniform bool u_reset_flag;
uniform bool u_use_envmap;
uniform bool u_use_dof;
uniform int u_random;

uniform samplerCube u_envmap;

uniform vec3 u_camera_position;
uniform float u_camera_fov;
uniform float u_camera_aperture;
uniform float u_camera_focal_length;

uniform vec3 u_camera_forward;
uniform vec3 u_camera_up;
uniform vec3 u_camera_right;

vec2 frag_coord;
float aspect_ratio;

struct Ray {
  vec3 origin;
  vec3 direction;
};

struct HitInfo {
  float t;
  vec3 point;
  vec3 normal;
  int material;
};

#define STACK_SIZE 5

struct Stack {
  int top;
  uint items[STACK_SIZE];
};

void init(inout Stack s) {
  s.top = -1;
}

bool is_empty(in Stack s) {
  return s.top == -1;
}

bool is_full(in Stack s) {
  return s.top == (STACK_SIZE - 1);
}

void push(inout Stack s, uint v) {
  if (!is_full(s)) {
    s.items[++s.top] = v;
  }
}

uint pop(inout Stack s) {
  if (!is_empty(s)) {
    return s.items[s.top--];
  } else {
    return INVALID;
  }
}

//RNG from code by Moroz Mykhailo (https://www.shadertoy.com/view/wltcRS)
//internal RNG state 
uvec4 seed;

void init_rand(vec2 p, int frame)
{
  seed = uvec4(p, uint(frame), uint(p.x) + uint(p.y) + uint(frame));
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

Ray camera_ray(vec2 xy) {
  vec3 forward = u_camera_forward;
  vec3 right = u_camera_right;
  vec3 up = u_camera_up;

  float half_width = tan(u_camera_fov / 2);
  float half_height = half_width * aspect_ratio;

  float width = 2.0 * half_width;
  float height = 2.0 * half_height;

  vec3 target = u_camera_position + forward;

  vec3 view_point = target + (right * width * xy.x) + (up * height * xy.y);

  vec3 direction = normalize(view_point - u_camera_position);


  if (u_use_dof) {
    vec3 jitter = random_in_sphere() * u_camera_aperture;

    vec3 origin = u_camera_position + jitter;

    vec3 focal_point = u_camera_position + direction * u_camera_focal_length;

    return Ray(origin, normalize(focal_point - origin));

  } else {
    return Ray(u_camera_position, normalize(view_point - u_camera_position));
  }
}

float sphere_intersect(Ray r, Sphere s) {
    vec3 pos = s.center;
    float rad = s.radius;
    vec3 op = pos - r.origin;
    float eps = 0.001;
    float b = dot(op, r.direction);
    float det = b * b - dot(op, op) + rad * rad;
    if (det < 0.0)
        return INF;

    det = sqrt(det);
    float t1 = b - det;
    if (t1 > eps)
        return t1;

    float t2 = b + det;
    if (t2 > eps)
        return t2;

    return INF;
}


float triangle_intersect(Ray r, vec3 v0, vec3 v1, vec3 v2) {
    float min_t = INF;

    vec3 center_u = r.direction;
    vec3 center_v = cross(r.direction, r.origin);
    
    // Constant
    vec3 v0_u = (v1 - v0);
    vec3 v0_v = cross(v1, v0);
    
    vec3 v1_u = (v2 - v1);
    vec3 v1_v = cross(v2, v1);
    
    vec3 v2_u = (v0 - v2);
    vec3 v2_v = cross(v0, v2);
    
    vec3 normal = normalize(cross(v1 - v0, v2 - v0));
    float q = dot(r.direction, normal);
    
    // 6 dot intersection test
    if(dot(v0_u, center_v) + dot(v0_v, center_u) > 0.0 &&
       dot(v1_u, center_v) + dot(v1_v, center_u) > 0.0 &&
       dot(v2_u, center_v) + dot(v2_v, center_u) > 0.0)
    {
        
        float t = -dot(r.origin - v0, normal) / q;
        if(t < min_t)
        {
            return t;
        }
    }

    return INF;
}

bool aabb_intersect(Ray ray, vec4 aabb_min, vec4 aabb_max) {
  float tmin = 0.0, tmax = INF;

  for (int d = 0; d < 3; ++d) {
    float t1 = (aabb_min[d] - ray.origin[d]) / ray.direction[d];
    float t2 = (aabb_max[d] - ray.origin[d]) / ray.direction[d];

    tmin = max(tmin, min(t1, t2));
    tmax = min(tmax, max(t1, t2));
  }

  return tmin < tmax;
}

int traverse(Ray ray, inout HitInfo hit) {
  
  int closest = NO_HIT;

  int tests = 0;

  uint id = 0;

  Stack s;
  init(s);
  push(s, id);

  while (!is_empty(s)) {
    id = pop(s);
    Node node = nodes[id];

    if (!aabb_intersect(ray, node.min, node.max)) {
      continue;
    }

    if (node.left != INVALID) {
      push(s, node.left);
    }

    if (node.right != INVALID) {
      //push(s, node.right);
    }

    if (node.count > 0) {
      for (uint i = node.offset; i < node.offset + node.count; i++) {

#if 1
        float t = sphere_intersect(ray, spheres[i]);
        if (EPSILON < t && t < hit.t) {
          hit.t = t;
          hit.point = ray.origin + ray.direction * t;
          hit.normal = (hit.point - spheres[i].center) / spheres[i].radius;
          //hit.material = spheres[i].material;
          if (node.offset == 0) {
            hit.material = 0;
          } else {
            hit.material = 1;
          }
          closest = int(i);
        }
#else
        vec3 v0 = vec3(vertices[i * 3 + 0]);
        vec3 v1 = vec3(vertices[i * 3 + 1]);
        vec3 v2 = vec3(vertices[i * 3 + 2]);
        float t = triangle_intersect(ray, v0, v1, v2);

        if (EPSILON < t && t < hit.t) {
          hit.t = t;
          hit.point = ray.origin + ray.direction * t;
          hit.normal = normalize(cross(v1 - v0, v2 - v0));
          hit.material = int(vertices[v * 3].w);
          closest = i;
        }
#endif
      }
    }
  }

  return closest;
}

int find_closest_mesh(Ray ray, inout HitInfo hit) 
{
  float max_t = INF;
  int closest = NO_HIT;

  for (int i = 0; i < meshes.length(); i++) {
    Mesh mesh = meshes[i];
    uint offset = mesh.start;
    uint count = mesh.size;

    for (uint v = offset; v < offset + count; v++) {
      
      vec3 v0 = vec3(vertices[v * 3 + 0]);
      vec3 v1 = vec3(vertices[v * 3 + 1]);
      vec3 v2 = vec3(vertices[v * 3 + 2]);

      float t = triangle_intersect(ray, v0, v1, v2);

      if (EPSILON < t && t < max_t) {
        hit.t = t;
        hit.point = ray.origin + ray.direction * t;
        hit.normal = normalize(cross(v1 - v0, v2 - v0));
        hit.material = int(vertices[v * 3].w);
        max_t = hit.t;
        closest = i;
      }
    }
  }

  return closest;
}

int find_closest_sphere_range(Ray ray, inout HitInfo hit, int begin, int end)
{
  float max_t = INF;
  int closest = NO_HIT;

  float t;

  for (int i = begin; i < end || i < spheres.length(); i++) 
  {
    float t = sphere_intersect(ray, spheres[i]);

    if (EPSILON < t && t < max_t) {
      hit.t = t;
      hit.point = ray.origin + ray.direction * t;
      hit.normal = (hit.point - spheres[i].center) / spheres[i].radius;
      hit.material = spheres[i].material;
 
      max_t = hit.t;
      closest = i;
    }
  }

  return closest;

}

int find_closest_sphere(Ray ray, inout HitInfo hit)
{
  float max_t = INF;
  int closest = NO_HIT;

  float t;

  for (int i = 0; i < spheres.length(); i++) 
  {
    float t = sphere_intersect(ray, spheres[i]);

    if (EPSILON < t && t < max_t) {
      hit.t = t;
      hit.point = ray.origin + ray.direction * t;
      hit.normal = (hit.point - spheres[i].center) / spheres[i].radius;
      hit.material = spheres[i].material;
 
      max_t = hit.t;
      closest = i;
    }
  }

  return closest;
}

float fresnel_schlick(float f0, float cos_theta)
{
  float c = 1 - cos_theta;
  return  f0 + (1 - f0) * (c*c*c*c*c);
}

vec3 trace_path(Ray ray) 
{
  vec3 radiance = vec3(0.0);
  vec3 throughput = vec3(1.0);

  for (int bounce = 0; bounce < u_max_bounce; bounce++)
  {
    HitInfo hit1, hit2;
    hit1.t = INF;
    hit2.t = INF;
    HitInfo hit;

#if 1
    int i = find_closest_sphere(ray, hit1);
#else
    int i = traverse(ray, hit1);
#endif


    int j = find_closest_mesh(ray, hit2);

    if (i == NO_HIT && j == NO_HIT) {
      vec3 background = u_use_envmap ? texture(u_envmap, ray.direction).rgb : u_background;
      radiance += background * throughput;
      break;
    }

    hit = (hit1.t < hit2.t) ? hit1 : hit2;

    Material material = materials[hit.material];

    vec3 albedo = material.albedo.rgb;
    vec3 emission = material.emission.rgb;
    float smoothness = material.albedo.w;

    bool inside = dot(-ray.direction, hit.normal) < 0;

    vec3 point = hit.point;
    vec3 normal = hit.normal;

    ray.origin = point;

    if (material.type == 0) { // diffuse

      ray.direction = cosine_weighted(hit.normal);
      throughput *= albedo; 

    } else if (material.type == 1) { // specular

      vec3 diffuse = cosine_weighted(hit.normal);
      vec3 specular = reflect(ray.direction, hit.normal);
      ray.direction = mix(diffuse, specular, smoothness);

      throughput *= albedo; 

    } else if (material.type == 2) { // transparent

      // points inside the sphere if we are inside
      vec3 nl = inside ? -hit.normal : hit.normal;

      float nc = 1;   // air
      float nt = 1.4; // glass

      float nnt = inside ? nt / nc : nc / nt;

      float cos_theta = dot(ray.direction, nl);

      float cos_theta_2_sqr;

      // total internal reflection
      if ((cos_theta_2_sqr = 1 - nnt * nnt * (1 - cos_theta * cos_theta)) < 0) {
        throughput *= albedo;
        ray.direction = reflect(ray.direction, hit.normal);
        break;
      }

      vec3 transmission = refract(ray.direction, nl, nnt);

      float a = nt - nc;
      float b = nt + nc;
      float R0 = a * a / (b*b);

      float cos_theta_2 = dot(transmission, hit.normal);

      float tmp = (inside ? cos_theta_2 : -cos_theta);

      // reflection weight
      float Re = fresnel_schlick(R0, tmp); 
  
      // refraction weight
      float Tr = 1 - Re; 

      float P = 0.25 + 0.5 * Re;
      float RP = Re / P;
      float TP = Tr / (1 - P);

      if (rand() < P) {
        throughput *= (albedo * RP);
        ray.direction = reflect(ray.direction, hit.normal);
      } else {
        throughput *= (albedo * TP);
        ray.direction = transmission;
      }
    }

    radiance += emission * throughput;
  }

  return radiance;
}

void main() 
{
  ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);

  frag_coord = vec2(pixel_coords);
  vec2 resolution = vec2(imageSize(image));

  init_rand(frag_coord, u_random);

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
