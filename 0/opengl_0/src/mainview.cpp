#include "mainview.h"
#include "triangle.h"


QOpenGLShaderProgram shaderProgram;

/**
 * @brief MainView::MainView Mainview constructor. The ": QOpenGLWidget(parent)"
 * notation means calling the super class constructor of QOpenGLWidget with the
 * parameter "parent".
 * @param parent Parent widget.
 */
MainView::MainView(QWidget *parent) : QOpenGLWidget(parent) {
  qDebug() << "MainView constructor";

  //QOpenGLShaderProgram shaderProgram;

}

/**
 * @brief MainView::~MainView Destructor. Can be used to free memory.
 */
MainView::~MainView() {
  qDebug() << "MainView destructor";
    // Delete VBO
    //glDeleteBuffers(1, &nameOfVBO);

    // Delete VAO
    //glDeleteVertexArrays(1, &nameOfVAO);
  makeCurrent();
}


/**
 * @brief MainView::initializeGL Initialize the necessary OpenGL context.
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

  //added new: instantiating three vertices, one for each triangle corner
  Vertex v1 = {-1.0,0.0,1.0,0.0,0.0};//red, left bottom
  Vertex v2 = {1.0,0.0,0.0,1.0,0.0};//green, right bottom
  Vertex v3 = {0.0,-1.0,0.0,0.0,1.0};//blue top
  //creating struct with prev defined vertices
  Vertex vertices[]={v1,v2,v3};
  GLuint nameOfVBO;
  GLuint nameOfVAO;
  glGenBuffers(1,&nameOfVBO);
  glGenVertexArrays(1,&nameOfVAO);


  // Load and link shaders
  shaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vertshader.glsl");
  shaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshader.glsl");
  shaderProgram.link();
  shaderProgram.bind();
  glBindVertexArray(nameOfVAO);

  glBindBuffer(GL_ARRAY_BUFFER, nameOfVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);


  glEnableVertexAttribArray(0); // For position attribute
  glEnableVertexAttribArray(1); // For color attribute

  // For position attribute
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, x));

  // For color attribute
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, red));

}

/**
 * @brief MainView::resizeGL Called when the window is resized.
 * @param newWidth The new width of the window.
 * @param newHeight The new height of the window.
 */
void MainView::resizeGL(int newWidth, int newHeight) {
  Q_UNUSED(newWidth)
  Q_UNUSED(newHeight)
}

/**
 * @brief MainView::paintGL Draw function. TODO.
 */
void MainView::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES,0,3);
}

/**
 * @brief MainView::onMessageLogged Debug utility method.
 * @param Message The message to be logged.
 */
void MainView::onMessageLogged(QOpenGLDebugMessage Message) {
  qDebug() << " → Log:" << Message;
}
