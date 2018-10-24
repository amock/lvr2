#ifndef POINT_OCTREE
#define POINT_OCTREE

#include <vector>

#include <lvr2/io/PointBuffer.hpp>
#include <lvr2/geometry/BaseVector.hpp>
#include <lvr2/geometry/Vector.hpp>
#include <lvr2/geometry/BoundingBox.hpp>



namespace lvr2
{

  struct BOct
  {
      unsigned long m_child : 48;
      unsigned char m_valid : 8;
      unsigned char m_leaf : 8;
  };
  
  struct Leaf
  {
    int m_start;
    int m_size;
  };

  struct TmpLeaf
  {
    std::vector<Vector<BaseVector<float> > > pts;
  };

  template <typename T>
  struct isLeaf
  {
    static const bool val = false;
  };

  template <>
  struct isLeaf<TmpLeaf>
  {
     static const bool val = true;
  };

  class PointOctree
  {
    public:
      PointOctree(PointBufferPtr& points, int voxelSize);

      virtual ~PointOctree();

    private:
      int m_voxelSize;
      BOct* m_root;
      
      BoundingBox<BaseVector<float> > m_bbox;
      
      int getBBoxIndex(const Vector<BaseVector<float> >& point, const BoundingBox<BaseVector<float> >& bbox, BoundingBox<BaseVector<float> >& subOctBbox);

      void insertPoint(const Vector<BaseVector<float> >& point, BOct* oct, const BoundingBox<BaseVector<float> >& bbox);

      template <typename T>
      int getOctant(BOct* oct, int index);

      /* return is first free index in serial Buffer */
      void serializePointBuffer(BOct* oct, std::vector<Vector<BaseVector<float> > >& pts);
      
      void clear(BOct* oct);

  };

} // namespace lvr2

#endif