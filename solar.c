#include <GL/glut.h>
#include <SpiceUsr.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include "parse_hyg.h"

#define ALTITUDE_KM_OUTER 7E9
#define ALTITUDE_KM_INNER 5E8
#define AUTO_ALTITUDE_CUTOFF ALTITUDE_KM_INNER
#define PARSEC_KM 3.086E13
#define FOV_DEGREES 90.0
#define WIN_WIDTH_PIXEL 1024
#define WIN_HEIGHT_PIXEL 1024
#define SECONDS_PER_DAY 86400

static unsigned long long int timestep_days;
static unsigned long long int epoch_time;
static double max_distance_km;
static double max_mag;
static double altitude_km;

struct camera_position {
	double x; double y; double z;
	double lx; double ly; double lz;
	double up_x; double up_y; double up_z;
};

static struct camera_position cp = { 0 };

enum view_modes {
	OUTER,
	INNER,
	SAT,
	AUTO,
};

enum solar_satellites {
	mercury = 1,
	venus = 2,
	earth = 3,
	mars = 4,
	jupiter = 5,
	saturn = 6,
	uranus = 7,
	nepture = 8,
	pluto = 9,
};

static const double planet_colors[pluto][3] = {
	{.7, .2, .2},
	{.8, 1, .3},
	{0, 0, 1},
	{1, 0, 0},
	{1, .5, 0},
	{1, 1, .5},
	{0, 0, .5},
	{.2, .2, .5},
	{.6, .6, .6},
};

static struct star *stars;
static enum view_modes view_mode;

void render_planet(int planet_id)
{
	double state[3];
	double lt;
	double r, g, b;

	r = planet_colors[planet_id - 1][0];
	g = planet_colors[planet_id - 1][1];
	b = planet_colors[planet_id - 1][2];
	glColor4d(r, g, b, 1.0);

	/* planet relative to solar system barycenter */
	spkezp_c(planet_id, epoch_time, "J2000", "None", 0, state, &lt);

	glPointSize(10.0f);
	glBegin(GL_POINTS);
	glColor4d(r, g, b, 1.0);
	glVertex3f(state[0], state[1], state[2]);
	glEnd();
}

void render_sun()
{
	glPointSize(12.0f);
	glBegin(GL_POINTS);
	glColor4d(1, 1, 0, 1);
	glVertex3f(0, 0, 0);
	glEnd();
}

double mag_to_brightness(double mag)
{
	double brightness;

	brightness = 1 - log10(mag + max_mag);
	if(brightness < .4) 
		return .4;
	return brightness;
}

void render_stars(struct star *stars)
{
	struct star *curstar = stars;
	double brightness;

	glPointSize(2.5);
	glBegin(GL_POINTS);
	while(curstar->next != NULL) {
		brightness = mag_to_brightness(curstar->mag);
		glColor3d(brightness, brightness, brightness);
		glVertex3f(curstar->x * PARSEC_KM, curstar->y * PARSEC_KM, curstar->z * PARSEC_KM);
		curstar = curstar->next;
	}
	glEnd();
}

void idle(void)
{
	timestep_days += 1;
	epoch_time = SECONDS_PER_DAY * timestep_days;
	glutPostRedisplay();
}

void resize_window(int w, int h)
{
	float ratio;

	if (h == 0) {
		h = 1;
	}
	ratio =  w * 1.0 / h;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, w, h);
	gluPerspective(FOV_DEGREES, ratio, 0.2f, max_distance_km + altitude_km);
	glMatrixMode(GL_MODELVIEW);
}

void render(void)
{
	int id;

	glClearColor(0, 0, 0, 1);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	if (view_mode == SAT) {
		gluLookAt(0.0, 0.0, 0.0,
			  0.0, 1.0, 0.0,
			  0.0, 0.0, 1.0);
	} else if (view_mode == AUTO) {
		if(cp.z < AUTO_ALTITUDE_CUTOFF) {
			cp.z += fabs(AUTO_ALTITUDE_CUTOFF - cp.z) / 200;
		}
		gluLookAt(cp.x, cp.y, cp.z,
			  cp.lx, cp.ly, cp.lz,
			  cp.up_x, cp.up_y, cp.up_z);
	} else {
		gluLookAt(0.0, 0.0, -altitude_km,
			  0.0, 0.0, 0.0,
			  0.0, 1.0, 0.0);
	}
	render_sun();
	render_stars(stars);
	for (id = mercury; id <= pluto; id++) {
		render_planet(id);
	}
	glFlush();
	glutSwapBuffers();
}

int main(int argc, char* argv[]) {

	double max_distance_parsec;
	stars = parse_stars(&max_mag, &max_distance_parsec);
	max_distance_km = max_distance_parsec * PARSEC_KM;
	if (stars == NULL) {
		fprintf(stderr, "failed to load stars\n");
		return 1;
	}
	
	view_mode = OUTER;
	altitude_km = ALTITUDE_KM_OUTER;
	if(argc > 1) {
		if(strcmp(argv[1], "--inner") == 0) {
			view_mode = INNER;
			altitude_km = ALTITUDE_KM_INNER;
		} else if (strcmp(argv[1], "--sat") == 0) {
			view_mode = SAT;
		} else if (strcmp(argv[1], "--auto") == 0) {
			view_mode = AUTO;
			cp.z = -altitude_km;
			cp.y = -altitude_km;
			cp.up_z = 1.0;
		}
	}
	furnsh_c("de430.bsp");
	glutInit(&argc, argv);  
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WIN_WIDTH_PIXEL, WIN_HEIGHT_PIXEL);
	glutCreateWindow("astronav");
	glutDisplayFunc(render);
	glutReshapeFunc(resize_window);
	glutIdleFunc(idle);
	//glEnable(GL_DEPTH_TEST);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	timestep_days = 0;
	glutMainLoop();
	return EXIT_SUCCESS;  
}
