#pragma once
#include <Component/SimpleScene.h>
#include <Component/Transform/Transform.h>
#include <Core/GPU/Mesh.h>
#include <iostream>
#define VERTEX_MAX_NUM 40000
#define SQUARE_L 200
#define NUMBER_OF_BOUNCES 7
#define GRAV_ACC 0.2
#define FIXED_DROP_SOURCE_HEIGHT 3
#define SCALE_FACTOR 0.05

class Laborator7 : public SimpleScene
{
	public:
		Laborator7();
		~Laborator7();

		void Init() override;

		//Se declara structura ce contine datele legate de picatura.
		struct DropOfWater {
			unsigned char healthPoints;	
			float currentSpeed;
			glm::vec3 position;

			DropOfWater(glm::vec3 position)
				: currentSpeed(-GRAV_ACC) , healthPoints(NUMBER_OF_BOUNCES) , position(position) {}

			void moveDrop(float dT, float *initialSpeed, unsigned char &f) {
				if (healthPoints != 0) {
					if (position.y == 1) {
						healthPoints--;
						f = 1;
						if (healthPoints != 0) {
							currentSpeed = initialSpeed[healthPoints];
							position.y += currentSpeed * dT;
						}
					} else
						if (position.y + currentSpeed * dT < 1) {
							position.y = 1;
						}
						else {
							position.y += currentSpeed * dT;
						}
					currentSpeed -= GRAV_ACC * dT;
				}
			}
		};

	private:
		void FrameStart() override;
		void Update(float deltaTimeSeconds) override;
		void FrameEnd() override;

		void RenderSimpleMesh(Mesh *mesh, Shader *shader, const glm::mat4 &modelMatrix, const glm::vec3 &color = glm::vec3(1));

		void OnInputUpdate(float deltaTime, int mods) override;
		void OnKeyPress(int key, int mods) override;
		void OnKeyRelease(int key, int mods) override;
		void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
		void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
		void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
		void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
		void OnWindowResize(int width, int height) override;

		glm::vec3 dropSourcePosition, lightPosition;
		unsigned int materialShininess;
		float materialKd;
		float materialKs;
		float initialSpeed[NUMBER_OF_BOUNCES], scalingFactor[NUMBER_OF_BOUNCES];
		GLfloat vertexPosition[VERTEX_MAX_NUM][3];
		GLushort indeces[ 2 * ( 3 * ( SQUARE_L - 1 ) * (SQUARE_L - 1) + 2 * (SQUARE_L - 1) ) ];
		GLuint posVBO, VAO, triVBO, VBO1;
		float A[21], centerX[21], centerY[21], radius[21], length[21];
		float rate;
		std::vector<DropOfWater> dropData;
		int numberOfWaves;
};
