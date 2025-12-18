#ifndef GMATH_HPP
#define GMATH_HPP




struct Vec3 {
    double x;
    double y;
    double z;
	
      double length() const {
          return sqrtf(x*x + y*y + z*z);
      }
};

void add_inplace(Vec3& a, const Vec3& b);
Vec3 operator+(const Vec3& a, const Vec3& b);
Vec3 operator-(const Vec3& a, const Vec3& b);
Vec3 operator-(const Vec3& a, const Vec3& b);
Vec3 operator*(const Vec3& v, double t);
Vec3 operator/(const Vec3& v, double t);
std::ostream& operator<<(std::ostream& outstr, const Vec3 v);
std::string operator<<(const Vec3 v, const char* str);
Vec3 normalize(const Vec3& v);
double dot(Vec3 a, Vec3 b);
Vec3 lerp(Vec3 start, Vec3 end, double t);

#ifdef GMATH_IMPLEMENTATION

void add_inplace(Vec3& a, const Vec3& b) {
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
}

Vec3 operator+(const Vec3& a, const Vec3& b) {
    Vec3 v;
    v.x = a.x + b.x;
    v.y = a.y + b.y;
    v.z = a.z + b.z;
    return v;
}

Vec3 operator-(const Vec3& a, const Vec3& b) {
    Vec3 v;
    v.x = a.x - b.x;
    v.y = a.y - b.y;
    v.z = a.z - b.z;
    return v;
}

Vec3 operator*(const Vec3& v, double t) {
    Vec3 res;
    res.x = v.x * t;
    res.y = v.y * t;
    res.z = v.z * t;
    return res;
}

Vec3 operator/(const Vec3& v, double t) {
    Vec3 res;
    res.x = v.x / t;
    res.y = v.y / t;
    res.z = v.z / t;
    return res;
}

std::ostream& operator<<(std::ostream& outstr, const Vec3 v) {
	outstr << v.x << " " << v.y << " " << v.z; 
	return outstr;
}

std::string operator<<(const Vec3 v, const char* str) {
	return str;
}

Vec3 normalize(const Vec3& v) {
	return v / v.length();
}

double dot(Vec3 a, Vec3 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 lerp(Vec3 start, Vec3 end, double t) {
	return start * (1.f - t) + end * t; 
}

#endif //GMATH_IMPLEMENTATION


#endif //GMATH_HPP
