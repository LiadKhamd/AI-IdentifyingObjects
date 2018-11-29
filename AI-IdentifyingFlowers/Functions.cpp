#include "Functions.h"

unsigned char picture[TMPSZ][TMPSZ][3]; // for R,G,B
unsigned char screen[SCRSZ][SCRSZ][3]; // for R,G,B
unsigned char squares[SCRSZ][SCRSZ][3]; // for R,G,B

double input[INPUT_SZ];
double hidden[HIDDEN_SZ];
double output[OUTPUT_SZ];
double i2h[INPUT_SZ][HIDDEN_SZ];
double h2o[HIDDEN_SZ][OUTPUT_SZ];
double error[OUTPUT_SZ];
double delta_output[OUTPUT_SZ];
double delta_hidden[HIDDEN_SZ];
int network_digit = -1, tutor_digit = -1;
double learning_rate = 0.1;

int iterLern = 0;

bool test_ANN = false;
int testIter = 0;
bool start_learning = false, interDone = false;

unsigned char* bmp;

void LoadBitmap(char * filename)
{
	int sz;
	BITMAPFILEHEADER bf;
	BITMAPINFOHEADER bi;
	FILE* pf = fopen(filename, "rb"); // read binary file

	if (!pf)
	{
		printf("File problem: %s\n", filename);
		exit(1);
	}

	fread(&bf, sizeof(bf), 1, pf);
	fread(&bi, sizeof(bi), 1, pf);
	sz = bi.biHeight * bi.biWidth * 3;

	bmp = (unsigned char*)malloc(sz);

	fread(bmp, 1, sz, pf);
	fclose(pf);
}

void LoadImage(char* name) {
	int k, sz = TMPSZ*TMPSZ * 3;
	int i, j;

	LoadBitmap(name);

	for (k = 0, j = 0, i = 0; k < sz; k += 3)
	{
		picture[i][j][2] = bmp[k]; //blue
		picture[i][j][1] = bmp[k + 1]; // green
		picture[i][j][0] = bmp[k + 2]; // red
		j++;
		if (j == TMPSZ) // fill next line
		{
			j = 0;
			i++;
		}
	}

	free(bmp);

	// copy picture to screen
	for (i = 0; i < SCRSZ; i++)
		for (j = 0; j < SCRSZ; j++)
		{
			screen[i][j][0] = (picture[i * 2][j * 2][0] + picture[i * 2][j * 2 + 1][0] +
				picture[i * 2 + 1][j * 2][0] + picture[i * 2 + 1][j * 2 + 1][0]) / 4;
			screen[i][j][1] = (picture[i * 2][j * 2][1] + picture[i * 2][j * 2 + 1][1] +
				picture[i * 2 + 1][j * 2][1] + picture[i * 2 + 1][j * 2 + 1][1]) / 4;
			screen[i][j][2] = (picture[i * 2][j * 2][2] + picture[i * 2][j * 2 + 1][2] +
				picture[i * 2 + 1][j * 2][2] + picture[i * 2 + 1][j * 2 + 1][2]) / 4;
		}
}

double sigmoid(double x) {
	return(1.0 / (1.0 + exp(-x)));
}

void Clean()
{
	int i, j;
	for (i = 0; i < SCRSZ; i++)
		for (j = 0; j < SCRSZ; j++)
		{
			screen[i][j][0] = 255;
			screen[i][j][1] = 255;
			screen[i][j][2] = 255;
		}

	for (i = 0; i < SCRSZ; i++)
		for (j = 0; j < SCRSZ; j++)
		{
			squares[i][j][0] = 255;
			squares[i][j][1] = 255;
			squares[i][j][2] = 255;
		}
}

void init()
{
	int i, j;

	srand(time(0));
	Clean();

	// set random weights
	for (i = 0; i < INPUT_SZ; i++)
		for (j = 0; j < HIDDEN_SZ; j++)
			i2h[i][j] = ((rand() % 1000) - 500) / 1000.0;

	// set random weights
	for (i = 0; i < HIDDEN_SZ; i++)
		for (j = 0; j < OUTPUT_SZ; j++)
			h2o[i][j] = ((rand() % 1000) - 500) / 1000.0;

	glClearColor(GLclampf(0.3), GLclampf(0.3), GLclampf(0.3), 0);

	glOrtho(-1, 1, -1, 1, -1, 1);
}

void DrawSquares()
{
	double top = 0.25;
	double left = 0.1;
	double right = 0.2;
	double bottom = 0.15;

	for (int i = 0; i < OUTPUT_SZ; i++)
	{
		if (tutor_digit == i)
			glColor3d(0, 0.7, 0);
		else if (network_digit == i)
			glColor3d(0.7, 0.6, 0.2);
		else glColor3d(0.5, 0.5, 0.5);

		glBegin(GL_POLYGON);
		glVertex2d(left, top);
		glVertex2d(right, top);
		glVertex2d(right, bottom);
		glVertex2d(left, bottom);
		glEnd();

		glColor3d(1, 1, 1);

		glRasterPos2d(left + 0.15, bottom + 0.025);
		int len = strlen(FLOWERS[i]);
		for (int j = 0; j < len; j++)
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, FLOWERS[i][j]);

		top -= 0.2;
		bottom -= 0.2;
	}
}

void HPF()
{
	int i, j;
	for (i = 1; i < SCRSZ - 1; i++)
		for (j = 1; j < SCRSZ - 1; j++)
			squares[i][j][0] = squares[i][j][1] = squares[i][j][2] =
			(int)fabs(4 * screen[i][j][0] - screen[i - 1][j][0] -
				screen[i + 1][j][0] - screen[i][j - 1][0] - screen[i][j + 1][0]);
}

int MaxOutput()
{
	int i, max = 0;
	for (i = 1; i < OUTPUT_SZ; i++)
		if (output[i] > output[max])
			max = i;
	return max;
}

void FeedForward()
{

	int i, j;
	//printf("BEFORE OUTPUT\n");
	//for (i = 0; i < OUTPUT_SZ; i++)
	//	printf("%.3lf ", output[i]);

	//printf("\n\n");

	// 1. setup input layer
	for (i = 0; i < SCRSZ - 1; i += 2)
	{
		int c = 0;
		for (j = 0; j < 3; j++)
			c += squares[i][i][j] + squares[i][i + 1][j] + squares[i + 1][i][j] + squares[i + 1][i + 1][j];
		input[i] = c / 255;
		if (input[i] != 0)
			input[i] = input[i] / input[i];
	}

	input[INPUT_SZ - 1] = 1; // bias for input layer

	// 2. getting Hidden layer
	for (i = 0; i < HIDDEN_SZ; i++)
		hidden[i] = 0;
	for (i = 0; i < INPUT_SZ; i++)
		for (j = 0; j < HIDDEN_SZ; j++)
		{
			hidden[j] += input[i] * i2h[i][j];
		}

	// add sigmoid
	for (i = 0; i < HIDDEN_SZ; i++)
		hidden[i] = sigmoid(hidden[i]);

	// set bias for hidden layer
	hidden[HIDDEN_SZ - 1] = 1;

	// 3. getting output layer
	for (i = 0; i < OUTPUT_SZ; i++)
		output[i] = 0;
	for (i = 0; i < HIDDEN_SZ; i++)
		for (j = 0; j < OUTPUT_SZ; j++)
			output[j] += hidden[i] * h2o[i][j];

	// add sigmoid
	for (i = 0; i < OUTPUT_SZ; i++)
		output[i] = sigmoid(output[i]);

	// show it
	//printf("OUTPUT\n");
	//for (i = 0; i < OUTPUT_SZ; i++)
	//	printf("%.3lf ", output[i]);

	//printf("\n\n");

	network_digit = MaxOutput();
}

void Backpropagation()
{
	int i, j, k;
	// 1. Compute error E = (t(i)-y(i))
	for (i = 0; i < OUTPUT_SZ; i++)
	{
		if (i == tutor_digit)
			error[i] = (1 - output[i]);
		else
			error[i] = -output[i];
	}
	// 2. compute delta of output layer
	for (i = 0; i < OUTPUT_SZ; i++)
	{
		delta_output[i] = output[i] * (1 - output[i])*error[i];
	}
	// 3.  compute delta of hidden layer
	for (j = 0; j < HIDDEN_SZ; j++)
	{
		double tmp = 0;
		for (k = 0; k < OUTPUT_SZ; k++)
			tmp += delta_output[k] * h2o[j][k];

		delta_hidden[j] = hidden[j] * (1 - hidden[j])*tmp;
	}
	// 4. update weights in h2o
	for (i = 0; i < HIDDEN_SZ; i++)
		for (j = 0; j < OUTPUT_SZ; j++)
			h2o[i][j] += learning_rate*hidden[i] * delta_output[j];
	// 5. update weights in i2h
	for (i = 0; i < INPUT_SZ; i++)
		for (j = 0; j < HIDDEN_SZ; j++)
			i2h[i][j] += learning_rate*input[i] * delta_hidden[j];
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT);

	DrawSquares();
	// show screen
	glRasterPos2d(-0.95, 0.05);
	glDrawPixels(SCRSZ, SCRSZ, GL_RGB, GL_UNSIGNED_BYTE, screen);

	// show squares
	glRasterPos2d(-0.95, -0.90);
	glDrawPixels(SCRSZ, SCRSZ, GL_RGB, GL_UNSIGNED_BYTE, squares);

	glutSwapBuffers();// show what was drawn in "frame buffer"
}

void startLearning() {
	static int numberOfSession = 1;
	char name[7] = "  .bmp";
	if (iterLern < LEARNING_SEESION)
	{
		if (interDone || iterLern == 0)
		{
			name[0] = FLOWERS[rand() % OUTPUT_SZ][0];
			name[1] = (iterLern / OUTPUT_SZ) % PIC_TYPES + 1 + '0';
			//printf("%s\n", name);
			LoadImage(name);
			HPF();
			tutor_digit = iterLern%OUTPUT_SZ;
			iterLern++;
			interDone = false;
		}
		FeedForward();
		if (tutor_digit == network_digit)
			interDone = true;
		Backpropagation();
	}
	else
	{
		if (!test_ANN)
		{
			printf("ANN learn: %d iteration\n", numberOfSession*LEARNING_SEESION);
			if (numberOfSession >= NUMBER_OF_SEESION)
				start_learning = false;
			else
			{
				numberOfSession++;
				iterLern = 0;
			}
			test_ANN = true;
		}
		else
			testANN();
		tutor_digit = network_digit = -1;
		Clean();
	}
}

void testANN() {
	static int accept = 0;
	static int last = 0;
	char name[11] = "  Test.bmp";
	if (testIter < OUTPUT_SZ*TEST_ITEM)
	{
		name[0] = FLOWERS[testIter%OUTPUT_SZ][0];
		name[1] = testIter / OUTPUT_SZ + '0';
		//printf("%s\n", name);
		LoadImage(name);
		HPF();
		FeedForward();
		network_digit = MaxOutput();
		tutor_digit = testIter% OUTPUT_SZ;
		if (tutor_digit == network_digit)
			accept++;
		testIter++;
	}
	else
	{
		if (accept > last)
			learning_rate *= 0.90;
		else
		{
			if (accept < last)
				learning_rate *= 1.1;
			else
				learning_rate *= 0.95;
		}
		printf("Test Done!!!\tResult: %d\\%d\t%f\n", accept, OUTPUT_SZ*TEST_ITEM, learning_rate);
		last = accept;
		testIter = accept = 0;
		test_ANN = false;
	}
}

void Menu(int choice)
{
	switch (choice)
	{
	case 1:
		start_learning = true;
		break;
	case 2:
		test_ANN = true;
		break;
	}
}

void idle()
{
	if (start_learning)
		startLearning();
	if (test_ANN)
		testANN();
	glutPostRedisplay();// calls indirectly to display
}