#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include "parse_hyg.h"

#define DATABASE_FILE "hygdata_v3.csv"
#define MIN_MAG 5.0
#define ERRMAG 132

enum state {
	STARID,
	HIP,
	HD,
	HR,
	GL,
	BF,
	PROPER,
	RA,
	DEC,
	DIST,
	PMRA,
	PMDEC,
	RV,
	MAG,
	ABSMAG,
	SPECT,
	CI,
	X, Y, Z,
	VX, VY, VZ,
	RARAD,
	DECRAD,
	PMRARAD,
	PMDECRAD,
	BAYER,
	FLAM,
	CON,
	COMP,
	COMP_PRIMARY,
	BASE,
	LUM,
	VAR,
	VAR_MIN,
	VAR_MAX,
};

int parse_line(struct star *star, char *buf, ssize_t len, double *max_mag, double *max_distance)
{
	int c, last = 0;
	enum state state = STARID;
	for (c = 0; c < len; c++) {
		if (buf[c] == ',') {
			switch (state) {
			case STARID:
				star->id = strtol(buf + last, NULL, 10);
				if (star->id == 0) return -ERRMAG;
				break;
			case MAG:
				star->mag = strtod(buf + last, NULL);
				if (star->mag > MIN_MAG) {
					return -ERRMAG;
				}
				break;
			case X:
				star->x = strtod(buf + last, NULL);
				break;
			case Y:
				star->y = strtod(buf + last, NULL);
				break;
			case Z:
				star->z = strtod(buf + last, NULL);
				break;
			case VX:
				star->vx = strtod(buf + last, NULL);
				break;
			case VY:
				star->vy = strtod(buf + last, NULL);
				break;
			case VZ:
				star->vz = strtod(buf + last, NULL);
				break;
			}
			last = c + 1;
			state++;
		}
	}
	if (state != VAR_MAX) {
		fprintf(stderr, "poorly formed line! - end state: %d\n", state);
		return -EFAULT;
	}
	if (star->mag < *max_mag) {
		*max_mag = star->mag;
	}
	if (fabs(star->x) > *max_distance) {
		*max_distance = star->x;
	}
	if (fabs(star->y) > *max_distance) {
		*max_distance = star->y;
	}
	if (fabs(star->z) > *max_distance) {
		*max_distance = star->z;
	}
	//printf("star: %d %.2f %.2f %.2f %.2f\n", star->id, star->mag, star->x, star->y, star->z);
	return 0;
}

struct star *parse_stars(double *max_mag, double *max_distance)
{
	FILE *db;
	char *buf;
	struct star *stars, *curstar, *newstar;
	int line_count, star_count;
	size_t len = 0;
	ssize_t read;
	enum state state;
	int rc;

	*max_mag = MIN_MAG;
	*max_distance = 0;

	db = fopen(DATABASE_FILE, "r");
	if (db == NULL) {
		rc = errno;
		fprintf(stderr, "Failed to open %s\n", DATABASE_FILE);
		return NULL;
	}

	newstar = malloc(sizeof(struct star));
	if (newstar == NULL) {
		fprintf(stderr, "failed to allocate star\n");
		return NULL;
	}
	stars = newstar;
	curstar = newstar;

	line_count = 0;
	star_count = 0;
	while ((read = getline(&buf, &len, db)) != -1) {
		line_count++;
		rc = parse_line(curstar, buf, read, max_mag, max_distance);
		if (rc == -ERRMAG) {
			continue;
		} else if (rc < 0) {
			fprintf(stderr, "error on line %d\n", line_count);
			return NULL;
		}
		newstar = malloc(sizeof(struct star));
		if (newstar == NULL) {
			fprintf(stderr, "alloc failed %d\n", line_count);
			return NULL;
		}
		curstar->next = newstar;
		curstar = newstar;
		star_count++;
	}
	printf("loaded %d stars < %.2f\n", star_count, MIN_MAG);
	printf("max magnitude: %.2f\n", *max_mag);
	printf("max distance (parsec): %.2f\n", *max_distance);
	fclose(db);
	return stars;
}
