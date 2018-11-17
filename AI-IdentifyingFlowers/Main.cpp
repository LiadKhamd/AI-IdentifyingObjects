#include "Functions.h"

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(W, H);
	glutInitWindowPosition(200, 100);
	glutCreateWindow("AI-IdentifyingFlowers");

	glutDisplayFunc(display); // refresh function
	glutIdleFunc(idle); // idle: when nothing happens
	init();

	glutCreateMenu(Menu);
	glutAddMenuEntry("Start learning", 1);
	glutAddMenuEntry("Test ANN", 2);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	glutMainLoop();
}