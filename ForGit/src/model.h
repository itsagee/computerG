#ifndef MODEL_H
#define MODEL_H

#include <QString>
#include <QStringList>
#include <QVector2D>
#include <QVector3D>
#include <QVector>

/**
 * @brief A simple Model class. Represents a 3D triangle mesh and is able to
 * load this data from a Wavefront .obj file. IMPORTANT: Current only supports
 * TRIANGLE meshes! It will only load in the coordinates; not the normals or
 * texture coordinates.
 *
 * Support for other meshes can be implemented by students.
 *
 */
class Model {
 public:
  Model(const QString& filename);

  // Can be used for glDrawArrays()
  QVector<QVector3D> getMeshCoords();

  // Can be used for glDrawElements()
  QVector<QVector3D> getCoords();
  QVector<unsigned> getTriangleIndices();
  int getNumTriangles();

 private:
  // OBJ parsing
  void parseVertex(const QStringList& tokens);
  void parseFace(const QStringList& tokens);

  // Alignment of data
  void alignData();
  void unpackIndexes();

  // Intermediate storage of values
  QVector<QVector3D> coordsIndexed;
  QVector<unsigned> indices;

  QVector<QVector3D> coords;
};

#endif  // MODEL_H
