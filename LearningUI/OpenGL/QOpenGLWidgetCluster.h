#ifndef QOPENGLWIDGETCLUSTER_H
#define QOPENGLWIDGETCLUSTER_H

#include <QObject>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <QWidget>

#include "GeometryEngine.h"
#include <QBasicTimer>
#include <QQuaternion>

class QOpenGLWidgetCluster : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    QOpenGLWidgetCluster(QWidget* parent = nullptr);
    ~QOpenGLWidgetCluster();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    //    void timerEvent(QTimerEvent * e) override;

private:
private:
    //    QOpenGLVertexArrayObject m_vao;
    QOpenGLShaderProgram m_program;

    QOpenGLBuffer m_vbo;
    QOpenGLBuffer m_ebo;

    float m_vertices[36] {


        -0.5f, 0.5f, -2.0f,
        -0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,

        0.0f, 0.0f, 0.0f,
        0.5f, 0.0f, 0.0f,
        0.5f, 0.5f, 2.0f,

        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f, 0.0f, 0.0f,

        0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        0.0f, 0.0f, 0.0f,
    };

    //    QBasicTimer m_timer;

    QMatrix4x4 m_projection;
    QMatrix4x4 m_view;
    QMatrix4x4 m_model;
    int m_projection_loc;
    int m_view_loc;
    int m_model_loc;

    QVector2D m_mousePressPosition;
    qreal m_angularSpeed;
    QQuaternion m_rotation;
};

#endif // QOPENGLWIDGETCLUSTER_H
