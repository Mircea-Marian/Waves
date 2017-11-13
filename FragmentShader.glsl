#version 330
#define distance(a,b)\
	sqrt( pow( (a).x - (b).x , 2 ) + pow( (a).y - (b).y , 2 )\
	 + pow( (a).z - (b).z , 2 ) )

in vec3 world_position;
in vec3 world_normal;


uniform vec3 light_position;
uniform vec3 eye_position;
uniform float material_kd;
uniform float material_ks;
uniform int material_shininess;

uniform int type;

uniform vec3 object_color;

layout(location = 0) out vec4 out_color;

void main()
{
	vec3 L = normalize(light_position - world_position);
	vec3 V = normalize(eye_position - world_position);
	vec3 H = normalize(L + V);
	vec3 N = normalize(world_normal);
	
	float ambient_light = 0.1;

	float diffuse_light = material_kd * max(dot(world_normal, L), 0);

	float specular_light = 0;

	if (diffuse_light > 0) {
		specular_light = material_ks * pow(max(dot(world_normal, H), 0), material_shininess);
	}

	float light;
	float dis = distance(world_position,light_position)*distance(world_position,light_position);
	float atenuare = 1/dis;
	light = ambient_light + atenuare*(specular_light + diffuse_light);

	if(type == 0)
		out_color = vec4(light * object_color, 1);
	else 
		out_color = vec4(light * vec3(0,0,1),0.5);
}