#version 400 core

layout (location=0) out vec4 fragment;


//	I	= Ia + Id + Is
//	Ia	= Ka x La
//	Id	= max(N*L, 0) x Kd x Ld
//	Is	= max(N*H, 0)^Ns x Ks x Ls
//	H	= (L+V).normalize
//	L	= frag_position->Lpos
//	V	= frag_position->camera(0,0,0)

/**shading**/
uniform vec4 Lpos;
uniform vec3 La;
uniform vec3 Ld;
uniform vec3 Ls;

layout (std140) uniform Material{
	vec3 Ka;
	vec3 Kd;
	vec3 Ks;
	float Ns;
};

/**texture**/
uniform sampler2D tex0;
uniform int use_tex;

/**shadow map**/
uniform sampler2D shadow_map;
in vec4 light_proj_position;

/****/
in vec4 view_model_position;
in vec3 view_model_normal;
in vec2 uv;

void main() {
	
	vec3 N = view_model_normal;
	vec4 P = view_model_position;

	//Ia
	vec3 Ia = Ka*La;

	//Id
	vec3 L = normalize((Lpos*P.w - P*Lpos.w).xyz);
	vec3 Id = max(dot(N,L), 0)*Kd*Ld;

	//Is
	vec3 V = -normalize(P.xyz);
	vec3 H = normalize(V+L);
	vec3 Is = pow(max(dot(N, H), 0), Ns)*Ks*Ls;

	//shadow
	vec3 light_ndc = 0.5*(light_proj_position.xyz / light_proj_position.w)+0.5;
	float depth = texture(shadow_map, light_ndc.xy).x;
	float visibility = 1.0;
	if (depth < light_ndc.z - 0.0001) visibility = 0.3;
	
	//texture
	vec4 I = vec4((Ia+Id+Is)*visibility, 1.0);
	if (use_tex==0) fragment = I;
	//else fragment = texture(tex0,uv)*I;
	else fragment = vec4(1.0 - (1.0-texture(shadow_map, uv).x)*25.0)*I;

	//fragment = vertex_color*vec4(uv,1.0,1.0);
	//fragment = vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, 1.0);

	//fragment = vec4(0.3,0.3,0.3,1.0);
	//float r = gl_FragCoord.x/1000.0;
	//float g = gl_FragCoord.y/1000.0;
	//float b = gl_FragCoord.z/1000.0;
	//fragment = vec4(r,g,b,1.0);
}