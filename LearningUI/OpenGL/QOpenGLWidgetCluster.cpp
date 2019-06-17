#include "OpenGL/QOpenGLWidgetCluster.h"

#include <QtGlobal>
//#include <algorithm>
//#include <iterator>
//#include <iostream>
//#include <vector>
//#include <numeric>

#define MAX_DOTS 10000

static const char* vertexshader_source[] = {
    "#version 330 core\n\
        layout (location = 0) in vec3 vertex;\n\
        out vec4 ourColor;\n\
        uniform mat4 object;\n\
        uniform mat4 normalize;\n\
        uniform mat4 model;\n\
        uniform mat4 view;\n\
        uniform mat4 projection;\n\
        vec4 temp;\n\
        vec3 temp2;\n\
        void main()\n\
        {\n\
          gl_Position = projection * view * model * object * vec4(vertex, 1.0f);\n\
          //        temp = vec3(normalize * vec4(vertex, 1.0f));\n\
          //        temp2 = vec3(temp);\n\
          //      var rgb = temp * 0.5f  -vec3(1.0f, 1.0f, 1.0f);\n\
          //      ourColor = vec4(vertex * 0.5f + 0.5, 1.0);\n\
//          ourColor = vec4(vec3(object * vec4(vertex, 1.0f)), 1.0);\n\
            ourColor =   vec4(vertex, 1.0f);\n\
        }\n",

    "#version 330 core\n\
        layout (location = 0) in vec3 vertex;\n\
        layout (location = 1) in vec4 color;\n\
        out vec4 ourColor;\n\
        uniform mat4 object;\n\
        uniform mat4 normalize;\n\
        uniform mat4 model;\n\
        uniform mat4 view;\n\
        uniform mat4 projection;\n\
        void main()\n\
        {\n\
          gl_Position = projection * view * model * normalize * object * vec4(vertex, 1.0);\n\
          ourColor = color;\n\
        }\n",
};

static const char* fragmentshader_source[] = {
    "#version 330 core\n\
        in vec4 ourColor;\n\
        out vec4 color;\n\
        void main()\n\
        {\n\
          //            color = vec4(1.0, 0.0, 0.0, 1.0);\n\
          color = ourColor;\n\
        }\n",

    "#version 330 core\n\
        in vec4 ourColor;\n\
        out vec4 color;\n\
        void main()\n\
        {\n\
          color = ourColor;\n\
        }\n"
};

QOpenGLWidgetCluster::QOpenGLWidgetCluster(QWidget* parent)
    : QOpenGLWidget(parent)
{
    m_normalize.setToIdentity();
    m_object.setToIdentity();
    //    initializeOpenGLFunctions();
}

QOpenGLWidgetCluster::~QOpenGLWidgetCluster()
{
    glDeleteVertexArrays(2, m_vao);
    glDeleteBuffers(2, m_vbo);
}

void QOpenGLWidgetCluster::render(const std::vector<float> points, const std::vector<float> boxes)
{

    Q_ASSERT(points.size() % 7 == 0);
    Q_ASSERT(points.size() < BUFF_MAX);

    //    m_nbDots = nbDots;
    //    m_nbBoxes = nbBoxes;
    //    qDebug() << "setPoints : " << points.size() << nbDots << nbBoxes;
    //    qDebug() << points;

    //    m_dots.assign(triangle, triangle + sizeof(triangle) / sizeof(float));
    memcpy(&m_vbo_data[1], points.data(), points.size() * sizeof(float));
    m_vbo_dataLen[1] = points.size();

    memcpy(&m_vbo_data[1][points.size()], boxes.data(), boxes.size() * sizeof(float));
    m_boxesLen = boxes.size();
    //    m_dots = points;
    //    std::copy(m_dots.begin(), points.begin(), points.end());

    qDebug() << "rendered " << (m_vbo_dataLen[1] + m_boxesLen) / 7 << " points";

    glBindVertexArray(m_vao[1]);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo[1]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, (m_vbo_dataLen[1] + m_boxesLen) * sizeof(float), m_vbo_data[1]);

    //    glBufferData(GL_ARRAY_BUFFER, sizeof(m_vbo_data[1]), m_vbo_data[1], GL_DYNAMIC_DRAW);
    //    m_dots.assign(m_vertices, m_vertices + sizeof(m_vertices) / sizeof(float));
    //    m_dots.assign(triangle, triangle + sizeof(triangle) / sizeof(float));
    //    qDebug() << "m_dots = " << m_dots.data();
    //        glBufferSubData(GL_ARRAY_BUFFER, 0, m_dots.size() * sizeof(float), m_dots.data());
    //        glBufferSubData(GL_ARRAY_BUFFER, 0, points.size() * sizeof(float), points.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    update();
}

void QOpenGLWidgetCluster::setArea(std::vector<float> vbo_data, std::vector<uint> ebo_data)
{
    Q_ASSERT(vbo_data.size() < BUFF_MAX);
    Q_ASSERT(vbo_data.size() % 3 == 0);
    Q_ASSERT(ebo_data.size() < BUFF_MAX);
    //    makeCurrent();
    //    qDebug() << "fuck";
    //    QOpenGLFunctions_3_3_Core * f = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();
    //    initializeOpenGLFunctions();
    //    m_dots.assign(m_vertices, m_vertices + sizeof(m_vertices) / sizeof(float));
    //    m_nbDots = m_dots.size() / 7;
    //    m_vbo_data[0].assign(vbo_data, vbo_data + sizeof (vbo_data) / sizeof (float));
    //    float* vboPtr = m_vbo_data[0].data();
    //    uint* eboPtr = m_ebo_data.data();

    //    m_vbo_data[0] = vbo_data;
    //    m_ebo_data = ebo_data;

    //    Q_ASSERT(vboPtr == m_vbo_data[0].data());
    //    Q_ASSERT(eboPtr == m_ebo_data.data());
    //    initializeOpenGLFunctions();
    memcpy(&m_vbo_data[0], vbo_data.data(), vbo_data.size() * sizeof(float));
    m_vbo_dataLen[0] = vbo_data.size();

    memcpy(&m_ebo_data[0], ebo_data.data(), ebo_data.size() * sizeof(uint));
    m_ebo_dataLen[0] = ebo_data.size();

    qDebug() << "set area " << vbo_data.size() / 3 << " points";
    //    glGenVertexArrays(1, &m_vao[0]);

    //    doneCurrent();
    //    update();
}

void QOpenGLWidgetCluster::initializeGL()
{
    //    qDebug() << "bitch";
    initializeOpenGLFunctions();

    m_program[0].addShaderFromSourceCode(QOpenGLShader::Vertex, vertexshader_source[0]);
    m_program[0].addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentshader_source[0]);
    m_program[0].link();
    //    m_program[0].bind();

    m_program[1].addShaderFromSourceCode(QOpenGLShader::Vertex, vertexshader_source[1]);
    m_program[1].addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentshader_source[1]);
    m_program[1].link();
    //    m_program[1].bind();

    glGenVertexArrays(2, m_vao);
    glGenBuffers(2, m_vbo);

    glGenBuffers(2, m_ebo);

    // -------------------------------- AREA
    glBindVertexArray(m_vao[0]);
    //    glGenBuffers(1, &vbo);
    //    glGenBuffers(1, &m_vbo[0]);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo[0]);
    //    glBufferData(GL_ARRAY_BUFFER, sizeof(m_box), m_box, GL_STATIC_DRAW);
    //    m_vbo_data[0].insert()
    //    m_vbo_data[0].push_back(0);
    //    float* vboPtr = m_vbo_data[0].data();
    //    m_vbo_data[0].assign(m_box, m_box + sizeof(m_box) / sizeof(float));
    //    Q_ASSERT(m_vbo_data->data() != m_box);
    //    Q_ASSERT(vboPtr == m_vbo_data[0].data());
    //    qDebug() << sizeof (vboPtr);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_vbo_data[0]), m_vbo_data[0], GL_STATIC_DRAW);

    //    glGenBuffers(1, &m_ebo);
    //    uint* eboPtr = m_ebo_data.data();
    //    m_ebo_data.assign(m_boxIndices, m_boxIndices + sizeof(m_boxIndices) / sizeof(float));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_ebo_data[0]), m_ebo_data[0], GL_STATIC_DRAW);
    //    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_boxIndices), m_boxIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //    glBindVertexArray(0);

    //    m_dots.fill(m_vertices, sizeof(m_vertices));
    //    m_dots.assign(m_vertices, m_vertices + sizeof(m_vertices) / sizeof(float));
    //    m_nbDots = m_dots.size() / 7;
    //    m_dots.assign(triangle, triangle + sizeof(triangle) / sizeof(float));
    //    m_dots.assign(m_vertices, m_vertices + sizeof(m_vertices) / sizeof(float));
    //
    //    glGenVertexArrays(1, &vao2);
    // -------------------------------- POINTS
    //    memcpy(&m_vbo_data[1], triangle, sizeof (triangle));
    //    m_vbo_data[1] = triangle;
    //    m_vbo_dataLen[1] = sizeof (triangle) / sizeof(float);
    //    qDebug() << m_vbo_dataLen[1];

    glBindVertexArray(m_vao[1]);

    //    glGenBuffers(1, &vbo2);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo[1]);
    //    glBufferData(GL_ARRAY_BUFFER, m_dots.size() * sizeof(float), m_dots.data(), GL_DYNAMIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_vbo_data[1]), m_vbo_data[1], GL_DYNAMIC_DRAW);
    //    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_DYNAMIC_DRAW);
    //        glBufferSubData(GL_ARRAY_BUFFER, 0, points.size() * sizeof(float), points.data());

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
    //    m_model.scale(1.0f / 640.0f, 1.0f / 480.0f, 1);
    //    m_model_loc = m_program[0].uniformLocation("model");
    //    glUniformMatrix4fv(m_model_loc, 1, GL_FALSE, m_model.constData());

    m_view.setToIdentity();
    //    m_view.rotate(m_xRot, 1.0, 0.0, 0.0);
    //    m_view.rotate(m_yRot, 0.0, 1.0, 0.0);
    //    m_view.rotate(m_zRot, 0.0, 0.0, 1.0);
    m_view.translate(0.0, 0.0, m_zCamera);
    //    m_view_loc = m_program[0].uniformLocation("view");
    //    m_view_loc = glGetUniformLocation(0, "view");

    m_projection.perspective(m_fov, 1.0 * width() / height(), 0.1, 100.0);
    //    m_projection.perspective(m_fov, 1.0 * 640.0 / 480.0, 0.1, 100.0);
    //    m_projection_loc = m_program[0].uniformLocation("projection");

    //    m_program[0].bindAttributeLocation("model", m_model_loc);
    //    m_program[0].bindAttributeLocation("view", m_view_loc);
    //    m_program[0].bindAttributeLocation("projection", m_projection_loc);

    //    m_program[1].bindAttributeLocation("model", m_model_loc);
    m_object_loc[0] = m_program[0].uniformLocation("object");
    m_normalize_loc[0] = m_program[0].uniformLocation("normalize");
    m_model_loc[0] = m_program[0].uniformLocation("model");
    m_view_loc[0] = m_program[0].uniformLocation("view");
    m_projection_loc[0] = m_program[0].uniformLocation("projection");

    m_object_loc[1] = m_program[1].uniformLocation("object");
    m_normalize_loc[1] = m_program[1].uniformLocation("normalize");
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

    // -------------------------------- POINTS
    m_program[1].bind();
    //    QMatrix4x4 model = m_model;
    //    model.rotate(180, QVector3D(1.0, 0, 0));
    //    model.translate(-1, -1, 0);
    //    model.scale(1.0f / 320.0f, 1.0f / 240.0f, 1.0f);
    m_program[1].setUniformValue(m_object_loc[1], m_object);
    m_program[1].setUniformValue(m_normalize_loc[1], m_normalize);
    m_program[1].setUniformValue(m_model_loc[1], m_model);
    m_program[1].setUniformValue(m_view_loc[1], m_view);
    m_program[1].setUniformValue(m_projection_loc[1], m_projection);

    glBindVertexArray(m_vao[1]);
    //    glBindBuffer(GL_ARRAY_BUFFER, m_vbo[1]);
    //    glBufferSubData(GL_ARRAY_BUFFER, 0, m_vbo_dataLen[1] * sizeof(float), m_vbo_data[1]);
    //    m_program[1].setUniformValue(m_model_loc, m_model);
    //    m_program[1].setUniformValue(m_view_loc, m_view);
    //    m_program[1].setUniformValue(m_projection_loc, m_projection);

    //    qDebug() << "draw dots, nb dot = " << m_dots.size() / 6;
    //    qDebug() << "dots = " << m_dots;
    glDrawArrays(GL_POINTS, 0, m_vbo_dataLen[1] / 7);
    //    glDrawArrays(GL_LINES, m_nbDots, m_nbBoxes * 2 * 12);
    glDrawArrays(GL_LINES, m_vbo_dataLen[1] / 7, m_boxesLen / 7);
    //    qDebug() << m_nbBoxes;

    //
    // -------------------------------- AREA
    m_program[0].bind();
    m_program[0].setUniformValue(m_object_loc[0], m_object);
    m_program[0].setUniformValue(m_normalize_loc[0], m_normalize);
    m_program[0].setUniformValue(m_model_loc[0], m_model);
    m_program[0].setUniformValue(m_view_loc[0], m_view);
    m_program[0].setUniformValue(m_projection_loc[0], m_projection);

    glBindVertexArray(m_vao[0]);
    //    glDrawArrays(GL_LINES, 0, 12);
    //    glDrawElements(GL_LINES, sizeof(m_square) / (sizeof (float) * 3), GL_UNSIGNED_INT, (void*)0);
    //    glDrawElements(GL_LINES, sizeof(m_square) / sizeof(float), GL_UNSIGNED_INT, (void*)0);
    glDrawElements(GL_LINES, m_vbo_dataLen[0], GL_UNSIGNED_INT, (void*)0);
    //    glDrawElements(GL_LINES, sizeof(m_box) / sizeof(float), GL_UNSIGNED_INT, (void*)0);
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
        //        m_model.scale(1.0f / 640.0f, 1.0f / 480.0f, 1);
        //        m_view.setToIdentity();
        //        m_view.rotate(m_xRot + pos.y() -posFirstClicked.y(), 1.0, 0.0, 0.0);
        //        m_view.rotate(m_yRot + pos.x() - posFirstClicked.x(), 0.0, 1.0, 0.0);
        //        m_view.rotate(m_zRot, 0.0, 0.0, 1.0);
        //        m_view.translate(0.0, 0.0, m_zCamera);

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

void QOpenGLWidgetCluster::setObject(const QMatrix4x4& object)
{
    m_object = object;
}

void QOpenGLWidgetCluster::setZCamera(float zCamera)
{
    m_zCamera = zCamera;
}

void QOpenGLWidgetCluster::setNormalize(const QMatrix4x4& normalize)
{
    m_normalize = normalize;
}
