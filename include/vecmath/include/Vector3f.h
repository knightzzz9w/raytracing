#ifndef VECTOR_3F_H
#define VECTOR_3F_H

class Vector2f;

class Vector3f
{
public:

	static const Vector3f ZERO;
	static const Vector3f UP;
	static const Vector3f RIGHT;
	static const Vector3f FORWARD;

    Vector3f( float f = 0.f );
    Vector3f( float x, float y, float z );

	Vector3f( const Vector2f& xy, float z );
	Vector3f( float x, const Vector2f& yz );

	// copy constructors
    Vector3f( const Vector3f& rv );

	// assignment operators
    Vector3f& operator = ( const Vector3f& rv );



	// no destructor necessary

	// returns the ith element
    const float& operator [] ( int i ) const;
    float& operator [] ( int i );
	float norm() {return std::sqrt(m_elements[0] * m_elements[0] + 
	m_elements[1] * m_elements[1]  + m_elements[2] * m_elements[2]);}

    float& x();
	float& y();
	float& z();

	float x() const;
	float y() const;
	float z() const;

	Vector2f xy() const;
	Vector2f xz() const;
	Vector2f yz() const;

	Vector3f xyz() const;
	Vector3f yzx() const;
	Vector3f zxy() const;

	float length() const;
    float squaredLength() const;

	void normalize();
	Vector3f normalized() const;

	Vector2f homogenized() const;

	void negate();

	// ---- Utility ----
    operator const float* () const; // automatic type conversion for OpenGL
    operator float* (); // automatic type conversion for OpenGL 
	void print() const;	

	Vector3f& operator += ( const Vector3f& v );
	Vector3f& operator -= ( const Vector3f& v );
    Vector3f& operator *= ( float f );

    static float dot( const Vector3f& v0, const Vector3f& v1 );
	static Vector3f cross( const Vector3f& v0, const Vector3f& v1 );
    
    // computes the linear interpolation between v0 and v1 by alpha \in [0,1]
	// returns v0 * ( 1 - alpha ) * v1 * alpha
	static Vector3f lerp( const Vector3f& v0, const Vector3f& v1, float alpha );

	// computes the cubic catmull-rom interpolation between p0, p1, p2, p3
    // by t \in [0,1].  Guarantees that at t = 0, the result is p0 and
    // at p1, the result is p2.
	static Vector3f cubicInterpolate( const Vector3f& p0, const Vector3f& p1, const Vector3f& p2, const Vector3f& p3, float t );
	static Vector3f randomVectorOnSphere();
	Vector3f exp() const{
		return Vector3f(::exp(x()), ::exp(y()), ::exp(z()));
	}

	static Vector3f Min(const Vector3f &p1, const Vector3f &p2) {
	return Vector3f(std::min(p1.x(), p2.x()), std::min(p1.y(), p2.y()),
					std::min(p1.z(), p2.z()));
}

	static Vector3f Max(const Vector3f &p1, const Vector3f &p2) {
		return Vector3f(std::max(p1.x(), p2.x()), std::max(p1.y(), p2.y()),
						std::max(p1.z(), p2.z()));
	}

	float m_elements[ 3 ];

};

// component-wise operators
Vector3f operator + ( const Vector3f& v0, const Vector3f& v1 );
Vector3f operator - ( const Vector3f& v0, const Vector3f& v1 );
Vector3f operator * ( const Vector3f& v0, const Vector3f& v1 );
Vector3f operator / ( const Vector3f& v0, const Vector3f& v1 );

// unary negation
Vector3f operator - ( const Vector3f& v );

// multiply and divide by scalar
Vector3f operator * ( float f, const Vector3f& v );
Vector3f operator * ( const Vector3f& v, float f );
Vector3f operator / ( const Vector3f& v, float f );

bool operator == ( const Vector3f& v0, const Vector3f& v1 );
bool operator != ( const Vector3f& v0, const Vector3f& v1 );

inline float dotProduct(const Vector3f &a, const Vector3f &b)
{ return a.x() * b.x() + a.y() * b.y() + a.z() * b.z(); }

inline Vector3f crossProduct(const Vector3f &a, const Vector3f &b)
{
    return Vector3f(
            a.y() * b.z() - a.z() * b.y(),
            a.z() * b.x() - a.x() * b.z(),
            a.x() * b.y() - a.y() * b.x()
    );
}

inline Vector3f normalize(const Vector3f &v)
{
    float mag2 = v.x() * v.x() + v.y() * v.y() + v.z() * v.z();
    if (mag2 > 0) {
        float invMag = 1 / sqrtf(mag2);
        return Vector3f(v.x() * invMag, v.y() * invMag, v.z() * invMag);
    }

    return v;
}


#endif // VECTOR_3F_H
