#version 330 core

#define AIR_TYPE 1
#define SMOKE_TYPE 2
#define FIRE_TYPE 3
#define WATER_TYPE 4
#define OIL_TYPE 5
#define SAND_TYPE 6
#define NAPALM_TYPE 7
#define STONE_TYPE 8

uniform sampler2D u_grid_data_texture;

uniform vec2 u_resolution;
uniform vec2 u_grid_dim;
uniform vec2 u_mouse_pos;
uniform float u_time;

uniform float u_spawn_radius;
uniform float u_cell_size;

in vec2 tex_coord;
out vec4 frag_color;



// SHADERTOY https://www.shadertoy.com/view/fsjXDh

float hash21(vec2 p)
{
    p=fract(p*vec2(123.456,789.01));
    p+=dot(p,p+45.67);
    return fract(p.x*p.y);
}
float star(vec2 uv,float brightness)
{
    float color=0.0;
    float star=length(uv);
    float diffraction=abs(uv.x*uv.y);
    //diffraction *= abs((uv.x + 0.001953125) * (uv.y + 0.001953125));
    star=brightness/star;
    diffraction=pow(brightness,2.0)/diffraction;
    diffraction=min(star,diffraction);
    diffraction*=sqrt(star);
    color+=star*sqrt(brightness)*8.0;
    color+=diffraction*8.0;
    return color;
}
vec4 background(vec2 fragCoord)
{
    vec4 fragColor;
    vec2 UV=fragCoord.xy/u_resolution.yy;
    vec3 color=vec3(0.0);
    float dist=1.0;
    float brightness=.01;
    vec2 uv=(floor(UV*256.)/256.)-.51019;
    uv*=128.;
    uv+=floor((u_time)*64.)/3072.0;

    vec2 gv=fract(uv)-.5;
    vec2 id;
    float displacement;
    for(float y=-dist;y<=dist;y++)
    {
        for(float x=-dist;x<=dist;x++)
        {
            id=floor(uv);
            displacement=hash21(id+vec2(x,y));
            //color+=vec3(star(gv-vec2(x,y)-vec2(displacement,fract(displacement*16.))+.5,(hash21(id+vec2(x,y))/128.)));
            //color=min(color,.4);
        }
    }
    uv/=2.;
    gv=fract(uv)-.5;
    for(float y=-dist;y<=dist;y++)
    {
        for(float x=-dist;x<=dist;x++)
        {
            id=floor(uv);
            displacement=hash21(id+vec2(x,y));
            color+=vec3(star(gv-vec2(x,y)-vec2(displacement,fract(displacement*16.))+.5,(hash21(id+vec2(x,y))/128.)));
        }
    }
    uv/=8.;
    gv=fract(uv)-.5;
    for(float y=-dist;y<=dist;y++)
    {
        for(float x=-dist;x<=dist;x++)
        {
            id=floor(uv);
            displacement=hash21(id+vec2(x,y));
            color+=vec3(star(gv-vec2(x,y)-vec2(displacement,fract(displacement*16.))+.5,(hash21(id+vec2(x,y))/256.)));
        }
    }
    uv/=6.;
    gv=fract(uv)-.5;
    for(float y=-dist;y<=dist;y++)
    {
        for(float x=-dist;x<=dist;x++)
        {
            id=floor(uv);
            displacement=hash21(id+vec2(x,y));
            color+=vec3(star(gv-vec2(x,y)-vec2(displacement,fract(displacement*16.))+.5,(hash21(id+vec2(x,y))/256.)));
        }
    }
    color*=vec3(.5,.7,1.);
    //color = floor(0.01 + color * 16.0) / 16.0;
    fragColor=vec4(color,1.);
  return fragColor;
}

// END SHADERTOY



float rand(vec2 co) {
  // pseudo-random generator
  return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

float fbm(vec2 st, int octaves) {
  // fractal brownian motion noise function
  float value = 0.0;
  float amplitude = 0.5;
  float frequency = 1.0;
  for (int i = 0; i < octaves; ++i) {
    value += amplitude * rand(st * frequency);
    amplitude *= 0.5;
    frequency *= 2.0;
  }
  return value;
}

vec4 shade_mouse_ring() {
  // normalize screen space coords and convert to grid coords
  vec2 st = gl_FragCoord.xy / u_resolution;
  vec2 grid_coords = st * u_grid_dim;

  // convert mouse position to grid coordinates (invert y for opengl coord)
  vec2 mouse_grid_pos =
    vec2(u_mouse_pos.x, u_resolution.y - u_mouse_pos.y) / u_resolution * u_grid_dim;

  // thickness of ring in grid cells
  float thickness = 1.0;
  float inner_radius = u_spawn_radius - thickness / 2.0;
  float outer_radius = u_spawn_radius + thickness / 2.0;

  float dist = length(grid_coords - mouse_grid_pos);
  float ring_alpha = step(inner_radius, dist) * (1.0 - step(outer_radius, dist));

  return vec4(1.0, 1.0, 1.0, ring_alpha);
}

vec4 shade_air() {
  return vec4(0.5, 0.5, 0.5, 1.0); // gray
}

vec4 shade_smoke(float mass) {
  float noise = fbm(gl_FragCoord.xy / u_resolution * u_grid_dim, 4);
  float gray_value = mix(0.5, 1.0, mass);
  vec3 gray_color = vec3(gray_value, gray_value, gray_value);
  return vec4(gray_color * (0.5 + 0.5 * noise), 1.0);
}

vec4 shade_fire(float mass) {
  vec2 st = gl_FragCoord.xy / u_resolution * u_grid_dim;
  float noise = fbm(st, 4);

  // remap the mass value to a range of 0.0 to 1.0
  float normalized_mass = clamp(mass / 0.6, 0.0, 1.0);

  // create a gradient of colors from red to yellow based on mass and noise
  vec3 fire_color = mix(vec3(1.0, 1.0, 0.0), vec3(1.0, 0.0, 0.0), normalized_mass);
  fire_color *= 0.8 + 0.2 * noise;

  // make the fire appear more yellowish higher up in the flame
  fire_color *= mix(1.0, 1.4, smoothstep(0.0, 1.0, st.y / u_grid_dim.y));

  return vec4(fire_color, 1.0);
}

vec4 shade_water(float mass) {
  vec3 light_blue = vec3(0.0, 0.5, 1.0); // light blue
  vec3 dark_blue = vec3(0.0, 0.0, 0.5);  // dark blue
  return vec4(mix(light_blue, dark_blue, mass), 1.0); // interpolated
}

vec4 shade_oil() {
  // TODO: implement
  return vec4(0.0);
}

vec4 shade_sand() {
  float noise = fbm(gl_FragCoord.xy / u_resolution * u_grid_dim, 4);
  return vec4(vec3(0.9, 0.85, 0.4) * (0.5 + 0.5 * noise), 1.0);
}

vec4 shade_napalm() {
  return shade_fire(rand(gl_FragCoord.xy / u_resolution));
}

vec4 shade_stone() {
  return vec4(0.0, 0.0, 0.0, 1.0); // black
}

vec4 shade_default() {
  return vec4(0.0, 0.0, 0.0, 1.0); // black
}

void main() {
  vec4 grid_data = texture(u_grid_data_texture, tex_coord);

  int cell_type = int(grid_data.r);
  float mass = grid_data.g;

  vec4 color = vec4(0.0);
  switch (cell_type) {
  case AIR_TYPE:
    color = background(gl_FragCoord.xy); 
    break;
  case SMOKE_TYPE:
    color = shade_smoke(mass);
    break;
  case FIRE_TYPE:
    color = shade_fire(mass);
    break;
  case WATER_TYPE:
    color = shade_water(mass);
    break;
  case OIL_TYPE:
    color = shade_oil();
    break;
  case SAND_TYPE:
    color = shade_sand();
    break;
  case NAPALM_TYPE:
    color = shade_napalm();
    break;
  case STONE_TYPE:
    color = shade_stone();
    break;
  default:
    color = shade_default();
  }

  vec4 mouse_ring = shade_mouse_ring();
  frag_color = mix(color, mouse_ring, mouse_ring.a);
}
