/*
 * FurnitureFeatureClassifier.hpp
 *
 *  Created on: Nov 25, 2016
 *      Author: robot
 */

#ifndef INCLUDE_LVR_CLASSIFICATION_FURNITUREFEATURECLASSIFIER_HPP_
#define INCLUDE_LVR_CLASSIFICATION_FURNITUREFEATURECLASSIFIER_HPP_

#include <vector>
#include <iostream>

#include <lvr/classification/RegionClassifier.hpp>

namespace lvr {

struct PlanarClusterFeature
{
	int index;
	float cx;
	float cy;
	float cz;
	float nx;
	float ny;
	float nz;
	float area;
	float bbx;
	float bby;
	float bbz;
	float bbw;
	float bbh;
	float bbd;
    int orientation;
};

std::ofstream operator<<(const PlanarClusterFeature f, std::ofstream& os)
{
	os << "Index: " << f.index << std::endl;
	os << "Centroid: " << f.cx << " " << f.cy << " " << f.cz << std::endl;
	os << "Normal: " << f.nx << " " << f.ny << " " << f.nz << std::endl;
	os << "Area: " << f.area << std::endl;
	os << "Orientation: " << f.orientation << std::endl;
}

template<typename VertexT, typename NormalT>
class FurnitureFeatureClassifier : public RegionClassifier<VertexT, NormalT>{
public:
	FurnitureFeatureClassifier(vector<Region<VertexT, NormalT>* >* region);
	virtual ~FurnitureFeatureClassifier();


	/**
	 * @brief Returns the r component for the given region
	 */
	virtual uchar r(int region) {  return 0; }

	/**
	 * @brief Returns the g component for the given region
	 */
	virtual uchar g(int region) {  return 255;}

	/**
	 * @brief Returns the b component for the given region
	 */
	virtual uchar b(int region) {  return 0; }

	void classifyRegion(int region);

private:

	/// A vector containing the planar feature vectors for all
	/// classified regions
	vector<PlanarClusterFeature> m_features;
};

} /* namespace lvr */

#include "FurnitureFeatureClassifier.tcc"

#endif /* INCLUDE_LVR_CLASSIFICATION_FURNITUREFEATURECLASSIFIER_HPP_ */