#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>

#include <GL/glew.h>

#include <GLFW/glfw3.h>
GLFWwindow* window;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "../common/shader.hpp"
#include "Triangles.hpp"
#include "ShapeManager.hpp"

color colorWheel(double theta) {
    return color(
        (sin(theta + 0.000) + 1) / 2,
        (sin(theta + 2.094) + 1) / 2,
        (sin(theta + 4.188) + 1) / 2
    );
}

struct Object {
    point pos;
    point vel;
    point acc;
    Object(const point Pos, const point Acc) {
        pos = Pos;
        acc = Acc; 
    }
    void tick() {
        vel.x += acc.x;
        vel.y += acc.y;
        acc.x = 0;
        acc.y = 0;
        pos.x += vel.x;
        pos.y += vel.y;
    }
};

int main( void )
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	} 

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	//window = glfwCreateWindow( 2457, 1689, "Playground", NULL, NULL);
	point windowSize(2457, 1689);
	window = glfwCreateWindow( windowSize.x, windowSize.y, "Playground", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	//float tickTimer = glfwGetTime();

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	ShapeManager sm;

	RectShape rect(
		point(-1.0, -1.0),
		point(2.0, 2.0),
		color(1, 0, 0), sm
	);


	//printBuffer(tm.getColorBuffer());

	unsigned int VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	unsigned int programID = LoadShaders( "vertexShader.glsl", "fragmentShader.glsl" );

	unsigned int MatrixID = glGetUniformLocation(programID, "MVP");

	unsigned int vertexBufferID;
	glGenBuffers(1, &vertexBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);

	unsigned int indexBufferID;
	glGenBuffers(1, &indexBufferID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

	unsigned int colorBufferID;
	glGenBuffers(1, &colorBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);

	const float gridSize = 10.0f;
	const float gridRatio = windowSize.x / windowSize.y;

	glm::mat4 Projection = glm::ortho(-gridSize*gridRatio,gridSize*gridRatio,-gridSize,gridSize,0.0f,100.0f); // In world coordinates

	glm::mat4 View = glm::lookAt(
		glm::vec3(0,0,5), // Camera is at (4,3,3), in World Space
		glm::vec3(0,0,0), // and looks at the origin
		glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
	);

	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model = glm::mat4(1.0f);
	// Our ModelViewProjection : multiplication of our 3 matrices
	glm::mat4 MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around

	float tickTimer = glfwGetTime();

	Object orb(point(0, 0), point(0, 0));

	do{

		const std::vector<float> vertexBuffer = sm.tm.getVertexBuffer();
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertexBuffer[0]) * vertexBuffer.size(), &vertexBuffer[0], GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		const std::vector<unsigned int> indexBuffer = sm.tm.getIndexBuffer();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexBuffer[0]) * indexBuffer.size(), &indexBuffer[0], GL_DYNAMIC_DRAW);
		

		const std::vector<float> colorBuffer = sm.tm.getColorBuffer();
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(colorBuffer[0]) * colorBuffer.size(), &colorBuffer[0], GL_DYNAMIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		
		//glClear(GL_COLOR_BUFFER_BIT); // Clear Buffer

		//sm.updateColor(rect, colorWheel(glfwGetTime()));
		//if (glfwGetKey(window, GLFW_KEY_ENTER ) && pressed == false) {
		// if (glfwGetTime() - tickTimer >= 0.02) {
		// 	tickTimer = glfwGetTime();
		// }
		orb.acc.x = sin(glfwGetTime() * 3.0) / 100.0;
		orb.acc.y = cos(glfwGetTime() * 3.0) / 100.0;
		orb.tick();

		rect.setColor(colorWheel(glfwGetTime() * 2));
		rect.setPos(orb.pos);

		if (orb.pos.x >  10 * gridRatio){
            orb.pos.x = -10 * gridRatio;
        }
        if (orb.pos.x < -10 * gridRatio){
            orb.pos.x = 10 * gridRatio;
        }
        if (orb.pos.y > 10){
            orb.pos.y = -10;
        }
        if (orb.pos.y < -10){
            orb.pos.y = 10;
        }
		//sm.updateColor(rect, colorWheel(glfwGetTime() * 2));

		static bool leftPressed = false;
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !leftPressed) {
			leftPressed = true;
			double cursorX;
			double cursorY;
			glfwGetCursorPos(window, &cursorX, &cursorY);
			//if (cursorX) {}
			std::cout << "(" << cursorX << ", " << cursorY << ")" << std::endl;
		}
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE && leftPressed) {
			leftPressed = false;
		}


		// static bool pressed = false;;
		// if (glfwGetKey(window, GLFW_KEY_ENTER ) == GLFW_RELEASE) {
		// 	pressed = false;
		// }


		// if (glfwGetTime() - tickTimer >= 0.01 && pressed) {
		// 	tickTimer = glfwGetTime();
		// 	pressed = false;
		// }

		glDrawElements(GL_TRIANGLES, indexBuffer.size(), GL_UNSIGNED_INT, (void*)0);
		//glDrawArrays(GL_TRIANGLES, 0, (int)(buffer.size() / 2));

		glDisableVertexAttribArray(0);
		glUseProgram(programID);
		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	glDeleteBuffers(1, &vertexBufferID);
	glDeleteVertexArrays(1, &VertexArrayID);
	glDeleteProgram(programID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

