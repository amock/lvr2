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
 * ObjIO.cpp
 *
 *  @date 07.11.2011
 *  @author Florian Otte (fotte@uos.de)
 *  @author Kim Rinnewitz (krinnewitz@uos.de)
 *  @author Sven Schalk (sschalk@uos.de)
 *  @author Lars Kiesow (lkiesow@uos.de)
 *  @author Denis Meyer (denmeyer@uos.de)
 */

#include <lvr2/io/ObjIO.hpp>

#include <climits>
#include <iostream>
#include <fstream>
#include <string.h>
#include <locale.h>
#include <sstream>

#include <boost/filesystem.hpp>
#include "boost/tuple/tuple.hpp"

#include <lvr2/io/PLYIO.hpp>
#include <lvr2/io/Timestamp.hpp>
#include <lvr/geometry/Vertex.hpp>
#include <lvr/display/GlTexture.hpp>
#include <lvr/display/TextureFactory.hpp>

#include <lvr2/texture/Texture.hpp>
#include <lvr2/texture/Material.hpp>


namespace lvr2
{

using namespace std; // Bitte vergebt mir....
// Meinst du wirklich, dass ich dir so etwas durchgehen lassen kann?

template<typename T>
boost::shared_array<T> convert_vector_to_shared_array(vector<T> source)
{

    boost::shared_array<T> ret = boost::shared_array<T>( new T[source.size()] );
    for (int i = 0; i < source.size(); i++)
    {
       ret[i] = source[i]; 
    }

    return ret;
}

void tokenize(const string& str,
                      vector<string>& tokens,
                      const string& delimiters = " ")
{
    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

void ObjIO::parseMtlFile(
        map<string, int>& matNames,
        vector<RGBMaterial*>& materials,
        vector<GlTexture*>& textures,
        string mtlname)
{
    cout << "Parsing " << mtlname << endl;

    // Get path object
    boost::filesystem::path p(mtlname);
    p = p.remove_filename();

    ifstream in(mtlname.c_str());
    if(in.good())
    {
        char buffer[1024];
        RGBMaterial* m = 0;
        int matIndex = 0;
        while(in.good())
        {
            in.getline(buffer, 1024);

            // Skip comments
            if(buffer[0] == '#') continue;

            stringstream ss(buffer);
            string keyword;
            ss >> keyword;

            if(keyword == "newmtl")
            {
                string matName;
                ss >> matName;
                map<string, int>::iterator it = matNames.find(matName);
                if(it == matNames.end())
                {
                    m = new RGBMaterial;
                    m->r = 128;
                    m->g = 128;
                    m->b = 128;
                    m->texture_index = -1;
                    materials.push_back(m);
                    matNames[matName] = matIndex;
                    matIndex++;

                }
                else
                {
                    //m = materials[matNames[matName]];
                    cout << "ObjIO::parseMtlFile(): Warning: Duplicate material: " << matName << endl;
                }
            }
            else if(keyword == "Ka")
            {
                float r, g, b;
                ss >> r >> g >> b;
                
                m->r = (unsigned char)(r * 255.0);
                m->g = (unsigned char)(g * 255.0);
                m->b = (unsigned char)(b * 255.0);

            }
            else if(keyword == "map_Kd")
            {
                string texname;
                ss >> texname;

                // Add full path to texture file name
                boost::filesystem::path tex_file = p / texname;

                GlTexture* texture = TextureFactory::instance().getTexture(tex_file.string());
                textures.push_back(texture);
                m->texture_index = (GLuint)textures.size() - 1;
            }
            else
            {
                continue;
            }
        }
    }
    else
    {
        cout << "ObjIO::parseMtlFile(): Error opening '" << mtlname << "'." << endl;
    }
}

ModelPtr ObjIO::read(string filename)
{
    // Get path from filename
    boost::filesystem::path p(filename);

    ifstream in(filename.c_str());

    vector<float>         vertices;
    vector<float>         normals;
    vector<unsigned char>         colors;
    vector<float>         texcoords;
    vector<uint>        faceMaterials;
    vector<uint>          faces;
    vector<RGBMaterial*>     materials;
    vector<GlTexture*>    textures;

    map<string, int> matNames;

    int currentMat = 0;

    if(in.good())
    {
        char buffer[1024];
        while(in.good())
        {
            in.getline(buffer, 1024);

            // Skip comments
            if(buffer[0] == '#') continue;

            stringstream ss(buffer);
            string keyword;
            ss >> keyword;
            float x, y, z;
            float r, g, b;
            if(keyword == "v")
            {
                ss >> x >> y >> z;
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);
                // TODO: check if r is end of line
                std::string s;
                bool colors_exist = static_cast<bool>(ss >> r);
                // r = s[0];
                ss >> g >> b;
                if(colors_exist)
                {
                    colors.push_back(static_cast<unsigned char>(r*255.0 + 0.5));
                    colors.push_back(static_cast<unsigned char>(g*255.0 + 0.5));
                    colors.push_back(static_cast<unsigned char>(b*255.0 + 0.5));
                }
            }
            else if(keyword == "vt")
            {
                ss >> x >> y >> z;
                texcoords.push_back(x);
                texcoords.push_back(1.0 - y);
                texcoords.push_back(z);
            }
            else if(keyword == "vn")
            {
                ss >> x >> y >> z;
                normals.push_back(x);
                normals.push_back(y);
                normals.push_back(z);
            }
            else if(keyword == "f")
            {
                vector<string> tokens;
                tokenize(buffer, tokens);

                if(tokens.size() < 4)
                    continue;

                vector<string> tokens2;
                tokenize(tokens.at(1),tokens2,"/");
                int a = atoi(tokens2.at(0).c_str());
                tokens2.clear();

                tokenize(tokens.at(2),tokens2,"/");
                int b = atoi(tokens2.at(0).c_str());
                tokens2.clear();

                tokenize(tokens.at(3),tokens2,"/");
                int c = atoi(tokens2.at(0).c_str());
                tokens2.clear();

                faces.push_back(a - 1);
                faces.push_back(b - 1);
                faces.push_back(c - 1);

                // Use current material
                faceMaterials.push_back(currentMat);
            }
            else if(keyword == "usemtl")
            {
                string mtlname;
                ss >> mtlname;
                // Find name and set current material
                map<string, int>::iterator it = matNames.find(mtlname);
                if(it == matNames.end())
                {
                    cout << "ObjIO:read(): Warning material '" << mtlname << "' is undefined." << endl;
                }
                else
                {
                    currentMat = it->second;
                }
            }
            else if(keyword == "mtllib")
            {
                // Get current path
                p = p.remove_filename();

                // Append .mtl file name
                string mtlfile;
                ss >> mtlfile;
                p = p / mtlfile;

                // Get path as string and parse mtl
                string mtl_path = p.string();
                parseMtlFile(matNames, materials, textures, mtl_path);
            }
        }

    }
    else
    {
        cout << timestamp << "ObjIO::read(): Unable to open file'" << filename << "'." << endl;
    }

    MeshBuffer2Ptr mesh = MeshBuffer2Ptr(new MeshBuffer2);

    if(materials.size())
    {
        vector<Material> &mesh_mats = mesh->getMaterials();

        for (RGBMaterial *m : materials)
        {
            Material tmp;
            tmp.m_color = {m->r, m->g, m->b};
            tmp.m_texture = boost::none; 

            if (m->texture_index >= 0)
            {
                tmp.m_texture = TextureHandle(m->texture_index); 
            }
            
            mesh_mats.push_back(tmp);

            delete m;
        }
    }

    mesh->setVertices(convert_vector_to_shared_array(vertices), vertices.size() / 3);
    mesh->setFaceIndices(convert_vector_to_shared_array(faces), faces.size() / 3);

    if(faceMaterials.size() == faces.size() / 3)
    {

        indexArray indices = indexArray( new unsigned int[faceMaterials.size()] );

        for (unsigned int i = 0; i < faceMaterials.size(); i++)
        {
            indices[i] = faceMaterials[i]; 
        }

        mesh->setFaceMaterialIndices(indices);
    }
    else
    {
        cout << "ObjIO::read(): Warning: Face material index buffer does not match face number." << endl;
    }

    if(textures.size())
    {
        vector<Texture> &mesh_textures = mesh->getTextures();

        for (int i = 0; i < textures.size(); i++)
        {
            GlTexture * oldTexture = textures[i];
            Texture tex(i, oldTexture);
            delete oldTexture;

            mesh_textures.push_back(std::move(tex));              
        }
    }

    mesh->setTextureCoordinates(convert_vector_to_shared_array(texcoords));
    mesh->setVertexNormals(convert_vector_to_shared_array(normals));
    mesh->setVertexColors(convert_vector_to_shared_array(colors));

    ModelPtr m(new Model(mesh));
    m_model = m;

    return m;
}


class sort_indices
{
   private:
     uintArr faceMaterialIndices;
   public:
     sort_indices(uintArr faceMaterialIndices) : faceMaterialIndices(faceMaterialIndices) {}
     bool operator()(int i, int j) { return faceMaterialIndices[i]<faceMaterialIndices[j]; }
};

void ObjIO::save( string filename )
{

    typedef lvr::Vertex<unsigned char> ObjColor;

    unsigned dummy;
    size_t lenVertices = m_model->m_mesh->numVertices();
    size_t lenNormals = lenVertices;
    size_t lenFaces = m_model->m_mesh->numFaces();
    size_t lenTextureCoordinates = lenVertices;
    size_t lenFaceMaterialIndices = lenFaces;
    size_t lenColors = lenVertices;
    floatArr vertices           = m_model->m_mesh->getVertices();
    floatArr normals            = m_model->m_mesh->getVertexNormals();
    floatArr textureCoordinates = m_model->m_mesh->getTextureCoordinates();
    indexArray   faceIndices        = m_model->m_mesh->getFaceIndices();
    vector<Material> &materials     = m_model->m_mesh->getMaterials();
    indexArray faceMaterialIndices   = m_model->m_mesh->getFaceMaterialIndices();
    ucharArr colors               = m_model->m_mesh->getVertexColors(dummy);

    std::map<ObjColor, unsigned int> colorMap;


    std::set<unsigned int> materialIndexSet;
    std::set<unsigned int> colorIndexSet;

    ofstream out(filename.c_str());
    ofstream mtlFile("textures.mtl");

    if(out.good())
    {
        out<<"mtllib textures.mtl"<<endl;

        if ( !vertices )
        {
            cerr << "Received no vertices to store. Aborting save operation." << endl;
            return;
        }
        out << endl << endl << "##  Beginning of vertex definitions.\n";

        for( size_t i=0; i < lenVertices; ++i )
        {

            out << "v " << vertices[i*3 + 0] << " "
                    << vertices[i*3 + 1] << " "
                    << vertices[i*3 + 2] << " ";
                    if(lenColors>0){
                        unsigned char r = colors[i*3 + 0],
                                      g = colors[i*3 + 1],
                                      b = colors[i*3 + 2];

                        out << static_cast<float>(r)/255.0 << " "
                        << static_cast<float>(g)/255.0 << " "
                        << static_cast<float>(b)/255.0 ;
                    }
                    out << endl;
        }

        out<<endl;

        if (m_model->m_mesh->hasVertexNormals())
        {
            out << endl << endl << "##  Beginning of vertex normals.\n";
            for( size_t i=0; i < lenNormals; ++i )
            {
                out << "vn " << normals[i*3 + 0] << " "
                        << normals[i*3 + 1] << " "
                        << normals[i*3 + 2] << endl;
            }
        }

        out << endl << endl << "##  Beginning of vertexTextureCoordinates.\n";

        for( size_t i=0; i < lenTextureCoordinates; ++i )
        {
            out << "vt " << textureCoordinates[i*3 + 0] << " "
                    << 1.0 - textureCoordinates[i*3 + 1] << " "
                    << textureCoordinates[i*3 + 2] << endl;
        }


        out << endl << endl << "##  Beginning of faces.\n";
        // format of a face: f v/vt/vn
        //for( size_t i = 0; i < lenFaces; ++i )
        //{
            //cout << faceMaterialIndices[i] << " " << lenFaceMaterials << endl;
            //RGBMaterial* m = materials[faceMaterialIndices[i]];
            //if(m->texture_index >= 0)
            //{
                //out << "usemtl texture_" << m->texture_index << endl;
            //}
            //else
            //{
                //out << "usemtl color_" << faceMaterialIndices[i] << endl;
            //}

            ////unsigned int* faceTextureIndices
            ////float**       textureCoordinates
            ////usemtl....
            //// +1 after every index since in obj the 0-th vertex has index 1.
            //out << "f "
                    //<< faceIndices[i * 3 + 0] + 1 << "/"
                    //<< faceIndices[i * 3 + 0] + 1 << "/"
                    //<< faceIndices[i * 3 + 0] + 1 << " "
                    //<< faceIndices[i * 3 + 1] + 1 << "/"
                    //<< faceIndices[i * 3 + 1] + 1 << "/"
                    //<< faceIndices[i * 3 + 1] + 1 << " "
                    //<< faceIndices[i * 3 + 2] + 1 << "/"
                    //<< faceIndices[i * 3 + 2] + 1 << "/"
                    //<< faceIndices[i * 3 + 2] + 1 << endl;
        //}

        // format of a face: f v/vt/vn

        std::vector<int> color_indices, texture_indices;

        //splitting materials in colors an textures
        for(size_t i = 0; i< lenFaceMaterialIndices; ++i)
        {
            Material &m = materials[faceMaterialIndices[i]];
            if(m.m_texture)
            {
                texture_indices.push_back(i);
            }else{
                color_indices.push_back(i);
            }
        }
        //sort faceMaterialsIndices: colors, textur_indices
        //sort new index lists instead of the faceMaterialIndices
        std::sort(color_indices.begin(),color_indices.end(),sort_indices(faceMaterialIndices));
        std::sort(texture_indices.begin(),texture_indices.end(),sort_indices(faceMaterialIndices));

        //colors
        for(size_t i = 0; i<color_indices.size() ; i++)
        {
            unsigned int first = faceMaterialIndices[color_indices[i]];
            unsigned int face_index=color_indices[i];

            if( i == 0 || first != faceMaterialIndices[color_indices[i-1]] )
            {
                out << "usemtl color_" << faceMaterialIndices[color_indices[i]] << endl;
                out << "f "
                    << faceIndices[face_index * 3 + 0] + 1 << "/"
                    << faceIndices[face_index * 3 + 0] + 1 << "/"
                    << faceIndices[face_index * 3 + 0] + 1 << " "
                    << faceIndices[face_index * 3 + 1] + 1 << "/"
                    << faceIndices[face_index * 3 + 1] + 1 << "/"
                    << faceIndices[face_index * 3 + 1] + 1 << " "
                    << faceIndices[face_index * 3 + 2] + 1 << "/"
                    << faceIndices[face_index * 3 + 2] + 1 << "/"
                    << faceIndices[face_index * 3 + 2] + 1 << endl;

            }else if( first == faceMaterialIndices[color_indices[i-1]] )
            {
                out << "f "
                    << faceIndices[face_index * 3 + 0] + 1 << "/"
                    << faceIndices[face_index * 3 + 0] + 1 << "/"
                    << faceIndices[face_index * 3 + 0] + 1 << " "
                    << faceIndices[face_index * 3 + 1] + 1 << "/"
                    << faceIndices[face_index * 3 + 1] + 1 << "/"
                    << faceIndices[face_index * 3 + 1] + 1 << " "
                    << faceIndices[face_index * 3 + 2] + 1 << "/"
                    << faceIndices[face_index * 3 + 2] + 1 << "/"
                    << faceIndices[face_index * 3 + 2] + 1 << endl;
            }
        }

        out<<endl;

        //textures
        for(size_t i = 0; i<texture_indices.size() ; i++)
        {
            Material &first = materials[faceMaterialIndices[texture_indices[i]]];
            size_t face_index=texture_indices[i];

            if(i==0 || first.m_texture != materials[faceMaterialIndices[texture_indices[i-1]]].m_texture )
            {
                out << "usemtl texture_" << first.m_texture->idx() << endl;
                //std::cout << "usemtl texture_" << first->texture_index << std::endl;
                out << "f "
                    << faceIndices[face_index * 3 + 0] + 1 << "/"
                    << faceIndices[face_index * 3 + 0] + 1 << "/"
                    << faceIndices[face_index * 3 + 0] + 1 << " "
                    << faceIndices[face_index * 3 + 1] + 1 << "/"
                    << faceIndices[face_index * 3 + 1] + 1 << "/"
                    << faceIndices[face_index * 3 + 1] + 1 << " "
                    << faceIndices[face_index * 3 + 2] + 1 << "/"
                    << faceIndices[face_index * 3 + 2] + 1 << "/"
                    << faceIndices[face_index * 3 + 2] + 1 << endl;
            }else if(first.m_texture == materials[faceMaterialIndices[texture_indices[i-1]]].m_texture )
            {
                out << "f "
                    << faceIndices[face_index * 3 + 0] + 1 << "/"
                    << faceIndices[face_index * 3 + 0] + 1 << "/"
                    << faceIndices[face_index * 3 + 0] + 1 << " "
                    << faceIndices[face_index * 3 + 1] + 1 << "/"
                    << faceIndices[face_index * 3 + 1] + 1 << "/"
                    << faceIndices[face_index * 3 + 1] + 1 << " "
                    << faceIndices[face_index * 3 + 2] + 1 << "/"
                    << faceIndices[face_index * 3 + 2] + 1 << "/"
                    << faceIndices[face_index * 3 + 2] + 1 << endl;
            }
        }


        out<<endl;
        out.close();
    }
    else
    {
        cerr << "no good. file! \n";
    }



    if( mtlFile.good() )
    {

        for(int i = 0; i < materials.size(); i++)
        {
            const Material &m = materials[i];
            if(!m.m_texture)
            {
                mtlFile << "newmtl color_" << i << endl;
                mtlFile << "Ka "
                        << m.m_color->at(0) / 255.0f << " "
                        << m.m_color->at(1) / 255.0f << " "
                        << m.m_color->at(2) / 255.0f << endl;
                mtlFile << "Kd "
                        << m.m_color->at(0) / 255.0f << " "
                        << m.m_color->at(1) / 255.0f << " "
                        << m.m_color->at(2) / 255.0f << endl << endl;
            }
            else
            {
                mtlFile << "newmtl texture_"      << m.m_texture->idx() << endl;
                mtlFile << "Ka 1.000 1.000 1.000" << endl;
                mtlFile << "Kd 1.000 1.000 1.000" << endl;
                mtlFile << "map_Kd texture_"      << m.m_texture->idx() << ".ppm" << endl << endl;
            }
        }
    }
    mtlFile.close();
}


} // Namespace lvr2
