#ifndef __MATH_UTILS_H__
#define __MATH_UTILS_H__

/**
 * @file MathUtils.h
 *
 * This file contains basic math types and utility functions
 *
 * @author Silvain Beriault
 */


// header files to include
#include <math.h>
#include <iostream>
#include <string>
#include "VolumeTypes.h"
using namespace std;


/** The PI value */
#ifndef PI
#define PI 3.14159265
#endif

#define deg2rad(deg) (((double)(deg)) * PI / 180.0)
#define rad2deg(rad) (((double)(rad)) * 180.0 / PI)


namespace seeg {


    /**
     * A simple class to represent a 3D vector. It is expected that the
     * provided type T is a basic type (int, float, etc). This class also implements
     * the most comon operations on 3D vector
     */
    template <typename T>
    class Vector3D {
    public:

        /** The x value. */
        T x;

        /** The y value. */
        T y;

        /** The z value. */
        T z;

        /**
         * Constructor.
         *
         * @param x The x value
         * @param y The y value
         * @param z The z value
         */
        Vector3D(T x, T y, T z) : x(x), y(y), z(z) {}

        /**
         * No argument constructor. Initializes the vector to {0,0,0}.
         */
        Vector3D() : x(0),y(0),z(0) {}

        /**
         * Implementation of the operator== on Vector3D.
         *
         * @param v Reference to the other vector to compare.
         * @return true if the two vectors are equals, false otherwise
         */
        bool operator==(const Vector3D &v) const {
            return (x == v.x) && (y == v.y) && (z==v.z);
        }

        /**
         * Implementation of the operator!= for Vector3D.
         *
         * @param v Reference to the other vector to compare.
         * @return true if the two vectors are unequal, false otherwise
         */
        bool operator!=(const Vector3D &v) const {
            return (x != v.x) || (y != v.y) || (z!=v.z);
        }

        /**
         * Implementation of operator+ (addition between two vectors)
         *
         * @param v Reference to the other vector to add
         * @return The result of the addition of two Vector3D
         */
        Vector3D operator+(const Vector3D &v)const {
            Vector3D res;
            res.x = x + v.x;
            res.y = y + v.y;
            res.z = z + v.z;
            return res;
        }

        /**
         * Implementation of operator- (subtraction between two vectors)
         *
         * @param v Reference to the vector to subtract
         * @return The result of the vector subtraction
         */
        Vector3D operator-(const Vector3D &v)const {
            Vector3D res;
            res.x = x-v.x;
            res.y = y-v.y;
            res.z = z-v.z;
            return res;
        }

        /**
         * Implementation of operator- (negation operator)
         *
         * @return The result of vector negation
         */
        Vector3D operator-()const {
            Vector3D res;
            res.x = -x;
            res.y = -y;
            res.z = -z;
            return res;
        }

        /**
         * Implementation of operator* (multiplication by a constant scalar value)
         *
         * @param k A scalar value
         * @return The result of scalar multiplication
         */
        Vector3D operator*(const T k)const {
            //Multiplication by Scalar
            Vector3D res;
            res.x = k * x;
            res.y = k * y;
            res.z = k * z;
            return res;
        }

        /**
         * Implementation of operator/ (division by a constant scalar value)
         *
         * @param k A scalar value
         * @return The result of scalar multiplication
         */
        Vector3D operator/(const T k)const {
            //Division by Scalar
            Vector3D res;
            res.x = x / k;
            res.y = y / k;
            res.z = z / k;
            return res;
        }


        /**
         * Implementation of operator* (cross product between two vectors)
         *
         * @param Reference to the other vector for the cross-product
         * @return result of the cross product
         */
        Vector3D operator*(const Vector3D &v)const {
            //Cross Product

            Vector3D res;
            res.x = (y * v.z) - (z * v.y);
            res.y = (z * v.x) - (x * v.z);
            res.z = (x * v.y) - (y * v.x);
            return res;

        }

        /**
         * Calculate the dot product of two vectors
         *
         * @param v Reference to the other vector for the dot-product
         * @return A scalar for the result of the dot product
         */
        T DotProd(const Vector3D &v) {
            T v1v2;
            v1v2 = (x * v.x)+(y * v.y)+(z * v.z);
            return v1v2;
        }

        /**
         * Element by element multiplication of two Vector3D
         *
         * @param v Reference to the other vector
         * @return result of element-by-element multiplication
         */
        Vector3D DotMult(const Vector3D &v) {
            Vector3D res;
            res.x = x * v.x;
            res.y = y * v.y;
            res.z = z * v.z;
            return res;
        }


        /**
         * Implementation of the cast operator (eg. to cast from Vector3D<int> to
         * Vector3D<float>
         */
        template < typename NewTypeT >
        operator Vector3D< NewTypeT >() {
            Vector3D< NewTypeT > result;
            result.x = static_cast<NewTypeT>(x);
            result.y = static_cast<NewTypeT>(y);
            result.z = static_cast<NewTypeT>(z);
            return result;
        }

        /**
         * Implementation of operator < (comparison of 2 Vector3D) - RIZ
         * < is implemented with respect to the distance to origin.
         *
         * @param v Reference to the other vector to compare.
         * @return true if current vector has smaller norm than the other, false otherwise
         */
        bool operator<(const Vector3D &v) const {
            float norm1,norm2;
            norm1 = sqrt(pow(x, 2) + pow(y,2) + pow(z,2));
            norm2 = sqrt(pow(v.x, 2) + pow(v.y,2) + pow(v.z,2));
            return (norm1 < norm2);
        }

        /**
         * Implementation of operator > (comparison of 2 Vector3D) - RIZ
         * > is implemented with respect to the distance to origin.
         *
         * @param v Reference to the other vector to compare.
         * @return true if current vector has larger norm than the other, false otherwise
         */
        bool operator>(const Vector3D &v) const {
            float norm1,norm2;
            norm1 = sqrt(pow(x, 2) + pow(y,2) + pow(z,2));
            norm2 = sqrt(pow(v.x, 2) + pow(v.y,2) + pow(v.z,2));
            return (norm1 > norm2);
        }

        /**
         * Implementation of operator <= (comparison of 2 Vector3D) - RIZ
         * <= is implemented with respect to the distance to origin.
         *
         * @param v Reference to the other vector to compare.
         * @return true if current vector has smaller or equal norm than the other, false otherwise
         */
        bool operator<=(const Vector3D &v) const {
            float norm1,norm2;
            norm1 = sqrt(pow(x, 2) + pow(y,2) + pow(z,2));
            norm2 = sqrt(pow(v.x, 2) + pow(v.y,2) + pow(v.z,2));
            return (norm1 <= norm2);
        }

        /**
         * Implementation of operator >= (comparison of 2 Vector3D) - RIZ
         * >= is implemented with respect to the distance to origin.
         *
         * @param v Reference to the other vector to compare.
         * @return true if current vector has larger or equal norm than the other, false otherwise
         */
        bool operator>=(const Vector3D &v) const {
            float norm1,norm2;
            norm1 = sqrt(pow(x, 2) + pow(y,2) + pow(z,2));
            norm2 = sqrt(pow(v.x, 2) + pow(v.y,2) + pow(v.z,2));
            return (norm1 >= norm2);
        }

    };



    /**
     * Overload of operator<< for printing a Vector3D to an ostream (e.g. cout)
     *
     * @param ostr Reference to an ostream
     * @param output Reference to a Vector3D to output to the ostream
     * @return Reference to the modified ostream
     */
    template <typename T>
    ostream& operator<< (ostream& ostr, const Vector3D<T>& output) {
        return ostr << "{" << output.x << ", " << output.y << ", " << output.z << "}";
    }

    /**
     * Calculates the L2-norm of a vector
     *
     * @param v Reference to a Vector3D
     */
    template <typename T>
    float norm(const Vector3D<T>& v) {
        float norm;
        norm = sqrt(pow(v.x, 2) + pow(v.y,2) + pow(v.z,2));
        return norm;
    }



    /** typedef for integer vector */
    typedef Vector3D<int> Vector3D_i;

    /** typedef for unsigned integer vector */
    typedef Vector3D<unsigned int> Vector3D_u;

    /** typedef for long vector */
    typedef Vector3D<long> Vector3D_l;

    /** typedef for float vector */
    typedef Vector3D<float> Vector3D_f;

    /** typedef for double vector */
    typedef Vector3D<double> Vector3D_lf;


    void Resize3DLineSegmentSingleSide(Point3D p1, Point3D p2, double len, Point3D &p2_out);
    void Resize3DLineSegmentBothSides(Point3D p1, Point3D p2, double len, Point3D &p1_out, Point3D &p2_out);
    float CalcLineLength(Point3D p1, Point3D p2);

    double FindClosestPoint3D(Point3D aPoint, vector<Point3D>pointList, Point3D &closestPt, int *indexInList=0);
    double CalcMean(vector<double> values);
    double CalcStdDev(vector<double> values);
    double CalcMean2(vector<double> values);
    double CalcStdDev2(vector<double> values);
    double CalcMedian(vector<double> values);
    double CalcMax(vector<double> values);
    double CalcMin(vector<double> values);


    double CalcProbabilityUsingNormalDistribution(double measurement, double mean, double stdDev);
    double CalcProbabilityUsingUniformDistribution(double measurement, double minVal, double maxVal);
}

#endif
