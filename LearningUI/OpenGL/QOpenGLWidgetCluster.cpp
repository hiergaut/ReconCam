#include "OpenGL/QOpenGLWidgetCluster.h"

#include <QtGlobal>

static const char* vertexshader_source = "#version 330 core\n\
        layout (location = 0) in vec3 vertex;\n\
        uniform mat4 model;\n\
        uniform mat4 view;\n\
        uniform mat4 projection;\n\
        void main()\n\
        {\n\
//            gl_Position = vec4(vertex, 1.0);\n\
            gl_Position = projection * view * model * vec4(vertex, 1.0);\n\
        }\n";

static const char* fragmentshader_source = "#version 330 core\n\
        out vec4 color;\n\
        void main()\n\
        {\n\
            color = vec4(1.0f, 0.5f, 0.2f, 0.5);\n\
        }\n";

static const char* fragmentshader_source2 = "#version 330 core\n\
        out vec4 color;\n\
        void main()\n\
        {\n\
            color = vec4(0, 1.0, 0.0, 1.0);\n\
        }\n";

QOpenGLWidgetCluster::QOpenGLWidgetCluster(QWidget* parent)
    : QOpenGLWidget(parent)
    , m_vbo(QOpenGLBuffer::VertexBuffer)
    , m_ebo(QOpenGLBuffer::IndexBuffer)
    , m_vbo2(QOpenGLBuffer::VertexBuffer)
{
}

QOpenGLWidgetCluster::~QOpenGLWidgetCluster()
{

    makeCurrent();

    m_vbo.destroy();
    m_ebo.destroy();
    m_vbo2.destroy();

    doneCurrent();

    glDeleteVertexArrays(2, vao);
    glDeleteBuffers(2, vbo);
}

void QOpenGLWidgetCluster::initializeGL()
{
    initializeOpenGLFunctions();

    // --------------------------- SHADERS
    //    int success;
    //    char infoLog[512];
    //    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    //    //    QFile shader(":/vshader.glsl");
    //    //    QTextStream in(&shader);
    //    //    const GLchar str = static_cast<const GLchar>(in.readAll().toStdString().c_str());
    //    //    glShaderSource(vertexShader, 1, (const GLchar*)str.c_str(), NULL);
    //    glShaderSource(vertexShader, 1, &vertexshader_source, NULL);
    //    glCompileShader(vertexShader);
    //    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    //    if (!success) {
    //        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    //        qDebug() << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
    //                 << infoLog;
    //    }

    //    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    //    glShaderSource(fragmentShader, 1, &fragmentshader_source, NULL);
    //    glCompileShader(fragmentShader);
    //    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    //    if (!success) {
    //        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    //        qDebug() << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
    //                 << infoLog;
    //    }
    //    shaderProgram = glCreateProgram();
    //    glAttachShader(shaderProgram, vertexShader);
    //    glAttachShader(shaderProgram, fragmentShader);
    //    glLinkProgram(shaderProgram);

    //    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    //    if (!success) {
    //        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    //        qDebug() << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
    //                 << infoLog;
    //    }
    //    //    glUseProgram(shaderProgram);
    //    glDeleteShader(vertexShader);
    //    glDeleteShader(fragmentShader);

    //    m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vshader.glsl");
    //    m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fshader.glsl");
    m_program[0].addShaderFromSourceCode(QOpenGLShader::Vertex, vertexshader_source);
    m_program[0].addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentshader_source);
    m_program[0].link();
    //    m_program[0].bind();

    m_program[1].addShaderFromSourceCode(QOpenGLShader::Vertex, vertexshader_source);
    m_program[1].addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentshader_source2);
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

    //
    //    glGenVertexArrays(1, &vao2);
    glBindVertexArray(vao[1]);

    //    glGenBuffers(1, &vbo2);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices), m_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    //    //    memcpy(concat, m_box, sizeof(m_box));
    //    //    memcpy(&concat[24], m_vertices, sizeof(m_vertices));

    //    m_vao.create();
    //    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

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
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //    m_program[0].setUniformValue(m_model_loc, m_model);

    //    m_program[0].setUniformValue(m_view_loc, m_view);
    //    m_program[0].setUniformValue(m_projection_loc, m_projection);
    m_program[0].bind();
    m_program[0].setUniformValue(m_model_loc[0], m_model);
    m_program[0].setUniformValue(m_view_loc[0], m_view);
    m_program[0].setUniformValue(m_projection_loc[0], m_projection);

    glBindVertexArray(vao[0]);
    //    glDrawArrays(GL_LINES, 0, 12);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, (void*)0);
    //    qDebug() << sizeof(m_boxIndices);

    m_program[1].bind();
    m_program[1].setUniformValue(m_model_loc[1], m_model);
    m_program[1].setUniformValue(m_view_loc[1], m_view);
    m_program[1].setUniformValue(m_projection_loc[1], m_projection);

    glBindVertexArray(vao[1]);
    //    m_program[1].setUniformValue(m_model_loc, m_model);
    //    m_program[1].setUniformValue(m_view_loc, m_view);
    //    m_program[1].setUniformValue(m_projection_loc, m_projection);

    glDrawArrays(GL_POINTS, 0, 12);

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
