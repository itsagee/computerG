#include "mainview.h"
#include "triangle.h"
#include <QDateTime>

GLuint nameOfVBO;
GLuint nameOfVAO;
QMatrix4x4 modelTransform;
QMatrix4x4 projectionTransform;
float curScale=1.0;
float curRotX=0.0;
float curRotY=0.0;
float curRotZ=0.0;
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
  // Delete the buffer objects
  GLuint buffersToDelete[] = { nameOfVBO, nameOfVAO }; // Assuming you want to delete the VBO and VAO
  glDeleteBuffers(2, buffersToDelete);
  makeCurrent();
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

  // Set the color to be used by glClear. This is, effectively, the background
  // color.
  glClearColor(0.37f, 0.42f, 0.45f, 0.0f);


  //NEW CODE
  //added new: instantiating three vertices, one for each triangle corner
  Vertex v1 = {-1.0,1.0,1.0,   1.0,0.0,0.0};//red, left bottom
  Vertex v2 = {-1.0,-1.0,1.0,    0.0,0.0,1.0};//green, left up bottom
  Vertex v3 = {1.0,-1.0,1.0,   1.0,1.0,0.0};//blue right bottom
  Vertex v4 = {1.0,1.0,1.0,   0.0,1.0,0.0};//blue right up bottom
  Vertex v5 = {0.0,0.0,-1.0,   1.0,0.0,1.0};//top: pink
  //creating struct with prev defined vertices: last two are bottom
  Vertex vertices[]={v5,v1,v4,  v5,v4,v3,  v5,v2,v1,   v5,v3,v2,  v2,v3,v1,  v3,v4,v1};
     //{v5,v4,v1,  v5,v3,v4,  v5,v2,v1,   v5,v3,v2,  v2,v3,v1,  v3,v4,v1};//v4,v1,v3


  glGenBuffers(1,&nameOfVBO);
  glGenVertexArrays(1,&nameOfVAO);

  //binding to array (since thats how data is laid out) and loading vertex data in VBO
  glBindVertexArray(nameOfVAO);
  glBindBuffer(GL_ARRAY_BUFFER, nameOfVBO);

  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  //TBD: free vertices here




  glEnableVertexAttribArray(0); // For position attribute
  glEnableVertexAttribArray(1); // For color attribute

  // For position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, x));

  // For color attribute
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, red));

  //Wednesday

  modelTransform.setToIdentity(); // Initialize to identity matrix
  modelTransform.translate(QVector3D(-2.0f, 0.0f, -6.0f)); // Apply translation
  projectionTransform.setToIdentity();
  projectionTransform.perspective(60.0f, (float)width() / height(), 0.2f, 20.0f);

  //NEW CODE


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

  // Setting  values for the model and projection transformations
  shaderProgram.setUniformValue("modelTransform", modelTransform);
  shaderProgram.setUniformValue("projectionTransform", projectionTransform);
  glBindVertexArray(nameOfVAO);
  glBindBuffer(GL_ARRAY_BUFFER, nameOfVBO);
  // Draw here
  glDrawArrays(GL_TRIANGLES, 0, 18);
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
    // glViewport(0, 0, newWidth, newHeight);
    projectionTransform.setToIdentity();//initializing again so we dont mess up anything
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

    //modelTransform.setToIdentity();
    modelTransform.rotate(rotateX-curRotX, QVector3D(1.0f, 0.0f, 0.0f));// x axis rotation
    modelTransform.rotate(rotateY-curRotY, QVector3D(0.0f, 1.0f, 0.0f)); //for y axis
    modelTransform.rotate(rotateZ-curRotZ, QVector3D(0.0f, 0.0f, 1.0f));// for z axis
    curRotX= rotateX;
    curRotY= rotateY;
    curRotZ= rotateZ;
    update();//without that we just get an empty window
}

/**
 * @brief MainView::setScale Changes the scale of the displayed objects.
 * @param scale The new scale factor. A scale factor of 1.0 should scale the
 * mesh to its original size.
 */
void MainView::setScale(float scale) {
  qDebug() << "Scale changed to " << scale;
    //delTransform.setToIdentity();
  //added this to ensure scaling twice doesnt lead to multiplied scaling;
  modelTransform.scale(scale / curScale);

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
