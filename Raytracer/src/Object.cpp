#include "Object.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <gtc/constants.hpp>
#include <gtx/component_wise.hpp>

bool Sphere::Intersect(const Ray& ray, HitInfo& hitInfo) const {
	glm::vec3 distance = ray.pos - pos;
	float p1 = -glm::dot(ray.direction, distance);
	float p2sqr = p1 * p1 - glm::dot(distance, distance) + radius * radius;
	if (p2sqr < 0)
		return false;
	
	hitInfo.distance = p1 - sqrt(p2sqr);
	if (hitInfo.distance < 0)
		return false;

	hitInfo.point = ray.pos + hitInfo.distance * ray.direction;
	hitInfo.normal = glm::normalize(hitInfo.point - pos);
	hitInfo.object = (Object*)this;

	hitInfo.uv = glm::vec2(glm::atan(hitInfo.normal.x, hitInfo.normal.z) / (2*glm::pi<float>()) + 0.5f, hitInfo.normal.y * 0.5f + 0.5f);

	return true;
}
bool Sphere::GetBoundingBox(BoundingBox& outBox) const {
	outBox = BoundingBox(
		pos - glm::vec3(radius, radius, radius),
		pos + glm::vec3(radius, radius, radius)
	);
	return true;
}
bool AxisAlignedCube::Intersect(const Ray& ray, HitInfo& hitInfo) const {
	float tmin, tmax, tymin, tymax, tzmin, tzmax;
	glm::vec3 axisNormalCandidate;

	if (ray.direction.x >= 0) {
		tmin = (minCoord.x - ray.pos.x) / ray.direction.x;
		tmax = (maxCoord.x - ray.pos.x) / ray.direction.x;

		hitInfo.normal = { -1, 0, 0 };
	}
	else {
		tmin = (maxCoord.x - ray.pos.x) / ray.direction.x;
		tmax = (minCoord.x - ray.pos.x) / ray.direction.x;

		hitInfo.normal = { 1, 0, 0 };
	}

	if (ray.direction.y >= 0) {
		tymin = (minCoord.y - ray.pos.y) / ray.direction.y;
		tymax = (maxCoord.y - ray.pos.y) / ray.direction.y;

		axisNormalCandidate = { 0, -1, 0 };
	}
	else {
		tymin = (maxCoord.y - ray.pos.y) / ray.direction.y;
		tymax = (minCoord.y - ray.pos.y) / ray.direction.y;

		axisNormalCandidate = { 0, 1, 0 };
	}

	if ((tmin > tymax) || (tymin > tmax))
		return false;

	if (tymin > tmin) {
		tmin = tymin;
		hitInfo.normal = axisNormalCandidate;
	}

	if (tymax < tmax)
		tmax = tymax;

	if (ray.direction.z >= 0) {
		tzmin = (minCoord.z - ray.pos.z) / ray.direction.z;
		tzmax = (maxCoord.z - ray.pos.z) / ray.direction.z;

		axisNormalCandidate = { 0, 0, -1 };
	}
	else {
		tzmin = (maxCoord.z - ray.pos.z) / ray.direction.z;
		tzmax = (minCoord.z - ray.pos.z) / ray.direction.z;

		axisNormalCandidate = { 0, 0, 1 };
	}

	if ((tmin > tzmax) || (tzmin > tmax))
		return false;

	if (tzmin > tmin) {
		tmin = tzmin;
		hitInfo.normal = axisNormalCandidate;
	}

	if (tzmax < tmax)
		tmax = tzmax;

	// Filter out collisions where the min behind player.
	if (tmin < 0.0f)
		return false;

	hitInfo.distance = tmin;
	hitInfo.object = (Object*)this;
	hitInfo.point = ray.pos + ray.direction * tmin;
	hitInfo.point += hitInfo.normal * 0.02f;

	glm::vec3 minToMax01 = (hitInfo.point - minCoord) / (maxCoord - minCoord);
	if (hitInfo.normal.x == 1 || hitInfo.normal.x == -1)
		hitInfo.uv = { minToMax01.z, minToMax01.y };
	else if (hitInfo.normal.y == 1 || hitInfo.normal.y == -1)
		hitInfo.uv = { minToMax01.x, minToMax01.z };
	else
		hitInfo.uv = { minToMax01.x, minToMax01.y };

	return true;
}
bool AxisAlignedCube::GetBoundingBox(BoundingBox& outBox) const {
	outBox = BoundingBox(minCoord, maxCoord);
	return true;
}

bool YPlane::Intersect(const Ray& ray, HitInfo& hitInfo) const {
	float t = -(ray.pos.y - yPos) / ray.direction.y;
	if (t < 1e-3)
		return false;

	hitInfo.distance = t;
	hitInfo.point = ray.pos + t * ray.direction;
	hitInfo.normal = { 0.0f, 1.0f, 0.0f };
	hitInfo.object = (Object*)this;
	return true;
}

bool YPlane::GetBoundingBox(BoundingBox& outBox) const {
	return false;
}


BoundingBox GetSurroundingBox(BoundingBox box0, BoundingBox box1) {
	glm::vec3 small(fmin(box0.minCoord.x, box1.minCoord.x),
		fmin(box0.minCoord.y, box1.minCoord.y),
		fmin(box0.minCoord.z, box1.minCoord.z));

	glm::vec3 big(fmax(box0.maxCoord.x, box1.maxCoord.x),
		fmax(box0.maxCoord.y, box1.maxCoord.y),
		fmax(box0.maxCoord.z, box1.maxCoord.z));

	return BoundingBox(small, big);
}

inline bool CompareObjectAxis(const Object* a, const Object* b, int axis) {
	BoundingBox box_a;
	BoundingBox box_b;

	if (!a->GetBoundingBox(box_a) || !b->GetBoundingBox(box_b))
		std::cerr << "No bounding box in bvh_node constructor.\n";

	return box_a.minCoord[axis] < box_b.minCoord[axis];
}
bool box_x_compare(const Object* a, const Object* b) { return CompareObjectAxis(a, b, 0); }
bool box_y_compare(const Object* a, const Object* b) { return CompareObjectAxis(a, b, 1); }
bool box_z_compare(const Object* a, const Object* b) { return CompareObjectAxis(a, b, 2); }

BVH_Node::BVH_Node(const std::vector<Object*>& scrObjects, int start, int end) {

	/*
	1. randomly choose an axis
	2. sort the primitives (using std::sort)
	3. put half in each subtree
	*/

	auto objects = scrObjects; // Create a modifiable array of the source scene objects

	int axis = (int)(Random01() * 3);
	auto comparator = (axis == 0) ? box_x_compare
					: (axis == 1) ? box_y_compare
								  : box_z_compare;

	int object_span = end - start;

	if (object_span == 1) {
		left = right = objects[start];
	}
	else if (object_span == 2) {
		if (comparator(objects[start], objects[start + 1])) {
			left = objects[start];
			right = objects[start + 1];
		}
		else {
			left = objects[start + 1];
			right = objects[start];
		}
	}
	else {
		std::sort(objects.begin() + start, objects.begin() + end, comparator);

		auto mid = start + object_span / 2;
		left = new BVH_Node(objects, start, mid);
		right = new BVH_Node(objects, mid, end);
	}

	BoundingBox box_left, box_right;

	if (!left->GetBoundingBox(box_left) || !right->GetBoundingBox(box_right))
		std::cerr << "No bounding box in bvh_node constructor.\n";

	boundingBox = GetSurroundingBox(box_left, box_right);
}

BVH_Node::~BVH_Node() {

	if (left == right) {
		delete left;
		return;
	}

	if (left != nullptr)
		delete left;
	if (right != nullptr)
		delete right;
}


bool BVH_Node::Intersect(const Ray& ray, HitInfo& hitInfo) const {
	if (!boundingBox.DoesRayHit(ray))
		return false;
	
	bool hit_left = left->Intersect(ray, hitInfo);
	HitInfo info2;
	bool hit_right = right->Intersect(ray, info2);
	if (hit_right && (!hit_left || (hit_left && info2.distance < hitInfo.distance)) )
		hitInfo = info2;

	return hit_left || hit_right;
}

inline void InitializeRotationTransform(const float& angle, const Object* target, float& sinTheta, float& cosTheta, bool& boxExists,
	glm::vec3& pivot, BoundingBox& box, int axisIndex1, int axisIndex2, int rotationAxis)
{
	sinTheta = glm::sin(angle);
	cosTheta = glm::cos(angle);
	boxExists = target->GetBoundingBox(box);
	pivot = box.CalculatePivot();
	box.minCoord -= pivot;
	box.maxCoord -= pivot;

	glm::vec3 min = glm::vec3{ std::numeric_limits<float>::max() };
	glm::vec3 max = glm::vec3{ std::numeric_limits<float>::min() };

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			for (int k = 0; k < 2; k++) {
				float axis1 = i * box.maxCoord[axisIndex1] + (1 - i) * box.minCoord[axisIndex1];
				float axis2 = j * box.maxCoord[axisIndex2] + (1 - j) * box.minCoord[axisIndex2];
				float axisLeftover = k * box.maxCoord[rotationAxis] + (1 - k) * box.minCoord[rotationAxis];

				float new1 = cosTheta * axis1 + sinTheta * axis2;
				float new2 = -sinTheta * axis1 + cosTheta * axis2;

				glm::vec3 tester;
				tester[axisIndex1] = new1;
				tester[axisIndex2] = new2;
				tester[rotationAxis] = axisLeftover;
				for (int c = 0; c < 3; c++) {
					min[c] = fmin(min[c], tester[c]);
					max[c] = fmax(max[c], tester[c]);
				}
			}
		}
	}

	box = BoundingBox(min, max);
	box.minCoord += pivot;
	box.maxCoord += pivot;
}
ApplyYRotation::ApplyYRotation(float degress, Object* target) : target(target), angle(glm::radians(degress)) {
	InitializeRotationTransform(angle, target, sinTheta, cosTheta, boxExists, pivot, box, 0, 2, 1);
}
ApplyZRotation::ApplyZRotation(float degress, Object* target) : target(target), angle(glm::radians(degress)) {
	InitializeRotationTransform(angle, target, sinTheta, cosTheta, boxExists, pivot, box, 0, 1, 2);
}
ApplyXRotation::ApplyXRotation(float degress, Object* target) : target(target), angle(glm::radians(degress)) {
	InitializeRotationTransform(angle, target, sinTheta, cosTheta, boxExists, pivot, box, 1, 2, 0);
}

bool RotationIntersect(const Ray& ray, HitInfo& hitInfo, const glm::vec3& pivot, const float& sinTheta,
					   const float& cosTheta, Object*const & target, const int axisIndex1, const int axisIndex2) 
{
	glm::vec3 originOriginal = ray.pos - pivot;
	glm::vec3 originModified = originOriginal;
	glm::vec3 direction = ray.direction;

    originModified[axisIndex1] = cosTheta * originOriginal[axisIndex1] - sinTheta * originOriginal[axisIndex2];
    originModified[axisIndex2] = sinTheta*originOriginal[axisIndex1] + cosTheta*originOriginal[axisIndex2];

    direction[axisIndex1] = cosTheta*ray.direction[axisIndex1] - sinTheta*ray.direction[axisIndex2];
    direction[axisIndex2] = sinTheta*ray.direction[axisIndex1] + cosTheta*ray.direction[axisIndex2];

    Ray rotated(originModified + pivot, direction);

    if (!target->Intersect(rotated, hitInfo))
        return false;

	// Transform the data back
	originOriginal = hitInfo.point - pivot;
	originModified = originOriginal;
    glm::vec3 normal = hitInfo.normal;

    originModified[axisIndex1] =  cosTheta*originOriginal[axisIndex1] + sinTheta*originOriginal[axisIndex2];
    originModified[axisIndex2] = -sinTheta*originOriginal[axisIndex1] + cosTheta*originOriginal[axisIndex2];

    normal[axisIndex1] =  cosTheta*hitInfo.normal[axisIndex1] + sinTheta*hitInfo.normal[axisIndex2];
    normal[axisIndex2] = -sinTheta*hitInfo.normal[axisIndex1] + cosTheta*hitInfo.normal[axisIndex2];

    hitInfo.point = originModified + pivot;
	hitInfo.normal = normal;

    return true;
}

bool ApplyYRotation::Intersect(const Ray& ray, HitInfo& hitInfo) const {
	return RotationIntersect(ray, hitInfo, pivot, sinTheta, cosTheta, target, 0, 2);
}
bool ApplyZRotation::Intersect(const Ray& ray, HitInfo& hitInfo) const {
	return RotationIntersect(ray, hitInfo, pivot, sinTheta, cosTheta, target, 0, 1);
}
bool ApplyXRotation::Intersect(const Ray& ray, HitInfo& hitInfo) const {
	return RotationIntersect(ray, hitInfo, pivot, sinTheta, cosTheta, target, 1, 2);
}

// genpfault on stackoverflow
std::vector<Vertex> LoadOBJ( std::istream& in ) {
	struct VertRef {
		VertRef( int v, int vt, int vn ) : v(v), vt(vt), vn(vn) { }
		int v, vt, vn;
	};

    std::vector< Vertex > verts;
    std::vector< glm::vec4 > positions( 1, glm::vec4( 0, 0, 0, 0 ) );
    std::vector< glm::vec3 > texcoords( 1, glm::vec3( 0, 0, 0 ) );
    std::vector< glm::vec3 > normals( 1, glm::vec3( 0, 0, 0 ) );
    std::string lineStr;
    while( std::getline( in, lineStr ) )
    {
        std::istringstream lineSS( lineStr );
        std::string lineType;
        lineSS >> lineType;

        // vertex
        if( lineType == "v" )
        {
            float x = 0, y = 0, z = 0, w = 1;
            lineSS >> x >> y >> z >> w;
            positions.push_back( glm::vec4( x, y, z, w ) );
        }

        // texture
        if( lineType == "vt" )
        {
            float u = 0, v = 0, w = 0;
            lineSS >> u >> v >> w;
            texcoords.push_back( glm::vec3( u, v, w ) );
        }

        // normal
        if( lineType == "vn" )
        {
            float i = 0, j = 0, k = 0;
            lineSS >> i >> j >> k;
            normals.push_back( glm::normalize( glm::vec3( i, j, k ) ) );
        }

        // polygon
        if( lineType == "f" )
        {
            std::vector< VertRef > refs;
            std::string refStr;
            while( lineSS >> refStr )
            {
                std::istringstream ref( refStr );
                std::string vStr, vtStr, vnStr;
                std::getline( ref, vStr, '/' );
                std::getline( ref, vtStr, '/' );
                std::getline( ref, vnStr, '/' );
                int v = atoi( vStr.c_str() );
                int vt = atoi( vtStr.c_str() );
                int vn = atoi( vnStr.c_str() );
                v  = (  v >= 0 ?  v : positions.size() +  v );
                vt = ( vt >= 0 ? vt : texcoords.size() + vt );
                vn = ( vn >= 0 ? vn : normals.size()   + vn );
                refs.push_back( VertRef( v, vt, vn ) );
            }

            // triangulate, assuming n>3-gons are convex and coplanar
            for( size_t i = 1; i+1 < refs.size(); ++i )
            {
                const VertRef* p[3] = { &refs[0], &refs[i], &refs[i+1] };

                // http://www.opengl.org/wiki/Calculating_a_Surface_Normal
                glm::vec3 U( positions[ p[1]->v ] - positions[ p[0]->v ] );
                glm::vec3 V( positions[ p[2]->v ] - positions[ p[0]->v ] );
                glm::vec3 faceNormal = glm::normalize( glm::cross( U, V ) );

                for( size_t j = 0; j < 3; ++j )
                {
                    Vertex vert;
                    vert.position = glm::vec3( positions[ p[j]->v ] );
					vert.texcoord = glm::vec2( texcoords[ p[j]->vt ] );

                    vert.normal = ( p[j]->vn != 0 ? normals[ p[j]->vn ] : faceNormal );
                    verts.push_back( vert );
                }
            }
        }
    }

    return verts;
}

BoundingBox GetExtentsFromPositionArray( const glm::vec3* pts, size_t stride, size_t count )
{
    unsigned char* base = (unsigned char*)pts;
    glm::vec3 pmin( *(glm::vec3*)base );
    glm::vec3 pmax( *(glm::vec3*)base );
    for( size_t i = 0; i < count; ++i, base += stride )
    {
        const glm::vec3& pt = *(glm::vec3*)base;
        pmin = glm::min( pmin, pt );
        pmax = glm::max( pmax, pt );
    }

	return BoundingBox(pmin, pmax);
}

PolygonMesh::PolygonMesh(std::string pathToObjFile, float size, Material mat) : Object(mat) {
	std::filebuf fb;
	if (!fb.open(pathToObjFile, std::ios::in)) {
		std::cout << "Couldn't load file in " << pathToObjFile << ". Currently in " << std::filesystem::current_path() << std::endl;
		fb.close();
		return;
	}

	std::istream istr(&fb);
	std::vector<Vertex> vertices = LoadOBJ(istr);

	// Scale it to the right size
	const glm::vec3* firstPosition = &vertices[0].position;
	const int stride = sizeof(Vertex);
	const int vertexCount = vertices.size();

    BoundingBox box = GetExtentsFromPositionArray(firstPosition, stride, vertexCount);
    const glm::vec3 center = box.minCoord * 0.5f + box.maxCoord * 0.5f;
    const float factor = size / glm::compMax( box.maxCoord - box.minCoord );
	unsigned char* base = (unsigned char*)firstPosition;

    for( size_t i = 0; i < vertexCount; ++i, base += stride ) {
        glm::vec3& pt = *(glm::vec3*)base;
        pt = ( pt - center ) * factor;
    }
	box.minCoord = (box.minCoord - center) * factor;
	box.maxCoord = (box.maxCoord - center) * factor;
	fb.close();

	// From vertices to triangles
	if (vertices.size() % 3 != 0)
		std::cout << "Vertex count was not a multiple of 3.\n";
	std::vector<Object*> tris;
	tris.resize(vertices.size() / 3);
	for (int i = 0; i < tris.size(); i++) {
		tris[i] = new Triangle(vertices[i * 3], vertices[i * 3 + 1], vertices[i * 3 + 2]);
	}
	triangles = new BVH_Node(tris);
}
bool PolygonMesh::Intersect(const Ray& ray, HitInfo& hitInfo) const {
	if (triangles->Intersect(ray, hitInfo)) {
		hitInfo.object = (Object*)this; return true;
	}
	else return false;
}

// modified Möller–Trumbore intersection algorithm
bool Triangle::Intersect(const Ray& ray, HitInfo& hitInfo) const {
	/*
	const float kEpsilon = 0.0000001f;
	const glm::vec3& v0 = vertices[0].position;
	const glm::vec3& v1 = vertices[1].position;
	const glm::vec3& v2 = vertices[2].position;

	// compute plane's normal
    glm::vec3 v0v1 = v1 - v0; 
    glm::vec3 v0v2 = v2 - v0; 
    // no need to normalize
    glm::vec3 N = glm::cross(v0v1, v0v2);
    float denom = glm::dot(N, N); 
 
    // check if ray and plane are parallel ?
    float NdotRayDirection = glm::dot(N, ray.direction); 
    if (fabs(NdotRayDirection) < kEpsilon)  //almost 0 
        return false;  //they are parallel so they don't intersect ! 
 
    // compute d parameter using equation 2
    float d = glm::dot(N, v0); 
 
    // compute t (equation 3)
    float t = (glm::dot(N, ray.pos) + d) / NdotRayDirection; 
    if (t < 0) return false;  //the triangle is behind 
 
    // compute the intersection point using equation 1
    glm::vec3 P = ray.pos + t * ray.direction; 
 
    glm::vec3 C;  //vector perpendicular to triangle's plane 
 
    // edge 0
    glm::vec3 edge0 = v1 - v0; 
    glm::vec3 vp0 = P - v0; 
    C = glm::cross(edge0, vp0); 
    if (glm::dot(N, C) < 0) return false;  //P is on the right side 
 
    // edge 1
    glm::vec3 edge1 = v2 - v1; 
    glm::vec3 vp1 = P - v1; 
    C = glm::cross(edge1, vp1); 
	float u, v;
    if ((u = glm::dot(N, C)) < 0)  return false;  //P is on the right side 
 
    // edge 2
    glm::vec3 edge2 = v0 - v2; 
    glm::vec3 vp2 = P - v2; 
    C = glm::cross(edge2, vp2); 
    if ((v = glm::dot(N, C)) < 0) return false;  //P is on the right side; 
 
    u /= denom; 
    v /= denom;

	float p1 = u;
	float p2 = v;
	float p3 = 1 - u - v;

	hitInfo.object = (Object*)this;
	hitInfo.normal = vertices[0].normal;
	hitInfo.point = P + hitInfo.normal * 0.01f;
	hitInfo.distance = t;
	hitInfo.uv = p1*vertices[0].texcoord + p2*vertices[1].texcoord + p3*vertices[2].texcoord;
	if (glm::compMax(hitInfo.uv) > 1 || glm::compMin(hitInfo.uv) < 0)
		hitInfo.uv = glm::clamp(hitInfo.uv, { 0 }, { 1.0 });
	hitInfo.uv = glm::vec2{ 0.8f };
 
    return true;  //this ray hits the triangle 
	*/

	const float EPSILON = 0.0000001f;
	const glm::vec3& vertex0 = vertices[0].position;
	const glm::vec3& vertex1 = vertices[1].position;
	const glm::vec3& vertex2 = vertices[2].position;
	glm::vec3 edge1, edge2, h, s, q;
	float a, f, u, v;
	edge1 = vertex1 - vertex0;
	edge2 = vertex2 - vertex0;
	h = glm::cross(ray.direction, edge2);
	a = glm::dot(edge1, h);
	if (a > -EPSILON && a < EPSILON)
		return false ;    // This ray is parallel to this triangle.
	f = 1.0f / a;
	s = ray.pos - vertex0;
	u = f * glm::dot(s, h);
	if (u < 0.0f || u > 1.0f)
		return false;
	q = glm::cross(s, edge1);
	v = f * glm::dot(ray.direction, q);
	if (v < 0.0f || u + v > 1.0f)
		return false;
	// At this stage we can compute t to find out where the intersection point is on the line.
	float t = f * glm::dot(edge2, q);
	if (t > EPSILON) // ray intersection
	{
		const glm::vec3& h  = ray.pos + ray.direction * t;
		const glm::vec3& p1 = vertex0;
		const glm::vec3& p2 = vertex1;
		const glm::vec3& p3 = vertex2;
		// Calculate UVs using barycentric coordinates
		float denominator = p1.y*(p2.z-p3.z)-p2.y*(p1.z-p3.z)+p3.y*(p1.z-p2.z);
		if (denominator == 0) // The points are colinear, so this can't be computed
			return false;

		hitInfo.distance = t;
		hitInfo.normal = vertices[0].normal; // They should all be the same, doesn't matter what we choose
		hitInfo.object = (Object*)this;
		hitInfo.point = h + hitInfo.normal * 0.01f;

		float percent0 =  (h.y*(p2.z-p3.z)-h.z*(p2.y-p3.y)+p2.y*p3.z-p3.y*p2.z)/denominator;
		float percent1 = -(h.y*(p1.z-p3.z)-h.z*(p1.y-p3.y)+p1.y*p3.z-p3.y*p1.z)/denominator;
		float percent2 =  (h.y*(p1.z-p2.z)-h.z*(p1.y-p2.y)+p1.y*p2.z-p2.y*p1.z)/denominator;
		hitInfo.uv = vertices[0].texcoord * percent0 + vertices[1].texcoord * percent1 + vertices[2].texcoord * percent2;

		// Wrapping
		hitInfo.uv -= (glm::uvec2)hitInfo.uv;
		if (hitInfo.uv.x < 0) hitInfo.uv.x += 1;
		if (hitInfo.uv.y < 0) hitInfo.uv.y += 1;

		return true;
	}
	else // This means that there is a line intersection but not a ray intersection.
		return false;
}
