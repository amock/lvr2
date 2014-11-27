// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2011 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2011 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2011 Mateusz Loskot, London, UK.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_GEOMETRY_HPP
#define BOOST_GEOMETRY_GEOMETRY_HPP

// Shortcut to include all header files


#include <boost148/geometry/core/cs.hpp>
#include <boost148/geometry/core/tag.hpp>
#include <boost148/geometry/core/tag_cast.hpp>
#include <boost148/geometry/core/tags.hpp>

// Core algorithms
#include <boost148/geometry/core/access.hpp>
#include <boost148/geometry/core/exterior_ring.hpp>
#include <boost148/geometry/core/interior_rings.hpp>
#include <boost148/geometry/core/radian_access.hpp>
#include <boost148/geometry/core/topological_dimension.hpp>


#include <boost148/geometry/arithmetic/arithmetic.hpp>
#include <boost148/geometry/arithmetic/dot_product.hpp>

#include <boost148/geometry/strategies/strategies.hpp>

#include <boost148/geometry/algorithms/append.hpp>
#include <boost148/geometry/algorithms/area.hpp>
#include <boost148/geometry/algorithms/assign.hpp>
#include <boost148/geometry/algorithms/buffer.hpp>
#include <boost148/geometry/algorithms/centroid.hpp>
#include <boost148/geometry/algorithms/clear.hpp>
#include <boost148/geometry/algorithms/convex_hull.hpp>
#include <boost148/geometry/algorithms/correct.hpp>
#include <boost148/geometry/algorithms/comparable_distance.hpp>
#include <boost148/geometry/algorithms/difference.hpp>
#include <boost148/geometry/algorithms/distance.hpp>
#include <boost148/geometry/algorithms/envelope.hpp>
#include <boost148/geometry/algorithms/for_each.hpp>
#include <boost148/geometry/algorithms/intersection.hpp>
#include <boost148/geometry/algorithms/intersects.hpp>
#include <boost148/geometry/algorithms/length.hpp>
#include <boost148/geometry/algorithms/make.hpp>
#include <boost148/geometry/algorithms/num_geometries.hpp>
#include <boost148/geometry/algorithms/num_interior_rings.hpp>
#include <boost148/geometry/algorithms/num_points.hpp>
#include <boost148/geometry/algorithms/overlaps.hpp>
#include <boost148/geometry/algorithms/perimeter.hpp>
#include <boost148/geometry/algorithms/reverse.hpp>
#include <boost148/geometry/algorithms/simplify.hpp>
#include <boost148/geometry/algorithms/sym_difference.hpp>
#include <boost148/geometry/algorithms/transform.hpp>
#include <boost148/geometry/algorithms/union.hpp>
#include <boost148/geometry/algorithms/unique.hpp>
#include <boost148/geometry/algorithms/within.hpp>

// Include multi a.o. because it can give weird effects
// if you don't (e.g. area=0 of a multipolygon)
#include <boost148/geometry/multi/multi.hpp>

// check includes all concepts
#include <boost148/geometry/geometries/concepts/check.hpp>

#include <boost148/geometry/util/for_each_coordinate.hpp>
#include <boost148/geometry/util/math.hpp>
#include <boost148/geometry/util/select_most_precise.hpp>
#include <boost148/geometry/util/select_coordinate_type.hpp>
#include <boost148/geometry/util/write_dsv.hpp>

#include <boost148/geometry/views/box_view.hpp>
#include <boost148/geometry/views/segment_view.hpp>

#include <boost148/geometry/domains/gis/io/wkt/wkt.hpp>


#endif // BOOST_GEOMETRY_GEOMETRY_HPP