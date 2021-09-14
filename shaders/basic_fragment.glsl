#version 450 core

in vec3 norm;
in vec3 fragPos;

struct Light{
	vec4 position;
	vec3 ambient;
	vec3 diffuse;
};

struct Material {
	vec3 diffuse;
};

uniform Material material;

out vec4 fColor;

void main(void){
	Light lights[3] = Light[3](
		Light(
			vec4(0.0, 3.0, 3.0, 0.0),
			vec3(0.05, 0.05, 0.05),
			vec3(0.8, 0.8, 0.8))
		,Light(
			vec4(-5.0, 3.0, -5.0, 0.0),
			vec3(0.05, 0.05, 0.05),
			vec3(0.8, 0.8, 0.8))
		,Light(
			vec4(4.0, 3.0, -5.0, 0.0),
			vec3(0.05, 0.05, 0.05),
			vec3(0.8, 0.8, 0.8))
			);

	vec3 diffuse = vec3(0.0);
	vec3 ambient = vec3(0.0);
	for(int i=0;i<3;++i)
	{
		vec3 lightDir = normalize(lights[i].position.xyz - fragPos);
		float diff = max(dot(norm, lightDir), 0.0);
		diffuse += diff * lights[i].diffuse * material.diffuse;
		ambient += material.diffuse * lights[i].ambient;
	}
	
	vec3 result = (ambient + diffuse);

	fColor = vec4(result, 1.0);
}
