#include "mainview.h"
#include "vertex.h"

#include <QDateTime>
#include <model.h>
#include <QVector3D>

// to load the model, get the vertex coordinates in order and make the vertex array
Model knot = Model(":/models/knot.obj");
QVector<QVector3D> knotCoords = knot.getMeshCoords();
QVector<Vertex> knotVertices;

/**
 * @brief MainView::MainView Constructs a new main view.
 *
 * @param parent Parent widget.
 */
MainView::MainView(QWidget *parent) : QOpenGLWidget(parent) {
  qDebug() << "MainView constructor";

  connect(&timer, SIGNAL(timeout()), this, SLOT(update()));
}

/**
 * @brief MainView::~MainView
 *
 * Destructor of MainView
 * This is the last function called, before exit of the program
 * Use this to clean up your variables, buffers etc.
 *
 */
MainView::~MainView() {
  qDebug() << "MainView destructor";

    makeCurrent();

    // Delete the buffer objects (assuming we want to delete all VBOs and VAOs)
    GLuint buffersToDelete[] = {vbo, vao, knotVAO, knotVBO};
    glDeleteBuffers(4, buffersToDelete);

}

// --- OpenGL initialization

/**
 * @brief MainView::initializeGL Called upon OpenGL initialization
 * Attaches a debugger and calls other init functions.
 */
void MainView::initializeGL() {
  qDebug() << ":: Initializing OpenGL";
  initializeOpenGLFunctions();

  connect(&debugLogger, SIGNAL(messageLogged(QOpenGLDebugMessage)), this,
          SLOT(onMessageLogged(QOpenGLDebugMessage)), Qt::DirectConnection);

  if (debugLogger.initialize()) {
    qDebug() << ":: Logging initialized";
    debugLogger.startLogging(QOpenGLDebugLogger::SynchronousLogging);
  }

  QString glVersion{reinterpret_cast<const char *>(glGetString(GL_VERSION))};
  qDebug() << ":: Using OpenGL" << qPrintable(glVersion);

  // Enable depth buffer
  glEnable(GL_DEPTH_TEST);

  // Enable backface culling
  glEnable(GL_CULL_FACE);

  // Default is GL_LESS
  glDepthFunc(GL_LEQUAL);

  // Set the color to be used by glClear. This is, effectively, the background color.
  glClearColor(0.37f, 0.42f, 0.45f, 0.0f);


  //NEW CODE
  //added new: first we instantiate three vertices, one for each triangle corner
  Vertex red    = {-1.0,1.0,1.0,    1.0,0.0,0.0};   //red, top left (base)      aka 1
  Vertex green  = {-1.0,-1.0,1.0,   0.0,0.0,1.0};   //green, top right (base)   aka v2
  Vertex yellow = {1.0,-1.0,1.0,    1.0,1.0,0.0};   //yellow down right (base)  aka v3
  Vertex blue   = {1.0,1.0,1.0,     0.0,1.0,0.0};   //blue down left (base)     aka v4
  Vertex pink   = {0.0,0.0,-1.0,    1.0,0.0,1.0};   //top: pink                 aka v5

  //creat struct with prev defined vertices: note that the last two are for the bottom aka the base of the pyramid
  Vertex vertices[]={pink,red,blue,  pink,blue,yellow,  pink,green,red,   pink,yellow,green,  green,yellow,red,  yellow,blue,red};

  glGenBuffers(1,&vbo);
  glGenVertexArrays(1,&vao);

  //bind to array (since thats how data is laid out) and load vertex data in VBO
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBindVertexArray(vao);

  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0); // For position attribute
  glEnableVertexAttribArray(1); // For color attribute

  // For position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, x));

  // For color attribute
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, r));

  //for part b
  modelTransform.setToIdentity(); // Initialize to identity matrix
  modelTransform.translate(QVector3D(-2.0f, 0.0f, -6.0f)); // Apply translation
  projectionTransform.setToIdentity();
  projectionTransform.perspective(60.0f, (float)width() / height(), 0.2f, 20.0f);

  // part d; knot
  for(const QVector3D & kCoord : knotCoords){    // in a for loop we get the coordinates and colour for each vertex of the knot
      Vertex vertex;

      vertex.x = kCoord.x();
      vertex.y = kCoord.y();
      vertex.z = kCoord.z();

      float x = vertex.x;
      float y = vertex.y;
      float z = vertex.z;

      vertex.r = std::min(1.0f, std::abs(x));
      vertex.g = std::min(1.0f, std::abs(y));
      vertex.b = std::min(1.0f, std::abs(z));

      // Finally we need to add each vertex to the array
      knotVertices.append(vertex);
  }

  // generate the vao and vbo for our knot
  glGenBuffers(1, &knotVBO);
  glGenVertexArrays(1, &knotVAO);

  // bind to array (since thats how data is laid out) and load vertex data in VBO
  glBindVertexArray(knotVAO);
  glBindBuffer(GL_ARRAY_BUFFER, knotVBO);

  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*knotVertices.size(), knotVertices.data(), GL_STATIC_DRAW);

  glEnableVertexAttribArray(0); // For position attribute
  glEnableVertexAttribArray(1); // For color attribute

  // For position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, x));

  // For color attribute
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, r));

  // initialize as identity matrix
  knotModelTransform.setToIdentity();
  // apply translation
  knotModelTransform.translate(QVector3D(2.0f, 0.0f, -6.0f));

  createShaderProgram();
}

/**
 * @brief MainView::createShaderProgram Creates a new shader program with a
 * vertex and fragment shader.
 */
void MainView::createShaderProgram() {
  shaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex,
                                        ":/shaders/vertshader.glsl");
  shaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment,
                                        ":/shaders/fragshader.glsl");
  shaderProgram.link();
}

/**
 * @brief MainView::paintGL Actual function used for drawing to the screen.
 *
 */
void MainView::paintGL() {
  // Clear the screen before rendering
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  shaderProgram.bind();

  // bind vao and shader program for the pyramid
  glBindVertexArray(vao);

  // Setting  values for the model and projection transformations
  shaderProgram.setUniformValue("modelTransform", modelTransform);
  shaderProgram.setUniformValue("projectionTransform", projectionTransform);

  // Draw pyramid here
  glDrawArrays(GL_TRIANGLES, 0, 18);


  // bind vao and shader program for the knot
  glBindVertexArray(knotVAO);

  // and for the knot
  shaderProgram.setUniformValue("modelTransform", knotModelTransform);

  // Draw knot here
  glDrawArrays(GL_TRIANGLES, 0, knotVertices.size());

  shaderProgram.release();

}

/**
 * @brief MainView::resizeGL Called upon resizing of the screen.
 *
 * @param newWidth The new width of the screen in pixels.
 * @param newHeight The new height of the screen in pixels.
 */
void MainView::resizeGL(int newWidth, int newHeight) {
  //glViewport defines the area in which gl renders, which should only be the new window size

    //initializing again so we dont mess up anything
    projectionTransform.setToIdentity();
    projectionTransform.perspective(60.0f, static_cast<float>(newWidth) / newHeight, 0.2f, 20.0f);

}

/**
 * @brief MainView::setRotation Changes the rotation of the displayed objects.
 * @param rotateX Number of degrees to rotate around the x axis.
 * @param rotateY Number of degrees to rotate around the y axis.
 * @param rotateZ Number of degrees to rotate around the z axis.
 */
void MainView::setRotation(int rotateX, int rotateY, int rotateZ) {
  qDebug() << "Rotation changed to (" << rotateX << "," << rotateY << ","
           << rotateZ << ")";

    // rotation for the pyramid
    modelTransform.rotate(rotateX-curRotX, QVector3D(1.0f, 0.0f, 0.0f));// x axis rotation
    modelTransform.rotate(rotateY-curRotY, QVector3D(0.0f, 1.0f, 0.0f)); //for y axis
    modelTransform.rotate(rotateZ-curRotZ, QVector3D(0.0f, 0.0f, 1.0f));// for z axis

    // rotation for the knot
    knotModelTransform.rotate(rotateX-curRotX, QVector3D(1.0f, 0.0f, 0.0f));// x axis rotation
    knotModelTransform.rotate(rotateY-curRotY, QVector3D(0.0f, 1.0f, 0.0f)); //for y axis
    knotModelTransform.rotate(rotateZ-curRotZ, QVector3D(0.0f, 0.0f, 1.0f));// for z axis

    // reset currents
    curRotX = rotateX;
    curRotY = rotateY;
    curRotZ = rotateZ;

    //without that we just get an empty window
    update();
}

/**
 * @brief MainView::setScale Changes the scale of the displayed objects.
 * @param scale The new scale factor. A scale factor of 1.0 should scale the
 * mesh to its original size.
 */
void MainView::setScale(float scale) {
  qDebug() << "Scale changed to " << scale;

    //added the division by curScale to ensure scaling twice doesnt lead to multiplied scaling;
    modelTransform.scale(scale / curScale);
    knotModelTransform.scale(scale / curScale);

    // Update the current scaling factor
    curScale = scale;

    update();
}

/**
 * @brief MainView::onMessageLogged OpenGL logging function, do not change.
 *
 * @param Message The message to be logged.
 */
void MainView::onMessageLogged(QOpenGLDebugMessage Message) {
  qDebug() << " → Log:" << Message;
}
