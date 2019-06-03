#include "GeometryEngine.h"

struct VertexData {
    QVector3D position;
};

GeometryEngine::GeometryEngine()
    : m_vbo(QOpenGLBuffer::VertexBuffer)
//    , m_ebo(QOpenGLBuffer::IndexBuffer)
{
    initializeOpenGLFunctions();

//    m_vao.create();
    m_vbo.create();
//    m_vbo.bind();
//    m_ebo.create();

    initGeometry();
}

GeometryEngine::~GeometryEngine()
{
    m_vbo.destroy();
//    m_ebo.destroy();
}

void GeometryEngine::drawGeometry(QOpenGLShaderProgram* program)
{
    m_vbo.bind();
//    m_ebo.bind();

    quintptr offset = 0;

//    int vertexLocation = program->attributeLocation("a_position");
//    program->enableAttributeArray(vertexLocation);
    program->enableAttributeArray(0);
//    program->setAttributeBuffer(0, GL_FLOAT, offset, 3, sizeof(VertexData));
    program->setAttributeBuffer(0, GL_FLOAT, offset, 3, 3 * sizeof(float));

//    glDrawElements(GL_TRIANGLES, m_ebo.size(), GL_UNSIGNED_SHORT, 0);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void GeometryEngine::initGeometry()
{
//    float vertices[] = {
//        0.5f, 0.5f, 0.0f,
//        0.5f, -0.5f, 0.0f,
//        -0.5f, -0.5f, 0.0f,
//        -0.5f, 0.5f, 0.0f,
//    };
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };

//    uint indices[] = {
//        0, 1, 3,
//        1, 2, 3
//    };

    m_vbo.bind();
    m_vbo.allocate(vertices, sizeof(vertices));


//    m_ebo.bind();
//    m_ebo.allocate(indices, 1 * sizeof(uint));
}
