#include "OpenGL/QOpenGLWidgetCluster.h"

#include <QtGlobal>
//#include <algorithm>
//#include <iterator>
//#include <iostream>
//#include <vector>
//#include <numeric>

#define MAX_DOTS 1000

static const char* vertexshader_source[] = {
    "#version 330 core\n\
        layout (location = 0) in vec3 vertex;\n\
        uniform mat4 model;\n\
        uniform mat4 view;\n\
        uniform mat4 projection;\n\
        void main()\n\
        {\n\
          gl_Position = projection * view * model * vec4(vertex, 1.0);\n\
        }\n",

    "#version 330 core\n\
        layout (location = 0) in vec3 vertex;\n\
        layout (location = 1) in vec4 color;\n\
        out vec4 ourColor;\n\
        uniform mat4 model;\n\
        uniform mat4 view;\n\
        uniform mat4 projection;\n\
        void main()\n\
        {\n\
          gl_Position = projection * view * model * vec4(vertex, 1.0);\n\
          ourColor = color;\n\
        }\n",
};

static const char* fragmentshader_source[] = {
    "#version 330 core\n\
        out vec4 color;\n\
        void main()\n\
        {\n\
          color = vec4(0.0f, 1.0f, 0.0f, 0.3);\n\
        }\n",

    "#version 330 core\n\
        in vec4 ourColor;\n\
        out vec4 color;\n\
        void main()\n\
        {\n\
          color = vec4(ourColor);\n\
        }\n"
};

QOpenGLWidgetCluster::QOpenGLWidgetCluster(QWidget* parent)
    : QOpenGLWidget(parent)
{
}

QOpenGLWidgetCluster::~QOpenGLWidgetCluster()
{
    glDeleteVertexArrays(2, vao);
    glDeleteBuffers(2, vbo);
}

void QOpenGLWidgetCluster::setPoints(const std::vector<float> points, int nbDots, int nbBoxes)
{

    Q_ASSERT(points.size() % 7 == 0);
    Q_ASSERT(points.size() < MAX_DOTS * 7);

    m_nbDots = nbDots;
    m_nbBoxes = nbBoxes;
    //    qDebug() << "setPoints : " << points.size() << nbDots << nbBoxes;
    //    qDebug() << points;

    glBindVertexArray(vao[1]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    //    m_dots.assign(triangle, triangle + sizeof(triangle) / sizeof(float));
    m_dots = points;
    //    std::copy(m_dots.begin(), points.begin(), points.end());

    //    m_dots.assign(m_vertices, m_vertices + sizeof(m_vertices) / sizeof(float));
    //    m_dots.assign(triangle, triangle + sizeof(triangle) / sizeof(float));
    //    qDebug() << "m_dots = " << m_dots.data();
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_dots.size() * sizeof(float), m_dots.data());
    //    glBufferSubData(GL_ARRAY_BUFFER, 0, points.size() * sizeof(float), points.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    update();
}

void QOpenGLWidgetCluster::initializeGL()
{
    initializeOpenGLFunctions();

    m_program[0].addShaderFromSourceCode(QOpenGLShader::Vertex, vertexshader_source[0]);
    m_program[0].addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentshader_source[0]);
    m_program[0].link();
    //    m_program[0].bind();

    m_program[1].addShaderFromSourceCode(QOpenGLShader::Vertex, vertexshader_source[1]);
    m_program[1].addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentshader_source[1]);
    m_program[1].link();
    //    m_program[1].bind();

    glGenVertexArrays(2, vao);
    glGenBuffers(2, vbo);

    glBindVertexArray(vao[0]);

    //    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_box), m_box, GL_STATIC_DRAW);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_boxIndices), m_boxIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //    glBindVertexArray(0);

    //    m_dots.fill(m_vertices, sizeof(m_vertices));
    m_dots.assign(m_vertices, m_vertices + sizeof(m_vertices) / sizeof(float));
    m_nbDots = m_dots.size() / 7;
    //    m_dots.assign(triangle, triangle + sizeof(triangle) / sizeof(float));
    //    m_dots.assign(m_vertices, m_vertices + sizeof(m_vertices) / sizeof(float));
    //
    //    glGenVertexArrays(1, &vao2);
    glBindVertexArray(vao[1]);

    //    glGenBuffers(1, &vbo2);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    //    glBufferData(GL_ARRAY_BUFFER, m_dots.size() * sizeof(float), m_dots.data(), GL_DYNAMIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, 7 * MAX_DOTS * sizeof(float), m_dots.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    //    //    memcpy(concat, m_box, sizeof(m_box));
    //    //    memcpy(&concat[24], m_vertices, sizeof(m_vertices));

    //    m_vao.create();
    //    memcpy(m_dots, m_vertices, sizeof(m_vertices));
    //    m_nbDots = sizeof(m_vertices) / (3 * sizeof(float));
    //    m_nbDots = 10;
    //    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    //    glBindVertexArray(vao[1]);
    //    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    //    m_dots.assign(triangle, triangle + sizeof(triangle) / sizeof(float));
    //    glBufferSubData(GL_ARRAY_BUFFER, 0, m_dots.size() * sizeof(float), m_dots.data());
    //    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //    glBindVertexArray(0);

    //    m_ebo.create();
    //    qDebug() << "ebo id : " << m_ebo.bufferId();
    //    m_ebo.bind();
    //    m_ebo.allocate(m_boxIndices, sizeof(m_boxIndices));

    ////    glBindBuffer(GL_ARRAY_BUFFER, 2);
    //    m_model_loc = glGetUniformLocation(m_program[0], "model");
    m_model.setToIdentity();
    m_model.rotate(m_xRot, 1.0, 0.0, 0.0);
    m_model.rotate(m_yRot, 0.0, 1.0, 0.0);
    m_model.rotate(m_zRot, 0.0, 0.0, 1.0);
    //    m_model_loc = m_program[0].uniformLocation("model");
    //    glUniformMatrix4fv(m_model_loc, 1, GL_FALSE, m_model.constData());

    m_view.setToIdentity();
    m_view.translate(0.0, 0.0, m_zCamera);
    //    m_view_loc = m_program[0].uniformLocation("view");
    //    m_view_loc = glGetUniformLocation(0, "view");

    m_projection.perspective(m_fov, 1.0 * width() / height(), 0.1, 100.0);
    //    m_projection_loc = m_program[0].uniformLocation("projection");

    //    m_program[0].bindAttributeLocation("model", m_model_loc);
    //    m_program[0].bindAttributeLocation("view", m_view_loc);
    //    m_program[0].bindAttributeLocation("projection", m_projection_loc);

    //    m_program[1].bindAttributeLocation("model", m_model_loc);
    m_model_loc[0] = m_program[0].uniformLocation("model");
    m_view_loc[0] = m_program[0].uniformLocation("view");
    m_projection_loc[0] = m_program[0].uniformLocation("projection");

    m_model_loc[1] = m_program[1].uniformLocation("model");
    m_view_loc[1] = m_program[1].uniformLocation("view");
    m_projection_loc[1] = m_program[1].uniformLocation("projection");

    //    m_program[1].bind();
    //    m_program[1].setUniformValue(m_model_loc[1], m_model);
    //    m_program[1].setUniformValue(m_view_loc[1], m_view);
    //    m_program[1].setUniformValue(m_projection_loc[1], m_projection);

    //    m_program[1].bindAttributeLocation("view", m_view_loc);
    //    m_program[1].bindAttributeLocation("projection", m_projection_loc);

    //    m_program[0].setUniformValue(m_model_loc[0], m_model);
    //    m_program[0].setUniformValue(m_view_loc[0], m_view);
    //    m_program[0].setUniformValue(m_projection_loc[0], m_projection);

    //    m_program[1].setUniformValue(m_model_loc[1], m_model);
    //    m_program[1].setUniformValue(m_view_loc[1], m_view);
    //    m_program[1].setUniformValue(m_projection_loc[1], m_projection);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glPointSize(5);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //    //    glEnable(GL_MULTISAMPLE);

    //    //    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //    //    glFrontFace(GL_CCW);
    //    //    glCullFace(GL_BACK);
    //    glViewport(0, 0, width(), height());
}

void QOpenGLWidgetCluster::resizeGL(int w, int h)
{
    updateProjection();
}

void QOpenGLWidgetCluster::paintGL()
{
    // display objects
    //    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //    m_program[0].setUniformValue(m_model_loc, m_model);

    //    m_program[0].setUniformValue(m_view_loc, m_view);
    //    m_program[0].setUniformValue(m_projection_loc, m_projection);

    m_program[1].bind();
    m_program[1].setUniformValue(m_model_loc[1], m_model);
    m_program[1].setUniformValue(m_view_loc[1], m_view);
    m_program[1].setUniformValue(m_projection_loc[1], m_projection);

    glBindVertexArray(vao[1]);
    //    m_program[1].setUniformValue(m_model_loc, m_model);
    //    m_program[1].setUniformValue(m_view_loc, m_view);
    //    m_program[1].setUniformValue(m_projection_loc, m_projection);

    //    qDebug() << "draw dots, nb dot = " << m_dots.size() / 6;
    //    qDebug() << "dots = " << m_dots;
    glDrawArrays(GL_POINTS, 0, m_nbDots);
    glDrawArrays(GL_LINES, m_nbDots, m_nbBoxes * 2 * 12);
    //    qDebug() << m_nbBoxes;

    //
    m_program[0].bind();
    m_program[0].setUniformValue(m_model_loc[0], m_model);
    m_program[0].setUniformValue(m_view_loc[0], m_view);
    m_program[0].setUniformValue(m_projection_loc[0], m_projection);

    glBindVertexArray(vao[0]);
    //    glDrawArrays(GL_LINES, 0, 12);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, (void*)0);
    //    qDebug() << sizeof(m_boxIndices);

    //
    glBindVertexArray(0);
    glUseProgram(0);
}

void QOpenGLWidgetCluster::wheelEvent(QWheelEvent* event)
{
    int ry = event->delta();

    if (ry > 0) {
        //        m_fov *= 0.7;
        m_zCamera *= 0.9;
    } else {
        //        m_fov = qMin(m_fov * 1.3f, 180.0f);
        m_zCamera *= 1.1;
    }
    m_view.setToIdentity();
    m_view.translate(0.0, 0.0, m_zCamera);

    update();
}

void QOpenGLWidgetCluster::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MouseButton::RightButton) {
        mouseClicked = true;
        posFirstClicked = event->pos() * 0.5;
    }
}

void QOpenGLWidgetCluster::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MouseButton::RightButton) {
        mouseClicked = false;
        QPoint pos = event->pos() * 0.5;
        m_yRot = m_yRot + pos.x() - posFirstClicked.x();
        m_xRot = m_xRot + pos.y() - posFirstClicked.y();
    }
}

void QOpenGLWidgetCluster::mouseMoveEvent(QMouseEvent* event)
{
    if (mouseClicked) {
        QPoint pos = event->pos() * 0.5;

        m_model.setToIdentity();
        m_model.rotate(m_xRot + pos.y() - posFirstClicked.y(), 1.0, 0.0, 0.0);
        m_model.rotate(m_yRot + pos.x() - posFirstClicked.x(), 0.0, 1.0, 0.0);
        m_model.rotate(m_zRot, 0.0, 0.0, 1.0);

        update();
    }
}

void QOpenGLWidgetCluster::updateProjection()
{
    qreal aspect = qreal(width()) / qreal(height() ? height() : 1);
    const qreal zNear = 0.1, zFar = 100.0;

    m_projection.setToIdentity();
    m_projection.perspective(m_fov, aspect, zNear, zFar);

    update();
}
