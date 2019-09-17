/**
 * Copyright (c) 2018, University Osnabrück
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University Osnabrück nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL University Osnabrück BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */



#ifndef VIRTUALGRID_H
#define VIRTUALGRID_H
#include "BigGrid.hpp"

#include <lvr2/geometry/BoundingBox.hpp>
#include <memory>
#include <vector>

namespace lvr2
{

template <typename BaseVecT>
class VirtualGrid
{
  public:

    /**
     *
     * @param bb
     * @param maxNodePoints
     * @param gridCellSize
     * @param voxelsize
     */
    VirtualGrid(BoundingBox<BaseVecT>& bb,
                  size_t maxNodePoints,
                  size_t gridCellSize,
                  float voxelsize);

    /**
     * Destructor
     */
    virtual ~VirtualGrid();

    /**
     *
     * @return the smaller BoundingBoxes
     */
    std::vector<BoundingBox<BaseVecT>*> getBoxes() { return m_boxes; }


  private:

    // BoundingBox of the input PointCloud
    BoundingBox<BaseVecT> m_original;

    // List of (smaller) BoundingBox, which overlap the original PointCloud
    std::vector<BoundingBox<BaseVecT>*> m_boxes;

    // size of the "virtual" GridCell aka size of the smaller BBoxes
    size_t m_gridCellSize;

    float m_voxelsize; // check if this is even used
    size_t maxNodePoints; // same




};

} // namespace lvr2

#include "lvr2/reconstruction/VirtualGrid.tcc"

#endif // VIRTUALGRID_H
