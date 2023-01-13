#pragma once
#include "linalg.h"


bool FacesCamera(Vec3d t1, Vec3d t2, Vec3d t3) {
  // Calculate the normal of the triangle
  Vec3d normal = Normal(t1,t2,t3);
  double d = normal.dot(t1);
  return d > 0;
}

class Triangle {
public:
	Vec2d p1, p2, p3;
	double distFromCamera;
	bool facesCamera;
	uint16_t col;
};

class Object {
public:
	Object(Vec3d const& centre) : _centre(centre) {}
	virtual void update(uint32_t time) = 0;
	virtual std::vector<Triangle> getTriangles(Vec3d const&) = 0;
	Vec3d _centre{};
	Vec3d _rotation{};

protected:
	Eigen::MatrixXd rotate(const Eigen::MatrixXd& vertices, const Vec3d& angles) {
	  // Create quaternions for each rotation
	  Eigen::Quaterniond q_x(Eigen::AngleAxisd(angles[0] * M_PI/180.0, Eigen::Vector3d::UnitX()));
	  Eigen::Quaterniond q_y(Eigen::AngleAxisd(angles[1] * M_PI/180.0, Eigen::Vector3d::UnitY()));
	  Eigen::Quaterniond q_z(Eigen::AngleAxisd(angles[2] * M_PI/180.0, Eigen::Vector3d::UnitZ()));

	  // Rotate the vertices
	  return (q_z * q_y * q_x).toRotationMatrix() * vertices;
	}
};

class Cube : public Object {
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	Cube(Vec3d const& centre) : Object(centre) {}
	void update(uint32_t time){
		double const speed = 0.3;
		_rotation[0] = speed * 1 * time;
		_rotation[1] = speed * 2 * time;
		_rotation[2] = 30;
	}

	static double arrVert[8][3];

	std::vector<Triangle> getTriangles(Vec3d const& camera)
	{
		// Rotate + project the vertices onto the 2D plane
		// Vec3d vecPfc[8];
		Eigen::Matrix<double, 3, 8> vertices = Eigen::Matrix<double, 3, 8>::Map(arrVert[0]);
		Eigen::Matrix<double, 3, 8> matPfc = rotate(vertices, _rotation).colwise() + (_centre - camera);

		std::array<Vec2d, 8> projectedVertices;
		for (int i = 0; i < 8; i++) {
			projectedVertices[i] = {matPfc(0, i) / matPfc(2, i), matPfc(1, i) / matPfc(2, i)}; // focal plane 1 unit behind lens
		}

		std::vector<Triangle> triangles;
		triangles.reserve(12);

		// Create the triangles that make up the cube
		auto makeTri = [&](int i, int j, int k, uint16_t col) {
			bool facesCamera = FacesCamera(matPfc.col(i), matPfc.col(j), matPfc.col(k));

			if (facesCamera) {
				// shine if we face the light
				double facesLight = std::max(0.0, NormToPoint(matPfc.col(i), matPfc.col(j), matPfc.col(k), {2, 2, 0}));
				col = lerpCol(col, 0xffff, facesLight/2);

				// dark if we are far away
				double dist = matPfc.col(k)[2];//ShortestDistance(matPfc.col(i), matPfc.col(j), matPfc.col(k));
				double fade = clamp(sqrt(dist/20), 0.0, 2.0)/2;
				col = lerpCol(col, 0, fade);

				triangles.push_back({projectedVertices[i], projectedVertices[j], projectedVertices[k], dist, facesCamera, col});
			}
		};

		makeTri(0, 1, 2, mapColor(0.2));
		makeTri(2, 3, 0, mapColor(0.2));
		makeTri(1, 5, 6, mapColor(0.3));
		makeTri(6, 2, 1, mapColor(0.3));
		makeTri(7, 6, 5, mapColor(0.4));
		makeTri(5, 4, 7, mapColor(0.4));
		makeTri(4, 0, 3, mapColor(0.5));
		makeTri(3, 7, 4, mapColor(0.5));
		makeTri(4, 5, 1, mapColor(0.6));
		makeTri(1, 0, 4, mapColor(0.6));
		makeTri(3, 2, 6, mapColor(0.7));
		makeTri(6, 7, 3, mapColor(0.7));
		return triangles;
	}
};

double Cube::arrVert[8][3] = {{-0.5, -0.5, -0.5},
							  { 0.5, -0.5, -0.5},
							  { 0.5,  0.5, -0.5},
							  {-0.5,  0.5, -0.5},
							  {-0.5, -0.5,  0.5},
							  { 0.5, -0.5,  0.5},
							  { 0.5,  0.5,  0.5},
							  { -0.5, 0.5,  0.5}};
