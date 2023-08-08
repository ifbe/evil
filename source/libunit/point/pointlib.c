#include<math.h>




void vectornormalize(float* v)
{
	float norm = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
	v[0] /= norm;
	v[1] /= norm;
	v[2] /= norm;
}
void vectorcross(float* v, float* x)
{
	//a × b= [aybz-azby,azbx-axbz, axby-aybx]
	float t[3] = {v[0], v[1], v[2]};
	v[0] = t[1]*x[2] - t[2]*x[1];
	v[1] = t[2]*x[0] - t[0]*x[2];
	v[2] = t[0]*x[1] - t[1]*x[0];
}
void trianglenormal(float* n, float* a, float* b, float* c)
{
	float p[3];

	n[0] = b[0] - a[0];
	n[1] = b[1] - a[1];
	n[2] = b[2] - a[2];

	p[0] = c[0] - a[0];
	p[1] = c[1] - a[1];
	p[2] = c[2] - a[2];

	vectorcross(n, p);
	vectornormalize(n);
}
void quaternionrotate(float* v, float* q)
{
	//t = 2 * cross(q.xyz, v)
	//v' = v + q.w * t + cross(q.xyz, t)
	//float tx = 2*q[2]*v[2]
	float j[3];
	float k[3];
	j[0] = (q[1]*v[2]-q[2]*v[1]) * 2;
	j[1] = (q[2]*v[0]-q[0]*v[2]) * 2;
	j[2] = (q[0]*v[1]-q[1]*v[0]) * 2;

	k[0] = q[1]*j[2]-q[2]*j[1];
	k[1] = q[2]*j[0]-q[0]*j[2];
	k[2] = q[0]*j[1]-q[1]*j[0];

	v[0] += q[3]*j[0] + k[0];
	v[1] += q[3]*j[1] + k[1];
	v[2] += q[3]*j[2] + k[2];
}