/*
 * Main.cpp
 *
 *  Created on: 16.05.2020
 *      Author: Benedikt Schumacher (bschumacher@uos.de)
 */

/// New includes, to be evaluated, which we actually need

#include "Options.hpp"

#include "lvr2/algorithm/CleanupAlgorithms.hpp"
#include "lvr2/geometry/BaseVector.hpp"
#include "lvr2/geometry/HalfEdgeMesh.hpp"
#include "lvr2/io/MeshBuffer.hpp"
#include "lvr2/io/ModelFactory.hpp"
#include "lvr2/io/PointBuffer.hpp"
#include "lvr2/reconstruction/AdaptiveKSearchSurface.hpp"
#include "lvr2/reconstruction/PointsetSurface.hpp"


using namespace lvr2;

using Vec = BaseVector<float>;

template <typename BaseVecT>
PointsetSurfacePtr<BaseVecT> loadPointCloud(const dmc_reconstruction::Options& options,
                                            PointBufferPtr buffer)
{
    // Create a point cloud manager
    string pcm_name = options.getPcm();
    PointsetSurfacePtr<BaseVecT> surface;

    // Create point set surface object
    if (pcm_name == "PCL")
    {
        cout << timestamp << "Using PCL as point cloud manager is not implemented yet!" << endl;
        panic_unimplemented("PCL as point cloud manager");
    }
    else if (pcm_name == "STANN" || pcm_name == "FLANN" || pcm_name == "NABO" ||
             pcm_name == "NANOFLANN")
    {
        surface = make_shared<AdaptiveKSearchSurface<BaseVecT>>(
            buffer, pcm_name, options.getKn(), options.getKi(), options.getKd(), 1, "");
    }
    else
    {
        cout << timestamp << "Unable to create PointCloudManager." << endl;
        cout << timestamp << "Unknown option '" << pcm_name << "'." << endl;
        return nullptr;
    }

    // Set search options for normal estimation and distance evaluation
    surface->setKd(options.getKd());
    surface->setKi(options.getKi());
    surface->setKn(options.getKn());

    // calc normals if there are none
    if (!buffer->hasNormals() && buffer.get()->numPoints() < 1000000)
    {
        surface->calculateSurfaceNormals();
    }

    return surface;
}

int main(int argc, char** argv)
{

    dmc_reconstruction::Options options(argc, argv);

    options.printLogo();

    // if one of the needed parameters is missing,
    if (options.printUsage())
    {
        return EXIT_SUCCESS;
    }

    std::cout << options << std::endl;

    // try to parse the model
    ModelPtr model = ModelFactory::readModel(options.getInputFileName());

    // did model parse succeed
    if (!model)
    {
        cout << timestamp << "IO Error: Unable to parse " << options.getInputFileName() << endl;
        return EXIT_FAILURE;
    }

    PointBufferPtr buffer = model->m_pointCloud;

    if (!buffer)
    {
        cout << "Failed to create Buffer...exiting..." << endl;
        PointBuffer* pointBuffer = new PointBuffer(model.get()->m_mesh.get()->getVertices(),
                                                   model.get()->m_mesh.get()->numVertices());
        PointBufferPtr pointer(pointBuffer);
        buffer = pointer;
    }

    // Create a point cloud manager
    string pcm_name = options.getPcm();
    auto surface = loadPointCloud<Vec>(options, buffer);
    if (!surface)
    {
        cout << "Failed to create pointcloud. Exiting." << endl;
        return EXIT_FAILURE;
    }

    

    // saveMesh();

    return 0;
}