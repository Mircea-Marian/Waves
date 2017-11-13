#include "Laborator7.h"

#include <vector>
#include <string>
#include <iostream>

#include <Core/Engine.h>

using namespace std;

Laborator7::Laborator7()
{
}

Laborator7::~Laborator7()
{
}

void Laborator7::Init()
{
	//Se declara meshu-l care va fii folosit pentru desenarea
	{
		Mesh* mesh = new Mesh("sphere");
		mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "sphere.obj");
		meshes[mesh->GetMeshID()] = mesh;
	}
	
	//Se declara shader-ele.
	{	
		//Se initializeaza shader-ul care se ocupa cu iluminarea si calcularea normalelor si pozitiilor.
		Shader *shader = new Shader("ShaderLab7");
		shader->AddShader("Source/Laboratoare/Laborator7/Shaders/VertexShader.glsl", GL_VERTEX_SHADER);
		shader->AddShader("Source/Laboratoare/Laborator7/Shaders/FragmentShader.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}
	{
		//Se initializeaza un shader simplu care se ocupa cu desenarea picaturilor si sferelor de generare a picaturilor si de lumina.
		Shader *shader = new Shader("MeinShader");
		shader->AddShader("Source/Laboratoare/Laborator7/Shaders/MeinVertexShader.glsl", GL_VERTEX_SHADER);
		shader->AddShader("Source/Laboratoare/Laborator7/Shaders/MeinFragmentShader.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	//Se initializeaza proprietatile luminii si pozitia surselor.
	{
		dropSourcePosition = glm::vec3(0, FIXED_DROP_SOURCE_HEIGHT , 0);
		lightPosition = dropSourcePosition;
		materialShininess = 30;
		materialKd = 1.2;
		materialKs = 1.2;
	}
	
	//Se genereaza punctele si indicii necesari desenarii.
	GLfloat i, j;
	unsigned int k = 0, l;
	const float start = (0 - SQUARE_L / 2) * SCALE_FACTOR;
	const float stop = (SQUARE_L / 2) * SCALE_FACTOR;
	for (i = start ; i <= stop; i += SCALE_FACTOR)
		for (j = start; j <= stop; j += SCALE_FACTOR) {
			vertexPosition[k][0] = i;
			vertexPosition[k][2] = j;
			vertexPosition[k][1] = 1;
			k++;
		}
	unsigned int m = 0;
	for (l = 0; l < SQUARE_L - 1; l++) {
		for (k = 0; k < SQUARE_L - 1; k++) {
			//linia verticala
			indeces[m++] = l * SQUARE_L + k;
			indeces[m++] = (l + 1) * SQUARE_L + k;

			//linia oblica
			indeces[m++] = l * SQUARE_L + k;
			indeces[m++] = (l + 1) * SQUARE_L + k + 1;

			//linia orizontala
			indeces[m++] = l * SQUARE_L + k;
			indeces[m++] = l * SQUARE_L + k + 1;
		}
		//margine dreapta
		indeces[m++] = l * SQUARE_L + SQUARE_L - 1;
		indeces[m++] = (l + 1) * SQUARE_L + SQUARE_L - 1;
	}
	//marginea de jos
	for (k = 0; k < SQUARE_L - 1; k++) {
		indeces[m++] = (SQUARE_L - 1) * SQUARE_L + k;
		indeces[m++] = (SQUARE_L - 1) * SQUARE_L + k + 1;
	}
	
	m = 0;
	GLushort indices1[3 * 2 * (SQUARE_L - 1) * (SQUARE_L - 1)];
	for (l = 0; l < SQUARE_L - 1; l++) {
		for (k = 0; k < SQUARE_L - 1; k++) {
			//triunghi de sus
			indices1[m++] = l * SQUARE_L + k;
			indices1[m++] = (l+1) * SQUARE_L + k + 1;
			indices1[m++] = l * SQUARE_L + k + 1;

			//triunghi de jos
			indices1[m++] = l * SQUARE_L + k;
			indices1[m++] = (l + 1) * SQUARE_L + k;
			indices1[m++] = (l + 1) * SQUARE_L + k + 1;
		}
	}

	//Se initializeaza valorile vitezelor picaturilor imediat dupa contactul cu apa.
	//float v_max = 0.85 * sqrt(2 * GRAV_ACC * FIXED_DROP_SOURCE_HEIGHT);
	float v_max = 1 * sqrt(2 * GRAV_ACC * FIXED_DROP_SOURCE_HEIGHT);
	for (m = 1; m < NUMBER_OF_BOUNCES; m++) {
		//initialSpeed[m] = v_max * m / NUMBER_OF_BOUNCES;
		initialSpeed[m] = v_max / pow(2, (NUMBER_OF_BOUNCES - m));
	}
	for (m = 0; m < NUMBER_OF_BOUNCES; m++)
		//scalingFactor[m] = 0.3 * (m + 1) / NUMBER_OF_BOUNCES;
		scalingFactor[m] = 0.7 / pow(2, (NUMBER_OF_BOUNCES - m));

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &posVBO);
	glBindBuffer(GL_ARRAY_BUFFER, posVBO);
	glBufferData(GL_ARRAY_BUFFER, 3 * VERTEX_MAX_NUM * sizeof(GLfloat), vertexPosition, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &triVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indeces), indeces, GL_STATIC_DRAW);

	glGenBuffers(1, &VBO1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices1), indices1, GL_STATIC_DRAW);
	
	numberOfWaves = 0;
	rate = 1.0;
}

void Laborator7::FrameStart()
{
	// clears the color buffer (using the previously set color) and depth buffer
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::ivec2 resolution = window->GetResolution();
	// sets the screen area where to draw
	glViewport(0, 0, resolution.x, resolution.y);

}

void Laborator7::Update(float deltaTimeSeconds)
{

	// Render the point light in the scene
	{
		//sursa de picaturi
		glm::mat4 modelMatrix = glm::mat4(1);
		modelMatrix = glm::translate(modelMatrix, dropSourcePosition);
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f));
		RenderSimpleMesh(meshes["sphere"], shaders["MeinShader"], modelMatrix, glm::vec3(0, 0, 1));
	}
	{
		//sursa de lumina
		glm::mat4 modelMatrix = glm::mat4(1);
		modelMatrix = glm::translate(modelMatrix, lightPosition);
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f));
		RenderSimpleMesh(meshes["sphere"], shaders["MeinShader"], modelMatrix, glm::vec3(255, 255, 0));
	}
	//picaturile
	unsigned int i;
	for (i = 0; i < dropData.size(); i++) 
		if(dropData[i].healthPoints) {
			glm::mat4 modelMatrix = glm::mat4(1);
			modelMatrix = glm::translate(modelMatrix, dropData[i].position);
			modelMatrix = glm::scale( modelMatrix, glm::vec3( scalingFactor[ dropData[ i ].healthPoints - 1 ] ) );
			RenderSimpleMesh(meshes["sphere"], shaders["MeinShader"], modelMatrix, glm::vec3(0, 0, 1));
		}

	//Se rendeaza grid-ul.
	glUseProgram(shaders["ShaderLab7"]->program);

	int a1 = glGetUniformLocation(shaders["ShaderLab7"]->program, "numberOfWaves");
	glUniform1i(a1, numberOfWaves);

	int a2 = glGetUniformLocation(shaders["ShaderLab7"]->program, "centerX");
	glUniform1fv(a2, 21, centerX);

	int a3 = glGetUniformLocation(shaders["ShaderLab7"]->program, "centerY");
	glUniform1fv(a3, 21, centerY);

	int a4 = glGetUniformLocation(shaders["ShaderLab7"]->program, "amplitude");
	glUniform1fv(a4, 21, A);

	int a5 = glGetUniformLocation(shaders["ShaderLab7"]->program, "radius");
	glUniform1fv(a5, 21, radius);
	
	int a6 = glGetUniformLocation(shaders["ShaderLab7"]->program, "length");
	glUniform1fv(a6, 21, length);

	int light_position = glGetUniformLocation(shaders["ShaderLab7"]->program, "light_position");
	glUniform3f(light_position, lightPosition.x, lightPosition.y, lightPosition.z);

	glm::vec3 eyePosition = GetSceneCamera()->transform->GetWorldPosition();
	int eye_position = glGetUniformLocation(shaders["ShaderLab7"]->program, "eye_position");
	glUniform3f(eye_position, eyePosition.x, eyePosition.y, eyePosition.z);

	int material_shininess = glGetUniformLocation(shaders["ShaderLab7"]->program, "material_shininess");
	glUniform1i(material_shininess, materialShininess);

	int material_kd = glGetUniformLocation(shaders["ShaderLab7"]->program, "material_kd");
	glUniform1f(material_kd, materialKd);

	int material_ks = glGetUniformLocation(shaders["ShaderLab7"]->program, "material_ks");
	glUniform1f(material_ks, materialKs);

	int object_color = glGetUniformLocation(shaders["ShaderLab7"]->program, "object_color");
	glUniform3f(object_color, 0, 0, 1);

	glm::mat4 viewMatrix = GetSceneCamera()->GetViewMatrix();
	int loc_view_matrix = glGetUniformLocation(shaders["ShaderLab7"]->program, "View");
	glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	glm::mat4 projectionMatrix = GetSceneCamera()->GetProjectionMatrix();
	int loc_projection_matrix = glGetUniformLocation(shaders["ShaderLab7"]->program, "Projection");
	glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	
	glBindVertexArray(VAO);
	int a7 = glGetUniformLocation(shaders["ShaderLab7"]->program, "type");
	glUniform1i(a7, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triVBO);
	glDrawElements(GL_LINES, 2 * (3 * (SQUARE_L - 1) * (SQUARE_L - 1) + 2 * (SQUARE_L - 1)), GL_UNSIGNED_SHORT, 0);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	int a8 = glGetUniformLocation(shaders["ShaderLab7"]->program, "type");
	glUniform1i(a8, 1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO1);
	glDrawElements(GL_TRIANGLES, 3 * 2 * (SQUARE_L - 1) * (SQUARE_L - 1), GL_UNSIGNED_SHORT, 0);
	glDisable(GL_BLEND);
}
void Laborator7::FrameEnd()
{
	//DrawCoordinatSystem();
}

void Laborator7::RenderSimpleMesh(Mesh *mesh, Shader *shader, const glm::mat4 & modelMatrix, const glm::vec3 &color)
{
	if (!mesh || !shader || !shader->GetProgramID())
		return;

	// render an object using the specified shader and the specified position
	glUseProgram(shader->program);

	int a = glGetUniformLocation(shader->program, "object_color");
	glUniform3f(a, color.x, color.y, color.z);

	// Bind model matrix
	GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
	glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

	// Bind view matrix
	glm::mat4 viewMatrix = GetSceneCamera()->GetViewMatrix();
	int loc_view_matrix = glGetUniformLocation(shader->program, "View");
	glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	// Bind projection matrix
	glm::mat4 projectionMatrix = GetSceneCamera()->GetProjectionMatrix();
	int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
	glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	// Draw the object
	glBindVertexArray(mesh->GetBuffers()->VAO);
	glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_SHORT, 0);
	
}

// Documentation for the input functions can be found in: "/Source/Core/Window/InputController.h" or
// https://github.com/UPB-Graphics/Framework-EGC/blob/master/Source/Core/Window/InputController.h

void Laborator7::OnInputUpdate(float deltaTime, int mods)
{

	float speed = 2;

	if (!window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
	{
		glm::vec3 up = glm::vec3(0, 1, 0);
		glm::vec3 right = GetSceneCamera()->transform->GetLocalOXVector();
		glm::vec3 forward = GetSceneCamera()->transform->GetLocalOZVector();
		forward = glm::normalize(glm::vec3(forward.x, 0, forward.z));

		// Control light position using on W, A, S, D, E, Q
		if (window->KeyHold(GLFW_KEY_W)) dropSourcePosition -= forward * deltaTime * speed;
		if (window->KeyHold(GLFW_KEY_A)) dropSourcePosition -= right * deltaTime * speed;
		if (window->KeyHold(GLFW_KEY_S)) dropSourcePosition += forward * deltaTime * speed;
		if (window->KeyHold(GLFW_KEY_D)) dropSourcePosition += right * deltaTime * speed;


		if (window->KeyHold(GLFW_KEY_Y)) lightPosition -= forward * deltaTime * speed;
		if (window->KeyHold(GLFW_KEY_G)) lightPosition -= right * deltaTime * speed;
		if (window->KeyHold(GLFW_KEY_H)) lightPosition += forward * deltaTime * speed;
		if (window->KeyHold(GLFW_KEY_J)) lightPosition += right * deltaTime * speed;
		if (window->KeyHold(GLFW_KEY_U)) lightPosition += up * deltaTime * speed;
		if (window->KeyHold(GLFW_KEY_T)) lightPosition -= up * deltaTime * speed;
	}

	//Se actualizeaza miscarea valurilor si stergerea unora dintre ele.
	unsigned int i,j,m;
	for (i = 0; i < numberOfWaves; i++) {
		radius[i] += rate * deltaTime;
		length[i] += rate * 0.5 * deltaTime;
		if (A[i] > 0) {
			A[i] -= rate * 0.1 * deltaTime;
		}
		else {
			for (j = i + 1; j < numberOfWaves; j++) {
				A[j - 1] = A[j];
				centerX[j - 1] = centerX[j];
				centerY[j - 1] = centerY[j];
				radius[j - 1] = radius[j];
				length[j - 1] = length[j];
			}
			numberOfWaves--;
		}
	}

	//Se distrug picaturi si se creeaza valuri.
	for (i = 0; i < dropData.size(); i++) {
		if (dropData[i].healthPoints == 0)
			dropData.erase(dropData.begin() + i);
		else {
			unsigned char hasHitSurface = 0;
			dropData[i].moveDrop(deltaTime, initialSpeed, hasHitSurface);
			if (hasHitSurface == 1 && numberOfWaves < 21) {
				//A[numberOfWaves] = ((float)dropData[i].healthPoints) * 0.05;
				A[numberOfWaves] =  0.5 / pow(2, (NUMBER_OF_BOUNCES - dropData[i].healthPoints));
				centerX[numberOfWaves] = dropData[i].position.x;
				centerY[numberOfWaves] = dropData[i].position.z;
				radius[numberOfWaves] = 0;
				length[numberOfWaves] = 0.4;
				numberOfWaves++;
			}
		}
	}
}

void Laborator7::OnKeyPress(int key, int mods)
{
	//Se creeaza picaturi.
	if (key == GLFW_KEY_SPACE) {
		dropData.push_back(DropOfWater(dropSourcePosition));
	}

	//Se decide efect de slow/fast motion.
	if (key == GLFW_KEY_O && rate > 0.1 )
		rate -= 0.1;
	if (key == GLFW_KEY_P)
		rate += 0.1;
}

void Laborator7::OnKeyRelease(int key, int mods)
{
	// add key release event
}

void Laborator7::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
	// add mouse move event
}

void Laborator7::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
	// add mouse button press event
}

void Laborator7::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
	// add mouse button release event
}

void Laborator7::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}

void Laborator7::OnWindowResize(int width, int height)
{
}
