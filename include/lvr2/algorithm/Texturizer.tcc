/* Copyright (C) 2011 Uni Osnabrück
* This file is part of the LAS VEGAS Reconstruction Toolkit,
*
* LAS VEGAS is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* LAS VEGAS is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
*/

/*
* Texturizer.tcc
*
*  @date 15.09.2017
*  @author Jan Philipp Vogtherr <jvogtherr@uni-osnabrueck.de>
*  @author Kristin Schmidt <krschmidt@uni-osnabrueck.de>
*/

#include <lvr/io/Progress.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

namespace lvr2
{

template<typename BaseVecT>
Texturizer<BaseVecT>::Texturizer(
    float texelSize,
    int texMinClusterSize,
    int texMaxClusterSize
) :
    m_texelSize(texelSize),
    m_texMinClusterSize(texMinClusterSize),
    m_texMaxClusterSize(texMaxClusterSize)
{
}



template<typename BaseVecT>
Texture<BaseVecT> Texturizer<BaseVecT>::getTexture(TextureHandle h)
{
    return m_textures[h];
}

template<typename BaseVecT>
StableVector<TextureHandle, Texture<BaseVecT>> Texturizer<BaseVecT>::getTextures()
{
    return m_textures;
}

template<typename BaseVecT>
int Texturizer<BaseVecT>::getTextureIndex(TextureHandle h)
{
    return m_textures[h].m_index;
}

template<typename BaseVecT>
void Texturizer<BaseVecT>::saveTextures()
{
    string comment = lvr::timestamp.getElapsedTime() + "Saving textures ";
    lvr::ProgressBar progress(m_textures.numUsed(), comment);
    for (auto h : m_textures)
    {
        m_textures[h].save();
        ++progress;
    }
    std::cout << std::endl;
}

template<typename BaseVecT>
TexCoords Texturizer<BaseVecT>::calculateTexCoords(
    TextureHandle h,
    const BoundingRectangle<BaseVecT>& br,
    BaseVecT v
)
{
    //return m_textures[h].calcTexCoords(boundingRect, v);
    auto texelSize = m_textures[h].m_texelSize;
    auto width = m_textures[h].m_width;
    auto height = m_textures[h].m_height;

    BaseVecT w =  v - ((br.m_vec1 * br.m_minDistA) + (br.m_vec2 * br.m_minDistB) + br.m_supportVector);
    float x = (br.m_vec1 * (w.dot(br.m_vec1))).length() / texelSize / width;
    float y = (br.m_vec2 * (w.dot(br.m_vec2))).length() / texelSize / height;

    return TexCoords(x,y);
}

template<typename BaseVecT>
TextureHandle Texturizer<BaseVecT>::generateTexture(
    int index,
    const PointsetSurface<BaseVecT>& surface,
    const BoundingRectangle<BaseVecT>& boundingRect
)
{

    // Calculate the texture size
    unsigned short int sizeX = ceil((boundingRect.m_maxDistA - boundingRect.m_minDistA) / m_texelSize);
    unsigned short int sizeY = ceil((boundingRect.m_maxDistB - boundingRect.m_minDistB) / m_texelSize);

    // Create texture
    Texture<BaseVecT> texture(index, sizeX, sizeY, 3, 1, m_texelSize);

    string comment = lvr::timestamp.getElapsedTime() + "Computing texture pixels ";
    lvr::ProgressBar progress(sizeX * sizeY, comment);

    if (surface.pointBuffer()->hasRgbColor())
    {
        #pragma omp parallel for schedule(dynamic,1) collapse(2)
        for (int y = 0; y < sizeY; y++)
        {
            for (int x = 0; x < sizeX; x++)
            {
                // std::vector<char> v;

                int k = 1; // k-nearest-neighbors

                vector<size_t> cv;

                Point<BaseVecT> currentPos =
                    boundingRect.m_supportVector
                    + boundingRect.m_vec1 * (x * m_texelSize + boundingRect.m_minDistA - m_texelSize / 2.0)
                    + boundingRect.m_vec2 * (y * m_texelSize + boundingRect.m_minDistB - m_texelSize / 2.0);

                surface.searchTree().kSearch(currentPos, k, cv);

                uint8_t r = 0, g = 0, b = 0;

                for (size_t pointIdx : cv)
                {
                    array<uint8_t,3> colors = *(surface.pointBuffer()->getRgbColor(pointIdx));
                    r += colors[0];
                    g += colors[1];
                    b += colors[2];
                }

                r /= k;
                g /= k;
                b /= k;

                texture.m_data[(sizeY - y - 1) * (sizeX * 3) + 3 * x + 0] = r;
                texture.m_data[(sizeY - y - 1) * (sizeX * 3) + 3 * x + 1] = g;
                texture.m_data[(sizeY - y - 1) * (sizeX * 3) + 3 * x + 2] = b;

                ++progress;
            }
        }
        std::cout << std::endl;
    }
    else
    {
        for (int i = 0; i < sizeX * sizeY; i++)
        {
            texture.m_data[i] = 0;
        }
    }

    return m_textures.push(texture);
}

template<typename BaseVecT>
void Texturizer<BaseVecT>::findKeyPointsInTexture(const TextureHandle texH,
        const BoundingRectangle<BaseVecT>& boundingRect,
        const cv::Ptr<cv::Feature2D>& detector,
        std::vector<cv::KeyPoint>& keypoints, cv::Mat& descriptors)
{
    const Texture<BaseVecT> texture = m_textures[texH];
    if (texture.m_height <= 32 && texture.m_width <= 32)
    {
        return;
    }

    const unsigned char* img_data = texture.m_data;
    cv::Mat image(texture.m_height, texture.m_width, CV_8UC3, (void*)img_data);

    detector->detectAndCompute(image, cv::noArray(), keypoints, descriptors);

}

template<typename BaseVecT>
std::vector<BaseVecT> Texturizer<BaseVecT>::keypoints23d(const std::vector<cv::KeyPoint>&
        keypoints, const BoundingRectangle<BaseVecT>& boundingRect)
{
    const size_t N = keypoints.size();
    std::vector<BaseVecT> keypoints3d(N);

    for (size_t p_idx = 0; p_idx < N; ++p_idx)
    {
        // for opencv image coordinatate invert y-coordinate
        const cv::Point2f keypoint = keypoints[p_idx].pt;
        const float s1 = keypoint.x  + boundingRect.m_minDistA;
        const float s2 = -keypoint.y + boundingRect.m_minDistB;
        keypoints3d[p_idx] = boundingRect.m_supportVector
                             + boundingRect.m_vec1 * s1
                             + boundingRect.m_vec2 * s2;
    }
    return keypoints3d;
}

} // namespace lvr2
