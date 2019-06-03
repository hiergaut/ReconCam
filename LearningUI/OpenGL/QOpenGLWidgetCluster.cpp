#include "OpenGL/QOpenGLWidgetCluster.h"

QOpenGLWidgetCluster::QOpenGLWidgetCluster(QWidget* parent)
    : QOpenGLWidget(parent)
    //    m_program(0)
    , m_vbo(QOpenGLBuffer::VertexBuffer)
    , m_ebo(QOpenGLBuffer::IndexBuffer)
{
}

QOpenGLWidgetCluster::~QOpenGLWidgetCluster()
{
    //    if (m_program == nullptr)
    //        return;

    makeCurrent();

    m_vbo.destroy();
    m_ebo.destroy();
    //    m_vao.destroy();
    //    delete m_program;
    //    m_program = 0;

    doneCurrent();
}

void QOpenGLWidgetCluster::initializeGL()
{
    initializeOpenGLFunctions();
    //    glClearColor(0, 0, 0, 0);

    //    m_program = new QOpenGLShaderProgram;
    //        m_program.bind();
    if (!m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vshader.glsl"))
        close();

    if (!m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fshader.glsl"))
        close();

    if (!m_program.link())
        close();

    if (!m_program.bind())
        close();

    //    m_program.setUniformValue("model", m_model);
    //    m_program.setUniformValue("projection", m_projection);
    //    m_program.setUniformValue("view", m_view);
    //    m_projection_loc = m_program.uniformLocation("projection");
    //    m_view_loc = m_program.uniformLocation("view");
    //    m_model_loc = m_program.uniformLocation("model");

    //    m_view.setToIdentity();
    //    m_view.translate(0, 0, -1);

    //    qreal aspect = qreal(width()) / qreal(height() ? height() : 1);
    //    const qreal zNear = 3.0, zFar = 7.0, fov = 45.0;
    //    m_projection.setToIdentity();
    //    m_projection.perspective(fov, aspect, zNear, zFar);

    //    m_model.setToIdentity();
    //    m_program->bindAttributeLocation("vertex", 0);
    //    if (!m_program.link()) {
    //        qDebug() << "no link shader";
    //    }

    //    m_program.bind();
    //    m_vao = new QOpenGLVertexArrayObject;
    //    m_vao.create();
    //    m_vao.bind();
    //    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    //    float vertices[] = {
    //        -0.5f, -0.5f, 0.0f,
    //         0.5f, -0.5f, 0.0f,
    //         0.0f,  0.5f, 0.0f
    //    };
    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(m_vertices, sizeof(m_vertices));
    //    m_vbo.allocate(m_vertices, sizeof(m_vertices));
    //    glEnableVertexAttribArray(0);
    //    m_program.enableAttributeArray(0);
    //    glEnableVertexAttribArray(0);
    //    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), 0);
    //    m_program.setAttributeBuffer(0, GL_FLOAT, 0, 3, 3 * sizeof(float));
    glVertexPointer(3, GL_FLOAT, 0, NULL);
    m_vbo.release();

    //    m_program.release();
    //    m_program.bind();
    //    glEnableVertexAttribArray(0);
    //    m_program.enableAttributeArray(0);
    //    m_program.setAttributeBuffer(0, GL_FLOAT, 0, 3);

    //    quintptr offset =0;
    //    int vertexLocation = m_program.attributeLocation("vertex");
    //    m_program.enableAttributeArray(vertexLocation);
    //    m_program.setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, 3 * sizeof(float));

    //    m_vbo.create();
    //    m_vbo.bind();
    //    m_vbo.allocate(vertices, sizeof(vertices));

    //    QOpenGLFunctions * f = QOpenGLContext::currentContext()->functions();
    //    m_program.bind();
    //    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    //    glShadeModel(GL_SMOOTH);
    //    glEnable(GL_LIGHTING);
    //    glEnable(GL_LIGHT0);

    //    m_vbo.release();

    //        m_program.release();
}

void QOpenGLWidgetCluster::resizeGL(int w, int h)
{
    qreal aspect = qreal(w) / qreal(h ? h : 1);
    const qreal zNear = 3.0, zFar = 7.0, fov = 45.0;

    m_projection.setToIdentity();
    m_projection.perspective(fov, aspect, zNear, zFar);
}

void QOpenGLWidgetCluster::paintGL()
{
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //        glEnableClientState(GL_VERTEX_ARRAY);

    //        m_model.setToIdentity();

    //    glVertexPointer(3, GL_FLOAT, 0, m_vertices);
    //    glDrawArrays(GL_TRIANGLES, 0, 3);
    //    glDisableClientState(GL_VERTEX_ARRAY);

    glEnableClientState(GL_VERTEX_ARRAY);
    //    m_vbo.bind();
    //    glVertexPointer(3, GL_FLOAT, 0, NULL);
    //    m_vbo.release();

    //    m_program.bind();
    //    m_program.setUniformValue(m_projection_loc, m_projection);
    //    m_program.setUniformValue(m_view_loc, m_view);
    //    m_program.setUniformValue(m_model_loc, m_model);
    glDrawArrays(GL_TRIANGLES, 0, 12);
    //    m_program.release();

    glDisableClientState(GL_VERTEX_ARRAY);

    //    m_program.bind();
    //    glShadeModel(GL_SMOOTH);
    //    m_vbo.bind();

    //    glEnable(GL_DEPTH_TEST);
    //    glEnable(GL_CULL_FACE);

    //    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    //    QMatrix4x4 modelView;
    //    modelView.translate(0.0, 0.0, -5.0);

    //    m_program.bind();
    //    glDrawArrays(GL_TRIANGLES, 0, 3);
    //    m_vbo.release();

    //    m_program.release();
    //    m_program.disableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
}
