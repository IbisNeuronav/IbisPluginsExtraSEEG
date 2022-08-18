#include "MathUtils.h"
#include <algorithm>

namespace seeg {

    void Resize3DLineSegmentSingleSide( Point3D p1,
                                        Point3D p2,
                                        double len,
                                        Point3D &p2_out) {

        Vector3D_lf p1_v(p1[0], p1[1], p1[2]);
        Vector3D_lf p2_v(p2[0], p2[1], p2[2]);
        double d = norm(p2_v-p1_v);

        double deltaX = (p2[0] - p1[0]) / d;
        double deltaY = (p2[1] - p1[1]) / d;
        double deltaZ = (p2[2] - p1[2]) / d;

        p2_out[0] = p2[0] + (len-d) * deltaX;
        p2_out[1] = p2[1] + (len-d) * deltaY;
        p2_out[2] = p2[2] + (len-d) * deltaZ;

    }

    void Resize3DLineSegmentBothSides(  Point3D p1,
                                        Point3D p2,
                                        double len,
                                        Point3D &p1_out,
                                        Point3D &p2_out) {

        Vector3D_lf p1_v (p1[0], p1[1], p1[2]);
        Vector3D_lf p2_v (p2[0], p2[1], p2[2]);

        double d = norm(p2_v-p1_v);

        double deltaX = (p2[0] - p1[0]) / d;
        double deltaY = (p2[1] - p1[1]) / d;
        double deltaZ = (p2[2] - p1[2]) / d;

        p1_out[0] = p1[0] - (len-d)/2.0 * deltaX;
        p1_out[1] = p1[1] - (len-d)/2.0 * deltaY;
        p1_out[2] = p1[2] - (len-d)/2.0 * deltaZ;

        p2_out[0] = p2[0] + (len-d)/2.0 * deltaX;
        p2_out[1] = p2[1] + (len-d)/2.0 * deltaY;
        p2_out[2] = p2[2] + (len-d)/2.0 * deltaZ;
    }

    float CalcLineLength(Point3D p1, Point3D p2) {
        Vector3D_lf p1_v (p1[0], p1[1], p1[2]);
        Vector3D_lf p2_v (p2[0], p2[1], p2[2]);
        return norm(p1_v - p2_v);
    }

    double FindClosestPoint3D(Point3D aPoint, vector<Point3D>pointList, Point3D &closestPt, int *indexInList) {
        Vector3D_lf p1,p2;
        double minDist = -1;
        int index = -1;
        p1 = Vector3D_lf(aPoint[0], aPoint[1], aPoint[2]);
        for (int i=0; i<pointList.size(); i++) {
            p2 = Vector3D_lf(pointList[i][0], pointList[i][1], pointList[i][2]);
            double dist = norm(p2-p1);
            if (minDist<0 || minDist > dist) {
                minDist = dist;
                closestPt = pointList[i];
                index = i;
            }
        }

        if (indexInList) {
            *indexInList = index;
        }
        return minDist;
    }

    // provisional mean (http://en.wikipedia.org/wiki/Algorithms_for_calculating_variance)
    double CalcMean(vector<double> values) {
        if (values.size() == 0) {
            return 0;
        }

        double mean = 0;

        // provisional average method (in case of large vector values)
        for (int i=0; i<values.size(); i++) {
            mean += (values[i] - mean)/(i+1);
        }
        return mean;
    }

    double CalcMean2(vector<double> values) {
        if (values.size() == 0) {
            return 0;
        }
        double sum = 0;
        for (int i=0; i<values.size(); i++) {
            sum += values[i];
        }
        return sum/values.size();
    }

    double CalcStdDev(vector<double> values) {

        double mean = CalcMean(values);
        int n = values.size();
        double sum2 = 0;
        for (int j=0; j<values.size(); j++) {
            sum2+=(values[j]-mean)*(values[j]-mean);
        }
        double variance = sum2 / (n-1);
        return sqrt(variance);
    }

    double CalcStdDev2(vector<double> values) {

        double mean = CalcMean2(values);
        int n = values.size();
        double sum = 0;
        for (int j=0; j<values.size(); j++) {
            sum += (values[j]-mean)*(values[j]-mean);
        }
        double variance = sum/n;
        return sqrt(variance);
    }



    double CalcMedian(vector<double> values) {
        vector<double> tmp = values;
        if (tmp.size()==0) {
            return 0;
        }
        sort(tmp.begin(),tmp.end());
        return tmp[tmp.size()/2];
    }

    double CalcMax(vector<double> values) {
        if (values.size() == 0) {
            return 0;
        }

        double maxVal = values[0];
        for (int i=1; i<values.size(); i++) {
            if (values[i] > maxVal) {
                maxVal = values[i];
            }
        }
        return maxVal;
    }

    double CalcMin(vector<double> values) {
        if (values.size() == 0) {
            return 0;
        }

        double minVal = values[0];
        for (int i=1; i<values.size(); i++) {
            if (values[i] < minVal) {
                minVal = values[i];
            }
        }
        return minVal;
    }

    double CalcProbabilityUsingNormalDistribution(double measurement, double mean, double stdDev) {
        double v = measurement-mean;
        v = v*v;
        double f = exp(-v/(2*stdDev*stdDev));
        f = f / (stdDev*sqrt(2*PI));
        return f;
    }

    double CalcProbabilityUsingUniformDistribution(double measurement, double minVal, double maxVal) {
        if (measurement >= minVal && measurement <= maxVal) {
            return 1.0 / (maxVal - minVal);
        } else {
            return 0.0;
        }
    }
}
