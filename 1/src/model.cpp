#include "model.h"

#include <QDebug>
#include <QFile>
#include <QTextStream>

/**
 * @brief Model::Model Constructs a new model from a Wavefront .obj file.
 * @param filename The filename. Should be a .obj file
 */
Model::Model(const QString& filename) {
  qDebug() << ":: Loading model:" << filename;
  QFile file(filename);
  if (file.open(QIODevice::ReadOnly)) {
    QTextStream in(&file);

    QString line;
    QStringList tokens;

    while (!in.atEnd()) {
      line = in.readLine();
      if (line.startsWith("#")) continue;  // skip comments
      tokens = line.split(" ", Qt::SkipEmptyParts);
      // Switch depending on first element
      if (tokens[0] == "v") {
        parseVertex(tokens);
      } else if (tokens[0] == "f") {
        parseFace(tokens);
      }
    }
    file.close();

    // create an array version of the data
    unpackIndexes();

    // Allign all vertex indices with the right normal/texturecoord indices
    alignData();
  }
}

/**
 * @brief Model::parseVertex Parses the coordinates of a vertex from the
 * .obj file.
 * @param tokens Tokens on the line in question.
 */
void Model::parseVertex(const QStringList& tokens) {
  float x = tokens[1].toFloat();
  float y = tokens[2].toFloat();
  float z = tokens[3].toFloat();
  coordsIndexed.append(QVector3D(x, y, z));
}

/**
 * @brief Model::parseFace Parses a face from the .obj file.
 * @param tokens Tokens on the line in question.
 */
void Model::parseFace(const QStringList& tokens) {
  QStringList elements;

  for (int i = 1; i != tokens.size(); ++i) {
    elements = tokens[i].split("/");
    // -1 since .obj count from 1
    indices.append(elements[0].toUInt() - 1);
  }
}

/**
 * @brief Model::alignData
 *
 * Make sure that the indices from the vertices align with those
 * of the normals and the texture coordinates, create extra vertices
 * if vertex has multiple normals or texturecoords.
 */
void Model::alignData() {
  QVector<QVector3D> verts;
  verts.reserve(coordsIndexed.size());

  QVector<unsigned> ind;
  ind.reserve(indices.size());

  unsigned currentIndex = 0;

  for (int i = 0; i != indices.size(); ++i) {
    QVector3D v = coordsIndexed[indices[i]];

    if (verts.contains(v)) {
      // Vertex already exists, use that index
      ind.append(verts.indexOf(v));
    } else {
      // Create a new vertex
      verts.append(v);
      ind.append(currentIndex);
      ++currentIndex;
    }
  }
  // Remove old data
  coordsIndexed.clear();
  indices.clear();

  // Set the new data
  coordsIndexed = verts;
  indices = ind;
}

/**
 * @brief Model::unpackIndexes Unpacks indices so that they are available for
 * glDrawArrays().
 */
void Model::unpackIndexes() {
  coords.clear();
  for (int i = 0; i != indices.size(); ++i) {
    coords.append(coordsIndexed[indices[i]]);
  }
}

// Getters

/**
 * @brief Model::getCoords Returns the coordinates of the mesh. The coordinates
 * are ordered in such a way that they can be directly used in glDrawArrays.
 * I.e. it contains for every triangle, 3 coordinates.
 * @return A list of coordinates.
 */
QVector<QVector3D> Model::getMeshCoords() { return coords; }

/**
 * @brief Model::getCoords Returns the unique coordinates of the mesh. These
 * coordinates do not fully describe the triangles in the mesh; only the
 * location of every vertex. I.e. it contains for every triangle, 3 coordinates.
 * Can be used in conjunction with getTriangleIndices if you want to use indexed
 * rendering (optional).
 * @return A list of unique coordinates.
 */
QVector<QVector3D> Model::getCoords() { return coordsIndexed; }

/**
 * @brief Model::getTriangleIndices Returns a list of indices that describe how
 * the vertices retrieved from getCoords make up the triangles in the mesh.
 * @return A list of indices.
 */
QVector<unsigned> Model::getTriangleIndices() { return indices; }

/**
 * @brief Model::getNumTriangles Retrieves the number of triangles in this mesh.
 * @return The number of triangles in this mesh.
 */
int Model::getNumTriangles() { return coords.size() / 3; }
