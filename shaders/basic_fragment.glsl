#version 450 core

in vec3 norm;
in vec3 fragPos;

uniform vec3 color;

out vec4 fColor;

void main(void){
	float ambientStrength = 0.2;
	vec3 ambientLight = vec3(1.0);
	vec3 lightPos = vec3(0.0, 3.0, 3.0);
	vec3 lightDir = normalize(lightPos - fragPos);

	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * ambientLight;

	vec3 ambient = ambientStrength * ambientLight;
	vec3 result = (ambient + diffuse) * color;

	fColor = vec4(result, 1.0);
}
