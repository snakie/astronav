struct star {
	int id;
	double mag;
	double x; double y; double z;
	double vx; double vy; double vz;
	struct star *next;
};

struct star *parse_stars(double *max_mag, double *max_distance);
