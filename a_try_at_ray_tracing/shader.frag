#version 450
//------------------------------------STRUCT DEFS------------------------------------------------------------------------------------------------------
struct Ray{
	vec3 ray_origin;
	vec3 ray_dir;
};
struct Material{
	vec3 albedo;
	vec3 emmission_color;
	float emmission_strength;
};
struct Sphere{
	vec3 pos;
	float r;
	int mat_idx;
};
struct HitInfo{
	bool did_hit;
	float hit_dist;
	vec3 p;
	vec3 norm;
	int obj_idx;
};
layout(binding = 0)uniform UBO {
	int max_bounces;
	int n_traces;
	uvec2 res;
	vec2 iMouse;
	float cam_dist;
	
}ubo;
//layout(std140, binding = 1) readonly buffer {
//	Material materials[ ];
//}buff;
layout(std430, binding = 1) readonly buffer sphere_buff {
	Sphere spheres[6];
}sp;

layout(location = 0) out vec4 outColor;
layout(location = 0)in vec2 uv;
//------------------------------------RANDOM FUNCS AND STUFF-------------------------------------------------------------------------------------------
float RandomValue(inout uint state) {
		state = (state) * 747796405 + 2891336453;
		uint result = ((state >> ((state >> 28) + 4u)) ^ state) * 277803737u;
		result = (result >> 22) ^ result;
		return result / 4294967295.0;
}
float RandomValueNormalDistribution(inout uint state) {
		float theta = 2 * 3.1415926 * RandomValue(state);
		float rho = sqrt(-2 * log(RandomValue(state)));
		return rho * cos(theta);
}
vec3 RandomSphereDir(inout uint state) {
		float x = RandomValueNormalDistribution(state);
		float y = RandomValueNormalDistribution(state);
		float z = RandomValueNormalDistribution(state);
		return normalize(vec3(x, y, z));
}
vec3 RandomHemiSphereDir(vec3 norm, inout uint state) {
		vec3 dir = RandomSphereDir(state);

		return dot(dir, norm) < 0 ? -dir : dir;
}
mat2 rot2D(float ang){
	float s = sin(ang);
	float c = cos(ang);
	return mat2(c, -s, s, c);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------
//Material materials[] = {
//	Material(vec3(1, 1, 1), vec3(1, 1, 1), 2.4),
//	Material(vec3(0, 0.3, 0.8), vec3(0, 0, 0), 100),
//	Material(vec3(1, 1, 1), vec3(0, 0, 0), 5),
//
//};
//Sphere spheres[] = {
//	Sphere(vec3(-1, 0, 0), 1, 0),
//	Sphere(vec3(1, 0, 0), 1, 2),
//	Sphere(vec3(0, -101, 0), 100, 1)
//};
Material materials[] = {
	Material(vec3(1,0,1), vec3(0, 0, 0), 0),
	Material(vec3(0.4,1,0.995), vec3(0, 0, 0), 0),
	Material(vec3(1,1,1), vec3(1,0.995681,0.88968), 4.8),
	Material(vec3(1,1,1), vec3(0, 0, 0), 0),
	Material(vec3(0,1,0.861789), vec3(0, 0, 0), 0),
	Material(vec3(1,0,0), vec3(0, 0, 0), 0),


};
int n_spheres = 6;
//Sphere spheres[] = {
//	Sphere(vec3(2.3,-0.04,-0.2), 1, 0),
//	Sphere(vec3(0,-60,0), 59, 3),
//	Sphere(vec3(0,2.4,69), 28.2, 2),
//	Sphere(vec3(0.2,-0.2,-0.3), 0.88, 3),
//	Sphere(vec3(-1.4,-0.4,-0.1), 0.6, 5),
//	Sphere(vec3(-2.7,-0.65,0.2), 0.4, 4)
//};
HitInfo rayIntresectSphere(Sphere sphere, Ray ray){
	vec3 origin = ray.ray_origin - sphere.pos;
	float a = dot(ray.ray_dir, ray.ray_dir);
	float b = 2 * dot(ray.ray_dir , origin);
	float c = dot(origin, origin) - sphere.r * sphere.r;
	float d = b * b - 4 * a * c;
	if (d < 0){
		return HitInfo(false, -1.0, vec3(0), vec3(0), 0);
	}
	HitInfo hitinfo;
	hitinfo.did_hit = true;
	hitinfo.hit_dist = (-b - sqrt(d)) / (2 * a);
	hitinfo.p = origin + ray.ray_dir * hitinfo.hit_dist;
	hitinfo.norm = normalize(hitinfo.p);
	hitinfo.p += sphere.pos;

	return hitinfo;
}
HitInfo Miss(Ray ray){
	return HitInfo(false, -1.0, vec3(0), vec3(0), -1);
}
HitInfo TraceRay(Ray ray){
	float closestT = 3.402823466e+38F;
	int closestS = -1;
	for (int i = 0; i < n_spheres; i++){
		HitInfo hi = rayIntresectSphere(sp.spheres[i], ray);
		if (hi.did_hit){
			if ((hi.hit_dist > 0) && (hi.hit_dist < closestT)){
				closestT = hi.hit_dist;
				closestS = i;
			}
		}
	}
	if (closestS == -1)
		return Miss(ray);
	
	HitInfo hitinfo = rayIntresectSphere(sp.spheres[closestS], ray);
	hitinfo.obj_idx = closestS;
	return hitinfo;
}
vec3 RayGen(Ray initray, inout uint state){
	Ray ray = initray;
	vec3 incoming_light = vec3(0);
	vec3 ray_color = vec3(1);
	for (int i = 0; i < ubo.max_bounces; i++){
		HitInfo hi = TraceRay(ray);
		if (hi.did_hit){
			ray.ray_origin = hi.p;
			ray.ray_dir = normalize(hi.norm + RandomSphereDir(state));
			Material mat = materials[sp.spheres[hi.obj_idx].mat_idx];
			vec3 emitted_light = mat.emmission_color * mat.emmission_strength;
			ray_color *= mat.albedo;
			incoming_light += emitted_light * ray_color;
		}
		else{
			incoming_light += ray_color * vec3(0.6f, 0.7f, 0.9f) * 1;
			break;
		}
	}
	return incoming_light;
}
void main() {
	uvec2 n_pix = ubo.res.xy;
	uvec2 p_coord = uvec2((uv + 1) * 0.5 * n_pix);
	uint p_idx = p_coord.y * n_pix.x + p_coord.x;
	uint seed = p_idx;
	Ray ray;
	ray.ray_origin = vec3(0.0, 0.0, -ubo.cam_dist);
	ray.ray_dir = normalize(vec3(uv.xy, 1));
	ray.ray_origin.zy *= rot2D(ubo.iMouse.y);
	ray.ray_dir.zy *= rot2D(ubo.iMouse.y);
	ray.ray_origin.xz *= rot2D(-ubo.iMouse.x);
	ray.ray_dir.xz *= rot2D(-ubo.iMouse.x);
	vec3 incoming_light = vec3(0);
	for (int i = 0; i < ubo.n_traces;i++){
		incoming_light += RayGen(ray, seed);
	}
	incoming_light /= float(ubo.n_traces);
	outColor = vec4(incoming_light, 1.0);
	

}