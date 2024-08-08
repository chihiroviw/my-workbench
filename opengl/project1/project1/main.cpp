#include <stdio.h>
#include <cstdlib>
#include <string>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include <stack>

#define GLEW_STATIC
#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include "gl_matrix.h"
#include "shader_utility.h"
#include "shape.h"
#include "shape_index.h"
#include "glfw_window.h"
#include "gl_camera.h"
#include "solid_shape_index.h"
#include "instance_solid_shape_index.h"
#include "uniform_material.h"
#include "texture_test.h"
#include "shadow_map.h"
#include "cube_data.h"

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "libglew32d.lib")

void draw_many_cube_instance_draw(
	Matrix& projection, Matrix& view,
	std::unique_ptr<InsSolidShapeIndex>& shape,
	int dns, int frame, GLuint program);

std::unique_ptr<SolidShapeIndex> make_sphere(int slices, int stacks, float* rgb);

void render_sphere(
	int frame, GLuint program_sphere, Matrix &view, Matrix &projection,
	Uniform &sphere_color, TextureTest &texture_check,
	std::unique_ptr<SolidShapeIndex>& sphere0,
	std::unique_ptr<SolidShapeIndex>& sphere1,
	std::unique_ptr<SolidShapeIndex>& cube,
	float* vLpos);

//0 $ diX dtX
int main(void) {

	Window window;
	GLuint program_cube = load_create_program("cube_instance_vertex.glsl","cube_instance_fragment.glsl");
	GLuint program_sphere = load_create_program("sphere_vertex.glsl","sphere_fragment.glsl");
	GLuint program_shadow_map = load_create_program("shadow_map_vertex.glsl","shadow_map_fragment.glsl");

	// 図形データを作成する
	std::unique_ptr<InsSolidShapeIndex> shape(new InsSolidShapeIndex(3, 24, solidCubeVertex, 36, solidCubeIndex));
	float rgb[3] = { 0.7,0.9,0.9 };
	auto sphere0 = make_sphere(200, 200,rgb);
	auto sphere1 = make_sphere(3, 2, rgb);
	std::unique_ptr<SolidShapeIndex> cube(new SolidShapeIndex(3, 24, solidCubeVertex, 36, solidCubeIndex));
	Uniform sphere_color(NULL);
	TextureTest texture_check;

	//make shadow map
	ShadowMapFbo shadow_map_fbo;

	Camera camera;

	int frame = 0;

	while (window.poll_events()){
		window.swap_buffers();
		window.print_fps();
		frame++;

		/**model**/
		Matrix model = Matrix::identity();

		/**view**/
		Matrix view = camera.update(window.get_key_response(), window.get_mouse_offset());

		auto t = window.get_mouse_offset();
		printf("%f %f\n", t[0], t[1]);


		/**projection**/
		GLfloat* size = window.get_size();
		Matrix projection = Matrix::frustum(-size[0]/2000, size[0]/2000, -size[1]/2000, size[1]/2000, 1.0f, 1000.0f);


		/**draw object**/
		glUseProgram(program_cube);
		draw_many_cube_instance_draw(projection, view, shape, 10, frame, program_cube);

	
		/**draw object**/
		/*
		//make shadow map
		float vLpos[4] = {(float)sin(frame/100.0)*25,40,0,1};
		Vector3f e(vLpos[0], vLpos[1], vLpos[2]);
		Vector3f g = Vector3f(0,0,0);
		Matrix light_view = Matrix::lookat(	e, g,Vector3f(0,0,1));
		Matrix light_proj = Matrix::perspective(120*3.141592/180.0, 1, 1.0f, 200.0f);
		Matrix light_proj_view = light_proj*light_view;
		GLint lpv_loc = glGetUniformLocation(program_shadow_map, "light_proj_view");
		glUseProgram(program_shadow_map); 
		glUniformMatrix4fv(lpv_loc, 1, GL_FALSE, light_proj_view.data());

		shadow_map_fbo.bind_for_writing();
		glClear(GL_DEPTH_BUFFER_BIT);

		render_sphere(frame, program_shadow_map, light_view, light_proj, sphere_color, 
					texture_check, sphere0, sphere1, cube, vLpos);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		shadow_map_fbo.bind_for_reading(GL_TEXTURE1);
		GLint shadow_tex_loc = glGetUniformLocation(program_sphere, "shadow_map");

		//use shadow map to draw
		window.reset_viewport();
		glUseProgram(program_sphere);//依存関係あり useprogram -> uniform1i
		glUniform1i(shadow_tex_loc, 1);
		lpv_loc = glGetUniformLocation(program_sphere, "light_proj_view");
		glUniformMatrix4fv(lpv_loc, 1, GL_FALSE, light_proj_view.data());
		render_sphere(frame, program_sphere, view, projection, sphere_color, 
				texture_check, sphere0, sphere1, cube, vLpos);
		*/
	}
	return 0;
}

void draw_many_cube_instance_draw(
	Matrix& projection, Matrix& view,
	std::unique_ptr<InsSolidShapeIndex>& shape, 
	int dns, int frame, GLuint program){

	static std::vector<Matrix> matrix_vec(dns*dns*dns);

	for (int x = 0; x < dns; x++) {
		for (int y = 0; y < dns; y++) {
			for (int z = 0; z < dns; z++) {

				Matrix rx = Matrix::rotate((frame%360)*2*3.1415/360.0,1,0,1);
				Matrix ry = Matrix::rotate((frame%360)*2*3.1415/360.0,0,1,1);
				Matrix rz = Matrix::rotate((frame%360)*2*3.1415/360.0,1,1,0);
				Matrix trans = Matrix::translate(x*7, y*7, z*7);
				Matrix view_model = view*trans;// *rx* rz* ry;
				//glUniformMatrix4fv(vm_loc, 1, GL_FALSE, view_model.data());
				//shape->draw();//draw_elements
				matrix_vec.push_back(view_model);
			}
		}
	}
	
	//set projection matrix
	GLint p_loc = glGetUniformLocation(program, "proj");
	glUniformMatrix4fv(p_loc, 1, GL_FALSE, projection.data());

	//instance draw
	shape->instance_elements_draw(dns*dns*dns,matrix_vec.data());
	matrix_vec.clear();
}

std::unique_ptr<SolidShapeIndex> make_sphere(int slices, int stacks, float *rgb){
	GLuint vertices = (slices+1)*(stacks+1);
	GLuint faces = slices*stacks*2;

	//GLfloat Position[3];
	//Object::Vertex;
	GLuint Face[3];
	
	auto unique_position	= std::make_unique<Object::Vertex[]>(vertices);
	Object::Vertex* position = unique_position.get();
	auto unique_face		= std::make_unique<GLuint[]>(sizeof(Face)*faces);
	GLuint* face			= unique_face.get();

	/**頂点の位置**/
	for (int j = 0; j<=stacks; j++){
		float ph = 3.14159265*(float)j/(float)stacks;
		float y = cosf(ph);
		float r = sinf(ph);

		for (int i = 0; i<=slices; i++){
			float th = 2.0f * 3.14159265*(float)i/(float)slices;
			float x = r*cosf(th);
			float z = r*sinf(th);

			position->position[0] = x;
			position->position[1] = y;
			position->position[2] = z;

			position->color[0] = rgb[0];
			position->color[1] = rgb[1];
			position->color[2] = rgb[2];

			position->normal[0] = x;
			position->normal[1] = y;
			position->normal[2] = z;

			position++;
		}
	}

	/**面の指標**/
	for (int j = 0; j<stacks; j++){
		for (int i = 0; i<slices; i++){
			int count = (slices+1) * j + i;

			face[0] = count;
			face[1] = count+1;
			face[2] = count+slices+2;
			face += 3;

			face[0] = count;
			face[1] = count+slices+2;
			face[2] = count+slices+1;
			face += 3;
		}
	}

	auto unique_sphere 
		= std::make_unique<SolidShapeIndex>(3, vertices, unique_position.get(), faces*3, unique_face.get());
	return std::move(unique_sphere);
}

void render_sphere(int frame, GLuint program_sphere, Matrix& view, Matrix& projection,
					Uniform& sphere_color, TextureTest &texture_check, 
					std::unique_ptr<SolidShapeIndex> &sphere0, 
					std::unique_ptr<SolidShapeIndex> &sphere1,
					std::unique_ptr<SolidShapeIndex> &cube,
					float *vLpos){

	/**shading**/
		GLint Lpos = glGetUniformLocation(program_sphere, "Lpos");
		//float vLpos[4] = {(float)sin(frame/100.0)*25,15,0,1};
		float vLpos_view_trans[4];
		//float vLpos[4] = {0,30,0,1};
		view.mul_vec4(vLpos, vLpos_view_trans);
		glUniform4fv(Lpos,1,vLpos_view_trans);

		//light ambient diffuse specler
		GLint La = glGetUniformLocation(program_sphere, "La");
		glUniform3f(La, 0.6,0.6,0.6);
		GLint Ld = glGetUniformLocation(program_sphere, "Ld");
		glUniform3f(Ld, 1,1,1);
		GLint Ls = glGetUniformLocation(program_sphere, "Ls");
		glUniform3f(Ls, 1,1,1);
		
		// material ambient diffuse specler
		// uniform block の場所を取得する
		GLint material_loc = glGetUniformBlockIndex(program_sphere, "Material");
		// uniform block の場所を 0 番の結合ポイントに結びつける
		glUniformBlockBinding(program_sphere, material_loc, 0);
		
		Material color = {	0.5,0.5,0.5,//ka
							0.5,0.5,0.5,//kd
							0.3,0.3,0.3,//ks
							30 };//Ns
		sphere_color.set(&color);
		sphere_color.select(0);

		/**matrix**/
		GLint p_loc = glGetUniformLocation(program_sphere, "proj");
		glUniformMatrix4fv(p_loc, 1, GL_FALSE, projection.data());

		GLint m_loc = glGetUniformLocation(program_sphere, "model");
		GLint vm_loc = glGetUniformLocation(program_sphere, "view_model");
		GLint nm_loc = glGetUniformLocation(program_sphere, "normal_mat");
		GLfloat nm[9];

		//disnable tex
		GLint use_tex_loc = glGetUniformLocation(program_sphere, "use_tex");
		glUniform1i(use_tex_loc,0);

		//light sphere
		
		Matrix model = Matrix::identity();
		model = Matrix::translate(vLpos[0],vLpos[1],vLpos[2])*Matrix::scale(3, 3, 3);
		glUniformMatrix4fv(m_loc, 1, GL_FALSE, model.data());
		glUniformMatrix4fv(vm_loc, 1, GL_FALSE, (view*model).data());
		(view*model).getNormalMatrix(nm);
		glUniformMatrix3fv(nm_loc, 1, GL_FALSE, nm);
		sphere0->draw();

		//model 0
		model = Matrix::translate(0,5,0)*Matrix::scale(10, 10, 10);
		glUniformMatrix4fv(m_loc, 1, GL_FALSE, model.data());
		glUniformMatrix4fv(vm_loc, 1, GL_FALSE, (view*model).data());
		(view*model).getNormalMatrix(nm);
		glUniformMatrix3fv(nm_loc, 1, GL_FALSE, nm);
		sphere0->draw();

		//model 1
		model = Matrix::translate(-25,0,0)*Matrix::scale(10, 10, 10);
		glUniformMatrix4fv(m_loc, 1, GL_FALSE, model.data());
		glUniformMatrix4fv(vm_loc, 1, GL_FALSE, (view*model).data());
		(view*model).getNormalMatrix(nm);
		glUniformMatrix3fv(nm_loc, 1, GL_FALSE, nm);
		sphere0->draw();

		//model 3 cube
		model = Matrix::translate(0, -20, 0)*Matrix::scale(100, 1, 20);
		glUniformMatrix4fv(m_loc, 1, GL_FALSE, model.data());
		glUniformMatrix4fv(vm_loc, 1, GL_FALSE, (view*model).data());
		(view*model).getNormalMatrix(nm);
		glUniformMatrix3fv(nm_loc, 1, GL_FALSE, nm);
		cube->draw();


		//enable tex
		glUniform1i(use_tex_loc,1);
		texture_check.bind(GL_TEXTURE0);
		GLint tex_loc = glGetUniformLocation(program_sphere, "tex0");
		glUniform1i(tex_loc, 0);

		//model 2
		float rotate_speed = 2.0;
		model = Matrix::translate(20, 0, 0)*Matrix::scale(10, 10, 10);
				//*Matrix::rotate((frame%(360*(int)rotate_speed)/rotate_speed)*3.1415/(180.0),0,1,0);
		glUniformMatrix4fv(m_loc, 1, GL_FALSE, model.data());
		glUniformMatrix4fv(vm_loc, 1, GL_FALSE, (view*model).data());
		(view*model).getNormalMatrix(nm);
		glUniformMatrix3fv(nm_loc, 1, GL_FALSE, nm);
		sphere1->draw();

		glUniform1i(use_tex_loc,0);
	}
