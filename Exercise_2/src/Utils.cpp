#include "Utils.hpp"
#include "PolygonalMesh.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <Eigen/Eigen>

namespace PolymeshLibrary {

bool ImportCell0Ds(const string &filename,PolygonalMesh& mesh)
{
    ifstream file;
    file.open(filename);

    if(file.fail())
        return false;

    list<string> listLines;
    string line;
    while (getline(file, line))
    {
        std::replace(line.begin(),line.end(),';',' ');
        listLines.push_back(line);
    }
    file.close();

    listLines.pop_front();

    mesh.NumberCell0D = listLines.size();

    if (mesh.NumberCell0D == 0)
    {
        cerr << "There is no cell 0D" << endl;
        return false;
    }

    mesh.Cell0DId.reserve(mesh.NumberCell0D);
    mesh.Cell0DCoordinates.reserve(mesh.NumberCell0D);

    for (const string& line : listLines)
    {
        istringstream converter(line);

        unsigned int id;
        unsigned int marker;
        Vector2d coord;

        converter >>  id >> marker >> coord(0) >> coord(1);

        mesh.Cell0DId.push_back(id);
        mesh.Cell0DCoordinates.push_back(coord);


        if( marker != 0)
        {
            auto ret = mesh.Cell0DMarkers.insert({marker, {id}});
            if(!ret.second)
                (ret.first)->second.push_back(id);
         }

    }
    file.close();
    return true;
}

bool ImportCell1Ds(const string &filename,PolygonalMesh& mesh)
{
    ifstream file;
    file.open(filename);

    if(file.fail())
        return false;

    list<string> listLines;
    string line;
    while (getline(file, line))
    {
        std::replace(line.begin(),line.end(),';',' ');
        listLines.push_back(line);
    }

    file.close();

    listLines.pop_front();

    mesh.NumberCell1D = listLines.size();

    if (mesh.NumberCell1D == 0)
    {
        cerr << "There is no cell 1D" << endl;
        return false;
    }

    mesh.Cell1DId.reserve(mesh.NumberCell1D);
    mesh.Cell1DVertices.reserve(mesh.NumberCell1D);

    for (const string& line : listLines)
    {
        istringstream converter(line);

        unsigned int id;
        unsigned int marker;
        Vector2i vertices;

        converter >>  id >> marker >> vertices(0) >> vertices(1);

        mesh.Cell1DId.push_back(id);
        mesh.Cell1DVertices.push_back(vertices);

        if( marker != 0)
        {
            auto ret = mesh.Cell1DMarkers.insert({marker, {id}});
            if(!ret.second)
                (ret.first)->second.push_back(id);
        }
    }
    file.close();

    return true;
}

bool ImportCell2Ds(const string &filename,PolygonalMesh& mesh)
{
    ifstream file;
    file.open(filename);

    if(file.fail())
       return false;

    list<string> listLines;
    string line;
    while (getline(file, line))
    {
        std::replace(line.begin(),line.end(),';',' ');
        listLines.push_back(line);
    }
    file.close();

    listLines.pop_front();

    mesh.NumberCell2D = listLines.size();

    if (mesh.NumberCell2D == 0)
    {
        cerr << "There is no cell 2D" << endl;
        return false;
    }

    mesh.Cell2DId.reserve(mesh.NumberCell2D);
    mesh.Cell2DVertices.reserve(mesh.NumberCell2D);
    mesh.Cell2DEdges.reserve(mesh.NumberCell2D);

    for (const string& line : listLines)
    {
        istringstream converter(line);

        unsigned int id;
        unsigned int marker;
        unsigned int num_vertices;
        unsigned int num_edges;

        converter >>  id;
        converter >> marker;
        converter >> num_vertices;
        VectorXi vertices;
        vertices.resize(num_vertices);
        for(unsigned int i = 0; i < num_vertices; i++)
            converter >> vertices[i];

        converter >> num_edges;
        VectorXi edges;
        edges.resize(num_edges);
        for(unsigned int i = 0; i < num_edges; i++)
            converter >> edges[i];

        mesh.Cell2DId.push_back(id);
        mesh.Cell2DVertices.push_back(vertices);
        mesh.Cell2DEdges.push_back(edges);
    }

    return true;
}

bool ImportMesh(const string& filepath, PolygonalMesh& mesh)
{

    if(!ImportCell0Ds(filepath + "/Cell0Ds.csv", mesh))
    {
        return false;
    }
    else
    {
        cout << "Cell0D marker:" << endl;
        for(auto iterator = mesh.Cell0DMarkers.begin(); iterator != mesh.Cell0DMarkers.end(); iterator++)
        {
            cout << "key:\t" << iterator -> first << "\t values:";
            for(const unsigned int id : iterator -> second)
                cout << "\t" << id;
            cout << endl;
        }
    }

    if(!ImportCell1Ds(filepath + "/Cell1Ds.csv",mesh))
    {
        return false;
    }
    else
    {
        cout << "Cell1D marker:" << endl;
        for(auto iterator = mesh.Cell1DMarkers.begin(); iterator != mesh.Cell1DMarkers.end(); iterator++)
        {
            cout << "key:\t" << iterator -> first << "\t values:";
            for(const unsigned int id : iterator -> second)
                cout << "\t" << id;

            cout << endl;
        }
    }
    if(!ImportCell2Ds(filepath + "/Cell2Ds.csv", mesh))
    {
        return false;
    }
    else
    {
        for (unsigned int i=0; i < mesh.NumberCell2D; i++)
        {
            const unsigned int numEdges = mesh.Cell2DEdges[i].size();
            VectorXi edges = mesh.Cell2DEdges[i];

            for(unsigned int k = 0; k < numEdges; k++){
                const unsigned int origin = mesh.Cell1DVertices[edges[k]][0];
                const unsigned int end = mesh.Cell1DVertices[edges[k]][1];

                auto findOrigin = find(mesh.Cell2DVertices[i].begin(), mesh.Cell2DVertices[i].end(), origin);
                if(findOrigin == mesh.Cell2DVertices[i].end())
                {
                    cerr << "Wrong mesh" << endl;
                    return 2;
                }

                auto findEnd = find(mesh.Cell2DVertices[i].begin(), mesh.Cell2DVertices[i].end(), end);
                if(findEnd == mesh.Cell2DVertices[i].end())
                {
                    cerr << "Wrong mesh" << endl;
                    return 3;
                }
            }
        }
    }
    return true;
}

bool CheckNonZeroEdgeLengths(PolygonalMesh& mesh)
{
    for (const auto& cell1D : mesh.Cell1DVertices) {
        Vector2d origin = mesh.Cell0DCoordinates[cell1D(0)];
        Vector2d end = mesh.Cell0DCoordinates[cell1D(1)];
        Vector2d edge = end - origin;
        if (edge.norm() <= 2*numeric_limits<double>::epsilon()) {
            return false;
        }
    }
    cout << "The lengths are all nonzero" << endl;
    return true;

}

bool CheckNonZeroPolygonArea(PolygonalMesh& mesh)
{
        for (const auto& cell2D : mesh.Cell2DVertices) {
        if (cell2D.size() < 3) {
            return false;
        }

        unsigned int n = cell2D.size();
        double Area = 0.0;
        list<Vector2d> listVectors;
        for (unsigned int k = 0; k < n; k++) {
            listVectors.push_back(mesh.Cell0DCoordinates[cell2D[k]]);
        }
        auto iterator = listVectors.begin();
        auto j = next(iterator);

        for (unsigned int k = 0; k < n; k++, iterator++, j++) {
            if (j == listVectors.end())
                j = listVectors.begin();

            Area += (iterator->x() * j->y() - j->x() * iterator->y());
        }

        double TotalArea = abs(Area)*0.5;

        if (TotalArea <= 1e-6) {
            return false;
        }
    }
    cout << "All the areas are nonzero " << endl;
    return true;
}
}
