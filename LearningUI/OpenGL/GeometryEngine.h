#ifndef GEOMETRYENGINE_H
#define GEOMETRYENGINE_H

#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

class GeometryEngine : protected QOpenGLFunctions {
public:
    GeometryEngine();
    ~GeometryEngine();

    void drawGeometry(QOpenGLShaderProgram* program);

private:
    void initGeometry();

private:
//    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo;
//    QOpenGLBuffer m_ebo;
};

#endif // GEOMETRYENGINE_H
