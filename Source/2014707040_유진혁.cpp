#include <Windows.h>
#include <gl\GL.h>
#include <gl\GLU.h>
#include <gl\glut.h>
#include <fstream>
#include <cmath>
using namespace std;

typedef GLfloat Position[3];	// Position[0]: x, Position[1]: y, Position[2]: z
Position p[36][18];				// Coordinates of points
Position n[36][18];				// Normal vectors of vertex
GLubyte image[512][512][3];		// Texture image
string fileName = "marble.raw";

// Computing a vector between two points
void Vector(const Position& p1, const Position& p2, Position& v)
{
	v[0] = p2[0] - p1[0];
	v[1] = p2[1] - p1[1];
	v[2] = p2[2] - p1[2];
}

// Normalize a vector
void Normalize(Position& v)
{
	GLfloat d = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	if (d > 0)
	{
		GLfloat len = (1.0f / sqrt(d));
		v[0] *= len;
		v[1] *= len;
		v[2] *= len;
	}
}

// Cross product of v1 and v2
void CrossProduct(const Position& v1, const Position& v2, Position& r)
{
	r[0] = v1[1] * v2[2] - v1[2] * v2[1];
	r[1] = v1[2] * v2[0] - v1[0] * v2[2];
	r[2] = v1[0] * v2[1] - v1[1] * v2[0];
	Normalize(r);
}

// Initializes the coordinates of each point
void InitializePoint(float radius)
{
	float cos_10d = cos(10 * 3.14159 / 180.0);
	float sin_10d = sin(10 * 3.14159 / 180.0);
	// Initial circle on plane xy
	for (int j = 0; j < 18; j++)
	{
		float cos_th = cos(j * 20 * 3.14159 / 180.0);
		float sin_th = sin(j * 20 * 3.14159 / 180.0);
		p[0][j][0] = cos_th * radius + 3.0;
		p[0][j][1] = sin_th * radius + 3.0;
		p[0][j][2] = 0;
	}
	// Rotate initial circle around the y-axis
	for (int i = 1; i < 36; i++)
		for (int j = 0; j < 18; j++)
		{
			float sin_th = sin(j * 20 * 3.14159 / 180.0);
			p[i][j][0] = p[i - 1][j][0] * cos_10d + p[i - 1][j][2] * sin_10d;
			p[i][j][1] = sin_th * radius + 3.0;
			p[i][j][2] = -p[i - 1][j][0] * sin_10d + p[i - 1][j][2] * cos_10d;
		}
	// Assign normal vectors of vertex to array and normalize
	for (int i = 0; i < 36; i++)
		for (int j = 0; j < 18; j++)
		{
			Position v1, v2;
			Position n1;
			Vector(p[i][j], p[(i + 1) % 36][j], v1);
			Vector(p[i][j], p[i][(j + 1) % 18], v2);
			CrossProduct(v1, v2, n1);
			n[i][j][0] += n1[0]; n[i][j][1] += n1[1]; n[i][j][2] += n1[2];
			n[(i + 1) % 36][j][0] += n1[0]; n[(i + 1) % 36][j][1] += n1[1]; n[(i + 1) % 36][j][2] += n1[2];
			n[i][(j + 1) % 18][0] += n1[0]; n[i][(j + 1) % 18][1] += n1[1]; n[i][(j + 1) % 18][2] += n1[2];
			n[(i + 1) % 36][(j + 1) % 18][0] += n1[0]; n[(i + 1) % 36][(j + 1) % 18][1] += n1[1]; n[(i + 1) % 36][(j + 1) % 18][2] += n1[2];
		}
	for (int i = 0; i < 36; i++)
		for (int j = 0; j < 18; j++)
			Normalize(n[i][j]);
}

// Draw axes
void DrawAxis()
{
	glLineWidth(1);
	glBegin(GL_LINES);
	{
		glColor3f(1, 0, 0);		// red x-axis
		glVertex3f(0, 0, 0);
		glVertex3f(20, 0, 0);
		glColor3f(0, 1, 0);		// green y-axis
		glVertex3f(0, 0, 0);
		glVertex3f(0, 20, 0);
		glColor3f(0, 0, 1);		// blue z-axis
		glVertex3f(0, 0, 0);
		glVertex3f(0, 0, 20);

	}
	glEnd();
}

// Draw polygons
void DrawTorus()
{
	glBegin(GL_QUADS);
	{
		for (int i = 0; i < 36; i++)
		{
			for (int j = 0; j < 18; j++)
			{
				// The lower-left vertex of the quad
				glNormal3fv(n[i][j]);
				glTexCoord2f(i / 36.0, j / 18.0);
				glVertex3fv(p[i][j]);

				// The lower-right vertex of the quad
				glNormal3fv(n[(i + 1) % 36][j]);
				glTexCoord2f((i + 1) / 36.0, j / 18.0);
				glVertex3fv(p[(i + 1) % 36][j]);

				// The upper-right vertex of the quad
				glNormal3fv(n[(i + 1) % 36][(j + 1) % 18]);
				glTexCoord2f((i + 1) / 36.0, (j + 1) / 18.0);
				glVertex3fv(p[(i + 1) % 36][(j + 1) % 18]);

				// The upper-left vertex of the quad
				glNormal3fv(n[i][(j + 1) % 18]);
				glTexCoord2f(i / 36.0, (j + 1) / 18.0);
				glVertex3fv(p[i][(j + 1) % 18]);
			}
		}
	}
	glEnd();
}

void LoadTexture()
{
	// Open the texture image file
	ifstream fin;
	fin.open(fileName, ios::binary);
	for (int i = 0; i < 512; i++)
		for (int j = 0; j < 512; j++)
		{
			image[i][j][0] = fin.get();
			image[i][j][1] = fin.get();
			image[i][j][2] = fin.get();
		}
	fin.close();

	// Specify a texture image and parameters
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 512, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

// Render a torus
void RenderTorus()
{
	// Clear the window with current clearing color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(0.6, 0.8, 1, 0, 0, 0, 0.1, 1, 0);

	DrawAxis();

	// Use white as material
	glColor3f(1, 1, 1);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	
	// Turn on the light
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// Start drawing with the specified texture
	LoadTexture();
	glEnable(GL_TEXTURE_2D);

	InitializePoint(1.6);
	DrawTorus();

	// Finish drawing with the specified texture
	glDisable(GL_TEXTURE_2D);

	// Turn off the light
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);

	glutSwapBuffers();
}

// Setup the rendering state
void SetupRC(void)
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
}

// Called by GLUT library when the window has changed size
void ChangeSize(int w, int h)
{
	// Prevent a divide by zero
	if (h == 0)
		h = 1;

	// Set viewport to window dimension
	glViewport(0, 0, w, h);

	// Reset coordinate system
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Establish clipping volume (left, right, bottom, top, near, far)
	if (w <= h)
		glOrtho(-8.0, 8.0, -8.0*(float)h / (float)w, 8.0*(float)h / (float)w, -40.0, 40.0);
	else
		glOrtho(-8.0*(float)w / (float)h, 8.0*(float)w / (float)h, -8.0, 8.0, -40.0, 40.0);
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case '1':
		fileName = "marble.raw";
		glutPostRedisplay();
		break;
	case '2':
		fileName = "wood.raw";
		glutPostRedisplay();
		break;
	case '3':
		fileName = "check.raw";
		glutPostRedisplay();
		break;
	default:
		break;
	}
}

// Main program entry point
void main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(600, 600);
	glutCreateWindow("OpenGL HW 3");
	glutDisplayFunc(RenderTorus);
	glutReshapeFunc(ChangeSize);
	glutKeyboardFunc(keyboard);

	SetupRC();

	glutMainLoop();
}