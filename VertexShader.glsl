#version 330
#define PI 3.14159265359

layout(location = 0) in vec3 v_position;


// Uniform properties
uniform mat4 View;
uniform mat4 Projection;
uniform int numberOfWaves;
uniform float centerX[21];
uniform float centerY[21];
uniform float amplitude[21];
uniform float radius[21];
uniform float length[21];

// Output value to fragment shader
//out vec3 color;
out vec3 world_position;
out vec3 world_normal;

void main()
{
	

	vec3 vertex = v_position;
	float By = 0;
	float Ty = 0;
	int t = 0;

	int i;
	float distToC;
	//Se compun valuri si se calculeaza componente ale derivatei.
	for( i = 0 ; i < numberOfWaves ; i++ ){
		distToC = sqrt( pow( centerX[i] - v_position.x , 2 ) + pow( centerY[i] - v_position.z , 2 ) );
		if( distToC >= radius[i] && distToC <= radius[i] + length[i] ){
			if( t == 0 )
				t = 1;
			float d = distToC - radius[i];
			vertex.y += amplitude[i] * sin( ( d * 2 * PI / length[i] ) - PI );
			
			By += amplitude[i] * (v_position.x - centerX[i]) * cos( ( d * 2 * PI / length[i] ) - PI ) / distToC;
			Ty += amplitude[i] * (v_position.z - centerY[i]) * cos( ( d * 2 * PI / length[i] ) - PI ) / distToC;
		}
	}

	if( t == 1 )
		world_normal = normalize( vec3( -By , 1 , -Ty ) );
	else
		world_normal = vec3( 0 , 1 , 0 );

	world_position = vertex;

	gl_Position = Projection * View * vec4(vertex, 1.0);
}
